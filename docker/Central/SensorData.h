#ifndef P1_SENSORDATA_H

#define P1_SENSORDATA_H


#include <string>
#include <list>
#include <map>

class SensorData {
private:
    bool IPknown=false;
    std::multimap<std::string,std::string>* myDataMap;// [key] timestamp, [speed] etc value
    std::string sensorType, sensorIP;
public:
    void setSensorIp(const std::string &sensorIp);

    SensorData(std::string sensorType, std::string sensorIP);
    /**
     * adds Data to the myDataMap multimap
     * @param data the sensor values
     * @param timestamp timestamp of data creation
     * @param sensorIP self explanatory
     */
    void addData(std::string data, std::string timestamp, std::string sensorIP);
    /**
     * prints the sensor data from the map in valid json format
     * @return the valid structured data
     */
    std::string printSensorDataAsJson();



    const std::string &getSensorIp() const;

    const std::string &getSensorType() const;

    ~SensorData();


};


#endif //P1_SENSORDATA_H
