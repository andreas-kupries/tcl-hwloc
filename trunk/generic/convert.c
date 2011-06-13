#include <tcl.h>
#include <hwloc.h>
#include "convert.h"

/* A returned -1 indicates a conversion error */
hwloc_obj_type_t convert_obj2obj_type(Tcl_Interp *interp, Tcl_Obj *obj) {
    static const char *types[] = {
        "system", "machine", "node", "socket", "cache", "pu", "obj_group", "obj_misc", "obj_type_max", NULL
    };
    int index;

    if (Tcl_GetIndexFromObj(interp, obj, types, "obj_type", 0, &index) != TCL_OK)
        return TCL_ERROR;

    switch (index) {
        case 0: return HWLOC_OBJ_SYSTEM;
        case 1: return HWLOC_OBJ_MACHINE;
        case 2: return HWLOC_OBJ_NODE;
        case 3: return HWLOC_OBJ_SOCKET;
        case 4: return HWLOC_OBJ_CACHE;
        case 5: return HWLOC_OBJ_PU;
        case 6: return HWLOC_OBJ_GROUP;
        case 7: return HWLOC_OBJ_MISC;
        case 8: return HWLOC_OBJ_TYPE_MAX;
    }

    /* else */
    return TCL_ERROR;
}
