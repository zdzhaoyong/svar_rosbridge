# svar_rosbridge


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

### Moniter the topic with rostopic:

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






