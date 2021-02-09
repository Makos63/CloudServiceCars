#include "SensorData.h"

const std::string &SensorData::getSensorType() const {
    return sensorType;
}

const std::string &SensorData::getSensorIp() const {
    return sensorIP;
}

void SensorData::addData(std::string data,std::string timestamp, std::string sensorIP ) {
    if(!IPknown){
        setSensorIp(sensorIP);
    }

    myDataMap->insert(std::make_pair(timestamp,data));

}

SensorData::~SensorData() {
    delete myDataMap;
}

SensorData::SensorData(std::string sensorType, std::string sensorIP) {
    myDataMap = new std::multimap<std::string,std::string>;
    this->sensorType=sensorType;
    this->sensorIP=sensorIP;
}

std::string SensorData::printSensorDataAsJson() {
    std::string objStart="{\""+getSensorType()+"\":{";
    //std::string objIP=R"("ip":")"+getSensorIp()+"\",";
    std::string objData;

    //rbein rend for reverse to sort the data from newest to oldest
    for(auto i =myDataMap->rbegin();i!=myDataMap->rend();i++){
        objData+="\""+i->first+"\":{";


        objData+=R"("value":)";
        objData+="\""+i->second+"\"}";
        auto t =i;
        if(++t != myDataMap->rend()){
            objData+=",";
        }

    }





    std::string objClose ="}\n}";

    //std::string response = objStart+objIP+objData+objClose;
    std::string response = objStart+objData+objClose;

    return response;
}

void SensorData::setSensorIp(const std::string &sensorIp) {
    sensorIP = sensorIp;
    IPknown=true;
}
