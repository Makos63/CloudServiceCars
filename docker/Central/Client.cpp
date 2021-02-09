
#include <string>

#include <grpcpp/grpcpp.h>
#include "dataproto.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;


using dataproto::SensorDataMess;
using dataproto::SensorDataResponse;
using dataproto::EmptyMessage;
using dataproto::load;


class Client {
public:
    Client(std::shared_ptr<Channel> channel) : stub_(dataproto::sensorService::NewStub(channel)){};

    bool sendRequest(std::string myData, std::string sensorType,
                             std::string timestamp,std::string centralID) {
        SensorDataMess SensorDataSend;


        SensorDataSend.set_sensormydata(myData);
        SensorDataSend.set_sensortype(sensorType);
        SensorDataSend.set_sensortimestamp(timestamp);
        SensorDataSend.set_id(centralID);
        SensorDataResponse result;

        ClientContext context;

        Status status = stub_->sendRequest(&context, SensorDataSend, &result);

        if (status.ok()) {
            //return result.result();
            return true;
        } else {
/*
            std::cout<<"gRPC failed with:" << status.error_code() << ": " << status.error_message() << std::endl;
            return -1;
  */
            return false;
        }
    }


private:
    std::unique_ptr<dataproto::sensorService::Stub> stub_;
public:
    const std::unique_ptr<dataproto::sensorService::Stub> &getStub() const {
        return stub_;
    }
};
