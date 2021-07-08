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





