#include <tcl.h>
#include <hwloc.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "topology.h"
#include "membind.h"

static int parse_flags (Tcl_Interp *interp, int *result, Tcl_Obj *obj);
static int obj2policy (Tcl_Interp *interp, hwloc_membind_policy_t *policy, Tcl_Obj *obj);
static Tcl_Obj *policy2obj (Tcl_Interp *interp, hwloc_membind_policy_t policy);

/* topology membind get ?-type flags? ?-cpuset? ?-pid pid? */
/* topology membind set ?-type flags? ?-cpuset? ?-pid pid? nodeset */
int parse_membind_args(struct topo_data *data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    static const char* cmds[] = {
        "set",
        "get",
        NULL
    };
    enum options {
        MEMBIND_SET,
        MEMBIND_GET,
    };
    int index;

    if (Tcl_GetIndexFromObj(interp, objv[2], cmds, "option", 2, &index) != TCL_OK)
        return TCL_ERROR;

    int objv_idx = 3;

    int pid = 0;
    if (objc >= objv_idx+2 && strcmp((const char *) Tcl_GetString(objv[objv_idx]), "-pid") == 0) {
        if (Tcl_GetIntFromObj(interp, objv[objv_idx+1], &pid) == TCL_ERROR) 
            return TCL_ERROR;
        objv_idx += 2;
    }

    int flags = 0;
    if (objc >= objv_idx+2 && strcmp((const char *) Tcl_GetString(objv[objv_idx]), "-type") == 0) {
        int res = 0;
        if (parse_flags(interp, &res, objv[objv_idx+1]) == TCL_ERROR) 
            return TCL_ERROR;
        objv_idx += 2;
    }

    int cpuset = 0;
    if (objc >= objv_idx+1 && strcmp((const char *) Tcl_GetString(objv[objv_idx]), "-cpuset") == 0) {
        cpuset = 1;
        objv_idx++;
    }

    switch (index) {
        case MEMBIND_SET:
            {
                if (objc == objv_idx+2) {
                    const char *setstr = Tcl_GetString(objv[objv_idx]);
                    hwloc_bitmap_t set = hwloc_bitmap_alloc();
                    if (hwloc_bitmap_list_sscanf(set, setstr) == -1) {
                        Tcl_SetResult(interp, "failed to parse bitmap", TCL_STATIC);
                        hwloc_bitmap_free(set);
                        return TCL_ERROR;
                    }

                    hwloc_membind_policy_t policy = 0;
                    if (obj2policy(interp, &policy, objv[objv_idx+1]) == TCL_ERROR) 
                        return TCL_ERROR;

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
                if (objc > objv_idx) {
                    Tcl_WrongNumArgs(interp, objv_idx, objv, NULL);
                    return TCL_ERROR;
                }

                hwloc_bitmap_t set = hwloc_bitmap_alloc();
                hwloc_membind_policy_t policy = 0;

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

                char *list;
                if (hwloc_bitmap_list_asprintf(&list, set) == -1) {
                    Tcl_SetResult(interp, "hwloc_bitmap_list_asprintf() failed", TCL_STATIC);
                    hwloc_bitmap_free(set);
                    return TCL_ERROR;
                }

                Tcl_Obj *objPtr1 = Tcl_NewStringObj(list, -1);
                Tcl_Obj *objPtr2 = policy2obj(interp, policy);

                Tcl_Obj *listPtr = Tcl_NewListObj(0, NULL);
                Tcl_ListObjAppendElement(interp, listPtr, objPtr1);
                Tcl_ListObjAppendElement(interp, listPtr, objPtr2);
                Tcl_SetObjResult(interp, listPtr);

                hwloc_bitmap_free(set);
                free(list);
                break;
            }
    }

    return TCL_OK;
}

static int parse_flags(Tcl_Interp *interp, int *result, Tcl_Obj *obj) {
    static const char* flags[] = {
        "process",
        "thread",
        "strict",
        "migrate",
        "nocpubind",
        NULL
    };
    enum options {
        MEMBIND_PROCESS,
        MEMBIND_THREAD,
        MEMBIND_STRICT,
        MEMBIND_MIGRATE,
        MEMBIND_NOCPUBIND
    };
    int index;

    Tcl_Obj **obj_objv;
    int obj_objc;
    if (Tcl_ListObjGetElements(interp, obj, &obj_objc, &obj_objv) == TCL_ERROR) {
        Tcl_SetResult(interp, "parsing flags failed", TCL_STATIC);
        return TCL_ERROR;
    }

    int i;
    for (i = 0; i < obj_objc; i++) {  
        if (Tcl_GetIndexFromObj(interp, obj_objv[i], flags, "flag", 0, &index) != TCL_OK)
            return TCL_ERROR;

        switch(index) {
            case MEMBIND_PROCESS: { *result |= HWLOC_MEMBIND_PROCESS; break; }
            case MEMBIND_THREAD: { *result |= HWLOC_MEMBIND_THREAD; break; }
            case MEMBIND_STRICT: { *result |= HWLOC_MEMBIND_STRICT; break; }
            case MEMBIND_MIGRATE: { *result |= HWLOC_MEMBIND_MIGRATE; break; }
            case MEMBIND_NOCPUBIND: { *result |= HWLOC_MEMBIND_NOCPUBIND; break; }
            default:
            {
                Tcl_SetResult(interp, "unrecognized flag", TCL_STATIC);
                return TCL_ERROR;
            }
        }
    }

    return TCL_OK;
}

static int obj2policy (Tcl_Interp *interp, hwloc_membind_policy_t *policy, Tcl_Obj *obj) {
    static const char* flags[] = {
        "default",
        "firsttouch",
        "bind",
        "interleave",
        "replicate",
        "nexttouch",
        "mixed",
        NULL
    };
    enum options {
        POLICY_MEMBIND_DEFAULT,
        POLICY_MEMBIND_FIRSTTOUCH,
        POLICY_MEMBIND_BIND,
        POLICY_MEMBIND_INTERLEAVE,
        POLICY_MEMBIND_REPLICATE,
        POLICY_MEMBIND_NEXTTOUCH,
        POLICY_MEMBIND_MIXED
    };
    int index;

    if (Tcl_GetIndexFromObj(interp, obj, flags, "policy", 0, &index) != TCL_OK)
        return TCL_ERROR;

    switch(index) {
        case POLICY_MEMBIND_DEFAULT: { *policy = HWLOC_MEMBIND_DEFAULT; break; }
        case POLICY_MEMBIND_FIRSTTOUCH: { *policy = HWLOC_MEMBIND_FIRSTTOUCH; break; }
        case POLICY_MEMBIND_BIND: { *policy = HWLOC_MEMBIND_BIND; break; }
        case POLICY_MEMBIND_INTERLEAVE: { *policy = HWLOC_MEMBIND_INTERLEAVE; break; }
        case POLICY_MEMBIND_REPLICATE: { *policy = HWLOC_MEMBIND_REPLICATE; break; }
        case POLICY_MEMBIND_NEXTTOUCH: { *policy = HWLOC_MEMBIND_NEXTTOUCH; break; }
        case POLICY_MEMBIND_MIXED: { *policy = HWLOC_MEMBIND_MIXED; break; }
        default:
        {
           Tcl_SetResult(interp, "unrecognized policy", TCL_STATIC);
           return TCL_ERROR;
       }
    }

    return TCL_OK;
}

static Tcl_Obj *policy2obj (Tcl_Interp *interp, hwloc_membind_policy_t policy) {
    Tcl_Obj *obj;

    switch(policy) {
        case HWLOC_MEMBIND_DEFAULT: { obj = Tcl_NewStringObj("default", -1); break; }
        case HWLOC_MEMBIND_FIRSTTOUCH: { obj = Tcl_NewStringObj("firsttouch", -1); break; }
        case HWLOC_MEMBIND_BIND: { obj = Tcl_NewStringObj("bind", -1); break; }
        case HWLOC_MEMBIND_INTERLEAVE: { obj = Tcl_NewStringObj("interleave", -1); break; }
        case HWLOC_MEMBIND_REPLICATE: { obj = Tcl_NewStringObj("replicate", -1); break; }
        case HWLOC_MEMBIND_NEXTTOUCH: { obj = Tcl_NewStringObj("nexttouch", -1); break; }
        case HWLOC_MEMBIND_MIXED: { obj = Tcl_NewStringObj("mixed", -1); break; }
        default: { obj = Tcl_NewStringObj("error", -1); break; }
    }

    return obj;
}
