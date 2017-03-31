#include <soul/protobuf-mysql/MysqlGenerator.h>
#include "./proto/test.pb.h"
#include <soul/Log.h>
#include <iostream>

using namespace soul;

const std::string database = "mytest";
const std::string table = "t_test";

void TestCaseInsertNothing() {
    table_test t;
    MysqlGenerator g(database, table);
    g.GenerateSqlInsert(t);
}

void TestCaseInsertSingleSomeField() {
    table_test t;
    t.set_keyid(1000);
    t.set_field2(2);
    MysqlGenerator g(database, table);
    g.GenerateSqlInsert(t);
}

void TestCaseInsertSingleAllField() {
    table_test t;
    t.set_keyid(1000);
    t.set_field1(1);
    t.set_field2(2);
    table_field_message* m = t.mutable_field3();
    m->set_filedint(-1);
    m->set_fielduint(3);
    m->set_fieldstring("string");
    MysqlGenerator g(database, table);
    g.GenerateSqlInsert(t);
}

void TestCaseInsertRepeatedNothing() {
    table_test_repeated t;
    MysqlGenerator g(database, table);
    g.GenerateSqlInsert(t);
}

void TestCaseInsertRepeatedSomeFiled() {
    table_test_repeated t;
    table_test* field = t.add_fields();
    field->set_keyid(1000);
    field->set_field2(20);
    MysqlGenerator g(database, table);
    g.GenerateSqlInsert(t);
}

void TestCaseInsertRepeatedAllField() {
    table_test_repeated t;
    table_test* field = t.add_fields();
    field->set_keyid(1000);
    field->set_field1(10);
    field->set_field2(20);
    table_field_message* m = field->mutable_field3();
    m->set_filedint(-10);
    m->set_fielduint(30);
    m->set_fieldstring("strings");
    MysqlGenerator g(database, table);
    g.GenerateSqlInsert(t);
}

void TestCaseInsertRepeatedMultiSomeField() {
    table_test_repeated t;
    table_test* field = t.add_fields();
    field->set_keyid(1000);
    field->set_field2(20);
    table_test* field2 = t.add_fields();
    field2->set_keyid(2000);
    field2->set_field2(40);
    MysqlGenerator g(database, table);
    g.GenerateSqlInsert(t);
}

void TestCaseInsertRepeatedMultiAllField() {
    table_test_repeated t;
    for(int i = 0; i != 4; ++i) {
        table_test* field = t.add_fields();
        field->set_keyid(1000);
        field->set_field1(10);
        field->set_field2(20);
        table_field_message* m = field->mutable_field3();
        m->set_filedint(-10);
        m->set_fielduint(30);
        m->set_fieldstring("strings");
    }
    MysqlGenerator g(database, table);
    g.GenerateSqlInsert(t);
}

void TestCaseSelectSingleAllField() {
    table_test t;
    MysqlGenerator g(database, table);
    g.GenerateSqlSelect(t);
}

void TestCaseSelectSingle() {
    table_test t;
    t.set_keyid(1);
    MysqlGenerator g(database, table);
    g.GenerateSqlSelect(t);
}

void TestCaseSelectSingleWhereCondition() {
    table_test t;
    t.set_keyid(1);
    MysqlGenerator g(database, table, "where field2 = 3");
    g.GenerateSqlSelect(t);
}

void TestCaseSelectMulti() {
    table_test_repeated t;
    table_test* field = t.add_fields();
    field->set_keyid(1000);
    field->set_field1(10);
    field->set_field2(20);
    table_test* field2 = t.add_fields();
    field2->set_keyid(2000);
    field2->set_field1(20);
    field2->set_field2(40);
    MysqlGenerator g(database, table);
    g.GenerateSqlSelect(t);
}

void TestCaseSelectMultiNothing() {
    table_test_repeated t;
    MysqlGenerator g(database, table);
    g.GenerateSqlSelect(t);
}

void TestCaseUpdateSingleNothing() {
    table_test t;
    MysqlGenerator g(database, table);
    g.GenerateSqlUpdate(t);
}

void TestCaseUpdateSingleSomeField() {
    table_test t;
    t.set_keyid(3);
    t.set_field1(0);
    t.set_field2(1);
    MysqlGenerator g(database, table);
    g.GenerateSqlUpdate(t);
}

void TestCaseUpdateSingleWhereCondition() {
    table_test t;
    t.set_keyid(3);
    t.set_field1(0);
    MysqlGenerator g(database, table, "where field2 = 10");
    g.GenerateSqlUpdate(t);
}

void TestCaseUpdateMultiNothing() {
    table_test_repeated t;
    MysqlGenerator g(database, table);
    g.GenerateSqlUpdate(t);
}

void TestCaseUpdateMultiNoUpdateKey() {
    table_test_repeated t;
    table_test* field = t.add_fields();
    field->set_keyid(1);
    MysqlGenerator g(database, table);
    g.GenerateSqlUpdate(t);
}

void TestCaseUpdateMultiWithUpdateKey() {
    table_test_repeated t;
    table_test* field = t.add_fields();
    field->set_keyid(1);
    field->set_field1(2);
    MysqlGenerator g(database, table);
    g.GenerateSqlUpdate(t);
}

void TestCaseUpdateMulti() {
    table_test_repeated t;
    table_test* field = t.add_fields();
    field->set_keyid(1);
    field->set_field1(2);
    table_test* field2 = t.add_fields();
    field2->set_keyid(10);
    field2->set_field1(20);
    MysqlGenerator g(database, table);
    g.GenerateSqlUpdate(t);
}

void TestCaseDeleteNothing() {
    table_test t;
    MysqlGenerator g(database, table);
    g.GenerateSqlDelete(t);
}

void TestCaseDelete() {
    table_test t;
    t.set_keyid(3);
    t.set_field1(0);
    MysqlGenerator g(database, table);
    g.GenerateSqlDelete(t);
}

void TestCaseDeleteWithWhere() {
    table_test t;
    t.set_keyid(3);
    t.set_field1(0);
    MysqlGenerator g(database, table, "where field2 = 2");
    g.GenerateSqlDelete(t);
}

void TestUpdateOnInsert() {
    table_test t;
    t.set_keyid(3);
    t.set_field1(0);
    t.set_field2(1);
    MysqlGenerator g(database, table);
    g.GenerateSqlUpdateOnInsert(t);
}

void TestCaseTrim(std::string str, int expect) {
    MysqlGenerator::TrimString(str);
    std::cout << str.length() << ", " << expect << ", " << str << std::endl;
}

int main(int argc, char *argv[]) {
    START_ASYNC_LOG();

    //TestCaseInsertNothing();
    //TestCaseInsertSingleSomeField();
    //TestCaseInsertSingleAllField();
    //TestCaseInsertRepeatedNothing();
    //TestCaseInsertRepeatedSomeFiled();
    //TestCaseInsertRepeatedAllField();
    //TestCaseInsertRepeatedMultiSomeField();
    //TestCaseInsertRepeatedMultiAllField();


    //TestCaseSelectSingleAllField();
    //TestCaseSelectSingle();
    //TestCaseSelectSingleWhereCondition();
    //TestCaseSelectMulti();
    //TestCaseSelectMultiNothing();

    //TestCaseUpdateSingleNothing();
    //TestCaseUpdateSingleSomeField();
    //TestCaseUpdateSingleWhereCondition();
    //TestCaseUpdateMultiNothing();
    //TestCaseUpdateMultiNoUpdateKey();
    //TestCaseUpdateMultiWithUpdateKey();
    //TestCaseUpdateMulti();

    //TestUpdateOnInsert();

    //TestCaseDeleteNothing();
    //TestCaseDelete();
    //TestCaseDeleteWithWhere();

    TestCaseTrim(" ", 0);
    TestCaseTrim("\t", 0);
    TestCaseTrim(" abc", 3);
    TestCaseTrim(" abcd   ", 4);
    TestCaseTrim("  ab cde   ", 6);
    TestCaseTrim("\tabcdef", 6);
    TestCaseTrim("\tabcdefg\t", 7);
    TestCaseTrim("\tabc defghi\t  ", 10);
    TestCaseTrim("\tabcde\tfghij\t", 11);
    return 0;
}
