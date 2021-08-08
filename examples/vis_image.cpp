#include "Svar.h"
#include "opencv2/highgui/highgui.hpp"

auto ros=svar.import("svar_rosbridge");

void callback(sv::Svar msg){
    auto stamps= msg["header"]["stamp"];
    int width=msg["width"].as<int>();
    int height=msg["height"].as<int>();
    std::string encoding=msg["encoding"].as<std::string>();
    sv::SvarBuffer buf=msg["data"].as<sv::SvarBuffer>();

    int type=CV_8UC1;
    if(encoding=="mono8"){
        type=CV_8UC1;
    }
    else if(encoding=="bgr8"){
        type=CV_8UC3;
    }
    else {
        std::cerr<<"not support "<<encoding<<std::endl;
        return;
    }

    cv::Mat img(height,width,type,buf.ptr<uchar>());
    cv::imshow("image",img);
    cv::waitKey(10);
}

int main(int argc,char** argv){
    svar.parseMain(argc,argv);

    std::string topic = svar.arg<std::string>("topic","/example","The topic name");
    std::string url   = svar.arg<std::string>("url","127.0.0.1:9090","The rosbridge server address");

    sv::Svar config={{"url",url},{"compression","cbor"}};// type is optional when topic exists
    sv::Svar sub=ros["ROSSubscriber"](topic,callback,config);
    while(true) usleep(100000);
    return 0;
}
