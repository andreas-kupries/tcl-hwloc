#include <hwloc.h>
#include "misc.h"

/*
 * ============================================================================
 */

int
thwl_get_cpuset_type (Tcl_Interp* interp, Tcl_Obj* obj, thwl_cpuset_type* t)
{
    /*
     * Order in the string array must be kept in sync with enum
     * thwl_cpuset_type
     */

    static const char* types[] = {
        "allowed", "complete", "online", "topology",
        NULL
    };

    return Tcl_GetIndexFromObj(interp, obj, types, "cpuset-type", TCL_EXACT, (int*) t);
}

/*
 * ============================================================================
 */

int
thwl_get_nodeset_type (Tcl_Interp* interp, Tcl_Obj* obj, thwl_nodeset_type* t)
{
    /*
     * Order in the string array must be kept in sync with enum
     * thwl_nodeset_type
     */

    static const char* types[] = {
        "allowed", "complete", "topology",
        NULL
    };

    return Tcl_GetIndexFromObj(interp, obj, types, "nodeset-type", TCL_EXACT, (int*) t);
}

/* vim: set sts=4 sw=4 tw=80 et ft=c: */
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
