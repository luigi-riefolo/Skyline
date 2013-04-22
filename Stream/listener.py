#!/usr/bin/env python

# This script generates random datapoints(x,y,z,name) and write them to 
# /var/www/html/data.html. The number of datapoints to be generated is supplied
# through the first command line argument.

import time
import random
import string
import sys
import urllib2

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


htmlLen = 0

def open_URL():
	try:
		response = urllib2.urlopen('http://127.0.0.1/data.html')
		html = response.read()
		global htmlLen
		htmlLen = len(html)

		with open('~/Desktop/SkylineApp/stream.txt', 'w') as file:
			file.write(strip_tags(html[30:len(html)-17]))
	except IOError:
		file.close()
	finally:
		file.close()
	


def check_for_update():
	try:
		response = urllib2.urlopen('http://127.0.0.1/data.html')
		html = response.read()
		if htmlLen != len(html):
			try:
				print "New datapoint[s]"
				with open('~/Desktop/SkylineApp/stream.txt', 'w') as file:
					file.write(strip_tags(html[30:len(html)-17]))
			except IOError:
				file.close()
			finally:
				file.close()
		else:
			print "No updates"
		global htmlLen
		htmlLen = len(html)
			
	except (KeyboardInterrupt):
		print e.reason
	

open_URL()

while True:
	time.sleep(2)
	check_for_update()









