#include <soul/protobuf-mysql/MysqlGenerator.h>
#include <soul/protobuf-mysql/MysqlDescriptor.pb.h>
#include <soul/Log.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <boost/lexical_cast.hpp>

using namespace soul;

MysqlGenerator::MysqlGenerator(const std::string& database,
                               const std::string& table,
                               const std::string& where)
    : mDataBase(database),
      mTable(table),
      mWhere(where) 
{
    MysqlGenerator::TrimString(mWhere);
}

std::string MysqlGenerator::GenerateSqlSelect(const google::protobuf::Message& msg) const {
    return MysqlGenerator::OnlyHoldsOneRepeatedMessageField(msg) ? GenerateSqlSelectMulti(msg) : GenerateSqlSelectSingle(msg);
}

int MysqlGenerator::GenerateSqlSelectImpl(const google::protobuf::Message& msg, std::string& sql, bool selectAll) const {
    sql.clear(); sql = "select "; std::string sqlCondition;
    const int defaultSqlLength = sql.length();
    const google::protobuf::Descriptor* descriptor = msg.GetDescriptor();
    const google::protobuf::Reflection* reflection = msg.GetReflection();
    int emptyFieldCount = 0;
    for(int i = 0; i != descriptor->field_count(); ++i) {
        const google::protobuf::FieldDescriptor* field = descriptor->field(i);
        if(field->is_repeated()) {
            LOG_ERROR << "generate select sql error: has repeated field, sql will be empty";
            sql.clear();
            return -1;
        }
        bool hasField = reflection->HasField(msg, field);
        if(selectAll || hasField == false) {
            if(sql.length() > defaultSqlLength) {
                sql += ", ";
            }
            sql += field->name();
        }
        if(hasField == true) {
            if(mWhere.empty()) {
                if(!sqlCondition.empty()) {
                    sqlCondition += " and ";
                } else {
                    sqlCondition += " where ";
                }
                sqlCondition += field->name() + " = " + MysqlGenerator::GetFieldValue(reflection, msg, field);
            }
        } else {
            ++emptyFieldCount;
        }
    }
    sql += " from " + mDataBase + "." + mTable;
    if(mWhere.empty()) {
        sql += sqlCondition;
    } else {
        sql += " " + mWhere;
    }

    return emptyFieldCount;
}

std::string MysqlGenerator::GenerateSqlSelectSingle(const google::protobuf::Message& msg) const {
    std::string sql;
    int emptyFieldCount = GenerateSqlSelectImpl(msg, sql, false);

    const google::protobuf::Descriptor* descriptor = msg.GetDescriptor();
    if(emptyFieldCount == descriptor->field_count()) {
        sql.clear();
        GenerateSqlSelectImpl(msg, sql, true);
    } else if(emptyFieldCount == 0) {
        sql.clear();
        LOG_ERROR << "generate select sql error: all fields are not empty, sql will be empty";
    }
    MysqlGenerator::LogSql(sql);
    return sql;
}

std::string MysqlGenerator::GenerateSqlSelectMulti(const google::protobuf::Message& msg) const {
    const google::protobuf::Descriptor* descriptor = msg.GetDescriptor();
    const google::protobuf::Reflection* reflection = msg.GetReflection();
    if(descriptor->field_count() == 0) {
        LOG_ERROR << "generate multi select sql error: msg is empty, sql will be empty";
        return "";
    }
    const google::protobuf::FieldDescriptor* field = descriptor->field(0);
    const google::protobuf::RepeatedPtrField<google::protobuf::Message>& repeatedMsg = reflection->GetRepeatedPtrField<google::protobuf::Message>(msg, field);
    if(repeatedMsg.empty()) {
        LOG_ERROR << "generate multi select sql error: repeated field is empty, expect has one element, sql will be empty";
        return "";
    }
    return GenerateSqlSelectSingle(repeatedMsg[0]);
}

std::string MysqlGenerator::GenerateSqlInsert(const google::protobuf::Message& msg) const {
    return MysqlGenerator::OnlyHoldsOneRepeatedMessageField(msg) ? GenerateSqlInsertMulti(msg) : GenerateSqlInsertSingle(msg);
}

std::string MysqlGenerator::GenerateSqlInsertSingle(const google::protobuf::Message& msg, bool update) const {
    std::string sql = "insert into " + mDataBase + "." + mTable + " (";
    int defaultSqlLength = sql.length();
    std::string sqlCondition = "(";
    std::string updateSql = update ? " on duplicate key update " : "";
    int defaultUpdateSqlLength = updateSql.length();
    int emptyFieldCount = 0;
    const google::protobuf::Descriptor* descriptor = msg.GetDescriptor();
    const google::protobuf::Reflection* reflection = msg.GetReflection();
    for(int i = 0; i != descriptor->field_count(); ++i) {
        const google::protobuf::FieldDescriptor* field = descriptor->field(i);
        if(field->is_repeated()) {
            LOG_ERROR << "generate single insert sql error: field can not be repeated, sql will be empty";
            return "";
        }
        if(reflection->HasField(msg, field) == false) {
            ++emptyFieldCount;
            continue;
        }
        if(sql.length() > defaultSqlLength) {
            sql += ", ";
            sqlCondition += ", ";
        }
        sql += field->name();
        sqlCondition += MysqlGenerator::GetFieldValue(reflection, msg, field);
        if(update && field->options().GetExtension(primarykey) == false) {
            if(updateSql.length() > defaultUpdateSqlLength) {
                updateSql += ", ";
            }
            updateSql += field->name() + " = " + MysqlGenerator::GetFieldValue(reflection, msg, field);
        }
    }
    if(emptyFieldCount == descriptor->field_count()) {
        LOG_ERROR << "generate single insert sql error: no field is setted, sql will be empty";
        return "";
    }
    sqlCondition += ")";
    sql += ") values " + sqlCondition + updateSql;

    MysqlGenerator::LogSql(sql);
    return sql;
}

std::string MysqlGenerator::GenerateSqlInsertMulti(const google::protobuf::Message& msg) const {
    std::string sql = "insert into " + mDataBase + "." + mTable + " (";
    std::string sqlCondition;
    const google::protobuf::Descriptor* descriptor = msg.GetDescriptor();
    const google::protobuf::Reflection* reflection = msg.GetReflection();
    if(descriptor->field_count() == 0) {
        LOG_ERROR << "generate multi insert sql error: msg is empty, sql will be empty";
        return "";
    }
    const google::protobuf::FieldDescriptor* field = descriptor->field(0);
    const google::protobuf::RepeatedPtrField<google::protobuf::Message>& repeatedMsg = reflection->GetRepeatedPtrField<google::protobuf::Message>(msg, field);
    if(repeatedMsg.empty()) {
        LOG_ERROR << "generate multi insert sql error: repeated field is empty, sql will be empty";
        return "";
    }
    for(int i = 0; i != repeatedMsg.size(); ++i) {
        const google::protobuf::Message& subMsg = repeatedMsg[i];
        const google::protobuf::Descriptor* subDescriptor = subMsg.GetDescriptor();
        const google::protobuf::Reflection* subReflection = subMsg.GetReflection();
        sqlCondition += "(";
        for(int loop = 0; loop != subDescriptor->field_count(); ++loop) {
            const google::protobuf::FieldDescriptor* subMsgField = subDescriptor->field(loop);
            if(i == 0) {
                if(loop != 0) {
                    sql += ", ";
                }
                sql += subMsgField->name();
            }
            if(loop != 0) {
                sqlCondition += ", ";
            }
            sqlCondition += MysqlGenerator::GetFieldValue(subReflection, subMsg, subMsgField);
        }
        sqlCondition += ")";
        if(i != repeatedMsg.size() - 1) {
            sqlCondition += ", ";
        }
    }
    sql += ") values " + sqlCondition;

    MysqlGenerator::LogSql(sql);
    return sql;
}

std::vector<std::string> MysqlGenerator::GenerateSqlUpdate(const google::protobuf::Message& msg) const {
    if(MysqlGenerator::OnlyHoldsOneRepeatedMessageField(msg)) {
        return GenerateSqlUpdateMulti(msg);
    } else {
        std::vector<std::string> vec;
        std::string sql = GenerateSqlUpdateSingle(msg);
        if(!sql.empty()) {
            vec.push_back(sql);
        }
        return vec;
    }
}

std::string MysqlGenerator::GenerateSqlUpdateSingle(const google::protobuf::Message& msg) const {
    std::string sql = "update " + mDataBase + "." + mTable + " set ";
    int defaultSqlLength = sql.length();
    std::string sqlCondition;
    bool hasUpdateKey = false;
    const google::protobuf::Descriptor* descriptor = msg.GetDescriptor();
    const google::protobuf::Reflection* reflection = msg.GetReflection();
    for(int i = 0; i != descriptor->field_count(); ++i) {
        const google::protobuf::FieldDescriptor* field = descriptor->field(i);
        if(field->is_repeated()) {
            LOG_ERROR << "generate update sql error: field can not be repeated, sql will be empty";
            return "";
        }
        if(reflection->HasField(msg, field) == false) {
            if(field->options().GetExtension(updatekey) && mWhere.empty()) {
                LOG_ERROR << "generate update sql error: filed with option 'updatekey' can not be empty, sql will be empty";
                return "";
            }
            continue;
        }
        if(field->options().GetExtension(updatekey)) {
            hasUpdateKey = true;
            if(mWhere.empty()) {
                if(!sqlCondition.empty()) {
                    sqlCondition += ", ";
                }
                sqlCondition += field->name() + " = " + MysqlGenerator::GetFieldValue(reflection, msg, field);
            } else if(reflection->HasField(msg, field)) {
                if(sql.length() > defaultSqlLength) {
                    sql += ", ";
                }
                sql += field->name() + " = " + MysqlGenerator::GetFieldValue(reflection, msg, field);
            }
        } else {
            if(sql.length() > defaultSqlLength) {
                sql += ", ";
            }
            sql += field->name() + " = " + MysqlGenerator::GetFieldValue(reflection, msg, field);
        }
    }

    if(hasUpdateKey == false && mWhere.empty()) {
        LOG_ERROR << "generate upate sql error: not found option 'updatekey' and where condtion is emtpy, sql will be emtpy";
        return "";
    }

    if(mWhere.empty()) {
        if(!sqlCondition.empty()) {
            sql += " where " + sqlCondition;
        }
    } else {
        sql += " " + mWhere;
    }

    MysqlGenerator::LogSql(sql);
    return sql;
}

std::vector<std::string> MysqlGenerator::GenerateSqlUpdateMulti(const google::protobuf::Message& msg) const {
    std::vector<std::string> sqls;
    const google::protobuf::Descriptor* descriptor = msg.GetDescriptor();
    const google::protobuf::Reflection* reflection = msg.GetReflection();
    if(descriptor->field_count() == 0) {
        LOG_ERROR << "generate multi update sql error: msg is empty, sql will be empty";
        return sqls;
    }
    const google::protobuf::FieldDescriptor* field = descriptor->field(0);
    const google::protobuf::RepeatedPtrField<google::protobuf::Message>& repeatedMsg = reflection->GetRepeatedPtrField<google::protobuf::Message>(msg, field);
    if(repeatedMsg.empty()) {
        LOG_ERROR << "generate multi update sql error: repeated filed is empty, sql will be empty";
        return sqls;
    }
    for(int i = 0; i != repeatedMsg.size(); ++i) {
        std::string sql = GenerateSqlUpdateSingle(repeatedMsg[i]);
        if(!sql.empty()) {
            sqls.push_back(sql);
        }
    }
    return sqls;
}

std::string MysqlGenerator::GenerateSqlUpdateOnInsert(const google::protobuf::Message& msg) const {
    return GenerateSqlInsertSingle(msg, true);
}

std::string MysqlGenerator::GenerateSqlDeleteSingle(const google::protobuf::Message& msg) const {
    std::string sql = "delete from " + mDataBase + "." + mTable;
    if(mWhere.empty()) {
        std::string sqlCondition;
        const google::protobuf::Descriptor* descriptor = msg.GetDescriptor();
        const google::protobuf::Reflection* reflection = msg.GetReflection();
        for(int i = 0; i != descriptor->field_count(); ++i) {
            const google::protobuf::FieldDescriptor* field = descriptor->field(i);
            if(field->is_repeated()) {
                LOG_ERROR << "generate delete sql error: field can not be repeated, sql will be empty";
                return "";
            }
            if(reflection->HasField(msg, field) == false) continue;
            if(sqlCondition.empty()) {
                sqlCondition += " where ";
            } else {
                sqlCondition += " and ";
            }
            sqlCondition += field->name() + " = " + MysqlGenerator::GetFieldValue(reflection, msg, field);
        }
        if(sqlCondition.empty()) {
            LOG_ERROR << "generate delete sql error: all fields are empty, sql will be empty";
            return "";
        }
        sql += sqlCondition;
    } else {
        sql += " " + mWhere;
    }

    MysqlGenerator::LogSql(sql);
    return sql;
}

std::vector<std::string> MysqlGenerator::GenerateSqlDeleteMulti(const google::protobuf::Message& msg) const {
    std::vector<std::string> sqls;
    const google::protobuf::Descriptor* descriptor = msg.GetDescriptor();
    const google::protobuf::Reflection* reflection = msg.GetReflection();
    if(descriptor->field_count() == 0) {
        LOG_ERROR << "generate multi delete sql error: msg is empty, sql will be empty";
        return sqls;
    }
    const google::protobuf::FieldDescriptor* field = descriptor->field(0);
    const google::protobuf::RepeatedPtrField<google::protobuf::Message>& repeatedMsg = reflection->GetRepeatedPtrField<google::protobuf::Message>(msg, field);
    if(repeatedMsg.empty()) {
        LOG_ERROR << "generate multi delete sql error: repeated filed is empty, sql will be empty";
        return sqls;
    }
    for(int i = 0; i != repeatedMsg.size(); ++i) {
        std::string sql = GenerateSqlDeleteSingle(repeatedMsg[i]);
        if(!sql.empty()) {
            sqls.push_back(sql);
        }
    }
    return sqls;
}

std::vector<std::string> MysqlGenerator::GenerateSqlDelete(const google::protobuf::Message& msg) const {
    if(MysqlGenerator::OnlyHoldsOneRepeatedMessageField(msg)) {
        return GenerateSqlDeleteMulti(msg);
    } else {
        std::vector<std::string> vec;
        std::string sql = GenerateSqlDeleteSingle(msg);
        if(!sql.empty()) {
            vec.push_back(sql);
        }
        return vec;
    }
}

std::string MysqlGenerator::GetFieldValue(const google::protobuf::Reflection* reflection,
                                          const google::protobuf::Message& msg,
                                          const google::protobuf::FieldDescriptor* field) {
    std::string result;
    if(field->is_repeated()) return result;
    switch (field->cpp_type()) {
        case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
            result = boost::lexical_cast<std::string>(reflection->GetInt32(msg, field));
            break;
        case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
            result = boost::lexical_cast<std::string>(reflection->GetInt64(msg, field));
            break;
        case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
            result = boost::lexical_cast<std::string>(reflection->GetUInt32(msg, field));
            break;
        case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
            result = boost::lexical_cast<std::string>(reflection->GetUInt64(msg, field));
            break;
        case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
            result = boost::lexical_cast<std::string>(reflection->GetDouble(msg, field));
            break;
        case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
            result = boost::lexical_cast<std::string>(reflection->GetFloat(msg, field));
            break;
        case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
            result = reflection->GetBool(msg, field) ? "1" : "0";
            break;
        case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
            result = boost::lexical_cast<std::string>(reflection->GetEnumValue(msg, field));
            break;
        case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
            {
                result = reflection->GetString(msg, field);
                char* tmp = new char[result.length()];
                mysql_escape_string(tmp, result.c_str(), result.length());
                result = std::string("'") +  tmp + "'";
                delete [] tmp;
            }
            break;
        case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
            {
                result = reflection->GetMessage(msg, field).SerializeAsString();
                char* tmp = new char[result.length()];
                mysql_escape_string(tmp, result.c_str(), result.length());
                result = std::string("'") + tmp + "'";
                delete [] tmp;
            }
            break;
        default:
            break;
    }
    return result;
}

void MysqlGenerator::SetFieldValue(const char* rowdata, const google::protobuf::FieldDescriptor* field, google::protobuf::Message& result) {
    if(field->is_repeated()) return;
    const google::protobuf::Reflection* reflection = result.GetReflection();
    switch (field->cpp_type()) {
        case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
            reflection->SetInt32(&result, field, boost::lexical_cast<int32_t>(rowdata));
            break;
        case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
            reflection->SetInt64(&result, field, boost::lexical_cast<int64_t>(rowdata));
            break;
        case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
            reflection->SetUInt32(&result, field, boost::lexical_cast<uint32_t>(rowdata));
            break;
        case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
            reflection->SetUInt64(&result, field, boost::lexical_cast<uint64_t>(rowdata));
            break;
        case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
            reflection->SetDouble(&result, field, boost::lexical_cast<double>(rowdata));
            break;
        case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
            reflection->SetFloat(&result, field, boost::lexical_cast<float>(rowdata));
            break;
        case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
            reflection->SetBool(&result, field, boost::lexical_cast<int>(rowdata) ? true : false);
            break;
        case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
            reflection->SetEnum(&result, field, field->enum_type()->FindValueByNumber(boost::lexical_cast<int>(rowdata)));
            break;
        case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
            reflection->SetString(&result, field, rowdata);
            break;
        case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
            {
                const google::protobuf::Descriptor* subDescriptor = field->message_type();
                const google::protobuf::Message* protoType = google::protobuf::MessageFactory::generated_factory()->GetPrototype(subDescriptor);
                google::protobuf::Message* subMsg = protoType->New();
                subMsg->ParseFromArray(rowdata, strlen(rowdata));
                reflection->SetAllocatedMessage(&result, subMsg, field);
            }
            break;
        default:
            break;
    }
}

void MysqlGenerator::ApplySelectResult(google::protobuf::Message& result, const char* rowdata, MYSQL_FIELD* field) {
    const google::protobuf::FieldDescriptor* fieldDescriptor = result.GetDescriptor()->FindFieldByName(field->name);
    if(fieldDescriptor != nullptr) {
        MysqlGenerator::SetFieldValue(rowdata, fieldDescriptor, result);
    }
}

bool MysqlGenerator::OnlyHoldsOneRepeatedMessageField(const google::protobuf::Message& msg) {
    const google::protobuf::Descriptor* descriptor = msg.GetDescriptor();
    if(descriptor->field_count() == 1 && descriptor->field(0)->is_repeated()
            && descriptor->field(0)->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE) {
        return true;
    }

    return false;
}

void MysqlGenerator::TrimString(std::string& str) {
    if(str.empty()) return;
    std::size_t start = 0, finish = str.size();
    while(start != str.size()) {
        if(str[start] != ' ' && str[start] != '\t') {
            break;
        }
        ++start;
    }
    while(--finish != 0) {
        if(str[finish] != ' ' && str[finish] != '\t') {
            break;
        }
    }
    if(start <= finish) {
        str = str.substr(start, finish + 1 - start);
    } else {
        str.clear();
    }
}

void MysqlGenerator::LogSql(const std::string& sql) {
    if(sql.empty()) return;
    LOG_DEBUG << sql;
}
