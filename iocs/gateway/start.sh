#!/bin/bash -e

. /opt/epics/thisEPICS.sh

#IP to send relayed data off
SERVER_IP=slowcontrol.office.a2.kph

#Do not read pv data back from our own IP on the hall side
SOURCE_IP_IGNORE=slowcontrol.online.a2.kph

# Online network broadcast address
CLIENT_IP=10.32.167.255

echo "Starting EPICS Gateway."
# echo "See /var/log/epics_gateway/log for logs"
exec $EPICS_EXTENSIONS/bin/$EPICS_HOST_ARCH/gateway \
  -signore $SOURCE_IP_IGNORE \
  -sip $SERVER_IP \
  -cip $CLIENT_IP \
  -home /var/run/epics_gateway

# logging is disabled for now.
# messages go to the screen terminal
#  -log /var/log/epics_gateway/log \
