#!/usr/bin/env python3
# coding: utf-8


from __future__ import print_function
#import warnings; warnings.simplefilter('ignore')
import os
import sys
sys.path.append("/workspace/include")
import pcl_helper


import rospy
from sensor_msgs.msg import PointCloud2
import sensor_msgs.point_cloud2 as pc2

import cv2

import pcl
import pcl_msg

import matplotlib.pyplot as plt
from time import sleep

import struct
import sys
import time
import numpy as np
import serial

import math

def rect(r, theta):
    """theta in degrees

    returns tuple; (float, float); (x,y)
    """
    x = r * math.cos(theta) #math.cos(math.radians(theta))
    y = r * math.sin(theta) #math.sin(math.radians(theta))
    return x,y

def polar(x, y):
    """returns r, theta(degrees)
    """
    r = (x ** 2 + y ** 2) ** .5
    if y == 0:
        theta = 180 if x < 0 else 0
    elif x == 0:
        theta = 90 if y > 0 else 270
    else:
        theta = math.degrees(math.atan(float(y) / x))
    return r, theta


def validateChecksum(header):
    #h = typecast(uint8(header),'uint16');
    header = np.uint8(header)
    h = np.array(header, dtype=np.uint16)
    #a = uint32(sum(h));
    a = sum(h)
    a = np.uint32(a)
    #b = uint16(sum(typecast(a,'uint16')));
    b = np.array(a, dtype=np.uint16)
    b = sum(b)
    b = np.uint16(b)
    #CS = uint16(bitcmp(b));
    CS = ~b
    CS = np.uint16(CS)
    return CS




def parsePoint(data, tlvLength):    # Type : x06
    point_data = np.zeros([tlvLength/struct.calcsize('4f'),3],dtype=np.float32)
    for i in range(tlvLength/struct.calcsize('4f')):
        if(struct.calcsize('4f') == len(data[16*i:16*i+16])):
            parse_range, parse_angle, parse_doppler, parse_snr = struct.unpack('4f', data[16*i:16*i+16])  #struct.unpack('3H3h', data[4+12*i:4+12*i+12]) # heder 4, payload 12
            #print("Point : {}, {}".format(parse_range, parse_angle))
            x,y = rect(parse_range, parse_angle)
            datas = np.array([x,y,0])
            point_data[i,]=datas
            
            #np.append(point_data, datas)
            #point_data = np.vstack((point_data,datas))
            #plt.scatter(x, y)            
            #plt.show()
            #plt.pause(0.0001) #Note this correction

        else:
            #print("** parsePoint Size [{}]**".format(len(data[16*i:16*i+16])))
            pass
    
    pc = pcl.PointCloud(point_data)
    pcl_xyzrgb = pcl_helper.XYZ_to_XYZRGB(pc, [255,255,255])  
    out_ros_msg = pcl_helper.pcl_to_ros(pcl_xyzrgb)
    pub_point.publish(out_ros_msg)

def parseTargetIndex(data, tlvLength):    # Type : x08   
    index_data = np.zeros([tlvLength/struct.calcsize('B'),3],dtype=np.float32)  # 'I16f' for little endian , '>I16f' for big endian 
    for i in range(tlvLength/struct.calcsize('B')):
        if(struct.calcsize('B') == len(data[1*i:1*i+1])):
            targetID  = struct.unpack('B', data[1*i:1*i+1])  #76, 144, 212, 280
            print("TargetID : {}".format(targetID))
            datas = np.array([targetID,0,0])
            index_data[i,]=datas
            #np.append(taget_data, datas)
            #point_data = np.vstack((taget_data,datas))

            
        else:
            #print("** parseTargetIndex Size [{}]**".format(len(data[1*i:1*i+1])))
            pass



def parseTargetList(data, tlvLength):    # Type : x07
    target_data = np.zeros([tlvLength/struct.calcsize('I16f'),3],dtype=np.float32)  # 'I16f' for little endian , '>I16f' for big endian    
    for i in range(tlvLength/struct.calcsize('I16f')):
        if(struct.calcsize('I16f') == len(data[68*i:68*i+68])):
            tid, posX, posY, velX, velY, accX, accY, EC1, EC2,EC3,EC4,EC5,EC6,EC7,EC8,EC9, g  = struct.unpack('I16f', data[68*i:68*i+68])  #76, 144, 212, 280
            #print("Detect[{}] : {}, {}".format(tid, posX, posY))
            print("Detect[{}] : {}, {}".format(tid, abs((int(posX*100))), abs(int(posY*100))))
            datas = np.array([posX,posY,0])
            target_data[i,]=datas
            #np.append(taget_data, datas)
            #point_data = np.vstack((taget_data,datas))
            
            #plt.scatter(posX, posY)            
            #plt.show()
            #plt.pause(0.0001) #Note this correction

        else:
            #print("** parseTargetList Size[{}]**".format(len(data[68*i:68*i+68])))
            pass
    
    pc = pcl.PointCloud(target_data)
    pcl_xyzrgb = pcl_helper.XYZ_to_XYZRGB(pc, [255,255,255])  
    out_ros_msg = pcl_helper.pcl_to_ros(pcl_xyzrgb)
    pub_track.publish(out_ros_msg)

def tlvHeader(data):
    while data:       
        headerLength = 52
        try:
            magic, version, platform, cpuCycles, length, frameNum, subframeNumber, chirpMargin, frameMargin, uartSentTime, trackProcessTime, numTLVs, checksum = struct.unpack('Q10I2H', data[:headerLength])        
            #print("numTLVs : ", numTLVs)
            #print("length : ", length-52) #exclude header length 52
            
        except:
            break
        
        """checksum
        header = struct.unpack('52c', data[:headerLength])
        cs = validateChecksum(header)
        print (" checksum : {} == {}".format(cs, checksum))
        """

        pendingBytes = length - headerLength
        data = data[headerLength:]

        for i in range(numTLVs):
            if (len(data) != 0): 
                tlvType, tlvLength = tlvHeaderDecode(data[:8])                               
                data = data[8:]  # after TLV header 
                if (tlvType == 6):
                    parsePoint(data, tlvLength-8)
                elif (tlvType == 7):
                    parseTargetList(data, tlvLength-8)               
                elif (tlvType == 8): 
                    parseTargetIndex(data, tlvLength-8) 
                else:                    
                    print("- Unidentified tlv type :", hex(tlvType))  # this line print 0x02, 0x04
                data = data[tlvLength:]
                pendingBytes -= (8+tlvLength)
        data = data[pendingBytes:]
        yield length, frameNum

def tlvHeaderDecode(data):    
    if(struct.calcsize('2I') == len(data)):
        tlvType, tlvLength = struct.unpack('2I', data)
        return tlvType, tlvLength
    else:
        return 0,0

def read(self, size=1):
    """Read size bytes from the serial port. If a timeout is set it may
    return less characters as requested. With no timeout it will block
    until the requested number of bytes is read."""
    if not self.fd: raise portNotOpenError
    read = ''
    inp = None
    if size > 0:
        while len(read) < size:
            #print "\tread(): size",size, "have", len(read)    #debug
            ready,_,_ = select.select([self.fd],[],[], self.timeout)
            if not ready:
                break   #timeout
            buf = os.read(self.fd, size-len(read))
            read = read + buf
            if self.timeout >= 0 and not buf:
                break  #early abort on timeout
    return read

if __name__ == "__main__":
    
    rospy.init_node('radar', anonymous=True)
    pub_point = rospy.Publisher("/radar_point", PointCloud2, queue_size=1)
    pub_track = rospy.Publisher("/radar_track", PointCloud2, queue_size=1)
    
    fileName = "/dev/ttyACM1"

    if sys.byteorder == "little":
        print("Little-endian platform.")
    else:
        print("Big-endian platform.")

    #plt.ion() ## Note this correction
    #fig=plt.figure()
    #plt.axis([-10,10,-10,10])


    token = b'\x02\x01\x04\x03\x06\x05\x08\x07'

    rawDataFile = open(fileName, "rb")
    print("Read Data From : ", fileName)    
    while(1):
        #sleep(0.5)
        #fig.clear()
        rawData = rawDataFile.read()   
        
        magic = b'\x02\x01\x04\x03\x06\x05\x08\x07'
        offset = rawData.find(magic)
        rawData = rawData[offset:]
        for length, frameNum in tlvHeader(rawData):
		    continue
        




