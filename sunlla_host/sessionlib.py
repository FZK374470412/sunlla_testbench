#!/usr/bin/python

import socket
import urllib
import time
import os
import sys
import xml.etree.ElementTree as xmlparser
import __main__, ntpath

socket_timeout  = 5   # socket connection timeout, in seconds.
connect_num = 5

class create_session(object):
    def __init__(self):
        self.ip = None # store remote target ip
        self.port = None # store remote target port
        self.socket = None
        self.client = None
 
        ip = os.getenv('SUNLLA_IPADDR') # connect remote target ip
        if None == ip:
            ip = 'localhost'
        port = os.getenv('SUNLLA_PORT') # connect remote target port
        if None == port:
            port = 8888

        for i in range(connect_num):
            try:
            	print ('Connecting target %s %s' %(ip, port))
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.settimeout(socket_timeout)
                sock.connect((ip, port))
                self.socket = sock
    
                # when excute here means connected success
                self.client = create_client(self.socket)
                resp = self.client.get_status()

                status = resp.get('status')
                pid = resp.get('pid')
                if status != 'ok':
                    self.socket.close()                    

            except Exception, e:
                print e
                print 'try %d connect....' %(i+1)
                time.sleep(1)
                self.socket = None
            else:
                print 'connect target success'
                self.ip = ip
                self.port = int(port)				
                break
        if self.socket == None:
            print 'connect target failed'
        
    def close_socket(self):
        self.socket.close()
        self.socket = None
     
    def run_method_case(self, method):
        if self.client == None:
            print 'run_method_case() fail since client not exist'
            return 'FAIL'
        else:
            print ('run_method_case :%s' %(method['cmd']))

        return  self.client.send_method(method)  
		
class create_client(object):
    TRANS_RESP       = 0
    TRANS_XML        = 1
    TRANS_EXIT       = 2

    def __init__(self, socket):
        self.socket = socket

    def __check_crc(self, chek_org, arr):
        assert(isinstance(chek_org, int))
        assert(isinstance(arr, bytearray))
        for b in arr:
            chek_org ^= b
        return chek_org & 0xFF

    def __seq_to_u32(self, arr):
        assert(isinstance(arr, bytearray))
        assert(4 == len(arr))
        a = arr[0] << 24;
        b = arr[1] << 16
        c = arr[2] << 8
        d = arr[3]
        return a | b | c | d

    def __u32_to_seq(self, val):
        assert(isinstance(val, long) or isinstance(val, int))
        arr = bytearray(4)
        arr[0] = (val >> 24) & 0xFF
        arr[1] = (val >> 16) & 0xFF
        arr[2] = (val >> 8) & 0xFF
        arr[3] = val & 0xFF
        return arr

    def __str_to_arr(self, s):
        return bytearray(ord(c) for c in s);

    def __sock_send(self, data):
        if None == self.socket:
            return
        
        try:
            while len(data) > 0:
                d = self.socket.send(data)
                if None == d or 0 == d:
                    break
                data = data[d:]
        except Exception, e:
            print 'send data error:', e
 
    def __sock_recv(self, size):
        cur = 0
        data = ''

        if None == self.socket:
            return data
        
        try:
            while cur < size:
                d = self.socket.recv(size - cur)
                if None == d or len(d) < 1:
                    break
                cur += len(d)
                data += d
        except Exception, e:
            print 'receive data error:', e

        return data

    def __send_data(self, cmd_type, data):
        head = bytearray(5)
        crc = 0
        datalen = len(data)

        try:
            head[0] = cmd_type
            head[1:5] = self.__u32_to_seq(datalen)

            self.__sock_send(head)
            crc = self.__check_crc(crc, head)

            self.__sock_send(data)
            crc = self.__check_crc(crc, self.__str_to_arr(data))

            self.__sock_send(bytearray((crc,)))

            #check respond			
            crc = 0
            head = self.__sock_recv(5)
            assert(5 == len(head))
            head = self.__str_to_arr(head)
            datalen = self.__seq_to_u32(head[1:5])
            assert(datalen > 0)
            crc = self.__check_crc(crc, head)

            data = self.__sock_recv(datalen)
            assert(len(data) == datalen)
            crc = self.__check_crc(crc, self.__str_to_arr(data))

            crc2 = self.__sock_recv(1)
            assert(1 == len(crc2))
            assert(ord(crc2) == crc)
        except Exception, e:
            print '__send_data error ', e
            return None
        
        return data

    def __parse_result(self, xml):
        method = {}
        try:
            ret = xmlparser.fromstring(xml)
            for ele in ret.getchildren():
                key = ele.tag
                val = ele.text
                if method.has_key(key):
                    print 'the key = %s already exist' %key
                else:
                    method[key] = val

        except Exception, e:
            print 'parse xml error: ', e

        return method

    def __method_to_xml(self, method, xml):
        xml = xmlparser.Element(xml)
            
        for k,v in method.items():
            item = xmlparser.SubElement(xml, k)
            item.text = str(v)
       
        return xmlparser.tostring(xml)

    def send_method(self, args):
        if None == self.socket:
            print 'Invalid send_method'
            return {}

        data = self.__method_to_xml(args, 'test')
        data = self.__send_data(self.TRANS_XML, data)
        return self.__parse_result(data)


    def get_status(self):
        crc = 0
        head = self.__sock_recv(5)
        assert(5 == len(head))
        head = self.__str_to_arr(head)
        datalen = self.__seq_to_u32(head[1:5])
        assert(datalen > 0)
        crc = self.__check_crc(crc, head)

        data = self.__sock_recv(datalen)
        assert(len(data) == datalen)
        crc = self.__check_crc(crc, self.__str_to_arr(data))

        crc2 = self.__sock_recv(1)
        assert(1 == len(crc2))
        assert(ord(crc2) == crc)
        
        print 'recv', data
        return self.__parse_result(data)


#############################################################################
# interface called by user, parameter 'session' comes from create_session()
#############################################################################		
def exe_method(session, method):
    ''' This function excute one method like {'cmd':'I2C_OPEN', 'dev':1}
        reture  1: fail,  0: success
    '''
    if not method.has_key('cmd'):
            print "invalid method! must has 'cmd' tag "
            return 'FAIL'
    cmd_name = method['cmd']
    resp_method = session.run_method_case(method)
    return resp_method.get('status') != 'ok'

def exe_method_resp(session, method):
    ''' This function excute one method like {'cmd':'I2C_OPEN', 'dev':1}
        will return status and parameters come from server 
        reture  1: fail,  0: success
    '''
    if not method.has_key('cmd'):
            print "invalid method! must has 'cmd' tag "
            return 'FAIL'
    cmd_name = method['cmd']
    resp_method = session.run_method_case(method)
    return resp_method.get('status') != 'ok', resp_method

def exe_method_list(session, method_list):
    errs = []
    for method in method_list:
        if exe_method(session, method):
            errs.append(1)
            print 'method -> cmd = %s error' % method['cmd']
    return 1 if any(errs) else 0

def exe_method_done(session, errs):
	assert(isinstance(errs, list))
	result = 'FAIL' if any(errs) else 'PASS'
	name = ntpath.basename(__main__.__file__)
	print 'script %s : %s' % (name, result)
	session.close_socket()
