/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: Data.h,v 1.2.6.2 2007/09/27 09:02:22 paule Exp $

  Copyright (C) 2007 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of KVALOBS

  KVALOBS is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  KVALOBS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with KVALOBS; if not, write to the Free Software Foundation Inc.,
  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __BUFRDEFS_H__
#define __BUFRDEFS_H__

#define KELEM 4000    /* Max number of elements in values array. */
#define KVALS 20000   /* Max number of elements in CVALS array.
                       * Note that 200000 (as used in example Fortran programs
                       * from ECMWF) is too big; results in segmentation
                       * violation for cvals */
#define KDLEN 200     /* Max number of elements in kdata array */
#define MAX_BUFLEN 200000 /* Max lenght (in bytes) of bufr message */
#define RVIND 1.7E38  /* 'Missing' value */


#if defined(__cplusplus)
extern "C" {
#endif

extern void pbopen_(int *fd, char *filename, char *fmod, int *errorIndicator, int lengthOfFilename, int unknown /*set to 2*/);
extern void pbclose_(int *fd, int *errorIndicator );
extern void pbwrite_(int *fd, int *kbuff, int *nbytes, int *errorIndicator );
extern void bufren_(int *ksec0, int *ksec1, int *ksec2, int *ksec3, int *ksec4,
                    int *ktdlen, int *ktdlst, int *kdlen, int *kdata,
                    int *kelem, int *kvals, double *values, char **cvals,
                    int *kbufl, int *kbuff, int *errorIndicator);

#if defined(__cplusplus)
}
#endif

#endif
