#!/bin/bash -e

exec /opt/epics/modules/streamdevice/bin/$EPICS_HOST_ARCH/streamApp mwpc-hv.ioc
