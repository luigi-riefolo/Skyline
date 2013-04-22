#!/usr/bin/env python

# This script generates random datapoints(x,y,z,name) and write them to 
# /var/www/html/data.html. The number of datapoints to be generated is supplied
# through the first command line argument.

import time
import random
import string
import sys

from HTMLParser import HTMLParser

class MLStripper(HTMLParser):
    def __init__(self):
        self.reset()
        self.fed = []
    def handle_data(self, d):
        self.fed.append(d)
    def get_data(self):
        return ''.join(self.fed)

def strip_tags(html):
    s = MLStripper()
    s.feed(html)
    return s.get_data()

def generate_random_num():
	n = random.randint(1,100)
	return n 
	
def generate_name(size=6, chars=string.ascii_uppercase + string.digits):
	return ''.join(random.choice(chars) for x in range(size))
	
def generate_point():
	p="<p>"+str(generate_random_num())+","+str(generate_random_num())+","+str(generate_random_num())+","+generate_name()+"</p>\n\n"
	return p

def generate_stream(num):
	print "Initiating data stream\n"
	for i in range(0,int(num)):
		write_stream()
		time.sleep(4)

def initialize_page():
	f = open('/var/www/html/data.html','w')
	try:
		with f:
			f.write("<!DOCTYPE html>\n")
			f.write("<html>\n")
			f.write("<body>\n\n")
			f.write("</body>\n")
			f.write("</html>\n")
	except IOError:
		f.close()
	finally:
		print "Page intialized..."
		f.close()

def write_stream():
	try:
		with open('/var/www/html/data.html', 'r') as file:
			data = file.readlines()
			point = generate_point()
			data[len(data)-3] = point
			file.close()	
		with open('/var/www/html/data.html', 'w') as file:
			file.writelines(data)
			file.close()
			
	except IOError:
		file.close()
	finally:
		print "New data point " + strip_tags(point)
		file.close()
	

initialize_page()
generate_stream(sys.argv[1])
print "Done!"









