#include <hwloc.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "topology.h"
#include "object.h"
#include "cpubind.h"
#include "membind.h"

static int parse_cpuset_args  (topo_data* data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
static int parse_nodeset_args (topo_data* data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
static int parse_convert_args (topo_data* data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);

int TopologyCmd (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    topo_data* data = (struct topo_data *) clientData;

    static const char* cmds[] = {
        "destroy",
        "export",
        "depth",
        "type",
        "width",
        "local",
        "root",
        "object_by",
        "object",
        "cpubind",
        "membind",
        "cpuset",
        "nodeset",
        "convert",
        NULL
    };
    enum options {
        TOPO_DESTROY,
        TOPO_EXPORT,
        TOPO_DEPTH,
        TOPO_TYPE,
        TOPO_WIDTH,
        TOPO_LOCAL,
        TOPO_ROOT,
        TOPO_OBJECT_BY,
        TOPO_OBJECT,
        TOPO_CPUBIND,
        TOPO_MEMBIND,
        TOPO_CPUSET,
        TOPO_NODESET,
        TOPO_CONVERT
    };
    int index;

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "option ?arg? ...");
        return TCL_ERROR;
    }

    if (Tcl_GetIndexFromObj(interp, objv[1], cmds, "option", 0, &index) != TCL_OK)
        return TCL_ERROR;

    switch (index) {
    case TOPO_DESTROY: /* destory */
        {
            if (objc > 2) {
                Tcl_WrongNumArgs(interp, 2, objv, NULL);
                return TCL_ERROR;
            }
            Tcl_DeleteCommandFromToken(interp, data->cmdtoken);
            break;
        }
    case TOPO_EXPORT: /* export filename */
        {
            if (objc != 3) {
                Tcl_WrongNumArgs(interp, 2, objv, "filename");
                return TCL_ERROR;
            }
            hwloc_topology_export_xml(data->topology, (const char *) Tcl_GetString(objv[2]));
            break;
        }
    case TOPO_DEPTH: /* depth ?-type type? */
        {
            if (objc == 2) {
                Tcl_Obj *objPtr = Tcl_NewIntObj((int) hwloc_topology_get_depth(data->topology));
                Tcl_SetObjResult(interp, objPtr);
            } else if (objc == 4 && strcmp((const char *) Tcl_GetString(objv[2]), "-type") == 0) {
                hwloc_obj_type_t type = hwloc_obj_type_of_string((const char *) Tcl_GetString(objv[3]));
                if (type == -1) {
                    Tcl_SetResult(interp, "unrecognized object type", TCL_STATIC);
                    return TCL_ERROR;
                }

                Tcl_Obj *objPtr = Tcl_NewIntObj((int) hwloc_get_type_depth(data->topology, type));
                Tcl_SetObjResult(interp, objPtr);
            } else {
                Tcl_WrongNumArgs(interp, 2, objv, "?-type value?");
                return TCL_ERROR;
            }
            break;
        }
    case TOPO_TYPE: /* type -depth depth */
        {
            if (objc != 4 || strcmp((const char *) Tcl_GetString(objv[2]), "-depth")) {
                Tcl_WrongNumArgs(interp, 2, objv, "-depth value");
                return TCL_ERROR;
            } else {
                int depth = 0;
                if (Tcl_GetIntFromObj(interp, objv[3], &depth) == TCL_ERROR) 
                    return TCL_ERROR;

                if (depth < 0 || depth >= (int) hwloc_topology_get_depth(data->topology)) {
                    Tcl_SetResult(interp, "depth out of range", TCL_STATIC);
                    return TCL_ERROR;
                }

                hwloc_obj_type_t type = hwloc_get_depth_type(data->topology, (unsigned int) depth);
                Tcl_Obj *objPtr = Tcl_NewStringObj(hwloc_obj_type_string(type), -1);
                Tcl_SetObjResult(interp, objPtr);
            }
            break;
        }
    case TOPO_WIDTH: /* width {-depth depth | -type type} */
        {
            if (objc == 4 && strcmp((const char *) Tcl_GetString(objv[2]), "-depth") == 0) {
                int depth = 0;
                if (Tcl_GetIntFromObj(interp, objv[3], &depth) == TCL_ERROR) 
                    return TCL_ERROR;

                if (depth < 0 || depth >= (int) hwloc_topology_get_depth(data->topology)) {
                    Tcl_SetResult(interp, "depth out of range", TCL_STATIC);
                    return TCL_ERROR;
                }

                Tcl_Obj *objPtr = Tcl_NewIntObj((int) hwloc_get_nbobjs_by_depth(data->topology, (unsigned int) depth));
                Tcl_SetObjResult(interp, objPtr);
            } else if (objc == 4 && strcmp((const char *) Tcl_GetString(objv[2]), "-type") == 0) {
                hwloc_obj_type_t type = hwloc_obj_type_of_string((const char *) Tcl_GetString(objv[3]));
                if (type == -1) {
                    Tcl_SetResult(interp, "unrecognized object type", TCL_STATIC);
                    return TCL_ERROR;
                }

                Tcl_Obj *objPtr = Tcl_NewIntObj((int) hwloc_get_nbobjs_by_type(data->topology, type));
                Tcl_SetObjResult(interp, objPtr);
            } else {
                Tcl_WrongNumArgs(interp, 2, objv, "{-depth value | -type value}");
                return TCL_ERROR;
            }
            break;
        }
    case TOPO_LOCAL: /* local */
        {
            if (objc > 2) {
                Tcl_WrongNumArgs(interp, 2, objv, NULL);
                return TCL_ERROR;
            }
            Tcl_Obj *objPtr = Tcl_NewIntObj((int) hwloc_topology_is_thissystem(data->topology));
            Tcl_SetObjResult(interp, objPtr);
            break;
        }
    case TOPO_ROOT: /* root */
        {
            if (objc > 2) {
                Tcl_WrongNumArgs(interp, 2, objv, NULL);
                return TCL_ERROR;
            }
            hwloc_obj_t obj = hwloc_get_root_obj(data->topology);
            Tcl_Obj *depthPtr = Tcl_NewIntObj((int) obj->depth);
            Tcl_Obj *indexPtr = Tcl_NewIntObj((int) obj->logical_index);
            Tcl_Obj *listPtr = Tcl_NewListObj(0, NULL);
            Tcl_ListObjAppendElement(interp, listPtr, depthPtr);
            Tcl_ListObjAppendElement(interp, listPtr, indexPtr);
            Tcl_SetObjResult(interp, listPtr);
            break;
        }
    case TOPO_OBJECT_BY: /* object_by {-depth value|-type value} index */
        {
            if (objc != 5) {
                Tcl_WrongNumArgs(interp, 2, objv, "{-depth value|-type value} index");
                return TCL_ERROR;
            }

            hwloc_obj_t obj = NULL;
            if (strcmp((const char *) Tcl_GetString(objv[2]), "-depth") == 0) {
                int depth = 0;
                if (Tcl_GetIntFromObj(interp, objv[3], &depth) == TCL_ERROR) 
                    return TCL_ERROR;
                int idx = 0;
                if (Tcl_GetIntFromObj(interp, objv[4], &idx) == TCL_ERROR) 
                    return TCL_ERROR;

                obj = hwloc_get_obj_by_depth (data->topology, (unsigned) depth, (unsigned) idx);
                if (obj == NULL) {
                    Tcl_SetResult(interp, "object does not exist", TCL_STATIC);
                    return TCL_ERROR;
                }
            } else if (strcmp((const char *) Tcl_GetString(objv[2]), "-type") == 0) {
                hwloc_obj_type_t type = hwloc_obj_type_of_string((const char *) Tcl_GetString(objv[3]));
                if (type == -1) {
                    Tcl_SetResult(interp, "unrecognized object type", TCL_STATIC);
                    return TCL_ERROR;
                }
                int idx = 0;
                if (Tcl_GetIntFromObj(interp, objv[4], &idx) == TCL_ERROR) 
                    return TCL_ERROR;

                obj = hwloc_get_obj_by_type(data->topology, type, (unsigned) idx);
                if (obj == NULL) {
                    Tcl_SetResult(interp, "object does not exist", TCL_STATIC);
                    return TCL_ERROR;
                }
            } else {
                Tcl_SetResult(interp, "must specify -depth or -type", TCL_STATIC);
                return TCL_ERROR;
            }

            Tcl_Obj *depthPtr = Tcl_NewIntObj((int) obj->depth);
            Tcl_Obj *indexPtr = Tcl_NewIntObj((int) obj->logical_index);
            Tcl_Obj *listPtr = Tcl_NewListObj(0, NULL);
            Tcl_ListObjAppendElement(interp, listPtr, depthPtr);
            Tcl_ListObjAppendElement(interp, listPtr, indexPtr);
            Tcl_SetObjResult(interp, listPtr);
            break;
        }
    case TOPO_OBJECT: /* object id ... */
        {
            if (objc < 4) {
                Tcl_WrongNumArgs(interp, 2, objv, "object args..");
                return TCL_ERROR;
            }
            Tcl_Obj **obj_objv;
            int obj_objc;
            if (Tcl_ListObjGetElements(interp, objv[2], &obj_objc, &obj_objv) == TCL_ERROR || obj_objc != 2) {
                Tcl_SetResult(interp, "invalid object id", TCL_STATIC);
                return TCL_ERROR;
            }
            
            int depth = 0;
            if (Tcl_GetIntFromObj(interp, obj_objv[0], &depth) == TCL_ERROR) 
                return TCL_ERROR;

            int idx = 0;
            if (Tcl_GetIntFromObj(interp, obj_objv[1], &idx) == TCL_ERROR) 
                return TCL_ERROR;

            hwloc_obj_t obj = hwloc_get_obj_by_depth(data->topology, (unsigned) depth, (unsigned) idx);
            if (obj == NULL) {
                Tcl_SetResult(interp, "object does not exist", TCL_STATIC);
                return TCL_ERROR;
            }

            return parse_object_args(data, interp, objc, objv, obj);
            break;
        }
    case TOPO_CPUBIND: /* cpubind ...*/
        {
            if (objc < 3) {
                Tcl_WrongNumArgs(interp, 2, objv, "args..");
                return TCL_ERROR;
            }

            return parse_cpubind_args(data, interp, objc, objv);
            break;
        }
    case TOPO_MEMBIND: /* membind ...*/
        {
            if (objc < 3) {
                Tcl_WrongNumArgs(interp, 2, objv, "args..");
                return TCL_ERROR;
            }

            return parse_membind_args(data, interp, objc, objv);
            break;
        }
    case TOPO_CPUSET: /* cpuset...*/
        {
            if (objc < 3) {
                Tcl_WrongNumArgs(interp, 2, objv, "args..");
                return TCL_ERROR;
            }

            return parse_cpuset_args(data, interp, objc, objv);
            break;
        }
    case TOPO_NODESET: /* nodeset...*/
        {
            if (objc < 3) {
                Tcl_WrongNumArgs(interp, 2, objv, "args..");
                return TCL_ERROR;
            }

            return parse_nodeset_args(data, interp, objc, objv);
            break;
        }
    case TOPO_CONVERT: /* convert...*/
        {
            if (objc < 3) {
                Tcl_WrongNumArgs(interp, 2, objv, "args..");
                return TCL_ERROR;
            }

            return parse_convert_args(data, interp, objc, objv);
            break;
        }
    }

    return TCL_OK;
}

/* This is also executed automatically when the interpreter the command resides in, is deleted. */
void TopologyCmd_CleanUp(ClientData clientData) {
    topo_data* data = (struct topo_data *) clientData;
    hwloc_topology_destroy(data->topology);
    Tcl_DecrRefCount(data->name);
    ckfree((char *) data);
}

static int parse_cpuset_args(topo_data* data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    static const char* cmds[] = {
        "-complete",
        "-allowed",
        "-online",
        "-topology",
        NULL
    };
    enum options {
        CPUSET_COMPLETE,
        CPUSET_ALLOWED,
        CPUSET_ONLINE,
        CPUSET_TOPOLOGY
    };
    int index;

    if (Tcl_GetIndexFromObj(interp, objv[2], cmds, "option", 2, &index) != TCL_OK)
        return TCL_ERROR;

    hwloc_const_cpuset_t set = 0;

    switch (index) { 
    case CPUSET_COMPLETE: 
	{ 
	    set = hwloc_topology_get_complete_cpuset(data->topology);
	    break;
	}
    case CPUSET_ALLOWED: 
	{ 
	    set = hwloc_topology_get_allowed_cpuset(data->topology);
	    break;
	}
    case CPUSET_ONLINE: 
	{ 
	    set = hwloc_topology_get_online_cpuset(data->topology);
	    break;
	}
    case CPUSET_TOPOLOGY: 
	{ 
	    set = hwloc_topology_get_topology_cpuset(data->topology);
	    break;
	}
    }

    char *list;
    if (hwloc_bitmap_list_asprintf(&list, set) == -1) {
        Tcl_SetResult(interp, "hwloc_bitmap_list_asprintf() failed", TCL_STATIC);
        return TCL_ERROR;
    }

    Tcl_Obj *objPtr = Tcl_NewStringObj(list, -1);
    Tcl_SetObjResult(interp, objPtr);

    free(list);
    return TCL_OK;
}

static int parse_nodeset_args(topo_data* data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    static const char* cmds[] = {
        "-complete",
        "-allowed",
        "-topology",
        NULL
    };
    enum options {
        NODESET_COMPLETE,
        NODESET_ALLOWED,
        NODESET_TOPOLOGY
    };
    int index;

    if (Tcl_GetIndexFromObj(interp, objv[2], cmds, "option", 2, &index) != TCL_OK)
        return TCL_ERROR;

    hwloc_const_nodeset_t set = 0;

    switch (index) { 
    case NODESET_COMPLETE: 
	{ 
	    set = hwloc_topology_get_complete_nodeset(data->topology);
	    break;
	}
    case NODESET_ALLOWED: 
	{ 
	    set = hwloc_topology_get_allowed_nodeset(data->topology);
	    break;
	}
    case NODESET_TOPOLOGY: 
	{ 
	    set = hwloc_topology_get_topology_nodeset(data->topology);
	    break;
	}
    }

    char *list;
    if (hwloc_bitmap_list_asprintf(&list, set) == -1) {
        Tcl_SetResult(interp, "hwloc_bitmap_list_asprintf() failed", TCL_STATIC);
        return TCL_ERROR;
    }

    Tcl_Obj *objPtr = Tcl_NewStringObj(list, -1);
    Tcl_SetObjResult(interp, objPtr);

    free(list);
    return TCL_OK;
}

static int parse_convert_args(topo_data* data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    static const char* cmds[] = {
        "-to_cpuset",
        "-to_nodeset",
        NULL
    };
    enum options {
        CONVERT_TO_CPUSET,
        CONVERT_TO_NODESET
    };
    int index;

    int strict = 0;
    int offset = 0;
    if (objc == 5 && strcmp((const char *) Tcl_GetString(objv[2]), "-strict") == 0) {
        strict = 1;
        offset++;
    } else if (objc == 4) {
    } else {
        Tcl_WrongNumArgs(interp, 2, objv, "?-strict? -to_cpuset|-to_nodeset bitmap");
        return TCL_ERROR;
    }

    if (Tcl_GetIndexFromObj(interp, objv[2+offset], cmds, "option", 2, &index) != TCL_OK)
        return TCL_ERROR;

    const char *setstr = Tcl_GetString(objv[3+offset]);
    hwloc_bitmap_t from_set = hwloc_bitmap_alloc();
    if (hwloc_bitmap_list_sscanf(from_set, setstr) == -1) {
        Tcl_SetResult(interp, "failed to parse bitmap", TCL_STATIC);
        hwloc_bitmap_free(from_set);
        return TCL_ERROR;
    }

    hwloc_bitmap_t to_set = hwloc_bitmap_alloc();

    switch (index) { 
    case CONVERT_TO_CPUSET: 
	{ 
	    if (strict)
		hwloc_cpuset_from_nodeset_strict(data->topology, to_set, from_set);
	    else
		hwloc_cpuset_from_nodeset(data->topology, to_set, from_set);
	    break;
	}
    case CONVERT_TO_NODESET: 
	{ 
	    if (strict)
		hwloc_cpuset_to_nodeset_strict(data->topology, from_set, to_set);
	    else
		hwloc_cpuset_to_nodeset(data->topology, from_set, to_set);
	    break;
	}
    }

    char *list;
    if (hwloc_bitmap_list_asprintf(&list, to_set) == -1) {
        Tcl_SetResult(interp, "hwloc_bitmap_list_asprintf() failed", TCL_STATIC);
        hwloc_bitmap_free(to_set);
        hwloc_bitmap_free(from_set);
        return TCL_ERROR;
    }

    Tcl_Obj *objPtr = Tcl_NewStringObj(list, -1);
    Tcl_SetObjResult(interp, objPtr);

    hwloc_bitmap_free(to_set);
    hwloc_bitmap_free(from_set);
    free(list);

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
