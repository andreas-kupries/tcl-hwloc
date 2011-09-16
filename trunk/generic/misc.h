#ifndef _TCLHWLOC_MISC_H_
#define _TCLHWLOC_MISC_H_

#include <hwloc.h>
#include <tcl.h>

typedef enum thwl_cpuset_type {
    CPUSET_ALLOWED,
    CPUSET_COMPLETE,
    CPUSET_ONLINE,
    CPUSET_TOPOLOGY
} thwl_cpuset_type;

typedef enum thwl_nodeset_type {
    NODESET_ALLOWED,
    NODESET_COMPLETE,
    NODESET_TOPOLOGY
} thwl_nodeset_type;

/*
 * Get cpuset type.
 */

int thwl_get_cpuset_type  (Tcl_Interp* interp, Tcl_Obj* obj, thwl_cpuset_type* t);
int thwl_get_nodeset_type (Tcl_Interp* interp, Tcl_Obj* obj, thwl_nodeset_type* t);

#endif /* _TCLHWLOC_MISC_H_ */

/* vim: set sts=4 sw=4 tw=80 et ft=c: */
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
