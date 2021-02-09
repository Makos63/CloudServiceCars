#ifndef P1_SENSOR_H
#define P1_SENSOR_H

#include <unistd.h>
#include <iostream>
#include <string>
#include <ctime>
#include <sys/types.h>
#include <cstring>
#include <locale>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <unistd.h>
#include <chrono>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <mosquitto.h>

#define PARAM_COUNT_MIN 4
#define PARAM_COUNT_MAX 6


class Sensor {

public:
    void fuelSensor(std::string ip, std::string port, struct mosquitto * mosq);

    void mileageSensor(std::string ip, std::string port, struct mosquitto * mosq);

    void trafficSensor(std::string ip, std::string port, struct mosquitto * mosq);

    void avgSpeedSensor(std::string ip, std::string port, struct mosquitto * mosq);

    int getId() const;
    void setId(int id);

private:

    int id;
    std::string topic;
public:
    const std::string &getTopic() const;

    void setTopic(const std::string &topic);

private:
    std::string IntToString(int a);

    static std::string getCurrentTimestamp();

    void mqttPublischer(std::string data);

    std::string getHostIP();

    int UDPsendData(const std::string &ip, const std::string &port, std::string data, const std::string &sensorType);

};

#endif //P1_SENSOR_H
