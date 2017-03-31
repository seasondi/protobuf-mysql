#ifndef MYSQLGENERATOR_H 
#define MYSQLGENERATOR_H

#include <string>
#include <vector>
#include <mysql/mysql.h>

namespace google {
    namespace protobuf {
        class Message;
        template<typename T>
        class RepeatedPtrField;
        class Reflection;
        class FieldDescriptor;
    }
}

namespace soul {
    class MysqlGenerator {
        private:
            const std::string mDataBase;
            const std::string mTable;
            std::string mWhere;
        public:
            MysqlGenerator(const std::string& database, const std::string& table, const std::string& where = "");

            std::string GenerateSqlSelect(const google::protobuf::Message& msg) const;
            std::string GenerateSqlInsert(const google::protobuf::Message& msg) const;
            std::vector<std::string> GenerateSqlUpdate(const google::protobuf::Message& msg) const;
            std::string GenerateSqlUpdateOnInsert(const google::protobuf::Message& msg) const;
            std::vector<std::string> GenerateSqlDelete(const google::protobuf::Message& msg) const;

        public:
            static std::string GetFieldValue(const google::protobuf::Reflection* reflection,
                                          const google::protobuf::Message& msg,
                                          const google::protobuf::FieldDescriptor* field);
            static void SetFieldValue(const char* rowdata, const google::protobuf::FieldDescriptor* field, google::protobuf::Message& result);
            static void ApplySelectResult(google::protobuf::Message& result, const char* rowdata, MYSQL_FIELD* field);
            static bool OnlyHoldsOneRepeatedMessageField(const google::protobuf::Message& msg);
            static void TrimString(std::string& str);
        private:
            std::string GenerateSqlSelectSingle(const google::protobuf::Message& msg) const;
            std::string GenerateSqlSelectMulti(const google::protobuf::Message& msg) const;
            int GenerateSqlSelectImpl(const google::protobuf::Message& msg, std::string& sql, bool selectAll) const;
            std::string GenerateSqlInsertSingle(const google::protobuf::Message& msg, bool update = false) const;
            std::string GenerateSqlInsertMulti(const google::protobuf::Message& msg) const;
            std::string GenerateSqlUpdateSingle(const google::protobuf::Message& msg) const;
            std::vector<std::string> GenerateSqlUpdateMulti(const google::protobuf::Message& msg) const;
            std::string GenerateSqlDeleteSingle(const google::protobuf::Message& msg) const;
            std::vector<std::string> GenerateSqlDeleteMulti(const google::protobuf::Message& msg) const;
            static void LogSql(const std::string& sql);
    };
}

#endif /*MYSQLGENERATOR_H*/
