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






