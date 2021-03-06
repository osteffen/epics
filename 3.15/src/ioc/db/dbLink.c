/*************************************************************************\
* Copyright (c) 2010 UChicago Argonne LLC, as Operator of Argonne
 *     National Laboratory.
 * Copyright (c) 2002 The Regents of the University of California, as
 *     Operator of Los Alamos National Laboratory.
 * EPICS BASE is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 \*************************************************************************/
/* dbLink.c */
/* $Id$ */
/*
 *      Original Authors: Bob Dalesio, Marty Kraimer
 *      Current Author: Andrew Johnson
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "dbDefs.h"
#include "epicsThread.h"
#include "errlog.h"
#include "cantProceed.h"
#include "cvtFast.h"
#include "epicsTime.h"
#include "alarm.h"
#include "ellLib.h"
#include "dbStaticLib.h"
#include "dbBase.h"
#include "link.h"
#include "dbFldTypes.h"
#include "recSup.h"
#include "devSup.h"
#include "caeventmask.h"
#include "db_field_log.h"
#include "dbCommon.h"
#include "dbFldTypes.h"
#include "special.h"
#include "errMdef.h"
#define epicsExportSharedSymbols
#include "dbAddr.h"
#include "dbLink.h"
#include "callback.h"
#include "dbScan.h"
#include "dbLock.h"
#include "dbEvent.h"
#include "dbConvert.h"
#include "dbConvertFast.h"
#include "epicsEvent.h"
#include "dbCa.h"
#include "dbBkpt.h"
#include "dbNotify.h"
#include "dbAccessDefs.h"
#include "recGbl.h"

static void inherit_severity(const struct pv_link *ppv_link, dbCommon *pdest,
        epicsEnum16 stat, epicsEnum16 sevr)
{
    switch (ppv_link->pvlMask & pvlOptMsMode) {
    case pvlOptNMS:
        break;
    case pvlOptMSI:
        if (sevr < INVALID_ALARM)
            break;
        /* Fall through */
    case pvlOptMS:
        recGblSetSevr(pdest, LINK_ALARM, sevr);
        break;
    case pvlOptMSS:
        recGblSetSevr(pdest, stat, sevr);
        break;
    }
}

/***************************** Constant Links *****************************/

static long dbConstLoadLink(struct link *plink, short dbrType, void *pbuffer)
{
    if (!plink->value.constantStr)
        return S_db_badField;

    /* Constant strings are always numeric */
    if (dbrType== DBF_MENU || dbrType == DBF_ENUM || dbrType == DBF_DEVICE)
        dbrType = DBF_USHORT;

    return dbFastPutConvertRoutine[DBR_STRING][dbrType]
            (plink->value.constantStr, pbuffer, NULL);
}

static long dbConstGetNelements(const struct link *plink, long *nelements)
{
    *nelements = 0;
    return 0;
}

static long dbConstGetLink(struct link *plink, short dbrType, void *pbuffer,
        epicsEnum16 *pstat, epicsEnum16 *psevr, long *pnRequest)
{
    if (pnRequest)
        *pnRequest = 0;
    return 0;
}

/***************************** Database Links *****************************/

static long dbDbInitLink(struct link *plink, short dbfType)
{
    DBADDR dbaddr;
    long status;
    DBADDR *pdbAddr;

    status = dbNameToAddr(plink->value.pv_link.pvname, &dbaddr);
    if (status)
        return status;

    plink->type = DB_LINK;
    pdbAddr = dbCalloc(1, sizeof(struct dbAddr));
    *pdbAddr = dbaddr; /* structure copy */
    plink->value.pv_link.pvt = pdbAddr;
    return 0;
}

static long dbDbAddLink(struct link *plink, short dbfType)
{
    long status = dbDbInitLink(plink, dbfType);

    if (!status) {
        DBADDR *pdbAddr = (DBADDR *) plink->value.pv_link.pvt;
        dbCommon *precord = plink->value.pv_link.precord;

        dbLockSetRecordLock(pdbAddr->precord);
        dbLockSetMerge(precord, pdbAddr->precord);
    }
    return status;
}

static void dbDbRemoveLink(struct link *plink)
{
    free(plink->value.pv_link.pvt);
    plink->value.pv_link.pvt = 0;
    plink->value.pv_link.getCvt = 0;
    plink->value.pv_link.lastGetdbrType = 0;
    plink->type = PV_LINK;
    dbLockSetSplit(plink->value.pv_link.precord);
}

static int dbDbIsLinkConnected(const struct link *plink)
{
    return TRUE;
}

static int dbDbGetDBFtype(const struct link *plink)
{
    DBADDR *paddr = (DBADDR *) plink->value.pv_link.pvt;

    return paddr->field_type;
}

static long dbDbGetElements(const struct link *plink, long *nelements)
{
    DBADDR *paddr = (DBADDR *) plink->value.pv_link.pvt;

    *nelements = paddr->no_elements;
    return 0;
}

static long dbDbGetValue(struct link *plink, short dbrType, void *pbuffer,
        epicsEnum16 *pstat, epicsEnum16 *psevr, long *pnRequest)
{
    struct pv_link *ppv_link = &plink->value.pv_link;
    DBADDR *paddr = ppv_link->pvt;
    dbCommon *precord = plink->value.pv_link.precord;
    long status;

    /* scan passive records if link is process passive  */
    if (ppv_link->pvlMask & pvlOptPP) {
        unsigned char pact = precord->pact;

        precord->pact = TRUE;
        status = dbScanPassive(precord, paddr->precord);
        precord->pact = pact;
        if (status)
            return status;
    }
    *pstat = paddr->precord->stat;
    *psevr = paddr->precord->sevr;

    if (ppv_link->getCvt && ppv_link->lastGetdbrType == dbrType) {
        status = ppv_link->getCvt(paddr->pfield, pbuffer, paddr);
    } else {
        unsigned short dbfType = paddr->field_type;

        if (dbrType < 0 || dbrType > DBR_ENUM || dbfType > DBF_DEVICE)
            return S_db_badDbrtype;

        if (paddr->no_elements == 1 && (!pnRequest || *pnRequest == 1)
                && paddr->special != SPC_ATTRIBUTE) {
            ppv_link->getCvt = dbFastGetConvertRoutine[dbfType][dbrType];
            status = ppv_link->getCvt(paddr->pfield, pbuffer, paddr);
        } else {
            ppv_link->getCvt = NULL;
            status = dbGet(paddr, dbrType, pbuffer, NULL, pnRequest, NULL);
        }
        ppv_link->lastGetdbrType = dbrType;
    }
    return status;
}

static long dbDbGetControlLimits(const struct link *plink, double *low,
        double *high)
{
    DBADDR *paddr = (DBADDR *) plink->value.pv_link.pvt;
    struct buffer {
        DBRctrlDouble
        double value;
    } buffer;
    long options = DBR_CTRL_DOUBLE;
    long number_elements = 0;
    long status = dbGet(paddr, DBR_DOUBLE, &buffer, &options, &number_elements,
            NULL);

    if (status)
        return status;

    *low = buffer.lower_ctrl_limit;
    *high = buffer.upper_ctrl_limit;
    return 0;
}

static long dbDbGetGraphicLimits(const struct link *plink, double *low,
        double *high)
{
    DBADDR *paddr = (DBADDR *) plink->value.pv_link.pvt;
    struct buffer {
        DBRgrDouble
        double value;
    } buffer;
    long options = DBR_GR_DOUBLE;
    long number_elements = 0;
    long status = dbGet(paddr, DBR_DOUBLE, &buffer, &options, &number_elements,
            NULL);

    if (status)
        return status;

    *low = buffer.lower_disp_limit;
    *high = buffer.upper_disp_limit;
    return 0;
}

static long dbDbGetAlarmLimits(const struct link *plink, double *lolo,
        double *low, double *high, double *hihi)
{
    DBADDR *paddr = (DBADDR *) plink->value.pv_link.pvt;
    struct buffer {
        DBRalDouble
        double value;
    } buffer;
    long options = DBR_AL_DOUBLE;
    long number_elements = 0;
    long status = dbGet(paddr, DBR_DOUBLE, &buffer, &options, &number_elements,
            0);

    if (status)
        return status;

    *lolo = buffer.lower_alarm_limit;
    *low = buffer.lower_warning_limit;
    *high = buffer.upper_warning_limit;
    *hihi = buffer.upper_alarm_limit;
    return 0;
}

static long dbDbGetPrecision(const struct link *plink, short *precision)
{
    DBADDR *paddr = (DBADDR *) plink->value.pv_link.pvt;
    struct buffer {
        DBRprecision
        double value;
    } buffer;
    long options = DBR_PRECISION;
    long number_elements = 0;
    long status = dbGet(paddr, DBR_DOUBLE, &buffer, &options, &number_elements,
            0);

    if (status)
        return status;

    *precision = buffer.precision.dp;
    return 0;
}

static long dbDbGetUnits(const struct link *plink, char *units, int unitsSize)
{
    DBADDR *paddr = (DBADDR *) plink->value.pv_link.pvt;
    struct buffer {
        DBRunits
        double value;
    } buffer;
    long options = DBR_UNITS;
    long number_elements = 0;
    long status = dbGet(paddr, DBR_DOUBLE, &buffer, &options, &number_elements,
            0);

    if (status)
        return status;

    strncpy(units, buffer.units, unitsSize);
    return 0;
}

static long dbDbGetAlarm(const struct link *plink, epicsEnum16 *status,
        epicsEnum16 *severity)
{
    DBADDR *paddr = (DBADDR *) plink->value.pv_link.pvt;

    if (status)
        *status = paddr->precord->stat;
    if (severity)
        *severity = paddr->precord->sevr;
    return 0;
}

static long dbDbGetTimeStamp(const struct link *plink, epicsTimeStamp *pstamp)
{
    DBADDR *paddr = (DBADDR *) plink->value.pv_link.pvt;

    *pstamp = paddr->precord->time;
    return 0;
}

static long dbDbPutValue(struct link *plink, short dbrType,
        const void *pbuffer, long nRequest)
{
    struct pv_link *ppv_link = &plink->value.pv_link;
    struct dbCommon *psrce = ppv_link->precord;
    DBADDR *paddr = (DBADDR *) ppv_link->pvt;
    dbCommon *pdest = paddr->precord;
    long status = dbPut(paddr, dbrType, pbuffer, nRequest);

    inherit_severity(ppv_link, pdest, psrce->nsta, psrce->nsev);
    if (status)
        return status;

    if (paddr->pfield == (void *) &pdest->proc ||
            (ppv_link->pvlMask & pvlOptPP && pdest->scan == 0)) {
        /* if dbPutField caused asyn record to process */
        /* ask for reprocessing*/
        if (pdest->putf) {
            pdest->rpro = TRUE;
        } else { /* process dest record with source's PACT true */
            unsigned char pact;

            if (psrce && psrce->ppn)
                dbNotifyAdd(psrce, pdest);
            pact = psrce->pact;
            psrce->pact = TRUE;
            status = dbProcess(pdest);
            psrce->pact = pact;
        }
    }
    return status;
}

static void dbDbScanFwdLink(struct link *plink)
{
    dbCommon *precord = plink->value.pv_link.precord;
    dbAddr *paddr = (dbAddr *) plink->value.pv_link.pvt;

    dbScanPassive(precord, paddr->precord);
}

lset dbDb_lset = { dbDbInitLink, dbDbAddLink, NULL, dbDbRemoveLink,
        dbDbIsLinkConnected, dbDbGetDBFtype, dbDbGetElements, dbDbGetValue,
        dbDbGetControlLimits, dbDbGetGraphicLimits, dbDbGetAlarmLimits,
        dbDbGetPrecision, dbDbGetUnits, dbDbGetAlarm, dbDbGetTimeStamp,
        dbDbPutValue, dbDbScanFwdLink };

/***************************** Generic Link API *****************************/

void dbInitLink(struct dbCommon *precord, struct link *plink, short dbfType)
{
    plink->value.pv_link.precord = precord;

    if (plink == &precord->tsel)
        recGblTSELwasModified(plink);

    if (!(plink->value.pv_link.pvlMask & (pvlOptCA | pvlOptCP | pvlOptCPP))) {
        /* Make it a DB link if possible */
        if (!dbDbInitLink(plink, dbfType))
            return;
    }

    /* Make it a CA link */
    if (dbfType == DBF_INLINK)
        plink->value.pv_link.pvlMask |= pvlOptInpNative;

    dbCaAddLink(plink);
    if (dbfType == DBF_FWDLINK) {
        char *pperiod = strrchr(plink->value.pv_link.pvname, '.');

        if (pperiod && strstr(pperiod, "PROC")) {
            plink->value.pv_link.pvlMask |= pvlOptFWD;
        } else {
            errlogPrintf("%s.FLNK is a Channel Access Link "
                " but does not link to a PROC field\n", precord->name);
        }
    }
}

void dbAddLink(struct dbCommon *precord, struct link *plink, short dbfType)
{
    plink->value.pv_link.precord = precord;

    if (plink == &precord->tsel)
        recGblTSELwasModified(plink);

    if (!(plink->value.pv_link.pvlMask & (pvlOptCA | pvlOptCP | pvlOptCPP))) {
        /* Can we make it a DB link? */
        if (!dbDbAddLink(plink, dbfType))
            return;
    }

    /* Make it a CA link */
    if (dbfType == DBF_INLINK)
        plink->value.pv_link.pvlMask |= pvlOptInpNative;

    dbCaAddLink(plink);
    if (dbfType == DBF_FWDLINK) {
        char *pperiod = strrchr(plink->value.pv_link.pvname, '.');

        if (pperiod && strstr(pperiod, "PROC"))
            plink->value.pv_link.pvlMask |= pvlOptFWD;
    }
}

long dbLoadLink(struct link *plink, short dbrType, void *pbuffer)
{
    switch (plink->type) {
    case CONSTANT:
        return dbConstLoadLink(plink, dbrType, pbuffer);
    }
    return S_db_notFound;
}

void dbRemoveLink(struct link *plink)
{
    switch (plink->type) {
    case DB_LINK:
        dbDbRemoveLink(plink);
        break;
    case CA_LINK:
        dbCaRemoveLink(plink);
        break;
    default:
        cantProceed("dbRemoveLink: Unexpected link type %d\n", plink->type);
    }
    plink->type = PV_LINK;
    plink->value.pv_link.pvlMask = 0;
}

int dbIsLinkConnected(const struct link *plink)
{
    switch (plink->type) {
    case DB_LINK:
        return dbDbIsLinkConnected(plink);
    case CA_LINK:
        return dbCaIsLinkConnected(plink);
    }
    return FALSE;
}

int dbGetLinkDBFtype(const struct link *plink)
{
    switch (plink->type) {
    case DB_LINK:
        return dbDbGetDBFtype(plink);
    case CA_LINK:
        return dbCaGetLinkDBFtype(plink);
    }
    return -1;
}

long dbGetNelements(const struct link *plink, long *nelements)
{
    switch (plink->type) {
    case CONSTANT:
        return dbConstGetNelements(plink, nelements);
    case DB_LINK:
        return dbDbGetElements(plink, nelements);
    case CA_LINK:
        return dbCaGetNelements(plink, nelements);
    }
    return S_db_badField;
}

long dbGetLink(struct link *plink, short dbrType, void *pbuffer,
        long *poptions, long *pnRequest)
{
    struct dbCommon *precord = plink->value.pv_link.precord;
    epicsEnum16 sevr = 0, stat = 0;
    long status;

    if (poptions && *poptions) {
        printf("dbGetLinkValue: Use of poptions no longer supported\n");
        *poptions = 0;
    }

    switch (plink->type) {
    case CONSTANT:
        status = dbConstGetLink(plink, dbrType, pbuffer, &stat, &sevr,
                pnRequest);
        break;
    case DB_LINK:
        status = dbDbGetValue(plink, dbrType, pbuffer, &stat, &sevr, pnRequest);
        break;
    case CA_LINK:
        status = dbCaGetLink(plink, dbrType, pbuffer, &stat, &sevr, pnRequest);
        break;
    default:
        cantProceed("dbGetLinkValue: Illegal link type %d\n", plink->type);
        status = -1;
    }
    if (status) {
        recGblSetSevr(precord, LINK_ALARM, INVALID_ALARM);
    } else {
        inherit_severity(&plink->value.pv_link, precord, stat, sevr);
    }
    return status;
}

long dbGetControlLimits(const struct link *plink, double *low, double *high)
{
    switch (plink->type) {
    case DB_LINK:
        return dbDbGetControlLimits(plink, low, high);
    case CA_LINK:
        return dbCaGetControlLimits(plink, low, high);
    }
    return S_db_notFound;
}

long dbGetGraphicLimits(const struct link *plink, double *low, double *high)
{
    switch (plink->type) {
    case DB_LINK:
        return dbDbGetGraphicLimits(plink, low, high);
    case CA_LINK:
        return dbCaGetGraphicLimits(plink, low, high);
    }
    return S_db_notFound;
}

long dbGetAlarmLimits(const struct link *plink, double *lolo, double *low,
        double *high, double *hihi)
{
    switch (plink->type) {
    case DB_LINK:
        return dbDbGetAlarmLimits(plink, lolo, low, high, hihi);
    case CA_LINK:
        return dbCaGetAlarmLimits(plink, lolo, low, high, hihi);
    }
    return S_db_notFound;
}

long dbGetPrecision(const struct link *plink, short *precision)
{
    switch (plink->type) {
    case DB_LINK:
        return dbDbGetPrecision(plink, precision);
    case CA_LINK:
        return dbCaGetPrecision(plink, precision);
    }
    return S_db_notFound;
}

long dbGetUnits(const struct link *plink, char *units, int unitsSize)
{
    switch (plink->type) {
    case DB_LINK:
        return dbDbGetUnits(plink, units, unitsSize);
    case CA_LINK:
        return dbCaGetUnits(plink, units, unitsSize);
    }
    return S_db_notFound;
}

long dbGetAlarm(const struct link *plink, epicsEnum16 *status,
        epicsEnum16 *severity)
{
    switch (plink->type) {
    case DB_LINK:
        return dbDbGetAlarm(plink, status, severity);
    case CA_LINK:
        return dbCaGetAlarm(plink, status, severity);
    }
    return S_db_notFound;
}

long dbGetTimeStamp(const struct link *plink, epicsTimeStamp *pstamp)
{
    switch (plink->type) {
    case DB_LINK:
        return dbDbGetTimeStamp(plink, pstamp);
    case CA_LINK:
        return dbCaGetTimeStamp(plink, pstamp);
    }
    return S_db_notFound;
}

long dbPutLink(struct link *plink, short dbrType, const void *pbuffer,
        long nRequest)
{
    long status;

    switch (plink->type) {
    case CONSTANT:
        status = 0;
        break;
    case DB_LINK:
        status = dbDbPutValue(plink, dbrType, pbuffer, nRequest);
        break;
    case CA_LINK:
        status = dbCaPutLink(plink, dbrType, pbuffer, nRequest);
        break;
    default:
        cantProceed("dbPutLinkValue: Illegal link type %d\n", plink->type);
        status = -1;
    }
    if (status) {
        struct dbCommon *precord = plink->value.pv_link.precord;

        recGblSetSevr(precord, LINK_ALARM, INVALID_ALARM);
    }
    return status;
}

void dbScanFwdLink(struct link *plink)
{
    switch (plink->type) {
    case DB_LINK:
        dbDbScanFwdLink(plink);
        break;
    case CA_LINK:
        dbCaScanFwdLink(plink);
        break;
    }
}

/* Helper functions for long string support */

long dbLoadLinkLS(struct link *plink, char *pbuffer, epicsUInt32 size,
    epicsUInt32 *plen)
{
    if (plink->type == CONSTANT &&
        plink->value.constantStr) {
        strncpy(pbuffer, plink->value.constantStr, --size);
        pbuffer[size] = 0;
        *plen = strlen(pbuffer) + 1;
        return 0;
    }

    return S_db_notFound;
}

long dbGetLinkLS(struct link *plink, char *pbuffer, epicsUInt32 size,
    epicsUInt32 *plen)
{
    int dtyp = dbGetLinkDBFtype(plink);
    long len = size;
    long status;

    if (dtyp < 0)   /* Not connected */
        return 0;

    if (dtyp == DBR_CHAR || dtyp == DBF_UCHAR) {
        status = dbGetLink(plink, dtyp, pbuffer, 0, &len);
    }
    else if (size >= MAX_STRING_SIZE)
        status = dbGetLink(plink, DBR_STRING, pbuffer, 0, 0);
    else {
        /* pbuffer is too small to fetch using DBR_STRING */
        char tmp[MAX_STRING_SIZE];

        status = dbGetLink(plink, DBR_STRING, tmp, 0, 0);
        if (!status)
            strncpy(pbuffer, tmp, len - 1);
    }
    if (!status) {
        pbuffer[--len] = 0;
        *plen = strlen(pbuffer) + 1;
    }
    return status;
}

long dbPutLinkLS(struct link *plink, char *pbuffer, epicsUInt32 len)
{
    int dtyp = dbGetLinkDBFtype(plink);

    if (dtyp < 0)
        return 0;   /* Not connected */

    if (dtyp == DBR_CHAR || dtyp == DBF_UCHAR)
        return dbPutLink(plink, dtyp, pbuffer, len);

    return dbPutLink(plink, DBR_STRING, pbuffer, 1);
}

