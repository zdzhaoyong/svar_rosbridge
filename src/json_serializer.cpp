#include "Svar.h"

struct json_serializer{
    sv::Svar load(sv::SvarBuffer buf){
        return sv::Svar::parse_json(std::string(buf.ptr(),buf.size()));
    }

    sv::SvarBuffer dump(sv::Svar o){
        std::string str=o.dump_json();
        return sv::SvarBuffer(str.data(),str.size()).clone();
    }
};

REGISTER_SVAR_MODULE(jsonbase64){
    sv::Class<json_serializer>()
            .def("load",&json_serializer::load)
            .def("dump",&json_serializer::dump);

    svar["serializers"]["json"]=json_serializer();
}
