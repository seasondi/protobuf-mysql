#ifndef MYSQLINTERFACE_H 
#define MYSQLINTERFACE_H

#include <unistd.h>
#include <mysql/mysql.h>
#include <string>

namespace google {
    namespace protobuf {
        class Message;
        template<typename T>
        class RepeatedField;
        class Reflection;
        class FieldDescriptor;
    }
}

namespace soul {
    class MysqlGenerator;
    class MysqlInterface {
        private:
            MYSQL mSqlHandler;
            bool mAutoCommit;
            std::string mErrorStr;
        public:
            MysqlInterface();
            ~MysqlInterface();
            bool Connect(const char* host, uint16_t port, const char* user, const char* passwd);
            void SetAutoCommit(bool on);
            bool Commit();
            bool Rollback();
            int SwitchDB(const char* db);
            const std::string LastError() const;
            int ExecuteSqlSelect(const MysqlGenerator& generator, google::protobuf::Message& result);
            int ExecuteSqlInsert(const MysqlGenerator& generator, const google::protobuf::Message& msg);
            int ExecuteSqlUpdate(const MysqlGenerator& generator, const google::protobuf::Message& msg);
            int ExecuteSqlUpdateOnInsert(const MysqlGenerator& generator, const google::protobuf::Message& msg);
            int ExecuteSqlDelete(const MysqlGenerator& generator, const google::protobuf::Message& msg);
        private:
            int Query(const char* query, uint64_t len);
            const std::string& SetErrorMsg();
    };
}
#endif /*MYSQLINTERFACE_H*/
