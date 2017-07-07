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

config_path = "../src/service_config.json"

os.environ["VSOMEIP_CONFIGURATION"] = config_path

total_measurements = 1

cmd = " ../bin/service-example"
m_num = 1
measurement_output = "readings/measurement" + str(m_num) + ".csv"


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
    time.sleep(2)    
    end_script(proc, outfile)

# Find the timestamp for application start, config parsed, routing ready in each text file
def parse(fileno):
    start_time = 0
    parse_time = 0
    routing_time = 0
    reading = []

    with open("readings/"+ str(fileno) + ".txt") as f:
        for line in f:
            if(line.find("Parsed") != -1):
                # print line
                parse_time = re.search(r'\d\d\.\d{6}', line).group()
                reading.append(parse_time)
                # print "Parsed at: " + parse_time
            elif (line.find("Starting Service example") != -1):
                # print line
                start_time = re.search(r'\d\d\.\d{6}', line).group()
                reading.append(start_time)
                # print "Started at: " + start_time
            elif (line.find("Routing is Dhinchak!") != -1):
                # print line
                routing_time = re.search(r'\d\d\.\d{6}', line).group()
                # print "Routing ready at: " + routing_time
                reading.append(routing_time)

    # Additionally, add time for routing to be ready and config to be parsed in milliseconds
    routing_time = (float(routing_time) - float(start_time))*1000
    parse_time = (float(parse_time) - float(start_time))*1000

    reading.append(str(parse_time))
    reading.append(str(routing_time))

    with open(measurement_output, "a+") as f:

        writer = csv.writer(f, delimiter=',')
        writer.writerow(reading)
        # reading = str(start_time) + ", " + str(parse_time) + ", " + str(routing_time) + ";\n"
        # f.write(reading)

def main():
    if (len(sys.argv) < 2):
        print "Usage ./bootup_test.py <# of readings>"
        sys.exit(0)
    else:
        total_measurements = int(sys.argv[1])

    # Header of CSV file
    with open(measurement_output, "w") as f:        
            writer = csv.writer(f, delimiter=',')
            writer.writerow(["Start Timestamp", "Parse Config Timestamp", "Routing Ready Timestamp", "Parse Config time [ms]", "Routing ready time [ms]"])

    for i in range(total_measurements):
        print "Running measurement " + str(i)
        run_measurement(i)
        time.sleep(0.05)
        parse(i)
        time.sleep(0.05)   

    sys.exit(0)

 
if __name__=="__main__":
    main()
