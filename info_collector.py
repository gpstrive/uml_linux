# -*- coding:utf-8 -*- 
#!/usr/bin/python
'''
功能:远程到指定的设备收集温度，内存，引擎状态信息，写入csv文件
csv文件命名：ip_info.csv
使用注意：运行此脚本时，要查看生成的csv文件，请先拷贝到副件，再打开副件
egg.python info_collector.py 600
Created on 2012-05-22
@author: pengcong
'''
import time
import paramiko
import os
import sys
import time
import StringIO
import traceback
import socket
import subprocess
#PORT = 22
class InfoCollector():
    def __init__(self,freq):
       
        self.freq = freq  #收集信息的频率，单位为s
        self.info_list, self.last_record_filelist = self.read_device_info() #读取配置文件信息
        self.init_csv(self.info_list)  #初始化csv文件
        pass
    def read_device_info(self):
        """读取设备信息配置文件到列表"""
        file_handler = open('device.info','r')
        info_list = [line.split() for line in file_handler if line.strip() != ""]  #读取设备信息到列表
        last_record_filelist = {}
        for each in info_list:
           # print each[0]
            last_record_filelist[each[1]]= '/home/gaeng/watch/device.info' 
        file_handler.close()
        return info_list, last_record_filelist
    def init_csv(self,info_list):
        """判断监控的设备对应的csv文件是否已存在，若无，则新建"""
        for each in info_list:
            csv_name = '/home/gaeng/watch/devices/%s_info.csv' %(each[0])  
            path = os.path.join(sys.path[0],csv_name)
            if not os.path.isfile(path):
                file_handler = open(path,'w+')
                header = 'time,chip_temp,board_temp,free_mem,cfeapp_status\n'
                file_handler.write(header)
                file_handler.close()       
    def collector(self,info_list,last_record_filelist):
        """通过遍历info_list([ip,username,password],...)ssh到指定设备，获取信息，写入csv文件"""
        for device_info in info_list:
            
            (device_type,name,ip,port,username,password) = device_info
            port=int(port)
            print name, ip,port
            csv_name = '/home/gaeng/watch/devices/%s_info.csv' %(name)      
            time_str = time.strftime("%Y-%m-%d-%H-%M")
            dmg_name = '/home/gaeng/watch/devices/%s_%s.dmg' %(name, time_str)
                
            #创建ssh client
            ssh_client = paramiko.SSHClient()
            ssh_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
            try:
                ssh_client.connect(ip, port ,username, password,key_filename='/home/gaeng/.ssh/a.dsa',timeout=20)
                print "connected!"
            except Exception,e:
                print "can not connect to %s!" %ip
                fp = StringIO.StringIO()    #create object in memory
                traceback.print_exc(file=fp)
                msg = fp.getvalue()
                print msg
                continue
            if device_type == "000":
                #获取温度
                stdin,stdout,sterr = ssh_client.exec_command('cat /proc/environment')
                info = stdout.read().strip().split('\n')
                chip_temp = info[0].strip()[info[0].index(':')+2:]
                board_temp = info[1].strip()[info[1].index(':')+2:]
                #获取内存
                stdin,stdout,sterr = ssh_client.exec_command('free|grep Mem')
                free_mem = stdout.read().strip().split()[3]
                #获取引擎状态
                stdin,stdout,sterr = ssh_client.exec_command('cat /tmp/live_status.conf |grep "RUNNING"')
                info = stdout.read().strip()
                if info == "":
                    cfeapp_status = "DOWN"
                else:
                    cfeapp_status = "UP"

                stdin,stdout,sterr = ssh_client.exec_command('uptime');
                uptime=stdout.read()
                tmp="%s\n" %(uptime)
                print tmp
                stdin,stdout,sterr = ssh_client.exec_command('dmesg ')
                dmesg=stdout.read()
                ssh_client.close()    

                #写入csv
                time_str = time.strftime("%Y-%m-%d %H:%M:%S")
                file_handler = open(csv_name,'a+')
                temp = "%s,%s,%s,%s,%s\n" %(time_str,chip_temp,board_temp,free_mem,cfeapp_status)
                print temp
                file_handler.write(temp)
                file_handler.close()
                if dmesg != "" :
                    file_handler = open(dmg_name,'a+')
                    file_handler.write("%s" %dmesg)
                    file_handler.close()
                    #command="diff %s %s" %(last_record_filelist[name], dmg_name)
                    #command="\"/usr/bin/diff\" \"/home/gaopeng/watch/device.info\" \"/home/gaopeng/watch/devices/107_info.csv\""
                    #print command
                    std=subprocess.Popen(['diff',last_record_filelist[name],dmg_name],stdout=subprocess.PIPE,stderr=subprocess.PIPE)
                    #stdin,stdout,sterr = os.system(command)
                    result=std.stdout.read()
                    print result 


                    if result == "":
                        print "result is null"
                        os.system("rm %s" %dmg_name)
                    else:
                        print "result is not null"
                        last_record_filelist[name]=dmg_name
            elif device_type == "DS-M" or device_type == "A":
                stdin,stdout,stderr=ssh_client.exec_command(' top -b -n 1 | grep -E "Cpu|Mem" ')
                cpu_mem=stdout.read()
                stdin,stdout,stderr=ssh_client.exec_command(' df ')
                disk_usage = stdout.read()
                ssh_client.close()    
                file_handler = open(csv_name,'a+')
                time_str = time.strftime("%Y-%m-%d %H:%M:%S")
                temp = "%s,%s,%s\n" %(time_str,cpu_mem,disk_usage)
                print temp
                file_handler.write(temp)
                file_handler.close()
            elif device_type == "000":
                stdin,stdout,sterr = ssh_client.exec_command('free|grep Mem')
                free_mem = stdout.read().strip().split()[3]
                #获取引擎状态
                stdin,stdout,sterr = ssh_client.exec_command('cat /tmp/live_status.conf |grep "RUNNING"')
                info = stdout.read().strip()
                if info == "":
                    cfeapp_status = "DOWN"
                else:
                    cfeapp_status = "UP"
                stdin,stdout,sterr = ssh_client.exec_command('dmesg ')
                dmesg=stdout.read()
                ssh_client.close()    

                #写入csv
                time_str = time.strftime("%Y-%m-%d %H:%M:%S")
                file_handler = open(csv_name,'a+')
                temp = "%s,%s,%s\n" %(time_str,free_mem,cfeapp_status)
                print temp
                file_handler.write(temp)
                file_handler.close()
                if dmesg != "" :
                    file_handler = open(dmg_name,'a+')
                    file_handler.write("%s" %dmesg)
                    file_handler.close()
                    #command="diff %s %s" %(last_record_filelist[name], dmg_name)
                    #command="\"/usr/bin/diff\" \"/home/gaopeng/watch/device.info\" \"/home/gaopeng/watch/devices/107_info.csv\""
                    #print command
                    std=subprocess.Popen(['diff',last_record_filelist[name],dmg_name],stdout=subprocess.PIPE,stderr=subprocess.PIPE)
                    #stdin,stdout,sterr = os.system(command)
                    result=std.stdout.read()
                    print result 


                    if result == "":
                        print "result is null"
                        os.system("rm %s" %dmg_name)
                    else:
                        print "result is not null"
                        last_record_filelist[name]=dmg_name






        
    def main(self):
        first_flag=0
        while True:
            now = time.time()
            self.collector(self.info_list,self.last_record_filelist)
            time.sleep(self.freq - now % self.freq)
          
if __name__ == "__main__":
    
    if len(sys.argv) != 2 :
        print "Usage:info_collector.py seconds" 
        sys.exit(-1)
    (seconds,) = sys.argv[1:]
    InfoCollector(int(seconds)).main()

    
