#include <tcl.h>
#include <hwloc.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "topology.h"
#include "cpubind.h"

static int parse_flags (Tcl_Interp *interp, int *result, Tcl_Obj *obj);

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
    char *list;

    int index;
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
        int res = 0;
        if (parse_flags(interp, &res, objv[objv_idx+1]) == TCL_ERROR) {
            return TCL_ERROR;
	}
        objv_idx += 2;
    }

    switch (index) {
    case CPUBIND_SET:
	{
	    if (objc == objv_idx+1) {
		const char *setstr = Tcl_GetString(objv[objv_idx]);

	        thecpuset = hwloc_bitmap_alloc();

		if (hwloc_bitmap_list_sscanf(thecpuset, setstr) == -1) {
		    Tcl_SetResult(interp, "failed to parse cpuset", TCL_STATIC);
		    hwloc_bitmap_free(thecpuset);
		    return TCL_ERROR;
		}

		if (pid) {
		    if (hwloc_set_proc_cpubind(data->topology, pid, thecpuset, flags) == -1) {
			Tcl_SetResult(interp, "hwloc_set_proc_cpubind() failed", TCL_STATIC);
			hwloc_bitmap_free(thecpuset);
			return TCL_ERROR;
		    }
		} else {
		    if (hwloc_set_cpubind(data->topology, thecpuset, flags) == -1) {
			Tcl_SetResult(interp, "hwloc_set_cpubind() failed", TCL_STATIC);
			hwloc_bitmap_free(thecpuset);
			return TCL_ERROR;
		    }
		}

		hwloc_bitmap_free(thecpuset);
	    } else {
		Tcl_WrongNumArgs(interp, objv_idx, objv, "cpuset");
		return TCL_ERROR;
	    }
	    break;
	}
    case CPUBIND_GET:
	{
	    if (objc > objv_idx) {
		Tcl_WrongNumArgs(interp, objv_idx, objv, NULL);
		return TCL_ERROR;
	    }

	    thecpuset = hwloc_bitmap_alloc();

	    if (pid) {
		if (hwloc_get_proc_cpubind(data->topology, pid, thecpuset, flags) == -1) {
		    Tcl_SetResult(interp, "hwloc_get_proc_cpubind() failed", TCL_STATIC);
		    hwloc_bitmap_free(thecpuset);
		    return TCL_ERROR;
		}
	    } else {
		if (hwloc_get_cpubind(data->topology, thecpuset, flags) == -1) {
		    Tcl_SetResult(interp, "hwloc_get_cpubind() failed", TCL_STATIC);
		    hwloc_bitmap_free(thecpuset);
		    return TCL_ERROR;
		}
	    }

	    if (hwloc_bitmap_list_asprintf(&list, thecpuset) == -1) {
		Tcl_SetResult(interp, "hwloc_bitmap_list_asprintf() failed", TCL_STATIC);
		hwloc_bitmap_free(thecpuset);
		return TCL_ERROR;
	    }

	    Tcl_SetObjResult(interp, Tcl_NewStringObj (list, -1));

	    hwloc_bitmap_free(thecpuset);
	    free(list);
	    break;
	}
    case CPUBIND_LAST:
	{
	    if (objc > objv_idx) {
		Tcl_WrongNumArgs(interp, objv_idx, objv, NULL);
		return TCL_ERROR;
	    }

	    thecpuset = hwloc_bitmap_alloc();

	    if (pid) {
		if (hwloc_get_proc_last_cpu_location(data->topology, pid, thecpuset, flags) == -1) {
		    Tcl_SetResult(interp, "hwloc_get_proc_last_cpu_location() failed", TCL_STATIC);
		    hwloc_bitmap_free(thecpuset);
		    return TCL_ERROR;
		}
	    } else {
		if (hwloc_get_last_cpu_location(data->topology, thecpuset, flags) == -1) {
		    Tcl_SetResult(interp, "hwloc_get_last_cpu_location() failed", TCL_STATIC);
		    hwloc_bitmap_free(thecpuset);
		    return TCL_ERROR;
		}
	    }

	    if (hwloc_bitmap_list_asprintf(&list, thecpuset) == -1) {
		Tcl_SetResult(interp, "hwloc_bitmap_list_asprintf() failed", TCL_STATIC);
		hwloc_bitmap_free(thecpuset);
		return TCL_ERROR;
	    }

	    Tcl_SetObjResult(interp, Tcl_NewStringObj (list, -1));

	    hwloc_bitmap_free(thecpuset);
	    free(list);
	    break;
	}
    }

    return TCL_OK;
}

static int parse_flags (Tcl_Interp *interp, int *result, Tcl_Obj *obj) {
    static const char* flags[] = {
        "nomembind", "process", "strict", "thread", NULL
    };
    enum options {
        CPUBIND_NOMEMBIND, CPUBIND_PROCESS, CPUBIND_STRICT, CPUBIND_THREAD
    };

    int i, index;

    int       obj_objc;
    Tcl_Obj **obj_objv;

    if (Tcl_ListObjGetElements(interp, obj, &obj_objc, &obj_objv) == TCL_ERROR) {
        Tcl_SetResult(interp, "parsing flags failed", TCL_STATIC);
        return TCL_ERROR;
    }

    for (i = 0; i < obj_objc; i++) {  
        if (Tcl_GetIndexFromObj(interp, obj_objv[i], flags, "flag", 0, &index) != TCL_OK)
            return TCL_ERROR;

        switch(index) {
	case CPUBIND_PROCESS:   *result |= HWLOC_CPUBIND_PROCESS;   break;
	case CPUBIND_THREAD:    *result |= HWLOC_CPUBIND_THREAD;    break;
	case CPUBIND_STRICT:    *result |= HWLOC_CPUBIND_STRICT;    break;
	case CPUBIND_NOMEMBIND: *result |= HWLOC_CPUBIND_NOMEMBIND; break;
	default:
	    Tcl_SetResult(interp, "unrecognized flag", TCL_STATIC);
	    return TCL_ERROR;
        }
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
