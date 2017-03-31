#!/bin/bash

my='mysql -uroot -pseasondi'

db=mytest
tb=t_test

echo "create table  $db.$tb (
keyid int unsigned NOT NULL,
field1 int unsigned NOT NULL,
field2 int unsigned NOT NULL,
field3 varchar(1024) NOT NULL,

PRIMARY KEY(keyID)
)ENGINE=innodb DEFAULT CHARSET=utf8;" | `$my`   
echo "process $db.$tb done"
