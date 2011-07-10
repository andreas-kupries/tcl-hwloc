#include <hwloc.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "topology.h"
#include "object.h"

static int parse_object_args(struct topo_data *data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[], hwloc_obj_t obj);
static int parse_cpubind_args(struct topo_data *data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);

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
        "cpubind",
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
        TOPO_CPUBIND
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
            hwloc_topology_export_xml(data->topology, Tcl_GetString(objv[2]));
            break;
        }
        case TOPO_DEPTH: /* depth ?-type type? */
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
        case TOPO_TYPE: /* type -depth depth */
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
        case TOPO_WIDTH: /* width {-depth depth | -type type} */
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
        "attributes",
        "cpuset",
        "info",
        NULL
    };
    enum options {
        OBJ_CHILDREN,
        OBJ_PARENT,
        OBJ_NEXT_COUSIN,
        OBJ_PREV_COUSIN,
        OBJ_NEXT_SIBLING,
        OBJ_PREV_SIBLING,
        OBJ_FIRST_CHILD,
        OBJ_LAST_CHILD,
        OBJ_TYPE,
        OBJ_NAME,
        OBJ_DEPTH,
        OBJ_LOGICAL_INDEX,
        OBJ_SIBLING_RANK,
        OBJ_ARITY,
        OBJ_ATTRIBUTES,
        OBJ_CPUSET,
        OBJ_INFO
    };
    int index;

    if (Tcl_GetIndexFromObj(interp, objv[3], cmds, "option", 2, &index) != TCL_OK)
        return TCL_ERROR;

    if (objc > 4) {
        Tcl_WrongNumArgs(interp, 4, objv, NULL);
        return TCL_ERROR;
    }

    switch (index) {
        case OBJ_CHILDREN:
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
        case OBJ_PARENT:
            {
                obj = obj->parent;
                goto return_obj;
            }
        case OBJ_NEXT_COUSIN:
            {
                obj = obj->next_cousin;
                goto return_obj;
            }
        case OBJ_PREV_COUSIN:
            {
                obj = obj->prev_cousin;
                goto return_obj;
            }
        case OBJ_NEXT_SIBLING:
            {
                obj = obj->next_sibling;
                goto return_obj;
            }
        case OBJ_PREV_SIBLING:
            {
                obj = obj->prev_sibling;
                goto return_obj;
            }
        case OBJ_FIRST_CHILD:
            {
                obj = obj->first_child;
                goto return_obj;
            }
        case OBJ_LAST_CHILD:
            {
                obj = obj->last_child;
                goto return_obj;
            }
        case OBJ_TYPE:
            {
                Tcl_Obj *objPtr = Tcl_NewStringObj(hwloc_obj_type_string(obj->type), -1);
                Tcl_SetObjResult(interp, objPtr);
                break;
            }
        case OBJ_NAME:
            {
                Tcl_Obj *objPtr = Tcl_NewStringObj(obj->name, -1);
                Tcl_SetObjResult(interp, objPtr);
                break;
            }
        case OBJ_DEPTH:
            {
                Tcl_Obj *objPtr = Tcl_NewIntObj(obj->depth);
                Tcl_SetObjResult(interp, objPtr);
                break;
            }
        case OBJ_LOGICAL_INDEX:
            {
                Tcl_Obj *objPtr = Tcl_NewIntObj(obj->logical_index);
                Tcl_SetObjResult(interp, objPtr);
                break;
            }
        case OBJ_SIBLING_RANK:
            {
                Tcl_Obj *objPtr = Tcl_NewIntObj(obj->sibling_rank);
                Tcl_SetObjResult(interp, objPtr);
                break;
            }
        case OBJ_ARITY:
            {
                Tcl_Obj *objPtr = Tcl_NewIntObj(obj->arity);
                Tcl_SetObjResult(interp, objPtr);
                break;
            }
        case OBJ_ATTRIBUTES:
            {
                int len = hwloc_obj_attr_snprintf(NULL, 0, obj, NULL, 1);
                char buffer[len+2];
                hwloc_obj_attr_snprintf(buffer, len+1, obj, " ", 1);
                Tcl_Obj *objPtr = Tcl_NewStringObj(buffer, -1);
                Tcl_SetObjResult(interp, objPtr);
                break;
            }
        case OBJ_CPUSET:
            {
                int len = hwloc_obj_cpuset_snprintf(NULL, 0, 1, &obj); // XXX There can be an object array here.
                char buffer[len+2];
                hwloc_obj_cpuset_snprintf(buffer, len+1, 1, &obj);
                Tcl_Obj *objPtr = Tcl_NewStringObj(buffer, -1);
                Tcl_SetObjResult(interp, objPtr);
                break;
            }
        case OBJ_INFO:
            {
                Tcl_Obj *resultPtr = Tcl_NewListObj(0, NULL);
                int i;
                for (i = 0; i < obj->infos_count; i++) {
                    Tcl_Obj *tuplePtr = Tcl_NewListObj(0, NULL);
                    Tcl_Obj *namePtr = Tcl_NewStringObj(obj->infos[i].name, -1);
                    Tcl_Obj *valuePtr = Tcl_NewStringObj(obj->infos[i].value, -1);
                    Tcl_ListObjAppendElement(interp, tuplePtr, namePtr);
                    Tcl_ListObjAppendElement(interp, tuplePtr, valuePtr);
                    Tcl_ListObjAppendElement(interp, resultPtr, tuplePtr);
                }
                Tcl_SetObjResult(interp, resultPtr);
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

/* topology cpubind ... */
static int parse_cpubind_args(struct topo_data *data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    static const char* cmds[] = {
        "set",
        "get",
        "last",
        NULL
    };
    enum options {
        CPUBIND_SET,
        CPUBIND_GET,
        CPUBIND_LAST
    };
    int index;

    if (Tcl_GetIndexFromObj(interp, objv[2], cmds, "option", 2, &index) != TCL_OK)
        return TCL_ERROR;

    switch (index) {
        case CPUBIND_SET:
            {
                if (objc == 3) {
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
        case CPUBIND_GET:
            {
                hwloc_cpuset_t cpuset;
                hwloc_get_cpubind(data->topology, cpuset, 0); // XXX returns int + flags

                char *list;
                hwloc_bitmap_list_asprintf(&list, cpuset); // XXX returns int

                Tcl_Obj *objPtr = Tcl_NewStringObj(list, -1);
                Tcl_SetObjResult(interp, objPtr);
                break;
            }
        case CPUBIND_LAST:
            {
                break;
            }
    }

    return TCL_OK;
}
