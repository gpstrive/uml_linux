#!/usr/bin/env python
# -*- coding: utf-8 -*-
import telnetlib
import time
import getpass
import sys

enter_key="\n"
#user = raw_input("Enter your remote account: ")
user ="admin"
filename ="command.txt"
f =open(filename,'r')
lines = f.readlines()
def huawei_op():
    ip = raw_input("IP:")
    #password = raw_input("Password:")
    password = getpass.getpass()
    t = telnetlib.Telnet()
    t.open(ip)
    ret = t.read_until("Username:",5)
    t.write(user+enter_key)
    ret = t.read_until("Password:",5)
    t.write(password+enter_key)
    ret=t.read_until("<Quidway>",5)
    for command in lines:
        t.write(command+enter_key)
    #t.write("screen-length 0 tempor"+enter_key)
    #ret=t.read_until("<Quidway>",5)
    #t.write("dis current-configuration interface"+enter_key)
    #ret=t.read_until("<Quidway>",5)
    t.write("dis ip routing-table | include 20.20"+enter_key)
    t.write("quit"+enter_key)
    ret=t.read_until("<Quidway>",5)
    print ret
    t.close()

def cisco_op():
    ip = raw_input("IP:")
    #password = raw_input("Password:")
    password = getpass.getpass()
    t = telnetlib.Telnet()
    t.open(ip)
    #ret = t.read_until("login:",5)
    #t.write(user+enter_key)
    ret = t.read_until("Password:",5)
    print ret
    t.write(password+enter_key)
    ret=t.read_until("Cisco",5)
    t.write("enable"+enter_key)
    t.write(password+enter_key)
    ret=t.read_until("Cisco",5)
    t.write("terminal length 0"+enter_key)
    ret=t.read_until("Cisco",5)
    t.write("show int"+enter_key)
    ret=t.read_until("Cisco",5)
    print ret
    t.close()
def juniper_op():
    ip = raw_input("IP:")
    #password = raw_input("Password:")
    password = getpass.getpass()
    t = telnetlib.Telnet()
    t.open(ip)
    ret = t.read_until("login:",5)
    t.write(user+enter_key)
    ret = t.read_until("Password:",5)
    print ret
    t.write(password+enter_key)
    ret=t.read_until("admin",5)
    t.write("show int | no-more"+enter_key)
    ret=t.read_until("admin",5)
    print ret
    t.close()
def print_usage():
    print "%s [cisco | huawei]"%(sys.argv[0])

if __name__ == '__main__':
    if(len(sys.argv) != 2):
        print_usage()
        sys.exit(0)
    if(cmp(sys.argv[1].lower(),"cisco") == 0):
        cisco_op()
    elif(cmp(sys.argv[1].lower(),"huawei")==0):
        huawei_op()
    elif(cmp(sys.argv[1].lower(),"juniper")==0):
        juniper_op()
    else:
        print_usage()
    sys.exit(0)
