#/bin/bash -e

. /opt/epics/thisEPICS.sh

cd /opt/epics/iocs

screen -dm -S EPICS -c screenrc || echo "Error launching screen"

