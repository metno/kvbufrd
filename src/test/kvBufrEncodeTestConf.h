/*
 * kvBufferEncodeTestConf.h
 *
 *  Created on: Feb 23, 2010
 *      Author: borgem
 */

#ifndef __KVBUFRENCODETESTCONF_H__
#define __KVBUFRENCODETESTCONF_H__

const char *testconf =
"wmo_default{\n"
"  #default values\n"
"  copyto=\"/dnmi/norcom/data/incoming/kvalobs\"\n"
"  copy=\"false\"\n"
"  owner=\"AUTG\"\n"
"  list=\"99\"\n"
"  loglevel=9\n"
"}\n"
"\n"
"wmo_01389{\n"
"  #Rena\n"
"  stationid=7010\n"
"  typepriority=(312)\n"
"  owner=\"PIOG\"\n"
"  precipitation=(\"RRRtr\")\n"
"}\n"
"wmo_01001{\n"
"  #Rena\n"
"  stationid=7010\n"
"  typepriority=(330)\n"
"  owner=\"AUTG\"\n"
"  precipitation=(\"RA\")\n"
"}\n"
;

#endif
