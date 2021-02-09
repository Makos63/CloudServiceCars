//
// Created by Christian on 01.01.2021.
//

#ifndef VSLAB_MARIADB_H
#define VSLAB_MARIADB_H

#include "mysql/mysql.h"
#include <string>
#include <iostream>

struct connection_credentials {
    //const char *server, *user, *password, *database;
    const char *server, *user, *database;

};

class MariaDB {
public:
    MariaDB(connection_credentials mysql_details);
    /**
     * creates connection to db with server(ip), user, pw, dbname
     * @return
     */
    MYSQL *connection_init();

    /**
     * runs a query and returns the values with "use_result"
     * @param connection
     * @param sql_query
     * @return the values from the connection
     */
    MYSQL_RES *run_query(MYSQL *connection, const char *sql_query);

    /**
     * combines connection_init and run_query
     * @param sqlQuery
     */
    void query(std::string sqlQuery);

    MYSQL *getConn() const;

    MYSQL_RES *getRes() const;

private:
    connection_credentials myCredentials;
    MYSQL *conn;
    MYSQL_RES *res;

};


#endif //VSLAB_MARIADB_H
