#include <tcl.h>
#include <hwloc.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "topology.h"
#include "cpubind.h"
#include "bitmap.h"

static int
parse_flags (Tcl_Interp *interp, int *result, Tcl_Obj *obj);

/*
 * topology cpubind ?-pid pid? ?-type flags_list? ...
 */
int parse_cpubind_args (topo_data* data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    static const char* cmds[] = {
	"get", "last", "set", NULL
    };
    enum options {
        CPUBIND_GET, CPUBIND_LAST, CPUBIND_SET
    };

    hwloc_bitmap_t thecpuset;

    int index, res = 0;
    int objv_idx = 3;
    int pid = 0;
    int flags = 0;

    if (Tcl_GetIndexFromObj(interp, objv[2], cmds, "option", 2, &index) != TCL_OK) {
        return TCL_ERROR;
    }

    if ((objc >= objv_idx+2) &&
	strcmp((const char *) Tcl_GetString(objv[objv_idx]), "-pid") == 0) {
        if (Tcl_GetIntFromObj(interp, objv[objv_idx+1], &pid) == TCL_ERROR) {
            return TCL_ERROR;
	}
        objv_idx += 2;
    }

    if ((objc >= objv_idx+2) &&
	strcmp((const char *) Tcl_GetString(objv[objv_idx]), "-type") == 0) {
        if (parse_flags(interp, &flags, objv[objv_idx+1]) == TCL_ERROR) {
            return TCL_ERROR;
	}
        objv_idx += 2;
    }

    switch (index) {
    case CPUBIND_SET:
	if (objc != (objv_idx+1)) {
	    Tcl_WrongNumArgs(interp, objv_idx, objv, "cpuset");
	    return TCL_ERROR;
	}

	thecpuset = thwl_get_bitmap (interp, objv[objv_idx]);
	if (thecpuset == NULL) {
	    return TCL_ERROR;
	}

	if (pid) {
	    res = hwloc_set_proc_cpubind(data->topology, pid, thecpuset, flags);
	} else {
	    res = hwloc_set_cpubind(data->topology, thecpuset, flags);
	}

	hwloc_bitmap_free(thecpuset);
	if (res == -1) goto bind_error;
	break;

    case CPUBIND_GET:
    case CPUBIND_LAST:
	if (objc > objv_idx) {
	    Tcl_WrongNumArgs(interp, objv_idx, objv, NULL);
	    return TCL_ERROR;
	}

	thecpuset = hwloc_bitmap_alloc();

	switch (index) {
	case CPUBIND_GET:
	    if (pid) {
		res = hwloc_get_proc_cpubind(data->topology, pid, thecpuset, flags);
	    } else {
		res = hwloc_get_cpubind(data->topology, thecpuset, flags);
	    }
	    break;
	case CPUBIND_LAST:
	    if (pid) {
		res = hwloc_get_proc_last_cpu_location(data->topology, pid, thecpuset, flags);
	    } else {
		res = hwloc_get_last_cpu_location(data->topology, thecpuset, flags);
	    }
	    break;
	}

	if (res == -1) {
	    hwloc_bitmap_free(thecpuset);
	    goto bind_error;
	}

	return thwl_set_result_bitmap (interp, thecpuset);
    }

    return TCL_OK;

 bind_error:
    if (errno == ENOSYS) {
	Tcl_SetResult(interp, "operation not supported", TCL_STATIC);
    } else if (errno == EXDEV) {
	Tcl_SetResult(interp, "binding not enforceable", TCL_STATIC);
    } else {
	Tcl_SetResult(interp, "general cpu binding failure", TCL_STATIC);
    }
    return TCL_ERROR;
}

static int parse_flags (Tcl_Interp *interp, int *result, Tcl_Obj *obj) {
    static const char* flags[] = {
        "nomembind", "process",
	"strict",    "thread", NULL
    };
    enum options {
        CPUBIND_NOMEMBIND, CPUBIND_PROCESS,
	CPUBIND_STRICT,    CPUBIND_THREAD
    };
    static int map [] = {
	HWLOC_CPUBIND_NOMEMBIND, HWLOC_CPUBIND_PROCESS,
	HWLOC_CPUBIND_STRICT,    HWLOC_CPUBIND_THREAD
    };

    int i, index;

    int       obj_objc;
    Tcl_Obj **obj_objv;

    if (Tcl_ListObjGetElements(interp, obj, &obj_objc, &obj_objv) == TCL_ERROR) {
        Tcl_SetResult(interp, "parsing flags failed", TCL_STATIC);
        return TCL_ERROR;
    }

    for (i = 0; i < obj_objc; i++) {  
        if (Tcl_GetIndexFromObj(interp, obj_objv[i], flags, "flag", 0, &index) != TCL_OK) {
            return TCL_ERROR;
	}
	*result |= map [index];
    }

    return TCL_OK;
}

/* vim: set sts=4 sw=4 tw=80 et ft=c: */
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
