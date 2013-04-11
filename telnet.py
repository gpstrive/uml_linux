#!/usr/bin/python
# coding=gbk
import telnetlib
import time

#ip = raw_input("IP: ")
#port = raw_input("Port: ")
ip = "10.24.28.150"
port = 23

enter_key = "\r\n"

account = "admin"
password = "admin"

command = "display ip routing-table"

delay_sec = 30

t = telnetlib.Telnet()
t.set_debuglevel(5)
t.open(ip, port)

ret = t.read_until("Username:", 5)
print ret

t.write(account + enter_key)
ret = t.read_until("Password:", 5)

t.write(password + enter_key)

ret = t.read_until("<Quidway>",5)
print ret

cur_time = time.localtime()
log_file_name = "monitor_" + str(cur_time[0]) + "_" + \
                             str(cur_time[1]) + "_" + \
                             str(cur_time[2]) + "_" + \
                             str(cur_time[3]) + "_" + \
                             str(cur_time[4]) + "_" + \
                             str(cur_time[5]) + ".log"

log_file = open(log_file_name, "w")

#while (1) :
t.write(command + enter_key)
ret = t.read_until("<Quidway>",5)
print ret
log_file.write(ret)
log_file.flush()

log_file.close()
t.close()
