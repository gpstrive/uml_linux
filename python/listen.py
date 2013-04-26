#!/usr/bin/python 
# -*- coding: utf-8 -*-
import pcap
import struct
import sys
import os
'''
传入参数：32位的ip地址的二进制形式 
返回类型：ip地址的字符串形式（点格式）
''' 
def ip_ntoa(c_4):
    list = []
    for i in struct.unpack('cccc',c_4):
        list.append(str(ord(i)))
    return '.'.join(list)
'''
传入参数：端口的二进制形式(16位)
返回类型：int类型
'''
def port_ntoa(c_2):
    i=(ord(c_2[0])<<8)+ord(c_2[i])
    return str(i)
'''
def process_packet(pktlen,data,timestamp):
    offset=14
    src_ip=ip_ntoa(data[offset+12:offset+16])
    dst_ip=ip_ntoa(data[offset+16:offset+20])
    ip_len=ord(data[offset])&0x0f
    offset+=ip_len*4
    src_port=port_ntoa(data[offset:offset+2])
    dst_port=port_ntoa(data[offset+2:offset+4])
    tcp_len=ord(data[offset+12])<<4
    offset = tcp_len*4
    global 
'''
if __name__ == "__main__":
    if len(sys.argv) < 2:
        print 'usage: listen.py <interface>'
        sys.exit(0)
    p=pcap.pcapObject()
    dev = sys.argv[1]
    net,mask = pcap.lookupnet(dev)
    p.open_live(dev,1600,0,100)
    p.setfilter(
