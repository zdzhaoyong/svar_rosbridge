#include "Svar.h"

auto ros=svar.import("svar_rosbridge");

int main(int argc,char** argv){
    svar.parseMain(argc,argv);

    std::cout<<"get_time:"<<ros["call_service"]("/rosapi/get_time",sv::Svar::object(),sv::Svar())<<std::endl;

    std::cout<<"/rosversion:"<<ros["call_service"]("/rosapi/get_param",sv::Svar({"name","/rosversion"}),sv::Svar())<<std::endl;
    return 0;
}
