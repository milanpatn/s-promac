#!/usr/bin/env python

import re

fo = open("log1", "r")
patterns= ['8- ','1- ','3- ','2- ','5- ','7- ','4- ','6- ','9- ','0- ']

for pattern in patterns:
	extract = open(pattern+"hty.txt","w");
	for line in fo:
	    if re.match(pattern, line):
        	print >> extract , line,
	fo.seek(0,0)
	
