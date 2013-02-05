#ifndef __WITH_PUTOOLS__

#include <puTools/miTime.h>
#define __dnmi_miTime__
#include <miutil/timeconvert.h>

#define TO_PTIME( mi_time ) miutil::to_ptime( mi_time )
#define TO_MITIME( pt_time ) boost::posix_time::to_miTime( pt_time )

#else
#define TO_PTIME( mi_time ) mi_time
#define TO_MITIME( pt_time ) pt_time
#endif

