# executable path is below...
# ../../bin/linux-x86/snmp

< envPaths
cd "${TOP}"
epicsEnvSet("MIBDIRS", "+$(TOP)/mibs")
epicsEnvSet("W", "WIENER-CRATE-MIB::")

dbLoadDatabase("dbd/snmp.dbd")
snmp_registerRecordDeviceDriver(pdbbase)

#dbLoadRecords("db/wienerCrate.db","PREFIX=TRIG:CRATE,CRATE=crate-exptrigger")
dbLoadRecords("db/wienerCrate.db","PREFIX=TAPS:TRIG:CRATE,CRATE=crate-taps-trigger")
#dbLoadRecords("db/wienerCrate.db","PREFIX=TAPS:PWO:CRATE,CRATE=crate-taps-pwo")
#dbLoadRecords("db/wienerCrate.db","PREFIX=CB:CB:ADC_1:CRATE,CRATE=crate-cb-adc-1")
#dbLoadRecords("db/wienerCrate.db","PREFIX=CB:CB:ADC_2:CRATE,CRATE=crate-cb-adc-2")
#dbLoadRecords("db/wienerCrate.db","PREFIX=CB:MWPC:ADC:CRATE,CRATE=crate-mwpc-adc")
#dbLoadRecords("db/wienerCrate.db","PREFIX=BEAM:CRATE,CRATE=crate-beampolmon")
#dbLoadRecords("db/wienerCrate.db","PREFIX=TAGG:CRATE,CRATE=crate-tagger")
#dbLoadRecords("db/wienerCrate.db","PREFIX=CB:CB:TDC:CRATE,CRATE=crate-cb-tdc")

cd "${TOP}/iocBoot/${IOC}"
iocInit()
#dbl > "/tmp/ioc/snmp"

