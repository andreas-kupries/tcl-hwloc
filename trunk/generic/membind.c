#include <tcl.h>
#include <hwloc.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "topology.h"
#include "membind.h"
#include "bitmap.h"

static int      parse_options (Tcl_Interp* interp, int objc, Tcl_Obj* CONST objv[], int* pid, int* cpu, int* flags);
static int      obj2policy    (Tcl_Interp *interp, hwloc_membind_policy_t *policy, Tcl_Obj *obj);
static Tcl_Obj* policy2obj    (Tcl_Interp *interp, hwloc_membind_policy_t policy);

/*
 * topology membind get ?-type flags? ?-cpuset? ?-pid pid?
 * topology membind set ?-type flags? ?-cpuset? ?-pid pid? nodeset
 */
int
parse_membind_args(topo_data* data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    static const char* cmds[] = {
        "get", "set", NULL
    };
    enum options {
        MEMBIND_GET, MEMBIND_SET
    };

    hwloc_bitmap_t         thenodeset;
    hwloc_membind_policy_t policy = 0;

    int index, res, at;
    int pid = 0;
    int flags = 0;
    int cpu = 0;
    Tcl_Obj* lv [2];

    if (Tcl_GetIndexFromObj(interp, objv[2], cmds, "option", 2, &index) != TCL_OK)
        return TCL_ERROR;

    at = parse_options (interp, objc, objv, &pid, &cpu, &flags);
    if (at < 0) {
	return TCL_ERROR;
    }

    switch (index) {
    case MEMBIND_SET:
	if (at != (objc-2)) {
	    Tcl_WrongNumArgs(interp, 3, objv, "?options? nodeset policy");
	    return TCL_ERROR;
	}

	if (obj2policy(interp, &policy, objv[objc-1]) == TCL_ERROR) {
	    return TCL_ERROR;
	}

	thenodeset = thwl_get_bitmap (interp, objv[objc-2]);
	if (thenodeset == NULL) {
	    return TCL_ERROR;
	}

	if (pid) {
	    if (cpu) {
		res = hwloc_set_proc_membind(data->topology, pid, thenodeset, policy, flags);
	    } else {
		res = hwloc_set_proc_membind_nodeset(data->topology, pid, thenodeset, policy, flags);
	    }
	} else {
	    if (cpu) {
		res = hwloc_set_membind(data->topology, thenodeset, policy, flags);
	    } else {
		res = hwloc_set_membind_nodeset(data->topology, thenodeset, policy, flags);
	    }
	}

	hwloc_bitmap_free(thenodeset);

	if (res == -1) goto bind_error;
	break;

    case MEMBIND_GET:
	if (at != (objc-1)) {
	    Tcl_WrongNumArgs(interp, 3, objv, NULL);
	    return TCL_ERROR;
	}

	thenodeset = hwloc_bitmap_alloc();

	if (pid) {
	    if (cpu) {
		res = hwloc_get_proc_membind(data->topology, pid, thenodeset, &policy, flags);
	    } else {
		res = hwloc_get_proc_membind_nodeset(data->topology, pid, thenodeset, &policy, flags);
	    }
	} else {
	    if (cpu) {
		res = hwloc_get_membind(data->topology, thenodeset, &policy, flags);
	    } else {
		res = hwloc_get_membind_nodeset(data->topology, thenodeset, &policy, flags);
	    }
	}

	hwloc_bitmap_free(thenodeset);

	if (res == -1) {
	    hwloc_bitmap_free(thenodeset);
	    goto bind_error;
	}

	if (thwl_set_result_bitmap (interp, thenodeset) != TCL_OK) {
	    return TCL_ERROR;
	}

	lv [0] = Tcl_GetObjResult (interp);
	lv [1] = policy2obj(interp, policy);

	Tcl_SetObjResult(interp, Tcl_NewListObj(2, lv));
	break;
    }

    return TCL_OK;
 bind_error:
    Tcl_PosixError(interp);

    if (errno == ENOSYS) {
	Tcl_SetResult(interp, "operation not supported", TCL_STATIC);
    } else if (errno == EXDEV) {
	Tcl_SetResult(interp, "binding not enforceable", TCL_STATIC);
    } else {
	Tcl_SetResult(interp, "general NUMA binding failure", TCL_STATIC);
    }
    return TCL_ERROR;
}

static int
parse_options (Tcl_Interp* interp, int objc, Tcl_Obj* CONST objv[], int* pid, int* cpu, int* flags)
{
    static const char* options[] = {
	"-cpu",     "-migrate", "-nocpubind", "-pid",
	"-process", "-strict",  "-thread",
        NULL
    };
    enum options {
        MEMBIND_CPU,     MEMBIND_MIGRATE, MEMBIND_NOCPUBIND, MEMBIND_PID,
	MEMBIND_PROCESS, MEMBIND_STRICT,  MEMBIND_THREAD
    };
    static int map [] = {
	0,
	HWLOC_MEMBIND_MIGRATE,
	HWLOC_MEMBIND_NOCPUBIND,
	0,
	HWLOC_MEMBIND_PROCESS,
	HWLOC_MEMBIND_STRICT,
	HWLOC_MEMBIND_THREAD
    };

    int index, at;

    *pid   = 0;
    *flags = 0;
    *cpu = 0;

    /* topo/0 membind/1 method/2 ... => at = 3 */
    for (at = 3; at < objc; at++) {
        if (Tcl_GetIndexFromObj(interp, objv[at], options, "flag", 0, &index) != TCL_OK) {
	    if(Tcl_GetString (objv[at])[0] == '-') {
		return -1;
	    }
	    return at;
	}

	if (index == MEMBIND_CPU) {
	    *cpu = 1;
	    continue;
	}

	if (index != MEMBIND_PID) {
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

static const char* policy_flags[] = {
    "bind",       "default",
    "firsttouch", "interleave",
    "mixed",      "nexttouch",
    "replicate",
    NULL
};
static int policy_map [] = {
    HWLOC_MEMBIND_BIND,       HWLOC_MEMBIND_DEFAULT,
    HWLOC_MEMBIND_FIRSTTOUCH, HWLOC_MEMBIND_INTERLEAVE,
    HWLOC_MEMBIND_MIXED,      HWLOC_MEMBIND_NEXTTOUCH,
    HWLOC_MEMBIND_REPLICATE
};

static int
obj2policy (Tcl_Interp *interp, hwloc_membind_policy_t *policy, Tcl_Obj *obj) {
    enum options {
        POLICY_MEMBIND_BIND,       POLICY_MEMBIND_DEFAULT,
	POLICY_MEMBIND_FIRSTTOUCH, POLICY_MEMBIND_INTERLEAVE,
	POLICY_MEMBIND_MIXED,      POLICY_MEMBIND_NEXTTOUCH,
	POLICY_MEMBIND_REPLICATE
    };
    int index;

    if (Tcl_GetIndexFromObj(interp, obj, policy_flags, "policy", 0, &index) != TCL_OK) {
        return TCL_ERROR;
    }

    *policy = policy_map [index];
    return TCL_OK;
}

static Tcl_Obj*
policy2obj (Tcl_Interp *interp, hwloc_membind_policy_t policy) {
    int i, n = sizeof (policy_map) / sizeof (int);

    for (i=0; i < n ; i++) {
	if (policy_map[i] == policy) break;
    }

    if (i == n) {
	return Tcl_NewStringObj ("error", -1);
    }

    return Tcl_NewStringObj (policy_flags [i], -1);
}

/* vim: set sts=4 sw=4 tw=80 et ft=c: */
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
