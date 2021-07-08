#! /usr/bin/python3
import svar
import time
import argparse
import json

ros=svar.load('svar_rosbridge')

res=ros.call_service('/setbool',True,{})
print(res)
