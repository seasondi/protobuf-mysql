测试表结构
create table  mytest.t_test ( \
    keyid int unsigned NOT NULL, \
    field1 int unsigned NOT NULL, \
    field2 int unsigned NOT NULL, \
    field3 varchar(1024) NOT NULL, \
    PRIMARY KEY(keyID)\
)ENGINE=innodb DEFAULT CHARSET=utf8;


1.定义表结构对应的protobuf message

import "MysqlDescriptor.proto";

package soul;

message table_field_message { \
    optional int32 filedint = 1; \
    optional uint32 fielduint = 2; \
    optional string fieldstring = 3;\
}

message table_test { \
    optional uint32 keyid = 1[(primarykey)=true]; \
    optional uint32 field1 = 2[(updatekey)=true]; \
    optional uint32 field2 = 3; \
    optional table_field_message field3 = 4;\
}

message table_test_repeated { \
    repeated table_test fields = 1;\
}

2.所有作为ExecuteSql\*函数的传入参数的message,若有repeated字段，则该message类型要么只有一个为repeated message的字段,要么不能含有任何repeated字段 \
  复杂的条件通过构造MysqlGenerator时传入，传入的条件会覆盖默认规则构造的条件

3.select \
将要查询的结果字段定义为message类型，并且设置其中作为查询的key的字段的值 \
1)查询单条结果 \
table_test t; \
t.set_field2(20); \
表示查询table_test中除了field2字段以外的其他字段的值，并且以field2等于20作为查询条件,查询到的结果若有多条取第一条,结果存储在t中 \
2)查询多条结果 \
table_test_repeated r; \
table_test* t = r.add_fields(); \
t->set_field2(20); \
赋值repeated列表中的第一条作为查询条件,查询结果存储在r中

4.insert \
定义要插入的message,并给相应字段赋值即可

5.update \
为表对应的message相应字段赋值,仅赋值需要更新的字段字段，并且被制定为updatekey的字段必须赋值并且会作为更新条件

6.delete \
为表对应的message相应字段赋值作为删除条件
