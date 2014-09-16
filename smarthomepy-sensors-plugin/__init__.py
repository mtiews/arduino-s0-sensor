#!/usr/bin/env python
#########################################################################
# Copyright 2014 Marcel Tiews marcel.tiews@gmail.com
#########################################################################
# Helios-Plugin for SmartHome.py. http://mknx.github.io/smarthome/
#
# This plugin is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This plugin is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this plugin. If not, see <http://www.gnu.org/licenses/>.
#########################################################################

import sys
import serial
import logging
import socket
import threading
import struct
import time
import datetime
import array

logger = logging.getLogger("")

# Sensor-Schema
# [Type]#[ID]#[Key1]:[Val1]#[Key2]:[Val2]#...


class SensorsException(Exception):
    pass


class SensorsBase():

    def __init__(self, tty='/dev/ttyUSB0'):
        self._tty = tty
        self._is_connected = False
        self._port = False
        self._lock = threading.Lock()
     
    def connect(self):
        if self._is_connected and self._port:
            return True
            
        try:
            logger.debug("Sensors: Connecting...")
            self._port = serial.Serial(
                self._tty, 
                baudrate=115200, 
                bytesize=serial.EIGHTBITS, 
                parity=serial.PARITY_NONE, 
                stopbits=serial.STOPBITS_ONE, 
                timeout=5)
            self._is_connected = True
            return True
        except:
            logger.error("Sensors: Could not open {0}.".format(self._tty))
            return False
        
    def disconnect(self):
        if self._is_connected and self._port:
            logger.debug("SensorsBase: Disconnecting...")
            self._port.close()
            self._is_connected = False
    
    def getdata(self):
        #self.connect()
        line = str(self._port.readline(), encoding='ascii')
        if "#" not in line:
            return
        parts = line.split("#")
        if len(parts) < 3:
            return
        sensortype = parts[0]
        sensorid = parts[1]
        for part in parts[2:]:
            subparts = part.split(":")
            if len(subparts) == 2:
                key, val = subparts
                yield sensortype, sensorid, key, val
                
    
class Sensors(SensorsBase): 
    _items = {}
    
    def __init__(self, smarthome, tty):
        SensorsBase.__init__(self, tty)
        self._sh = smarthome
        self._alive = False
        
    def run(self):
        self.connect()
        self._alive = True
        while self._alive:
            for sensortype, sensorid, key, value in self.getdata():
                sid = sensorid + "/" + key
                if sid in self._items:
                    self._items[sid](value,"Sensors")

    def stop(self):
        self.disconnect()
        self._alive = False

    def parse_item(self, item):
        if 'sensors_var' in item.conf:
            varname = item.conf['sensors_var']
            self._items[varname] = item
        return None
    
    def update_item(self, item, caller=None, source=None, dest=None):
        pass
        
def main():
    import argparse 
    
    parser = argparse.ArgumentParser(
    description="Arduino Sensor Network command line interface.",
    epilog="Without arguments all readable values using default tty will be retrieved.",
    argument_default=argparse.SUPPRESS)
    parser.add_argument("-t", "--tty", dest="port", default="/dev/ttyUSB0", help="Serial device to use")
    parser.add_argument("-d", "--debug", dest="enable_debug", action="store_true", help="Prints debug statements.")
    args = vars(parser.parse_args())
 
    
    logger.setLevel(logging.DEBUG)
    ch = logging.StreamHandler()
    if "enable_debug" in args.keys():
        ch.setLevel(logging.DEBUG)
    else:
        ch.setLevel(logging.INFO)
    formatter = logging.Formatter("%(asctime)s - %(name)s - %(levelname)s - %(message)s")
    ch.setFormatter(formatter)
    logger.addHandler(ch)

    try:
        sensors = SensorsBase(args["port"])
        sensors.connect()
        if not sensors._is_connected:
            raise Exception("Not connected")
        
        while True:
            for sensortype, sensorid, key, value in sensors.getdata():
                print ("{0}/{1}/{2} -> {3}".format(sensortype,sensorid,key,value))
        
    except Exception as e:
        print("Exception: {0}".format(e))
        return 1
    finally:
        if sensors:
            sensors.disconnect()

if __name__ == "__main__":
    sys.exit(main())        
