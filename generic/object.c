#include <tcl.h>
#include <hwloc.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "topology.h"
#include "object.h"

/* topology object id ... */
int parse_object_args(topo_data* data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[], hwloc_obj_t obj) {
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
		Tcl_AppendElement(interp, (const char *) Tcl_GetString(listPtr));
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
	    /* XXX There can be an object array here. */
	    int len = hwloc_obj_cpuset_snprintf(NULL, 0, 1, &obj);
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

/* vim: set sts=4 sw=4 tw=80 et ft=c: */
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
