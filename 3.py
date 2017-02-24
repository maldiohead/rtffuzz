
import os
import sys
import random
import json
from random import choice


rtf="{\\rtf1"
oo="{\\*"
pp="}"


def  genrandomtagindex(len):
	index=[]
	for i in range(0,len-1):
		index.append(random.randint(0,1700))
	return index

def genstarttagitem(obj,target,pp):
	group=[]
	i=0
	while i< len(pp):
		if  obj==str(pp[i]["description"])and target!=i:
			cr=str(pp[i]["controlwd"])
			indexN=cr.find("N")
			if indexN!=-1:
				value=genrandomvalue(1000)
				cr=cr[:indexN]+str(value)
			group.append(cr)
		i=i+1
	return group

def getgrouptag(returntag):
	count=random.randint(len(returntag)/2,len(returntag))
	slice1=random.sample(returntag,count)
	tag=""
	j=0
	while(len(slice1)>j):
		tag=tag+slice1[j]
		j=j+1
	return tag



def genrandomvalue(max):
	return random.randint(0,max)

def genrtf(obj):
	pp=json.loads(obj)
	index=genrandomtagindex(10)
	item=""
	rtf="{\\rtf1"
	oo="{\\*"
	returntag=[]
	selfgroup=""
	filename="filename"
	for i in index:
		cr=str(pp[i]["controlwd"])
		filename=cr
		indexN=cr.find("N")
		if indexN!=-1:
			value=genrandomvalue(1000)
			cr=cr[:indexN]+str(value)
		if pp[i]["type"]=="Destination":
			cr=oo+str(pp[i]["controlwd"])
			returntag=genstarttagitem(pp[i]["description"],i,pp)
			selfgroup=getgrouptag(returntag)
			cr=cr+selfgroup+"}"
		item=cr+item
	#item=item+"}"
	rtf=rtf+item+"}"
	value=random.randint(0,0x20000)
	name="F:\\research\\rtffuzzer\\rtf\\"+filename+"%s"%value+".rtf"
	print name
	fp=open(name,"wb")
	fp.write(rtf)
	fp.close()


			




	
if __name__ == '__main__':
	fp=open("rtfnew.json","rb")
	obj=fp.read()
	for i in range(0,0x2000):
		genrtf(obj)








