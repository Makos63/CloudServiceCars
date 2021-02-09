#ifndef P1_CENTRAL_H
#define P1_CENTRAL_H
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <map>
#include <pthread.h>
#include <thread>
#include <condition_variable>
#include <mosquitto.h>
#include "Client.cpp"
#include "SensorData.h"
#include <grpcpp/grpcpp.h>
#include "dataproto.grpc.pb.h"
#include "dataproto.pb.h"
#include <random>
#include "time.h"

#define TCPPORT 8080
#define UDPPORT 50000
#define MAXLINE 2048


/**
 * gRPC generierte Services und Funktionen
 */
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;


using dataproto::SensorDataMess;
using dataproto::SensorDataResponse;
using dataproto::EmptyMessage;
using dataproto::load;
using dataproto::sensorService;

std::map<std::string,bool>sensorOnline;
std::map<std::string,std::string>*sensorTimes= new std::map<std::string,std::string>();
std::map<std::string, SensorData*> sensorDataMap;
std::string centralID,centralTopic;
std::random_device rd; // obtain a random number from hardware
std::uniform_int_distribution<> distr(0, 1); // define the range
std::mt19937 gen(rd()); // seed the generator
class Central {

public:
    /**
     *implements tcpSocket for HTTP Requests
     */
    void tcpListener();
    /**
     * udpSocket for recieving Sensordata
     * legacy but still functional
     */
    void udpListener();
    /**
     * gives the data to the rpc to send it to the reciever (provider)
     * @param type sensortype of the message
     * @param data the sensordata -> payload
     * @param timestamp when the data was created
     * @return true/false -> was the transfer successful
     */
    bool TransferDataRPC(std::string type ,std::string data,std::string timestamp,std::string centralID);

    /**
     * loads data from the database - provider system
     * @param SensorDataMap the map to store the sensor values
     * @return true/false if load was a success
     */
    bool loadData(std::map<std::string, SensorData*>SensorDataMap);

    /**
     * for stub creation to connect the rpc
     * @param ip parameter to know, where to connect to (it is an IP)
     * @return ip if connection init was a success
     */
    std::string initNewConnection(std::string ip);

    /**
     * tries the prividers if one fails to get a new one
     * @return ip of the connected provider
     */
    std::string getNextProvider();

    /**
     * constructor for the central. it initializes the maps - if needed
     * @param ip
     * @param port
     */
    Central(std::string ip, std::string port);

    /**
     * constructor for the central. it initializes the maps - if needed
     * @param ips IPs of the rpc - service providers
     * @param port ports for the provider
     */
    Central(std::string ips[], std::string port);

    /**
     * deletes the rpc connection (client)
     */
    ~Central();

    void randTriedIPs();

    std::string getCurrentTimestamp();

    std::string isSensorOnline();

private:
    std::string rpcPort;
    std::string *rpcDesIps;
    bool triedIps[3]={false};
    Client *myClient;


    /**
     * extracts header options from http
     * @param wholeHeader http header to get split up
     * @return map with the extracted header options
     */
    std::map<std::string, std::string> headerInterpreter(std::string wholeHeader);
    /**
     * checks for get request, otherwise returns 403-forbidden
     * @param options all header options
     * @param path where to return
     * @return fitting response + http header
     */
    std::string checkForGet(std::map<std::string, std::string> options, std::string path);
    /**
     * prepares the general Body of the html retur
     * @param path which data should be displayed
     * @return header + the Data in json format
     */
    std::string prepResponseBodyGet(std::string path);




    //void on_connect(struct mosquitto *mosq, void *obj, int rc);

    //void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg);
    std::unique_ptr<dataproto::sensorService::Stub> stub_;

};


#endif //P1_CENTRAL_H
