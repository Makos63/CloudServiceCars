#include <string>

#include <grpcpp/grpcpp.h>
#include "dataproto.grpc.pb.h"
//#include  <mysql/mysql.h>
#include "MariaDB.h"
#include "exception"
#include <random>
#include <unistd.h>
#define  PARAM_COUNT 3

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;


using dataproto::SensorDataMess;
using dataproto::SensorDataResponse;
using dataproto::EmptyMessage;
std::random_device rd; // obtain a random number from hardware
std::uniform_int_distribution<> distr(0, 1); // define the range
std::mt19937 gen(rd()); // seed the generator


class Provider final : public dataproto::sensorService::Service {
private:
    MariaDB *maria;
    bool triedDBs[3] = {false};

    void randTriedDBs(){
        srand(time(NULL));
        for(int i = 0 ; i<3;i++){
            triedDBs[i] = rand() % 2;
        }
        if(triedDBs[0] && triedDBs[1] && triedDBs[2]){
            triedDBs[0]=false;
        }
    }


public:
    //set values for the db-connection
    void initMaria() {
        connection_credentials myMariaDB;
        myMariaDB.server = "172.20.0.10";
        //myMariaDB.server = "localhost";
        //myMariaDB.user = "public";
        myMariaDB.user = "root";
        // myMariaDB.password = "public";
        myMariaDB.database = "vsTest";
        this->maria = new MariaDB(myMariaDB);
        //triedDBs[0] = true;
        randTriedDBs();
    }

    void node1Init() {
        connection_credentials myMariaDB;
        myMariaDB.server = "172.20.0.10";
        //myMariaDB.server = "localhost";
        //myMariaDB.user = "public";
        myMariaDB.user = "root";
        // myMariaDB.password = "public";
        myMariaDB.database = "vsTest";
        this->maria = new MariaDB(myMariaDB);
    }

    void node2Init() {
        connection_credentials myMariaDB;
        myMariaDB.server = "172.20.0.11";
        //myMariaDB.server = "localhost";
        //myMariaDB.user = "public";
        myMariaDB.user = "root";
        // myMariaDB.password = "public";
        myMariaDB.database = "vsTest";
        this->maria = new MariaDB(myMariaDB);
    }

    void node3Init() {
        connection_credentials myMariaDB;
        myMariaDB.server = "172.20.0.12";
        //myMariaDB.server = "localhost";
        //myMariaDB.user = "public";
        myMariaDB.user = "root";
        // myMariaDB.password = "public";
        myMariaDB.database = "vsTest";
        this->maria = new MariaDB(myMariaDB);
    }

    void setMariaFree() {
        delete this->maria;
    }

    int nextDb() {
        while (true) {
            if (triedDBs[0] == false) {
                triedDBs[0] = true;
                node1Init();
                return 1;
            } else if (triedDBs[1] == false) {
                triedDBs[1] = true;
                node2Init();
                return 2;
            } else if (triedDBs[2] == false) {
                triedDBs[2] = true;
                node3Init();
                return 3;
            } else {
                std::cout << "Tried all dbs. None available for connection" << std::endl;
                triedDBs[0] = false;
                triedDBs[1] = false;
                triedDBs[2] = false;
            }
        }
    }

    Status sendRequest(
            //verwaltet verbindung
            ServerContext *context,
            const SensorDataMess *request,
            SensorDataResponse *reply
    ) override {

        std::string type = request->sensortype();
        std::string data = request->sensormydata();
        std::string timestamp = request->sensortimestamp();
        std::string centralId = request->id();

        while (true) {
            try {


                //check if data is empty and returns type for debug
                //if data not empty, sends the sql query to insert the data into the db

                if (!type.empty() && !data.empty() && !timestamp.empty()) {
                    std::cout << "type:" << type << std::endl;

                    //maria->query("insert into centralData values (null,\"" + type + "\",\"" + data + "\",\"" + timestamp +"\")"); //centralid
                    maria->query("insert into centralData values (null,\"" + type + "\",\"" + data + "\",\"" + timestamp +"\",\""+ centralId + "\")");


                    mysql_close(maria->getConn());
                    return Status::OK;
                } else {
                    return Status::CANCELLED;
                }
            } catch (char const *c) {
                //chose next dbnode
                std::cout << c << std::endl;
                std::cout << "trying next database point" << std::endl;

                setMariaFree();
                sleep(1);
                std::cout << "usingDatabase: " << nextDb() << std::endl;


            }catch (const std::exception &e) {
                //chose next dbnode
                std::cout << e.what() << std::endl;
                std::cout << "trying next database point" << std::endl;
                sleep(1);
                setMariaFree();
                std::cout << "usingDatabase: " << nextDb() << std::endl;


            }

        }

    }

    Status loadData(::grpc::ServerContext *context, const ::dataproto::EmptyMessage *request,
                    ::dataproto::load *response) override {

        std::cout << "caught loadData" << std::endl;
        //query to get all data from the db for system startup
        //maria->query("select id, sensorType, sensorValue, sensorTimestamp from centralData;");//where centralID= protID


        std::string query= "select id, sensorType, sensorValue, sensorTimestamp from centralData where centralId=";
        query+=request->id();
        //std::cout<<"db query: "<<query<<std::endl;

        //maria->query("select id, sensorType, sensorValue, sensorTimestamp from centralData;");//where centralID= protID
        maria->query(query);
        MYSQL_ROW row;
        std::cout << "loading response..." << std::endl;
        //inserting the values similar to ewa with the array
        for (unsigned int i = 0; (row = mysql_fetch_row(maria->getRes())) != NULL; ++i) {
            auto pair = response->mutable_sensordata();
            SensorDataMess Data;
            Data.set_sensortype(row[1]);
            Data.set_sensormydata(row[2]);
            Data.set_sensortimestamp(row[3]);
            (*pair)[i] = Data;
        }
        mysql_free_result(maria->getRes());
        mysql_close(maria->getConn());
        return Status::OK;
    }


};

//init of rpc
void rpcServer(std::string ip, std::string port) {
    std::string address = ip + ":" + port;
    //init for provider and builder
    Provider service;

    ServerBuilder builder;
    //says to listen
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

//init of maria

    service.initMaria();
    std::cout << "Server listening on port: " << address << std::endl;

    std::unique_ptr<Server> server(builder.BuildAndStart());


    server->Wait();
    //if the connection and writing fails->output of the error

}


int main(int argc, char **argv) {
    if (argc == PARAM_COUNT) {
        std::string ip = argv[1];
        std::string port = argv[2];

        rpcServer(ip, port);
    } else {
        std::cout << "wrong arguments...";
    }
    return 0;
}