#Name : Suchintan Pati
#Roll no. : 18CS10064
#Please use : python3 parse.py http.xml
# takes around 5-10 min to execute
import sys
import csv
from ip2geotools.databases.noncommercial import DbIpCity #pip3 install ip2geotools
import pycountry #pip3 install pycountry
import xml.etree.ElementTree as ET 
tree = ET.parse(str(sys.argv[1])) #filename as argument
root = tree.getroot()
d={}
iplist=[]
for elem in root.findall("./packet/proto/field/[@name='http.x_forwarded_for']"):#find all the elements that have the attribute 'http.x_forwarded_for'
    ip=str(elem.attrib['show'])#ip address of the user is given by the attribute "show"
    if ip in iplist:# check if the ip already exists (i.e we need number of users rather than number of requests)
        continue
    iplist.append(ip)
    response = DbIpCity.get(ip, api_key='free') #get details about the location of the ip address
    temp=response.country # country code
    c=pycountry.countries.get(alpha_2=temp) #c.name gives the country name
    try:                          # dictionary d stores the frequency for each country i.e number of users from each country
        d[c.name]+=1
    except:
        d[c.name]=1
with open('data.csv', 'w') as f: #storing the dictionary d values in 'data.csv' file
    f.write("%s,%s\n"%('Country name','Number of users'))
    for key in d.keys():
        f.write("%s,%s\n"%(key,d[key]))

