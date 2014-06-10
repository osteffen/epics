#!/bin/bash -e

. /opt/epics/thisEPICS.sh

#IP to collent pv data from. This is the hall side
SOURCE_IP=10.32.168.174

#Do not read pv data back from our own IP on the hall side
SOURCE_IP_IGNORE=10.32.161.39

#Output dv data on this IP in the office net
CLIENT_IP=10.32.167.255

exec $EPICS_EXTENSIONS/bin/$EPICS_HOST_ARCH/gateway -sip $SOURCE_IP -signore $SOURCE_IP_IGNORE -cip $CLIENT_IP

