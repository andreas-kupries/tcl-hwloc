#include <tcl.h>
#include <hwloc.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "topology.h"
#include "cpubind.h"
#include "bitmap.h"

/*
 * Helper function for parse_cpubind_args below. Processes all the options
 */

static int
parse_options (Tcl_Interp* interp, int objc, Tcl_Obj* CONST objv[], int* pid, int* flags);

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

    int at, index, res = 0;
    int pid = 0;
    int flags = 0;

    if (Tcl_GetIndexFromObj(interp, objv[2], cmds, "option", 2, &index) != TCL_OK) {
        return TCL_ERROR;
    }

    at = parse_options (interp, objc, objv, &pid, &flags);
    if (at < 0) {
	return TCL_ERROR;
    }

    switch (index) {
    case CPUBIND_SET:
	if (at != (objc-1)) {
	    Tcl_WrongNumArgs(interp, 3, objv, "?options? cpuset");
	    return TCL_ERROR;
	}

	thecpuset = thwl_get_bitmap (interp, objv[objc-1]);
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
	if (at != objc) {
	    Tcl_WrongNumArgs(interp, 3, objv, "?options?");
	    return TCL_ERROR;
	}

	if (pid && (flags & HWLOC_CPUBIND_THREAD)) {
	    Tcl_SetResult (interp, "Illegal use of -thread together with -pid", TCL_STATIC);
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
    Tcl_PosixError(interp);

    if (errno == ENOSYS) {
	Tcl_SetResult(interp, "operation not supported", TCL_STATIC);
    } else if (errno == EXDEV) {
	Tcl_SetResult(interp, "binding not enforceable", TCL_STATIC);
    } else {
	Tcl_SetResult(interp, "general cpu binding failure", TCL_STATIC);
    }
    return TCL_ERROR;
}

/*
 * Helper function for parse_cpubind_args below. Processes all the options
 */

static int
parse_options (Tcl_Interp* interp, int objc, Tcl_Obj* CONST objv[], int* pid, int* flags)
{
    static const char* options[] = {
        "-nomembind", "-pid",    "-process",
	"-strict",    "-thread", NULL
    };
    enum options {
        CPUBIND_NOMEMBIND, CPUBIND_PID, CPUBIND_PROCESS,
	CPUBIND_STRICT,    CPUBIND_THREAD
    };
    static int map [] = {
	HWLOC_CPUBIND_NOMEMBIND, 0, HWLOC_CPUBIND_PROCESS,
	HWLOC_CPUBIND_STRICT,    HWLOC_CPUBIND_THREAD
    };

    int index, at;

    *pid   = 0;
    *flags = 0;

    /* topo/0 cpubind/1 method/2 ... => at = 3 */
    for (at = 3; at < objc; at++) {
        if (Tcl_GetIndexFromObj(interp, objv[at], options, "flag", 0, &index) != TCL_OK) {
	    if(Tcl_GetString (objv[at])[0] == '-') {
		return -1;
	    }
	    return at;
	}

	if (index != CPUBIND_PID) {
	    *flags |= map [index];
	    continue;
	}

	/*
	 * -pid has an argument. Check for and process it.
	 */

	at ++;
	if (at >= objc) {
	    Tcl_SetResult (interp, "Missing argument for -pid.", TCL_STATIC);
	    return -1;
	}

        if (Tcl_GetIntFromObj(interp, objv[at], pid) == TCL_ERROR) {
            return -1;
	}
    }

    return at;
}

/* vim: set sts=4 sw=4 tw=80 et ft=c: */
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
