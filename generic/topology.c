#include <hwloc.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "topology.h"
#include "object.h"
#include "convert.h"

static int parse_object_args(struct topo_data *data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[], hwloc_obj_t obj);

int TopologyCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    struct topo_data *data = (struct topo_data *) clientData;

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
        NULL
    };
    int index;

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "option ?arg? ...");
        return TCL_ERROR;
    }

    if (Tcl_GetIndexFromObj(interp, objv[1], cmds, "option", 0, &index) != TCL_OK)
        return TCL_ERROR;

    switch (index) {
        case 0: /* destory */
        {
            if (objc > 2) {
                Tcl_WrongNumArgs(interp, 2, objv, NULL);
                return TCL_ERROR;
            }
            Tcl_DeleteCommandFromToken(interp, data->cmdtoken);
            break;
        }
        case 1: /* export filename */
        {
            if (objc != 3) {
                Tcl_WrongNumArgs(interp, 2, objv, "filename");
                return TCL_ERROR;
            }
            hwloc_topology_export_xml (data->topology, Tcl_GetString(objv[2]));
            break;
        }
        case 2: /* depth ?-type type? */
        {
            if (objc == 2) {
                Tcl_Obj *objPtr = Tcl_NewIntObj((int) hwloc_topology_get_depth(data->topology));
                Tcl_SetObjResult(interp, objPtr);
            } else if (objc == 4 && strcmp(Tcl_GetString(objv[2]), "-type") == 0) {
                hwloc_obj_type_t type = hwloc_obj_type_of_string(Tcl_GetString(objv[3]));
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
        case 3: /* type -depth depth */
        {
            if (objc != 4 || strcmp(Tcl_GetString(objv[2]), "-depth")) {
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
        case 4: /* width {-depth depth | -type type} */
        {
            if (objc == 4 && strcmp(Tcl_GetString(objv[2]), "-depth") == 0) {
                int depth = 0;
                if (Tcl_GetIntFromObj(interp, objv[3], &depth) == TCL_ERROR) 
                    return TCL_ERROR;

                if (depth < 0 || depth >= (int) hwloc_topology_get_depth(data->topology)) {
                    Tcl_SetResult(interp, "depth out of range", TCL_STATIC);
                    return TCL_ERROR;
                }

                Tcl_Obj *objPtr = Tcl_NewIntObj((int) hwloc_get_nbobjs_by_depth(data->topology, (unsigned int) depth));
                Tcl_SetObjResult(interp, objPtr);
            } else if (objc == 4 && strcmp(Tcl_GetString(objv[2]), "-type") == 0) {
                hwloc_obj_type_t type = hwloc_obj_type_of_string(Tcl_GetString(objv[3]));
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
        case 5: /* local */
        {
            if (objc > 2) {
                Tcl_WrongNumArgs(interp, 2, objv, NULL);
                return TCL_ERROR;
            }
            Tcl_Obj *objPtr = Tcl_NewIntObj((int) hwloc_topology_is_thissystem(data->topology));
            Tcl_SetObjResult(interp, objPtr);
            break;
        }
        case 6: /* root */
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
        case 7: /* object_by {-depth value|-type value} index */
        {
            if (objc != 5) {
                Tcl_WrongNumArgs(interp, 2, objv, "{-depth value|-type value} index");
                return TCL_ERROR;
            }

            hwloc_obj_t obj = NULL;
            if (strcmp(Tcl_GetString(objv[2]), "-depth") == 0) {
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
            } else if (strcmp(Tcl_GetString(objv[2]), "-type") == 0) {
                hwloc_obj_type_t type = hwloc_obj_type_of_string(Tcl_GetString(objv[3]));
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
        case 8: /* object id args */
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
    }

    return TCL_OK;
}

/* This is also executed automatically when the interpreter the command resides in, is deleted. */
void TopologyCmd_CleanUp(ClientData clientData) {
    struct topo_data *data = (struct topo_data *) clientData;
    hwloc_topology_destroy(data->topology);
    Tcl_DecrRefCount(data->name);
    ckfree((char *) data);
}

/* topology object id ... */
static int parse_object_args(struct topo_data *data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[], hwloc_obj_t obj) {
    static const char* cmds[] = {
        "children",
        "parent",
        "next_cousin",
        "prev_cousin",
        "next_sibling",
        "prev_sibling",
        "first_child",
        "last_child",

        "type",
        "name",
        "depth",
        "logical_index",
        "sibling_rank",
        "arity",
        NULL
    };
    int index;

    if (Tcl_GetIndexFromObj(interp, objv[3], cmds, "option", 2, &index) != TCL_OK)
        return TCL_ERROR;

    if (objc > 4) {
        Tcl_WrongNumArgs(interp, 4, objv, NULL);
        return TCL_ERROR;
    }

    switch (index) {
        case 0: /* children */
            {
                int i;
                for (i = 0; i < obj->arity; i++) {
                    Tcl_Obj *depthPtr = Tcl_NewIntObj((int) obj->children[i]->depth);
                    Tcl_Obj *indexPtr = Tcl_NewIntObj((int) obj->children[i]->logical_index);
                    Tcl_Obj *listPtr = Tcl_NewListObj(0, NULL);
                    Tcl_ListObjAppendElement(interp, listPtr, depthPtr);
                    Tcl_ListObjAppendElement(interp, listPtr, indexPtr);
                    Tcl_AppendElement(interp, Tcl_GetString(listPtr));
                }
                break;
            }
        case 1: /* parent */
            {
                obj = obj->parent;
                goto return_obj;
            }
        case 2: /* next_cousin */
            {
                obj = obj->next_cousin;
                goto return_obj;
            }
        case 3: /* prev_cousin */
            {
                obj = obj->prev_cousin;
                goto return_obj;
            }
        case 4: /* next_sibling */
            {
                obj = obj->next_sibling;
                goto return_obj;
            }
        case 5: /* prev_sibling */
            {
                obj = obj->prev_sibling;
                goto return_obj;
            }
        case 6: /* first_child */
            {
                obj = obj->first_child;
                goto return_obj;
            }
        case 7: /* last_child */
            {
                obj = obj->last_child;
                goto return_obj;
            }
        case 8: /* type */
            {
                Tcl_Obj *objPtr = Tcl_NewStringObj(hwloc_obj_type_string(obj->type), -1);
                Tcl_SetObjResult(interp, objPtr);
                break;
            }
        case 9: /* name */
            {
                Tcl_Obj *objPtr = Tcl_NewStringObj(obj->name, -1);
                Tcl_SetObjResult(interp, objPtr);
                break;
            }
        case 10: /* depth */
            {
                Tcl_Obj *objPtr = Tcl_NewIntObj(obj->depth);
                Tcl_SetObjResult(interp, objPtr);
                break;
            }
        case 11: /* logical_index */
            {
                Tcl_Obj *objPtr = Tcl_NewIntObj(obj->logical_index);
                Tcl_SetObjResult(interp, objPtr);
                break;
            }
        case 12: /* sibling_rank */
            {
                Tcl_Obj *objPtr = Tcl_NewIntObj(obj->sibling_rank);
                Tcl_SetObjResult(interp, objPtr);
                break;
            }
        case 13: /* arity */
            {
                Tcl_Obj *objPtr = Tcl_NewIntObj(obj->arity);
                Tcl_SetObjResult(interp, objPtr);
                break;
            }
    }

    return TCL_OK;

return_obj:
    if (obj == NULL) {
        Tcl_SetResult(interp, "requested object does not exist", TCL_STATIC);
        return TCL_ERROR;
    }
    Tcl_Obj *depthPtr = Tcl_NewIntObj((int) obj->depth);
    Tcl_Obj *indexPtr = Tcl_NewIntObj((int) obj->logical_index);
    Tcl_Obj *listPtr = Tcl_NewListObj(0, NULL);
    Tcl_ListObjAppendElement(interp, listPtr, depthPtr);
    Tcl_ListObjAppendElement(interp, listPtr, indexPtr);
    Tcl_SetObjResult(interp, listPtr);
    return TCL_OK;
}
