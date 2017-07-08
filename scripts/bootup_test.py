#!/usr/bin/python
 
# Launch service example, note down value of timestamp for Parsed config, (or time when app actually launched)
# Time when Parsed config/when applicaiton started/ time when Routing setup finished


import subprocess
import sys
import signal
import time
import os
import re
import csv

# application type client/service
app_type = "client"

# duration of application run before killing with SIGINT in seconds
app_duration = 4

config_path = "../src/" + app_type + "_config.json"

os.environ["VSOMEIP_CONFIGURATION"] = config_path

total_measurements = 1

cmd = " ../bin/" + app_type + "-example"
m_index = 1
measurement_output = "readings/measurement" + str(m_index) + ".csv"


# Start the application process and open a file for storing STDOUT and STDERROR
def start_script(fileno):
    outfile = open("readings/" + str(fileno) +".txt", 'w')    
    p = subprocess.Popen("exec " + cmd, stdout=outfile, stderr=outfile, shell=True)
    return p, outfile

# Send SIGINT (Ctrl+C) to end the application process
def end_script(p, outfile):
    p.send_signal(signal.SIGINT)
    outfile.close()

def run_measurement(index):
    proc, outfile = start_script(index)
    # Time to output all messages by daemon
    time.sleep(app_duration)    
    end_script(proc, outfile)

# Find the timestamp for application start and times for config parsed, routing ready, service discovery in each text file
def parse(fileno, fieldnames):
    fieldnames = ['start_timestamp', 'parse_time', 'routing_time', 'service_time']
    start_time = 0
    parse_time = 0
    routing_time = 0
    service_time = 0

    reading = {'start_timestamp': start_time,
                 'parse_time' : parse_time,
                 'routing_time': routing_time, 
                 'service_time': service_time
                 }

    with open("readings/"+ str(fileno) + ".txt") as f:
        for line in f:
            if(line.find("Parsed") != -1):
                # print line
                parse_time = re.search(r'\d\d\.\d{6}', line).group()                
                print "Parsed at: " + parse_time
            elif (line.find("Starting " + app_type + " example") != -1):
                # print line
                start_time = re.search(r'\d\d\.\d{6}', line).group()
                reading['start_timestamp'] = start_time
                print "Started at: " + start_time
            elif (line.find("Routing is Dhinchak!") != -1):
                # print line
                routing_time = re.search(r'\d\d\.\d{6}', line).group()
                print "Routing ready at: " + routing_time
            elif (line.find("is available.") != -1):
                service_time = re.search(r'\d\d.\d{6}', line).group()
                print "Service discovered at: " + service_time
    
    if(not float(start_time)):
        print "Could not parse starting timestamp!"
    else:
        # Update values for time for parsing config, routing ready in milliseconds
        # TODO: Consider changing to loop if number of fields more
        routing_time = round((float(routing_time) - float(start_time))*1000, 3)
        parse_time = round((float(parse_time) - float(start_time))*1000, 3)

        reading['routing_time'] = routing_time
        reading['parse_time'] = parse_time

        # In case SD enabled and service discovery time to be measured        
        if(float(service_time)):
            service_time = round((float(service_time) - float(start_time))*1000, 3)
            reading['service_time'] = service_time
    
    with open(measurement_output, "ab+") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames, delimiter=',')
        writer.writerow(reading)        

def main():
    if (len(sys.argv) < 2):
        print "Usage ./bootup_test.py <# of readings>"
        sys.exit(0)
    else:
        total_measurements = int(sys.argv[1])

    # Header of CSV file
    with open(measurement_output, "w") as f:
            # storing application start timestamp and other event times relative to it in milliseconds
            fieldnames = ['start_timestamp', 'parse_time', 'routing_time', 'service_time'] 
            writer = csv.DictWriter(f, fieldnames=fieldnames, delimiter=',', restval="-1")
            writer.writeheader()
            # writer.writerow(["Start Timestamp","Parse Config time [ms]", "Routing ready time [ms]", "Service Discovery Time [ms]"])

    for i in range(total_measurements):
        print "Running measurement " + str(i)
        run_measurement(i)
        time.sleep(0.05)
        parse(i, fieldnames)
        time.sleep(0.05)   

    sys.exit(0)

 
if __name__=="__main__":
    main()
