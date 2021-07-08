#! /usr/bin/python3
import svar
import time

ros=svar.load('svar_rosbridge')

def setbool(args):
  print(args)
  return {"success":args["data"],"message":"hello"}

service=ros.ROSService('/setbool',setbool,{"type":"std_srvs/SetBool"})
time.sleep(1000)
