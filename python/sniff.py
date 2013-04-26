#! /usr/bin/env python
"""
Example to sniff all HTTP traffic on eth0 interface:
    sudo ./sniff.py eth0 "port 80"
"""

import sys
import pcap
import string
import time
import socket
import struct
import threading

protocols={socket.IPPROTO_TCP:'tcp',
            socket.IPPROTO_UDP:'udp',
            socket.IPPROTO_ICMP:'icmp'}

def decode_ip_packet(s):
    d={}
    d['version']=(ord(s[0]) & 0xf0) >> 4
    d['header_len']=ord(s[0]) & 0x0f
    d['tos']=ord(s[1])
    d['total_len']=socket.ntohs(struct.unpack('H',s[2:4])[0])
    d['id']=socket.ntohs(struct.unpack('H',s[4:6])[0])
    d['flags']=(ord(s[6]) & 0xe0) >> 5
    d['fragment_offset']=socket.ntohs(struct.unpack('H',s[6:8])[0] & 0x1f)
    d['ttl']=ord(s[8])
    d['protocol']=ord(s[9])
    d['checksum']=socket.ntohs(struct.unpack('H',s[10:12])[0])
    d['source_address']=pcap.ntoa(struct.unpack('i',s[12:16])[0])
    d['destination_address']=pcap.ntoa(struct.unpack('i',s[16:20])[0])
    if d['header_len']>5:
        d['options']=s[20:4*(d['header_len']-5)]
    else:
        d['options']=None
    d['data']=s[4*d['header_len']:]
    return d

class learn_ip_mac(threading.Thread):
    def __init__(self,dev):
        threading.Thread.__init__(self)
        self.dev=dev
    def run(self):
        p = pcap.pcapObject()
        #dev = pcap.lookupdev()
        #dev = sys.argv[1]
        while 1:
            for i in dev
                p.open_live(i, 1600, 0, 100)
                try:
                     while 1:
                         p.dispatch(1, print_arp_packet)
                         sleep(10)

            # specify 'None' to dump to dumpfile, assuming you have called
            # the dump_open method
            #    p.dispatch(0, None)

            # the loop method is another way of doing things
            #    p.loop(1, print_packet)

            # as is the next() method
            # p.next() returns a (pktlen, data, timestamp) tuple 
            #    apply(print_packet,p.next())
                except KeyboardInterrupt:
                    print '%s' % sys.exc_type
                    print 'shutting down'
                    print '%d packets received, %d packets dropped, %d packets dropped by interface' % p.stats()

def iterate(s):
    for d in s.keys():
        print '%s\n' %(d)
        print '%02x:%02x:%02x:%02x:%02x:%02x\n' %(ord(s[d][0]),ord(s[d][1]),ord(s[d][2]),ord(s[d][3]),ord(s[d][4]),ord(s[d][5]))

class print_ip_mac(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
    def run(self):
        while 1:
            time.sleep(60*10)
            iterate(ip_mac_dic);
def decode_arp_packet(s):
    d={}
    d['linktype']=socket.ntohs(struct.unpack('H',s[0:2])[0])
    d['mac']=s[8:14]#struct.unpack("HHH",s[8:14]))
    d['srcip']=pcap.ntoa(struct.unpack("i",s[14:18])[0])
    #print '%x %x %x %x %x %x\n' %(d['mac'][0],d['mac'][1],d['mac'][2],d['mac'][3],d['mac'][4],d['mac'][5])

    if d['srcip'] in ip_mac_dic.keys():
        if ip_mac_dic[d['srcip']]!=d['mac']:
            print 'new mac\n'
            ip_mac_dic[d['srcip']]=d['mac']
            print '%s\n' %(d['srcip'])
            print '%02x:%02x:%02x:%02x:%02x:%02x\n' %(ord(d['mac'][0]),ord(d['mac'][1]),ord(d['mac'][2]),ord(d['mac'][3]),ord(d['mac'][4]),ord(d['mac'][5]))
    else:
        ip_mac_dic[d['srcip']]=d['mac']
        print 'new srcip and mac'
        print '%s\n' %(d['srcip'])
        print '%02x:%02x:%02x:%02x:%02x:%02x\n' %(ord(d['mac'][0]),ord(d['mac'][1]),ord(d['mac'][2]),ord(d['mac'][3]),ord(d['mac'][4]),ord(d['mac'][5]))


def dumphex(s):
    bytes = map(lambda x: '%.2x' % x, map(ord, s))
    for i in xrange(0,len(bytes)/16):
        print '        %s' % string.join(bytes[i*16:(i+1)*16],' ')
    print '        %s' % string.join(bytes[(i+1)*16:],' ')
        
def print_arp_packet(pktlen, data, timestamp):
    if not data:
        return

    if data[12:14]=='\x08\x06':  # listen arp
        decoded=decode_arp_packet(data[14:])


def print_packet(pktlen, data, timestamp):
    if not data:
        return

    if data[12:14]=='\x08\x06':  # listen arp
        decoded=decode_arp_packet(data[14:])
        print '\n%s.%f %s > %s' % (time.strftime('%H:%M',
                                time.localtime(timestamp)),
                                timestamp % 60,
                                decoded['source_address'],
                                decoded['destination_address'])
        for key in ['version', 'header_len', 'tos', 'total_len', 'id',
                                'flags', 'fragment_offset', 'ttl']:
            print '    %s: %d' % (key, decoded[key])
        print '    protocol: %s' % protocols[decoded['protocol']]
        print '    header checksum: %d' % decoded['checksum']
        print '    data:'
        dumphex(decoded['data'])
 

if __name__=='__main__':

    if len(sys.argv) < 3:
        print 'usage: sniff.py <interface> <expr>'
        sys.exit(0)
    #p.dump_open('dumpfile')
    #p.setfilter(string.join(sys.argv[2:],' '), 0, 0)

    # try-except block to catch keyboard interrupt.    Failure to shut
    # down cleanly can result in the interface not being taken out of promisc.
    # mode
    #p.setnonblock(1)
    #tmp
  #  ip_mac_dic={}
  #  threads=[]
  #  threads.append(learn_ip_mac())
  #  threads.append(print_ip_mac())
  #  for t in threads:
  #      t.start()
  #  for t in threads:
  #      t.join()  
  #  
    dev=[]
    addrlist=pcap.findalldevs()
    for i in addrlist:
        print i[0]
        dev.append(i[0])
    for i in dev:
        print i
       
    #p = pcap.pcapObject()


  


# vim:set ts=4 sw=4 et:
