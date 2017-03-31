#include <soul/protobuf-mysql/MysqlInterface.h>
#include <soul/protobuf-mysql/MysqlGenerator.h>
#include "./proto/test.pb.h"
#include <soul/Log.h>
#include <iostream>

using namespace soul;

const std::string database = "mytest";
const std::string table = "t_test";

void TestCaseInsertOneRow(MysqlInterface& interface) {
    table_test t;
    t.set_keyid(1);
    t.set_field1(2);
    t.set_field2(20);
    table_field_message* m = t.mutable_field3();
    m->set_filedint(-1);
    m->set_fielduint(10);
    m->set_fieldstring("test string");

    interface.ExecuteSqlInsert(MysqlGenerator(database, table), t);
}

void TestCaseInsertMultiRows(MysqlInterface& interface) {
    table_test_repeated r;
    table_test* t1 = r.add_fields();
    t1->set_keyid(2);
    t1->set_field1(10);
    t1->set_field2(20);
    table_field_message* m1 = t1->mutable_field3();
    m1->set_filedint(-10);
    m1->set_fielduint(10);

    table_test* t2 = r.add_fields();
    t2->set_keyid(3);
    t2->set_field1(20);
    t2->set_field2(30);
    table_field_message* m2 = t2->mutable_field3();
    m2->set_filedint(-20);
    m2->set_fielduint(30);

    interface.ExecuteSqlInsert(MysqlGenerator(database, table), r);
}

void TestCaseSelectRows(MysqlInterface& interface) {
    table_test t;
    //t.set_keyid(1);
    t.set_field2(20);

    interface.ExecuteSqlSelect(MysqlGenerator(database, table), t);

    LOG_DEBUG << "result: " << t.ShortDebugString();
}

void TestCaseSelectMultiRows(MysqlInterface& interface) {
    table_test_repeated r;
    table_test* t = r.add_fields();
    t->set_field2(20);

    int ret = interface.ExecuteSqlSelect(MysqlGenerator(database, table), r);
    LOG_DEBUG << "result: " << ret << ", " << r.ShortDebugString();
}

void TestCaseUpdateOnInsert(MysqlInterface& interface) {
    table_test t;
    t.set_field1(2);
    t.set_keyid(10);

    int ret = interface.ExecuteSqlUpdateOnInsert(MysqlGenerator(database, table), t);
    if(ret) {
        LOG_DEBUG << interface.LastError();
    }
}

void TestCaseDelete(MysqlInterface& interface) {
    table_test t;
    t.set_field2(30);

    int ret = interface.ExecuteSqlDelete(MysqlGenerator(database, table), t);
    if(ret) {
        LOG_DEBUG << interface.LastError();
    }
}

void TestCaseDeleteMulti(MysqlInterface& interface) {
    table_test_repeated r;
    table_test* t1 = r.add_fields();
    t1->set_field1(2);
    table_test* t2 = r.add_fields();
    t2->set_field1(20);

    int ret = interface.ExecuteSqlDelete(MysqlGenerator(database, table), r);
    if(ret) {
        LOG_DEBUG << interface.LastError();
    }
}

int main(int argc, char *argv[]) {
    START_ASYNC_LOG();

    MysqlInterface interface;
    if(interface.Connect("127.0.0.1", 3306, "root", "seasondi") == false) {
        std::cout << "connect to msyql fail" << std::endl;
        return -1;
    }
    interface.SetAutoCommit(false);

    TestCaseInsertOneRow(interface);
    TestCaseInsertMultiRows(interface);

    TestCaseSelectRows(interface);
    TestCaseSelectMultiRows(interface);

    TestCaseUpdateOnInsert(interface);

    TestCaseDelete(interface);
    TestCaseDeleteMulti(interface);

    interface.Commit();
    return 0;
}
