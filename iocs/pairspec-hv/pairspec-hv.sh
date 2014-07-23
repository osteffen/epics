#!/bin/bash -e

cd /opt/epics/iocs/pairspec-hv/

exec /opt/epics/modules/streamdevice/bin/$EPICS_HOST_ARCH/streamApp pairspec-hv.ioc
