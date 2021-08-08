# svar_rosbridge

## Build and Install

Install rosbridge with apt:

```
sudo apt install ros-$ROS_DISTRO-rosbridge-server
```

Direct build and install with cmake:

```
mkdir build;cd build;cmake ..;sudo make install
```

## C++ Demo Usages

### Start rosbridge service

```
roslaunch rosbridge_server rosbridge_websocket.launch
```

### Simple publish and subscribe

This below is a little demo to publish and subscribe an integer message (also see [example/simples_pubsub.cpp](examples/simple_pubsub.cpp)):

```
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
```

Run the above application with: 

```
./simple_pubsub
```



### Subscribe sensor_msgs/Image with CBOR

For messages contains buffer such as sensor_msgs/Image, JSON has to encode the buffer with base64 or hex which is computing expensive and low efficiency. Use CBOR will accelerate the serialization and here below is a sample to subscribe Image and visualize with OpenCV:

```
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

```

Run the above application with: 

```
./vis_image -topic /camera/fisheye1/image_raw
```



## How ROS represent a message with JSON

Please see the detail information about a message type with rosmsg:

```
svar_rosbridge> rosmsg info std_msgs/Int32
Failed to load Python extension for LZ4 support. LZ4 compression will not be available.
int32 data

svar_rosbridge> rosmsg info sensor_msgs/Image
Failed to load Python extension for LZ4 support. LZ4 compression will not be available.
std_msgs/Header header
  uint32 seq
  time stamp
  string frame_id
uint32 height
uint32 width
string encoding
uint8 is_bigendian
uint32 step
uint8[] data
```

### Call Services

```
#include "Svar.h"

auto ros=svar.import("svar_rosbridge");

int main(int argc,char** argv){
    svar.parseMain(argc,argv);

    std::cout<<"get_time:"<<ros["call_service"]("/rosapi/get_time",sv::Svar::object(),sv::Svar())<<std::endl;

    std::cout<<"/rosversion:"<<ros["call_service"]("/rosapi/get_param",sv::Svar({"name","/rosversion"}),sv::Svar())<<std::endl;
    return 0;
}

```

## Python Demo Usages


### Please install Svar firstly:

```
pip3 install svar
```

### Subscribe the topic with rosbridge:

Use rosbridge_echo to replace rostopic, if the topic already exists, you don't need to input the type:

```
python3 rosbridge_echo.py -type 'std_msgs/Int32' -topic "/hello"
```

The source code of the above script is:

```
#! /usr/bin/python3
import svar
import time
import argparse

ros=svar.load('svar_rosbridge')

# 1. Argument parsing
parser = argparse.ArgumentParser()
parser.add_argument("-topic",default="",help="The topic name")
parser.add_argument("-url",default="localhost:9090",type=str,help="The rosbridge url")
parser.add_argument("-cbor",default=False,action='store_true',help="Enable cbor")
parser.add_argument("-type",default="",help="The payload type")
args = parser.parse_args()


def cbk(msg):
  print(msg)

config={"url":args.url}

if args.cbor:
  config["compression"]='cbor'

if len(args.type) > 0:
  config["type"]=args.type

sub = ros.ROSSubscriber(args.topic,cbk,config)

while True:
 time.sleep(1)

```

### Monitor the topic with rostopic:

```
rostopic echo /hello
```

### Publish a message through rosbridge:

Publish a JSON message through rosbridge, if the topic already exists, you don't need to input the type:
```
python3 rosbridge_pub.py -topic '/hello' -msg '{"data":20}' -type 'std_msgs/Int32'
```
The source code is :

```
#! /usr/bin/python3
import svar
import time
import argparse
import json

ros=svar.load('svar_rosbridge')

# 1. Argument parsing
parser = argparse.ArgumentParser()
parser.add_argument("-topic",default="",help="The topic name")
parser.add_argument("-url",default="localhost:9090",type=str,help="The rosbridge url")
parser.add_argument("-type",default="",help="The payload type")
parser.add_argument("-msg",default="{}",help="The json message")
args = parser.parse_args()

config={"url":args.url}

if len(args.type) > 0:
  config["type"]=args.type

pub = ros.ROSPublisher(args.topic,config)

pub.publish(json.loads(args.msg))
```

### Call a service

```
#! /usr/bin/python3
import svar

ros=svar.load('svar_rosbridge')

res=ros.call_service('/rosapi/get_time',[],{})
print(res)
```

```
python3 rosbridge_service_call.py
```

### Advertise a service

Advertise a service through rosbridge:

```
python3 rosbridge_service.py
```






