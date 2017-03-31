#include <soul/protobuf-mysql/MysqlInterface.h>
#include <soul/protobuf-mysql/MysqlGenerator.h>
#include <soul/protobuf-mysql/MysqlError.h>
#include <soul/Log.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/reflection.h>
#include <boost/lexical_cast.hpp>

using namespace soul;

MysqlInterface::MysqlInterface() : mAutoCommit(true) {
    MYSQL* ret = mysql_init(&mSqlHandler);
    if(ret == nullptr) {
        LOG_ERROR << "mysql_init failed";
    }
}

MysqlInterface::~MysqlInterface() {
    mysql_close(&mSqlHandler);
}

bool MysqlInterface::Connect(const char* host, uint16_t port, const char* user, const char* passwd) {
    if(mysql_real_connect(&mSqlHandler, host, user, passwd, nullptr, port, nullptr, 0) == nullptr) {
        SetErrorMsg();
        LOG_ERROR << "mysql_real_connect failed: %s" << LastError();
        return false;
    } else {
        LOG_DEBUG << "connected to mysql: " << host << " : " << port;
        char reconnect = 1;
        mysql_options(&mSqlHandler, MYSQL_OPT_RECONNECT, (char *)&(reconnect));
        return true;
    }
}

void MysqlInterface::SetAutoCommit(bool on) {
    mAutoCommit = mysql_autocommit(&mSqlHandler, on);
}

int MysqlInterface::SwitchDB(const char* db) {
    return mysql_select_db(&mSqlHandler, db);
}

int MysqlInterface::Query(const char* query, uint64_t len) {
    return mysql_real_query(&mSqlHandler, query, len);
}

bool MysqlInterface::Commit() {
    if(mAutoCommit == false) {
        return mysql_commit(&mSqlHandler);
    }
    return true;
}

bool MysqlInterface::Rollback() {
    return mysql_rollback(&mSqlHandler);
}

const std::string MysqlInterface::LastError() const {
    return std::move(mErrorStr);
}

const std::string& MysqlInterface::SetErrorMsg() {
    mErrorStr = mysql_error(&mSqlHandler);
    return mErrorStr;
}

int MysqlInterface::ExecuteSqlSelect(const MysqlGenerator& generator, google::protobuf::Message& result) {
    int ret = 0;
    try {
        std::string sql = generator.GenerateSqlSelect(result);
        if(sql.empty()) return SQL_GENERATE_EMPTY;
        ret = Query(sql.c_str(), sql.length());
        if(ret != 0) {
            SetErrorMsg();
            LOG_ERROR << LastError();
        } else {
            MYSQL_RES* res = mysql_store_result(&mSqlHandler);
            if(res == nullptr) {
                SetErrorMsg();
                LOG_ERROR << LastError();
                ret = mysql_errno(&mSqlHandler);
            } else {
                my_ulonglong rowCount = mysql_num_rows(res);
                if(rowCount == 0) {
                    ret = ER_KEY_NOT_FOUND;
                } else {
                    MYSQL_ROW row;
                    if(MysqlGenerator::OnlyHoldsOneRepeatedMessageField(result)) {
                        result.Clear();
                        const google::protobuf::Descriptor* descriptor = result.GetDescriptor();
                        const google::protobuf::Reflection* reflection = result.GetReflection();
                        const google::protobuf::FieldDescriptor* field = descriptor->field(0);
                        const google::protobuf::MutableRepeatedFieldRef<google::protobuf::Message> repeatedMsg 
                                = reflection->GetMutableRepeatedFieldRef<google::protobuf::Message>(&result, field);
                        while((row = mysql_fetch_row(res)) != nullptr) {
                            uint32_t fieldCount = mysql_num_fields(res);
                            if(fieldCount == 0) continue;
                            google::protobuf::Message* subMsg = repeatedMsg.NewMessage();
                            for(uint32_t i = 0; i!= fieldCount; ++i) {
                                MysqlGenerator::ApplySelectResult(*subMsg, row[i], mysql_fetch_field_direct(res, i));
                            }
                            reflection->AddAllocatedMessage(&result, field, subMsg);
                        }
                    } else {
                        if(rowCount > 1) {
                            LOG_DEBUG << "select result rows: " << rowCount << ", use first one, sql: " << sql;
                        }
                        row = mysql_fetch_row(res);
                        if(row != nullptr) {
                            uint32_t fieldCount = mysql_num_fields(res);
                            for(uint32_t i = 0; i != fieldCount; ++i) {
                                MysqlGenerator::ApplySelectResult(result, row[i], mysql_fetch_field_direct(res, i));
                            }
                        }
                    }
                }
            }
            mysql_free_result(res);
        }
    } catch(boost::bad_lexical_cast& e) {
        LOG_ERROR << "genrate select sql catch exception, what: " << e.what();
        ret = SQL_GENERATE_FAIL;
    }

    return ret;
}

int MysqlInterface::ExecuteSqlInsert(const MysqlGenerator& generator, const google::protobuf::Message& msg) {
    int ret = 0;
    try {
        std::string sql = generator.GenerateSqlInsert(msg);
        if(sql.empty()) return SQL_GENERATE_EMPTY;
        ret = Query(sql.c_str(), sql.length());
        if(ret != 0) {
            SetErrorMsg();
            LOG_ERROR << LastError() << ", sql: " << sql;
            return ret;
        }
        my_ulonglong affected = mysql_affected_rows(&mSqlHandler);
        if(affected == 0) {
            LOG_DEBUG << "insert affected no rows, sql: " << sql;
        } else {
            LOG_DEBUG << "affect rows: " << affected << ", sql: " << sql;
        }
    } catch(boost::bad_lexical_cast& e) {
        LOG_ERROR << "generate insert sql catch exception, what: " << e.what();
        ret = SQL_GENERATE_FAIL;
    }

    return ret;
}

int MysqlInterface::ExecuteSqlUpdate(const MysqlGenerator& generator, const google::protobuf::Message& msg) {
    int ret = 0;
    try {
        std::vector<std::string> sqls = generator.GenerateSqlUpdate(msg);
        if(sqls.empty()) return SQL_GENERATE_EMPTY;
        my_ulonglong affected = 0;
        for(int i = 0; i != sqls.size(); ++i) {
            const std::string& sql = sqls[i];
            ret = Query(sql.c_str(), sql.length());
            if(ret) {
                SetErrorMsg();
                LOG_WARN << "update query error: " << LastError() << ", sql: " << sql;
                if(mAutoCommit == false) {
                    LOG_WARN << "update rollback";
                    return ret;
                }
            } else {
                affected += mysql_affected_rows(&mSqlHandler);
            }
        }
        LOG_DEBUG << "update total affect rows: " << affected;
    } catch(boost::bad_lexical_cast& e) {
        LOG_ERROR << "generate insert sql catch exception, what: " << e.what();
        ret = SQL_GENERATE_FAIL;
    }

    return ret;
}

int MysqlInterface::ExecuteSqlUpdateOnInsert(const MysqlGenerator& generator, const google::protobuf::Message& msg) {
    int ret = 0;
    try {
        std::string sql = generator.GenerateSqlUpdateOnInsert(msg);
        if(sql.empty()) return SQL_GENERATE_EMPTY;
        ret = Query(sql.c_str(), sql.length());
        if(ret != 0) {
            SetErrorMsg();
            LOG_ERROR << LastError() << ", sql: " << sql;
            return ret;
        }
        my_ulonglong affected = mysql_affected_rows(&mSqlHandler);
        if(affected == 0) {
            LOG_DEBUG << "update on insert affected no rows, sql: " << sql;
        } else {
            LOG_DEBUG << "affect rows: " << affected << ", sql: " << sql;
        }
    } catch(boost::bad_lexical_cast& e) {
        LOG_ERROR << "generate insert sql catch exception, what: " << e.what();
        ret = SQL_GENERATE_FAIL;
    }

    return ret;
}

int MysqlInterface::ExecuteSqlDelete(const MysqlGenerator& generator, const google::protobuf::Message& msg) {
    int ret = 0;
    try {
        std::vector<std::string> sqls = generator.GenerateSqlDelete(msg);
        if(sqls.empty()) return SQL_GENERATE_EMPTY;
        my_ulonglong affected = 0;
        for(int i = 0; i != sqls.size(); ++i) {
            const std::string& sql = sqls[i];
            ret = Query(sql.c_str(), sql.length());
            if(ret) {
                SetErrorMsg();
                LOG_WARN << "delete query error: " << LastError() << ", sql: " << sql;
                return ret;
            } else {
                affected += mysql_affected_rows(&mSqlHandler);
            }
        }
        LOG_DEBUG << "delete total affect rows: " << affected;
    } catch(boost::bad_lexical_cast& e) {
        LOG_ERROR << "generate delete sql catch exception, what: " << e.what();
        ret = SQL_GENERATE_FAIL;
    }

    return ret;
}
