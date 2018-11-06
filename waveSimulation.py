import os
import sys
# Network Performances for IEEE 802.11p - loop on Application protocol DataRate


# Useful variables
initialRun = int(sys.argv[1])
finalRun = int(sys.argv[2])
# distance = [2, 5, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 115, 120, 125, 130, 135, 140, 145, 150, 160]
# intPck = [8e5, 8e3, 800,  80, 8] # appRate = [10kb/s, 1Mb/s, 10Mb/s, 100Mb/s, 1Gb/s]
distance = [50]
intPck = [8e5] # appRate = [10kb/s, 1Mb/s, 10Mb/s, 100Mb/s, 1Gb/s]

# Run the script for each distance and multiple run
for k in intPck:
	for j in distance:
		i=initialRun
		while(i <= finalRun):
			cmd = "./waf --run wave-udp-e2e --command-template=\" %s --RngRun=" + str(i) + " --RngSeed=" + str(i) + " --distance=" + str(j) + " --intPck=" +  str(k) + "\""
			returned_value = os.system(cmd)

			i += 1
