#include "Svar.h"

auto ros=svar.import("svar_rosbridge");

void callback(sv::Svar msg){
    std::cout<<"received "<<msg<<std::endl;
}

int main(int argc,char** argv){
    svar.parseMain(argc,argv);

    std::string topic = svar.arg<std::string>("topic","/example","The topic name");
    std::string url   = svar.arg<std::string>("url","127.0.0.1:9090","The rosbridge server address");

    sv::Svar config={{"url",url},{"type","std_msgs/Int32"}};
    sv::Svar sub=ros["ROSSubscriber"](topic,callback,config);
    sv::Svar pub=ros["ROSPublisher"](topic,config);

    for(int i=0;i<10;i++){
        sv::Svar msg={{"data",i}};
        pub.call("publish",msg);
        usleep(1000000);
    }
    return 0;
}
