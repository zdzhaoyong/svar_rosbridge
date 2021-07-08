/*
 *  Created on: Apr 16, 2018
 *      Author: Poom Pianpak
 */

#include "rosbridge_ws_client.hpp"
#include <future>
//#include "Messenger.h"
#include "Svar.h"
#include "Glog.h"

using namespace sv;
using namespace GSLAM;

class ROSSubscriber{
public:
    ROSSubscriber(std::string topic,sv::Svar func,sv::Svar config=sv::Svar())
        : config(config),topic(topic),client(config.get<std::string>("url","localhost:9090")){
        sv::Svar msg=
        { {"op", "subscribe"},
          {"topic",topic},
          {"compression", config.get<std::string>("compression","json")}
        };

        if(config.exist("id"))   msg["id"]=config["id"];
        if(config.exist("type")) msg["type"]=config["type"];
        if(config.exist("throttle_rate")) msg["throttle_rate"]=config["throttle_rate"];
        if(config.exist("queue_length"))  msg["queue_length"] =config["queue_length"];
        if(config.exist("fragment_size")) msg["fragment_size"]=config["fragment_size"];

        client.on_open=[this,msg](std::shared_ptr<WsClient::Connection> connection){
            connection->send(msg.dump_json());
            this->connection=connection;
        };

        client.on_message=[this,func](std::shared_ptr<WsClient::Connection>, std::shared_ptr<WsClient::InMessage> msg){
            auto compression=this->config.get<std::string>("compression","json");
            if(compression=="json")
                func(sv::Svar::parse_json(msg->string())["msg"]);
            else if(compression=="cbor-raw"||compression=="cbor"){
                std::string str=msg->string();
                try{
                    sv::Svar obj=svar["serializers"]["cbor"].call("load",sv::SvarBuffer(str.data(),str.size()));
                    func(obj["msg"]);
                }
                catch(...){
                    LOG(ERROR)<<"Failed to parse cbor with size "<<str.size();
                }
            }
            else
                func(msg->string());
        };
        work_thread=std::thread([this](){client.start();});
    }

    ~ROSSubscriber(){
        sv::Svar msg={{ "op", "unsubscribe"},
                      {"topic",topic}};
        if(config.exist("id"))   msg["id"]=config["id"];
        client.stop();
        work_thread.join();
    }

    std::string topic;
    sv::Svar config;
    WsClient client;
    std::shared_ptr<WsClient::Connection> connection;
    std::thread work_thread;
};

class ROSPublisher{
public:
    ROSPublisher(std::string topic,sv::Svar config=sv::Svar())
        : topic(topic),config(config),
          client(config.get<std::string>("url","localhost:9090")),
          url(config.get<std::string>("url","localhost:9090")){
        sv::Svar msg=
        { {"op", "advertise"},
          {"topic",topic}
        };

        if(config.exist("id"))   msg["id"]=config["id"];
        if(config.exist("type")) msg["type"]=config["type"];

        client.on_open=[this,msg](std::shared_ptr<WsClient::Connection> connection){
            connection->send(msg.dump_json());
            this->connection=connection;
        };

        work_thread=std::thread([this](){client.start();});
    }

    void publish(sv::Svar msg){
        WsClient client(url);
        sv::Svar op={ {"op", "publish"},
                      {"topic", topic},
                      {"msg", msg}
                    };

        if(config.exist("id"))   op["id"]=config["id"];
        auto str = op.dump_json();
        client.on_open=[this,str](std::shared_ptr<WsClient::Connection> connection){
            connection->send(str);
            connection->send_close(1000);
        };
        client.start();
    }

    ~ROSPublisher(){
        sv::Svar msg={{ "op", "unadvertise"},
                      {"topic",topic}};
        if(config.exist("id"))   msg["id"]=config["id"];
        auto str=msg.dump_json();
        connection->send(str);
        client.stop();
        work_thread.join();
    }

    std::string topic;
    sv::Svar config;
    WsClient client;
    std::shared_ptr<WsClient::Connection> connection;
    std::thread work_thread;
    std::string url;
};

class ServiceCaller{
public:
    ServiceCaller(std::string name,sv::Svar args,sv::Svar config=sv::Svar())
        : topic(name),config(config),client(config.get<std::string>("url","localhost:9090")){
        sv::Svar msg=
        { {"op", "call_service"},
          {"service",topic}
        };

        if(config.exist("id"))   msg["id"]=config["id"];
        if(config.exist("fragment_size")) msg["fragment_size"]=config["fragment_size"];
        msg["args"] = args;

        client.on_open=[this,msg](std::shared_ptr<WsClient::Connection> connection){
            connection->send(msg.dump_json());
        };

        client.on_message=[this](std::shared_ptr<WsClient::Connection>, std::shared_ptr<WsClient::InMessage> msg){
            sv::Svar response=sv::Svar::parse_json(msg->string());
            result.set_value(response);
        };
        work_thread=std::thread([this](){client.start();});
    }

    sv::Svar wait_result(){
        sv::Svar ret=result.get_future().get();
        client.stop();
        return ret;
    }

    ~ServiceCaller(){
        while (!work_thread.joinable()) {
            usleep(1000);
        }
        work_thread.join();
    }

    std::string topic;
    sv::Svar    config;
    WsClient    client;
    std::thread work_thread;
    std::promise<sv::Svar> result;
};

sv::Svar call_service(std::string name,sv::Svar args,sv::Svar config=sv::Svar()){
    return ServiceCaller(name,args,config).wait_result();
}

class ROSService{
public:
    ROSService(std::string name,sv::Svar func,sv::Svar config=sv::Svar())
        : config(config),topic(name),client(config.get<std::string>("url","localhost:9090")){
        sv::Svar msg=
        { {"op", "advertise_service"},
          {"service",topic}
        };

        if(config.exist("type")) msg["type"]=config["type"];

        client.on_open=[this,msg](std::shared_ptr<WsClient::Connection> connection){
            connection->send(msg.dump_json());
            this->connection=connection;
        };

        client.on_message=[this,func](std::shared_ptr<WsClient::Connection> connection, std::shared_ptr<WsClient::InMessage> msg){
            auto res=sv::Svar::parse_json(msg->string());
            sv::Svar ret=func(res["args"]);
            sv::Svar response=
            { {"op", "service_response"},
              {"service",topic},
              {"result",true},
              {"values",ret},
              {"id",res["id"]}
            };

            LOG(INFO)<<response;
            connection->send(response.dump_json());
        };
        work_thread=std::thread([this](){client.start();});
    }

    ~ROSService(){
        sv::Svar msg={{ "op", "unadvertise_service"},
                      {"service",topic}};
        client.stop();
        work_thread.join();
    }

    std::string topic;
    sv::Svar config;
    WsClient client;
    std::shared_ptr<WsClient::Connection> connection;
    std::thread work_thread;
};

//class ROSBridgeService{
//public:
//    ROSBridgeService(sv::Svar config)
//        : subs(Svar::object()),pubs(Svar::object()),config(config){
//        std::string patten=config.get<std::string>("patten","@");

//        data["newpub"]=GSLAM::Messenger::instance().subscribe("messenger/newpub",0,[this](GSLAM::Publisher pub){
//            if(filter(pub.getTopic())) return;
//            reassignPublisher(pub,pub.getTopic());
//        });

//        data["newsub"]=GSLAM::Messenger::instance().subscribe("messenger/newsub",0,[this,patten](GSLAM::Subscriber sub){
//            if(filter(sub.getTopic())) return;

//            reassignSubscriber(sub.getTopic(),sub.impl_->queue_size_);
//        });
//    }

//    bool filter(std::string topic){
//        return true;
//    }

//    void reassignPublisher(GSLAM::Publisher pub, std::string topic){
//        std::shared_ptr<ROSPublisher> publisher=std::make_shared<ROSPublisher>(topic,config);
//        pub.impl_->pubFunc=[publisher,topic](const sv::Svar& msg){
//            publisher->publish(msg);
//        };
//    }

//    void reassignSubscriber(std::string topic,int queue_size){
//        if(subs.exist(topic)) return;
//        auto cfg=config.clone();
//        if(queue_size>0)
//            cfg["queue_length"]=queue_size;

//        subs[topic]=std::make_shared<ROSSubscriber>(topic,[topic](const sv::Svar& msg){
//            GSLAM::Messenger::instance().publish(topic,msg);
//        },cfg);
//    }

//    sv::Svar      subs,pubs,data,config;
//};

REGISTER_SVAR_MODULE(nsq){
    Class<ROSSubscriber>("ROSSubscriber")
            .unique_construct<std::string,Svar,Svar>()
            .unique_construct<std::string,Svar>();

    Class<ROSPublisher>("ROSPublisher")
            .unique_construct<std::string>()
            .unique_construct<std::string,Svar>()
            .def("publish",&ROSPublisher::publish);

    svar["call_service"] = call_service;

    Class<ROSService>("ROSService")
            .unique_construct<std::string,Svar,Svar>();

//    Class<ROSBridgeService>("ROSBridgeService")
//            .unique_construct<Svar>();

//    Class<Messenger>("Messenger")
//            .construct<>()
//            .def_static("instance",&Messenger::instance)
//            .def("getPublishers",&Messenger::getPublishers)
//            .def("getSubscribers",&Messenger::getSubscribers)
//            .def("introduction",&Messenger::introduction)
//            .def("advertise",[](Messenger msg,const std::string& topic,int queue_size){
//        return msg.advertise<sv::Svar>(topic,queue_size);
//    })
//    .def("subscribe",[](Messenger msger,
//         const std::string& topic, int queue_size,
//         const SvarFunction& callback){
//        return msger.subscribe(topic,queue_size,callback);
//    })
//    .def("publish",[](Messenger* msger,std::string topic,sv::Svar msg){return msger->publish(topic,msg);});

//    Class<Publisher>("Publisher")
//            .def("shutdown",&Publisher::shutdown)
//            .def("getTopic",&Publisher::getTopic)
//            .def("getTypeName",&Publisher::getTypeName)
//            .def("getNumSubscribers",&Publisher::getNumSubscribers)
//            .def("publish",[](Publisher* pubptr,sv::Svar msg){return pubptr->publish(msg);});

//    Class<Subscriber>("Subscriber")
//            .def("shutdown",&Subscriber::shutdown)
//            .def("getTopic",&Subscriber::getTopic)
//            .def("getTypeName",&Subscriber::getTypeName)
//            .def("getNumPublishers",&Subscriber::getNumPublishers);

//    auto global=sv::Registry::load("svar_messenger");
//    auto msg=global["messenger"];

//    if(!global["logger"].isUndefined())
//    {
//        GSLAM::getLogSinksGlobal()=global["logger"].as<std::shared_ptr<std::set<GSLAM::LogSink *> >>();
//    }
//    else
//        global["logger"]=GSLAM::getLogSinksGlobal();

//    if(msg.is<GSLAM::Messenger>()){
//        GSLAM::Messenger::instance()=msg.as<GSLAM::Messenger>();
//    }
//    svar["messenger"]=Messenger::instance();
}

EXPORT_SVAR_INSTANCE

//int main(){}
