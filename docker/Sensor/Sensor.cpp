#include "Sensor.h"


const char *brokerIp;

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

int main(int argc, char *argv[]) {
    std::string mode, ip, port, id;
    Sensor *mySensor = new Sensor();

    if (argc > PARAM_COUNT_MIN && argc <= PARAM_COUNT_MAX) {
        srand(time(NULL) + 1000 * getpid());

        mode = argv[1];
        ip = argv[2];
        port = argv[3];
        brokerIp = argv[4];
        mySensor->setId(std::stoi(argv[5]));

        std::string tmp = "c/";
        tmp += std::to_string(mySensor->getId());
        std::cout << "sensor topic: " << tmp << std::endl;

        mySensor->setTopic(tmp);

        std::cout << "starting new sensor..." << std::endl;
        std::cout << "mode: " + mode << std::endl;
        std::cout << "ip: " + ip << std::endl;
        std::cout << "port: " + port << std::endl;
        std::cout << "brokerIp: " << brokerIp << std::endl;
        int rc;
        struct mosquitto *mosq;
        //init for mosquitto connection
        mosquitto_lib_init();

        std::string pubName = "p-" + mode;
        //clientid, use clean session or not, if user+pw is required
        mosq = mosquitto_new(pubName.c_str(), true, NULL);
        while (true) {


            try {

                //clientinstance, hostname/brokerip, broker port, keepalive time in seconds
                rc = mosquitto_connect(mosq, brokerIp, 1883, 60);
                //check if connection is successful. if so = 0, if not !=0
                if (rc != 0) {
                    mosquitto_destroy(mosq);
                    throw "Client could not connect to broker!";
                }


                if (mode == "fuelSensor") {
                    mySensor->fuelSensor(brokerIp, port, mosq);
                } else if (mode == "mileageSensor") {
                    mySensor->mileageSensor(brokerIp, port, mosq);
                } else if (mode == "trafficSensor") {
                    mySensor->trafficSensor(brokerIp, port, mosq);
                } else if (mode == "avgSpeedSensor") {
                    mySensor->avgSpeedSensor(brokerIp, port, mosq);
                } else {
                    std::cout << "debug todo" << std::endl;
                }
            } catch (char const *c) {
                std::cout << "failed with: " << std::endl;
                std::cout << c << std::endl;
                std::cout << "restarting sensor" << std::endl;
                mosq = mosquitto_new(pubName.c_str(), true, NULL);
                sleep(5);
            }
        }
        /*mosquitto_disconnect(mosq);
        mosquitto_destroy(mosq);

        mosquitto_lib_cleanup();*/
    }

}


std::string Sensor::IntToString(int a) {
    std::stringstream temp;
    temp << a;
    return temp.str();
}


std::string Sensor::getCurrentTimestamp() {
    std::array<char, 64> buffer = {0};
    time_t rawtime;//format struct
    time(&rawtime);//set rawtime with calender date UTC
    const auto timeinfo = localtime(&rawtime);//get current timezone UTC +2
    strftime(buffer.data(), sizeof(buffer), "%d-%m-%Y %H:%M:%S", timeinfo);
    std::string timeStr(buffer.data());
    return timeStr;
}


std::string Sensor::getHostIP() {
    char hostbuffer[256];
    char *IPbuffer;
    struct hostent *host_entry;
    int hostname;

    // To retrieve hostname
    hostname = gethostname(hostbuffer, sizeof(hostbuffer));


    // To retrieve host information
    host_entry = gethostbyname(hostbuffer);

    // convert from binary to dot notation
    IPbuffer = inet_ntoa(*((struct in_addr *)
            host_entry->h_addr_list[0]));


    return IPbuffer;
}

int
Sensor::UDPsendData(const std::string &ip, const std::string &port, std::string data, const std::string &sensorType) {

    int fileDiscriptorID, remoteConnection;
    struct sockaddr_in remoteServAddr{};
    struct hostent *h;


    try {

        h = gethostbyname(ip.c_str());// ip is hostname from docker

        if (h == NULL) {
            throw "unkown host";
        }

        //The inet_ntoa() function converts the Internet host address in, given in network byte order,
        //to a string in IPv4 dotted-decimal notation. The string is returned in a statically allocated buffer,
        //which subsequent calls will overwrite.

        std::cout << "creating connection to:  " << h->h_name << " with IP: "
                  << inet_ntoa(*(struct in_addr *) h->h_addr_list[0]) << " and port: " << port << std::endl;


        remoteServAddr.sin_family = h->h_addrtype;
        memcpy((char *) &remoteServAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);

        remoteServAddr.sin_port = htons(atoi(port.c_str()));

        //htonl, htons, ntohl, ntohs - convert values between host and network byte order

        fileDiscriptorID = socket(AF_INET, SOCK_DGRAM, 0);
        if (fileDiscriptorID < 0) {
            throw "Could not reserve socket: " + (std::string) strerror(errno);
        }


        data = sensorType + "|" + data + "|" + getHostIP() + "|" + getCurrentTimestamp();

        std::cout << "sending data: " << data << std::endl;
        remoteConnection = sendto(fileDiscriptorID, data.c_str(), strlen(data.c_str()) + 1, 0,
                                  (struct sockaddr *) &remoteServAddr, sizeof(remoteServAddr)); //länge von ip adresse
        if (remoteConnection < 0) {
            close(fileDiscriptorID);
            throw "Data transfer failed: " + errno;
        }
    } catch (const std::exception &e) {
        std::cout << "UDP send failed with: " << std::endl;
        std::cout << e.what() << std::endl;
    } catch (char const *c) {
        std::cout << "UDP send failed with: " << std::endl;
        std::cout << c << std::endl;
    }
    return 0;
}


void Sensor::fuelSensor(std::string ip, std::string port, struct mosquitto *mosq) {
    int fuel = 50;
    fuel += rand() % 50;
    while (true) {
        if (fuel < 20) {
            fuel = 100;
            sleep(15);
        } else {

            if (checkIfAlive(ip)) {
                fuel -= rand() % 2;

                //UDPsendData(ip, port, IntToString(fuel), "fuelSensor");
                //creates string for sending
                std::string data = "fuelSensor|" + IntToString(fuel) + "|" + getHostIP() + "|" + getCurrentTimestamp();
                std::cout << data << std::endl;
                //publishes message to the topic with the length, the data itself, secure connection or not, it it should be retained
                mosquitto_publish(mosq, NULL, topic.c_str(), data.length(), data.c_str(), 0, false);
                sleep(30);
            } else {
                throw "connection to broker is lost";
            }
        }

    }
}

void Sensor::mileageSensor(std::string ip, std::string port, struct mosquitto *mosq) {
    int clock = rand() % 150000;
    //std::string topic= "test/"+ip;
    while (true) {
        if (checkIfAlive(ip)) {
            clock += rand() % 3;
            //UDPsendData(ip, port, IntToString(clock), "mileageSensor");
            std::string data = "mileageSensor|" + IntToString(clock) + "|" + getHostIP() + "|" + getCurrentTimestamp();
            std::cout << data << std::endl;
            //publishes message to the topic with the length, the data itself, secure connection or not, it it should be retained
            mosquitto_publish(mosq, NULL, topic.c_str(), data.length(), data.c_str(), 0, false);
            sleep(30);
        } else {
            throw "connection to broker is lost";
        }
    }
}

void Sensor::trafficSensor(std::string ip, std::string port, struct mosquitto *mosq) {
    std::string states[4]{"frei", "maessig", "stark", "stau"};
    std::string current;

    while (true) {
        if (checkIfAlive(ip)) {
            current = states[rand() % 4];
            //UDPsendData(ip, port, current, "trafficSensor");
            std::string data = "trafficSensor|" + current + "|" + getHostIP() + "|" + getCurrentTimestamp();
            std::cout << data << std::endl;
            //publishes message to the topic with the length, the data itself, secure connection or not, it it should be retained
            mosquitto_publish(mosq, NULL, topic.c_str(), data.length(), data.c_str(), 0, false);

            sleep(15);
        } else {
            throw "connection to broker is lost";
        }
    }
}

void Sensor::avgSpeedSensor(std::string ip, std::string port, struct mosquitto *mosq) {

    int speed = 0;
    while (true) {
        if (checkIfAlive(ip)) {
            speed = rand() % 250;
            //UDPsendData(ip, port, IntToString(speed), "avgSpeedSensor");
            std::string data = "avgSpeedSensor|" + IntToString(speed) + "|" + getHostIP() + "|" + getCurrentTimestamp();
            std::cout << data << std::endl;
            //publishes message to the topic with the length, the data itself, secure connection or not, it it should be retained
            mosquitto_publish(mosq, NULL, topic.c_str(), data.length(), data.c_str(), 0, false);

            sleep(15);
        } else {
            throw "connection to broker is lost";
        }
    }
}

void Sensor::mqttPublischer(std::string data) {
    int rc;
    struct mosquitto *mosq;

    mosquitto_lib_init();
    std::string t = "sen" + this->IntToString(this->id);
    mosq = mosquitto_new(t.c_str(), true, NULL);

    rc = mosquitto_connect(mosq, brokerIp, 1883, 60);
    if (rc != 0) {
        printf("Client could not connect to broker! Error Code: %d\n", rc);
        mosquitto_destroy(mosq);
    }

    mosquitto_publish(mosq, NULL, "test/t1", data.length(), data.c_str(), 0, false);

    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);

    mosquitto_lib_cleanup();

}

int Sensor::getId() const {
    return id;
}

void Sensor::setId(int id) {
    Sensor::id = id;
}

const std::string &Sensor::getTopic() const {
    return topic;
}

void Sensor::setTopic(const std::string &topic) {
    Sensor::topic = topic;
}
