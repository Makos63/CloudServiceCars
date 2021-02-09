//
// Created by Christian on 01.01.2021.
//

#include "MariaDB.h"

MariaDB::MariaDB(connection_credentials mysql_details) {
    this->myCredentials = mysql_details;
}

MYSQL *MariaDB::connection_init() {
    std::cout<<"starting connection_init"<<std::endl;

    if ( (conn = mysql_init(nullptr)) == nullptr)
    {
        std::cout<<"mysql_init() failed"<<std::endl;

    }
    //if(!mysql_real_connect(conn, this->myCredentials.server, this->myCredentials.user, this->myCredentials.password, this->myCredentials.database,3306,NULL,0)){
    if(!mysql_real_connect(conn, this->myCredentials.server, this->myCredentials.user, NULL, this->myCredentials.database,3306,NULL,0)){
        throw "Error with connection...";
    }
    return conn;
}

MYSQL_RES *MariaDB::run_query(MYSQL *connection, const char *sql_query) {   //const char* array oder string
    if (mysql_query(connection, sql_query)){
        throw "MariaDB Query Error...";
    }
    return mysql_use_result(connection);
}

void MariaDB::query(std::string sqlQuery) {
    this->conn = connection_init();
    this->res = run_query(this->conn, sqlQuery.c_str());

}

MYSQL *MariaDB::getConn() const {
    return conn;
}

MYSQL_RES *MariaDB::getRes() const {
    return res;
}
