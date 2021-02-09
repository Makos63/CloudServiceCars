#include "Central.h"

#define  PARAM_COUNT 3

Central *myCentral;



bool checkIfAlive(std::string ip) {
    std::string com = "ping -c1 -s1 ";
    com += ip;
    com += " > /dev/null 2>&1";

    int x = system(com.c_str());
    if (x == 0) {
        //isConnectionAlive = true;
        return true;
    } else {
        //isConnectionAlive = false;

        return false;
    }

}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    printf("New message with topic %s: %s\n", msg->topic, (char *) msg->payload);
    //std::cout << "Message" << std::endl;
    //get topic and the payload of the mosquitto message
    //std::cout << "Topic: " << msg->topic << std::endl;
    //std::cout << "Message: " << (char *) msg->payload << std::endl;


    std::string tmp((char *) msg->payload);
    std::stringstream dataS(tmp);
    std::string type, data, senIP, timestamp;

    //split the payload (string) into the values with delimiter
    std::getline(dataS, type, '|');
    std::getline(dataS, data, '|');
    std::getline(dataS, senIP, '|');
    std::getline(dataS, timestamp);
    sensorDataMap.find(type)->second->addData(data, timestamp, senIP);

    //saves timestamp for sensorStatus calculation
    sensorTimes->find(type)->second=timestamp;

    //data transfer to provider. if it fails because provider is down -> test next provider.
    //try for so long until data is send successfully
    bool transferComplete = false;
    std::cout << "transferring data to provider" << std::endl;
    while (!transferComplete) {
        std::cout <<"trying..."<< std::endl;
        try {
            if (!myCentral->TransferDataRPC(type, data, timestamp,centralID)) {
                throw "gRPC failed while connecting to provider";
            } else {
                transferComplete = true;
                std::cout <<"Transfer succeed"<<std::endl;
            }
        } catch (char const *c) {
            std::cout << c << std::endl;
            std::cout << "Trying next provider.. " << std::endl;
            std::cout << "using following provider: " << myCentral->getNextProvider() << std::endl;
        }
    }
}

//test for connection and connect, if successful
void on_connect(struct mosquitto *mosq, void *obj, int rc) {
    std::cout << "ID: " << *(int *) obj << std::endl;
    if (rc != 0) {
        std::cout << "Conenction to broker failed" << std::endl;
        exit(-1);
    }
    //clientobj, messageid, topic, quality for service level
    mosquitto_subscribe(mosq, NULL, centralTopic.c_str(), 0);
}


void subscriber(std::string brokerIP) {
    int rc, id = std::stoi(centralID);

    mosquitto_lib_init();
    //client obj struct
    struct mosquitto *mosq;
    //create object
    //clientname, clean session, subscriber id
    std::string t = "sub-" + centralID;
    mosq = mosquitto_new(t.c_str(), true, &id);

    //void (Central::*func)(struct mosquitto *, void *, int );
    //func = &Central::on_connect;

    //callback if client is connected to broker
    mosquitto_connect_callback_set(mosq, on_connect);
    //callback if client recieves a massage
    mosquitto_message_callback_set(mosq, on_message);

    //connect to broker
    rc = mosquitto_connect(mosq, brokerIP.c_str(), 1883, 10);
    if (rc) {
        throw "Could not connect to Broker";
    }


    //loop for listening
    mosquitto_loop_start(mosq);
    printf("Press Enter to quit...\n");
    //quit loop with force or not
    mosquitto_loop_stop(mosq, false);

    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

}


int main(int argc, char **argv) {
    std::cout << "Starting central...." << std::endl;
    std::string rpcDesPort, brokerIP, id;
    std::string rpcDesIp[3] = {"172.20.0.30", "172.20.0.31", "172.20.0.32"};
    if (argc >= PARAM_COUNT) {
        std::string t = argv[1];
/* maybe some day
        std::string tmp(t);
        std::stringstream dataS(tmp);

        //split the payload (string) into the values with delimiter
        for (int i = 0; i < 3; i++) {
            std::getline(dataS, rpcDesIp[i], ',');
        }
*/

        rpcDesPort = argv[2];
        brokerIP = argv[3];
        centralID = argv[4];
        centralTopic = "c/";
        centralTopic += centralID;
        std::cout << "centralTopic: " << centralTopic << std::endl;

    } else {
        std::cout << "wrong arguments...";
        return 0;
    }
    //init of Central
    myCentral = new Central(rpcDesIp, rpcDesPort);
    myCentral->randTriedIPs();

    int tries = 3;
    while (tries != 0) {
        --tries;
        std::cout << "trying to init while using following provider: " << myCentral->getNextProvider() << std::endl;

        //try to load the data from db
        std::cout << "call loadData" << std::endl;
        if (myCentral->loadData(sensorDataMap)) {
            tries = 0;
        }


    }
    std::cout << "starting tcp" << std::endl;
    std::thread tcpThread(&Central::tcpListener, myCentral);
    std::cout << "tcp done" << std::endl;

    while (true) {
        try {
            //subscribe to the mosquitto
            std::cout << "sub start" << std::endl;
            subscriber(brokerIP);
            std::cout << "sub end" << std::endl;

        } catch (char const *c) {
            //std::cout << "Trying next provider.. " << std::endl;
            //std::cout << "using following provider: " << myCentral->getNextProvider() << std::endl;
            std::cout << "failed with: " << std::endl;
            std::cout << c << std::endl;

        }
    }
    tcpThread.join();
    //delete myCentral;
    return 0;
}


void Central::tcpListener() {
    std::cout << "Starting tcp listener" << std::endl;
    int serverfd = 0, mySocket;// readValue;
    int opt = 1;
    struct sockaddr_in address{};
    int addressLen = sizeof(address);

    //char buf[MAXLINE] = {0};
    //descriptor ipv4,  type ->udp/tcp, protocol 0=ip
    serverfd = socket(AF_INET, SOCK_STREAM, 0);

    if (serverfd == 0) {
        throw "socket creation failed....";
    }

    //manipulate socket options -> bind to port
    //can be helpful with with reuse of address and port -> no "address already in use" error
    if (setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        throw "socket option failed went wrong";
    }
    address.sin_family = AF_INET;           //setzt auf ipv4
    address.sin_addr.s_addr = INADDR_ANY;   //hört auf alle interfaces ip
    address.sin_port = htons(TCPPORT);      //setzt port -> 80(docker)/8080(wsl)

    //bind socket to local adress
    if (bind(serverfd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        throw "bind failed....";
    }

    //puts socket in passive mode, set max number of pending connection requests -> 1024
    if (listen(serverfd, 1024) < 0) {
        throw "listen failed";
    }
    //takes first connection request and puts it through to the socket
    while (true) {
        if ((mySocket = accept(serverfd, (struct sockaddr *) &address, (socklen_t *) &addressLen)) < 0) {
            throw "accept failed";
        }


        char host[NI_MAXHOST];
        char service[NI_MAXSERV];


        //getnameinfo - address-to-name translation in protocol-independent manner
        //translation of address to name
        //we had timeout from browser
        if (getnameinfo((sockaddr *) &address, sizeof(address), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
            std::cout << host << " getnameinfo port: " << service << " Socket id: " << mySocket << std::endl;
        } else {
            //inet_ntop - convert IPv4 and IPv6 addresses from binary to text form
            inet_ntop(AF_INET, &address.sin_addr, host, NI_MAXHOST);
            std::cout << host << " inet_ntop port: " << ntohs(address.sin_port) << " Socket id: " << mySocket
                      << std::endl;
        }


        char newBuf[MAXLINE] = {0};
        int recvBytes = recv(mySocket, newBuf, MAXLINE, 0);
        if (recvBytes == -1) {
            throw "error on recv()";
        } else if (recvBytes == 0) {
            std::cout << "connection closed by client" << std::endl;
        } else {
            //extracts the message like substr.
            std::string recivedMessage = std::string(newBuf, 0, recvBytes);
            std::cout << "messIn: " << recivedMessage << std::endl;

            //map um HTTP Header zu speichern
            std::map<std::string, std::string> headerOptions = headerInterpreter(recivedMessage);
            std::string path = headerOptions.find("path")->second;

            //check if method is Get, otherwise->error
            std::string res = checkForGet(headerOptions, path);
            std::cout << "response: " << res << std::endl;
            send(mySocket, res.c_str(), res.size(), 0);


            close(mySocket);
        }
    }

}


std::string Central::prepResponseBodyGet(std::string path) {
    std::string header = "<html><head><meta charset='utf-8'>"
                         "<meta name='author' content='Christian Kehr 755493 and Maciej Krzyszton 756037'>"
                         "<title>Car-Central-n</title></head>";//todo get central number
    std::string json;

    if (path == "fuelSensor") {
        json = sensorDataMap.find("fuelSensor")->second->printSensorDataAsJson();
    } else if (path == "mileageSensor") {
        json = sensorDataMap.find("mileageSensor")->second->printSensorDataAsJson();
    } else if (path == "trafficSensor") {
        json = sensorDataMap.find("trafficSensor")->second->printSensorDataAsJson();
    } else if (path == "avgSpeedSensor") {
        json = sensorDataMap.find("avgSpeedSensor")->second->printSensorDataAsJson();
    } else if (path == "status"){
        json = isSensorOnline();
    } else {

        json += "{\"CentralData\":[";
        json += sensorDataMap.find("fuelSensor")->second->printSensorDataAsJson();
        json += ",";
        json += sensorDataMap.find("mileageSensor")->second->printSensorDataAsJson();
        json += ",";
        json += sensorDataMap.find("trafficSensor")->second->printSensorDataAsJson();
        json += ",";
        json += sensorDataMap.find("avgSpeedSensor")->second->printSensorDataAsJson();
        json += "]}";
    }

    return header + json;
}


std::string Central::checkForGet(std::map<std::string, std::string> options, std::string path) {
    std::string response, tmp, body;
    tmp = options.find("Method")->second;

    if (tmp.compare("GET")) {
        body = prepResponseBodyGet(path);
        response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nServer: " + options.find("Host")->second +
                   "\r\nConnection:"
                   + options.find("Connection")->second +
                   "\r\nCache-Control: no-cache\r\nPragma: no-cache\r\nContent-Length: " +
                   std::to_string(body.size()) + "\r\n\r\n" + body;


    } else {
        body = "<html><head><title>VS-Central</title></head><body><h1>403 FORBIDDEN</h1></body></html>";
        response = "HTTP/1.1 403 FORBIDDEN\r\nContent-Type: text/html\r\nServer: central.com /" +
                   options.find("Host")->second +
                   "\r\nConnection: close\r\nCache-Control: no-cache\r\nPragma: no-cache" +
                   std::to_string(body.size()) + "\r\n\r\n" + body;
    }
    return response + body;
}

std::map<std::string, std::string> Central::headerInterpreter(std::string wholeHeader) {
    std::map<std::string, std::string> headerOptions;
    std::istringstream iss(wholeHeader);
    std::string line;
    std::getline(iss, line, '/');
    std::cout << line << std::endl;
    headerOptions.insert(std::make_pair("Method", line));       //example GET


    std::getline(iss, line, ' ');
    std::cout << line << std::endl;
    headerOptions.insert(std::make_pair("path", line));         // /home.html

    std::getline(iss, line, '\n');
    std::cout << line << std::endl;
    headerOptions.insert(std::make_pair("Protocol", line));     //HTTP/1.1

    //saves all following fields
    while (std::getline(iss, line)) {
        std::istringstream tempS(line);
        std::string key, value;
        std::getline(tempS, key, ':');
        std::getline(tempS, value);
        headerOptions.insert(std::make_pair(key, value));
    }
    return headerOptions;
}


void Central::udpListener() {
    std::cout << "Starting udp listener" << std::endl;
    int sockfd;
    char buffer[MAXLINE];

    struct sockaddr_in servaddr, cliaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        throw "socket creation failed";
    }


    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(UDPPORT);

    if (bind(sockfd, (const struct sockaddr *) &servaddr,
             sizeof(servaddr)) < 0) {
        throw "bind failed";
    }

    int n;
    socklen_t fromlen;
    fromlen = sizeof(struct sockaddr_in);

    while (true) {
        n = recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr *) &cliaddr, &fromlen);
        if (n < 0) {
            throw "Error on Data Transmission";
        }
        std::string test = buffer;
        std::cout << "Message: " << test << std::endl;

        std::stringstream udpData(test);
        std::string type, data, senIP, timestamp;

        std::getline(udpData, type, '|');
        std::getline(udpData, data, '|');
        std::getline(udpData, senIP, '|');
        std::getline(udpData, timestamp);
        sensorDataMap.find(type)->second->addData(data, timestamp, senIP);
        TransferDataRPC(type, data, timestamp,"0");

    }

}

bool Central::TransferDataRPC(std::string type, std::string data, std::string timestamp,std::string centralIDa) {
    return myClient->sendRequest(data, type, timestamp,centralIDa);
}


Central::Central(std::string rpcDesIp, std::string rpcDesPort) {

    auto *fuelSensor = new SensorData("fuelSensor", "empty");
    sensorDataMap.insert(std::make_pair(fuelSensor->getSensorType(), fuelSensor));

    auto *mileageSensor = new SensorData("mileageSensor", "empty");
    sensorDataMap.insert(std::make_pair(mileageSensor->getSensorType(), mileageSensor));

    auto *trafficSensor = new SensorData("trafficSensor", "empty");
    sensorDataMap.insert(std::make_pair(trafficSensor->getSensorType(), trafficSensor));

    auto *avgSpeedSensor = new SensorData("avgSpeedSensor", "empty");
    sensorDataMap.insert(std::make_pair(avgSpeedSensor->getSensorType(), avgSpeedSensor));

    std::string address = rpcDesIp + ":" + rpcDesPort;
    myClient = new Client(
            grpc::CreateChannel(
                    address,
                    grpc::InsecureChannelCredentials()
            )
    );

}

Central::~Central() {
    for (auto &i : sensorDataMap) {
        delete i.second;
    }
    delete myClient;
}

bool Central::loadData(std::map<std::string, SensorData *> SensorDataMap) {
    std::cout << "starting loadData" << std::endl;

    load l;
    //messagetype
    EmptyMessage emptyMessage; //request to provider to load the data from the db with correct central id
    ClientContext context;
    emptyMessage.set_id(centralID);

    Status status = myClient->getStub()->loadData(&context, emptyMessage, &l);

    if (status.ok()) {
        //load messagelength
        std::cout << "Data map size: " << l.sensordata_size() << std::endl;
        if (l.sensordata_size() == 0) {
            std::cout << "Database is empty... : " << std::endl;
        } else {
            for (unsigned int i = 0; i < l.sensordata_size(); ++i) {
                //iterate through map and extract the values
                SensorDataMess mess = l.sensordata().at(i);
                //loads map of SensorDataMess. similar to the recordset from ewa
                SensorDataMap.find(mess.sensortype())->second->addData(mess.sensormydata(), mess.sensortimestamp(),
                                                                       "127.0.0.1");
            }
        }
    } else {
        //throw "cannot load Data";
        return false;
    }
    return true;
}

Central::Central(std::string *ips, std::string rpcDesPort) {

    auto *fuelSensor = new SensorData("fuelSensor", "empty");
    sensorDataMap.insert(std::make_pair(fuelSensor->getSensorType(), fuelSensor));

    auto *mileageSensor = new SensorData("mileageSensor", "empty");
    sensorDataMap.insert(std::make_pair(mileageSensor->getSensorType(), mileageSensor));

    auto *trafficSensor = new SensorData("trafficSensor", "empty");
    sensorDataMap.insert(std::make_pair(trafficSensor->getSensorType(), trafficSensor));

    auto *avgSpeedSensor = new SensorData("avgSpeedSensor", "empty");
    sensorDataMap.insert(std::make_pair(avgSpeedSensor->getSensorType(), avgSpeedSensor));

    rpcDesIps = ips;
    rpcPort = rpcDesPort;

    sensorTimes->insert(std::make_pair("fuelSensor","empty"));
    sensorTimes->insert(std::make_pair("mileageSensor","empty"));
    sensorTimes->insert(std::make_pair("trafficSensor","empty"));
    sensorTimes->insert(std::make_pair("avgSpeedSensor","empty"));

    sensorOnline["fuelSensor"] = false;
    sensorOnline["mileageSensor"] = false;
    sensorOnline["trafficSensor"] = false;
    sensorOnline["avgSpeedSensor"] = false;
}

std::string Central::getNextProvider() {

    while (true) {
        if (!triedIps[0]) {
            triedIps[0] = true;
            return initNewConnection("172.20.0.30:45000");
        } else if (!triedIps[1]) {
            triedIps[1] = true;
            return initNewConnection("172.20.0.31:45000");
        } else if (!triedIps[2]) {
            triedIps[2] = true;
            return initNewConnection("172.20.0.32:45000");
        } else {
            std::cout << "Tried all provides. None available for connection" << std::endl;
            triedIps[0] = false;
            triedIps[1] = false;
            triedIps[2] = false;

        }
        sleep(1); //for more readable logs
    }

}

std::string Central::initNewConnection(std::string ip) { //returns ip, if the "client" sends a ok == true
    myClient = new Client(
            grpc::CreateChannel(
                    ip,
                    grpc::InsecureChannelCredentials()
            )
    );
    return ip;
}


void Central::randTriedIPs(){
    srand(time(NULL));
    for(int i = 0 ; i<3;i++){
        triedIps[i] = rand() % 2;
    }
    if(triedIps[0] && triedIps[1] && triedIps[2]){
        triedIps[0]=false;      //to have at least one untried provider
    }
}

std::string Central::getCurrentTimestamp() {        //for the calculation of sensorStatus
    std::array<char, 64> buffer = {0};
    time_t rawtime;//format struct
    time(&rawtime);//set rawtime with calender date UTC
    const auto timeinfo = localtime(&rawtime);//get current timezone UTC +2
    strftime(buffer.data(), sizeof(buffer), "%d-%m-%Y %H:%M:%S", timeinfo);
    std::string timeStr(buffer.data());
    return timeStr;
}

std::string Central::isSensorOnline() {

    std::string answer;

    std::string currentTime = getCurrentTimestamp().substr(11,8);

    std::string speedTime = (sensorTimes->find("avgSpeedSensor")->second).substr(11,8);
    std::string fuelTime = (sensorTimes->find("fuelSensor")->second).substr(11,8);
    std::string trafficTime = (sensorTimes->find("trafficSensor")->second).substr(11,8);
    std::string mileageTime = (sensorTimes->find("mileageSensor")->second).substr(11,8);


    //first checks hour difference, if it is outside of the timeframe, it sets it to false
    int fuelHourDiff = stoi(currentTime.substr(0,2)) - stoi(fuelTime.substr(0,2));
    if (fuelHourDiff <= 1 || std::abs(fuelHourDiff) == 23){
        int fuelMinuteDiff = stoi(currentTime.substr(3,2)) - stoi(fuelTime.substr(3,2));
        if (fuelMinuteDiff <= 2 || std::abs(fuelMinuteDiff) == 58 || std::abs(fuelMinuteDiff) == 59){
            sensorOnline.find("fuelSensor")->second = true;
        }else{
            sensorOnline.find("fuelSensor")->second = false;
        }
    }
    else{
        sensorOnline.find("fuelSensor")->second = false;
    }

    int speedHourDiff = stoi(currentTime.substr(0,2)) - stoi(speedTime.substr(0,2));
    if (speedHourDiff <= 1 || std::abs(speedHourDiff) == 23){
        int speedMinuteDiff = stoi(currentTime.substr(3,2)) - stoi(speedTime.substr(3,2));
        if (speedMinuteDiff <= 2 || std::abs(speedMinuteDiff) == 58 || std::abs(speedMinuteDiff) == 59){
            //std::cout<<"speedTimeDifference"<< speedMinuteDiff <<std::endl;
            sensorOnline.find("avgSpeedSensor")->second = true;
        }else{
            sensorOnline.find("avgSpeedSensor")->second = false;
        }
    }
    else{
        sensorOnline.find("avgSpeedSensor")->second = false;
    }

    int trafficHourDiff = stoi(currentTime.substr(0,2)) - stoi(trafficTime.substr(0,2));
    if (trafficHourDiff <= 1 || std::abs(trafficHourDiff) == 23){
        int trafficMinuteDiff = stoi(currentTime.substr(3,2)) - stoi(trafficTime.substr(3,2));
        if (trafficMinuteDiff <= 2 || std::abs(trafficMinuteDiff) == 58 || std::abs(trafficMinuteDiff) == 59){
            sensorOnline.find("trafficSensor")->second = true;
        }else{
            sensorOnline.find("trafficSensor")->second = false;
        }
    }
    else{
        sensorOnline.find("trafficSensor")->second = false;
    }

    int mileageHourDiff = stoi(currentTime.substr(0,2)) - stoi(mileageTime.substr(0,2));
    if (mileageHourDiff <= 1 || std::abs(mileageHourDiff) == 23){
        int mileageMinuteDiff = stoi(currentTime.substr(3,2)) - stoi(mileageTime.substr(3,2));
        if (mileageMinuteDiff <= 2 || std::abs(mileageMinuteDiff) == 58 || std::abs(mileageMinuteDiff) == 59){
            sensorOnline.find("mileageSensor")->second = true;
        }else{
            sensorOnline.find("mileageSensor")->second = false;
        }
    }
    else{
        sensorOnline.find("mileageSensor")->second = false;
    }


    for(auto i =sensorOnline.begin();i!=sensorOnline.end();i++){
        answer+="\""+i->first+"\":{";

        answer+=R"("value":)";
        if(i->second){
            answer+="\"online";
        }else{
            answer+="\"offline";
        }

        answer+=+"\"}";
    }

    return answer;
}
