#ifndef MYSQLERROR_H 
#define MYSQLERROR_H

#include <mysql/mysqld_error.h>

enum SQL_ERROR {
    SQL_GENERATE_FAIL = 100000,
    SQL_GENERATE_EMPTY = 100001,
    SQL_ROLLBACK = 100002,
};

#endif /*MYSQLERROR_H*/
