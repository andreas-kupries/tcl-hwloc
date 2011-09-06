#include <tcl.h>
#include <hwloc.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "topology.h"
#include "membind.h"

static int      parse_flags (Tcl_Interp *interp, int *result, Tcl_Obj *obj);
static int      obj2policy  (Tcl_Interp *interp, hwloc_membind_policy_t *policy, Tcl_Obj *obj);
static Tcl_Obj* policy2obj  (Tcl_Interp *interp, hwloc_membind_policy_t policy);

/*
 * topology membind get ?-type flags? ?-cpuset? ?-pid pid?
 * topology membind set ?-type flags? ?-cpuset? ?-pid pid? nodeset
 */
int parse_membind_args(topo_data* data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    static const char* cmds[] = {
        "get", "set", NULL
    };
    enum options {
        MEMBIND_GET, MEMBIND_SET
    };

    int index;
    int objv_idx = 3;
    int pid = 0;
    int flags = 0;
    int cpuset = 0;

    if (Tcl_GetIndexFromObj(interp, objv[2], cmds, "option", 2, &index) != TCL_OK)
        return TCL_ERROR;


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

    if ((objc >= objv_idx+1) &&
	strcmp((const char *) Tcl_GetString(objv[objv_idx]), "-cpuset") == 0) {
        cpuset = 1;
        objv_idx++;
    }

    switch (index) {
    case MEMBIND_SET:
	{
	    if (objc == objv_idx+2) {
		hwloc_membind_policy_t policy = 0;
		const char *setstr = Tcl_GetString(objv[objv_idx]);
		hwloc_bitmap_t set = hwloc_bitmap_alloc();

		if (hwloc_bitmap_list_sscanf(set, setstr) == -1) {
		    Tcl_SetResult(interp, "failed to parse bitmap", TCL_STATIC);
		    hwloc_bitmap_free(set);
		    return TCL_ERROR;
		}

		if (obj2policy(interp, &policy, objv[objv_idx+1]) == TCL_ERROR) {
		    return TCL_ERROR;
		}

		if (pid) {
		    if (cpuset) {
			if (hwloc_set_proc_membind(data->topology, pid, set, policy, flags) == -1) {
			    Tcl_SetResult(interp, "hwloc_set_proc_membind() failed", TCL_STATIC);
			    hwloc_bitmap_free(set);
			    Tcl_PosixError(interp);
			    return TCL_ERROR;
			}
		    } else {
			if (hwloc_set_proc_membind_nodeset(data->topology, pid, set, policy, flags) == -1) {
			    Tcl_SetResult(interp, "hwloc_set_proc_membind_nodeset() failed", TCL_STATIC);
			    hwloc_bitmap_free(set);
			    Tcl_PosixError(interp);
			    return TCL_ERROR;
			}
		    }
		} else {
		    if (cpuset) {
			if (hwloc_set_membind(data->topology, set, policy, flags) == -1) {
			    Tcl_SetResult(interp, "hwloc_set_membind() failed", TCL_STATIC);
			    hwloc_bitmap_free(set);
			    Tcl_PosixError(interp);
			    return TCL_ERROR;
			}
		    } else {
			if (hwloc_set_membind_nodeset(data->topology, set, policy, flags) == -1) {
			    Tcl_SetResult(interp, "hwloc_set_membind_nodeset() failed", TCL_STATIC);
			    hwloc_bitmap_free(set);
			    Tcl_PosixError(interp);
			    return TCL_ERROR;
			}
		    }
		}

		hwloc_bitmap_free(set);
	    } else {
		Tcl_WrongNumArgs(interp, objv_idx, objv, "bitmap policy");
		return TCL_ERROR;
	    }
	    break;
	}
    case MEMBIND_GET:
	{
	    hwloc_bitmap_t set;
	    hwloc_membind_policy_t policy = 0;
	    char *list;
	    Tcl_Obj* lv [2];

	    if (objc > objv_idx) {
		Tcl_WrongNumArgs(interp, objv_idx, objv, NULL);
		return TCL_ERROR;
	    }

	    set = hwloc_bitmap_alloc();

	    if (pid) {
		if (cpuset) {
		    if (hwloc_get_proc_membind(data->topology, pid, set, &policy, flags) == -1) {
			Tcl_SetResult(interp, "hwloc_get_proc_membind() failed", TCL_STATIC);
			hwloc_bitmap_free(set);
			Tcl_PosixError(interp);
			return TCL_ERROR;
		    }
		} else {
		    if (hwloc_get_proc_membind_nodeset(data->topology, pid, set, &policy, flags) == -1) {
			Tcl_SetResult(interp, "hwloc_get_proc_membind_nodeset() failed", TCL_STATIC);
			hwloc_bitmap_free(set);
			Tcl_PosixError(interp);
			return TCL_ERROR;
		    }
		}
	    } else {
		if (cpuset) {
		    if (hwloc_get_membind(data->topology, set, &policy, flags) == -1) {
			Tcl_SetResult(interp, "hwloc_get_membind() failed", TCL_STATIC);
			hwloc_bitmap_free(set);
			Tcl_PosixError(interp);
			return TCL_ERROR;
		    }
		} else {
		    if (hwloc_get_membind_nodeset(data->topology, set, &policy, flags) == -1) {
			Tcl_SetResult(interp, "hwloc_get_membind_nodeset() failed", TCL_STATIC);
			hwloc_bitmap_free(set);
			Tcl_PosixError(interp);
			return TCL_ERROR;
		    }
		}
	    }

	    if (hwloc_bitmap_list_asprintf(&list, set) == -1) {
		Tcl_SetResult(interp, "hwloc_bitmap_list_asprintf() failed", TCL_STATIC);
		hwloc_bitmap_free(set);
		return TCL_ERROR;
	    }

	    lv [0] = Tcl_NewStringObj (list, -1);
	    lv [1] = policy2obj(interp, policy);

	    Tcl_SetObjResult(interp, Tcl_NewListObj(2, lv));

	    hwloc_bitmap_free(set);
	    free(list);
	    break;
	}
    }

    return TCL_OK;
}

static int
parse_flags (Tcl_Interp *interp, int *result, Tcl_Obj *obj) {
    static const char* flags[] = {
        "migrate", "nocpubind",
	"process", "strict",
	"thread",
        NULL
    };
    enum options {
        MEMBIND_MIGRATE, MEMBIND_NOCPUBIND,
	MEMBIND_PROCESS, MEMBIND_STRICT,
	MEMBIND_THREAD
    };

    int i, index;

    Tcl_Obj **obj_objv;
    int       obj_objc;

    if (Tcl_ListObjGetElements(interp, obj, &obj_objc, &obj_objv) == TCL_ERROR) {
        Tcl_SetResult(interp, "parsing flags failed", TCL_STATIC);
        return TCL_ERROR;
    }

    for (i = 0; i < obj_objc; i++) {  
        if (Tcl_GetIndexFromObj(interp, obj_objv[i], flags, "flag", 0, &index) != TCL_OK) {
            return TCL_ERROR;
	}
	*result |= index;
    }

    return TCL_OK;
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
