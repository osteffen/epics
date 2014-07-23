#!/bin/bash -e

cd /opt/epics/iocs/cb-hv

exec /opt/epics/modules/streamdevice/bin/$EPICS_HOST_ARCH/streamApp cb-hv.ioc
