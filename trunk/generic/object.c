#include <tcl.h>
#include <hwloc.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "topology.h"
#include "object.h"

/*
 * topology object id ...
 */
int
parse_object_args (topo_data* data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[],
		   hwloc_obj_t obj)
{
    static const char* cmds[] = {
	"arity",          "attributes", "children",     "cpuset",
	"depth",         "first_child", "info",         "last_child",
	"logical_index", "name",        "next_cousin",  "next_sibling",
	"parent",        "prev_cousin", "prev_sibling", "sibling_rank",
        "type",
        NULL
    };
    enum options {
	ELEM_ARITY,         ELEM_ATTRIBUTES,  ELEM_CHILDREN,     ELEM_CPUSET,
	ELEM_DEPTH,         ELEM_FIRST_CHILD, ELEM_INFO,         ELEM_LAST_CHILD,
	ELEM_LOGICAL_INDEX, ELEM_NAME,        ELEM_NEXT_COUSIN,  ELEM_NEXT_SIBLING,
	ELEM_PARENT,        ELEM_PREV_COUSIN, ELEM_PREV_SIBLING, ELEM_SIBLING_RANK,
	ELEM_TYPE
    };

    int index;
    Tcl_Obj* lv [2];

    if (objc > 4) {
        Tcl_WrongNumArgs(interp, 4, objv, NULL);
        return TCL_ERROR;
    }
    if (Tcl_GetIndexFromObj(interp, objv[3], cmds, "option", 2, &index) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (index) {
    case ELEM_CHILDREN:
	{
	    Tcl_Obj *resultPtr = Tcl_NewListObj(0, NULL);
	    int i;

	    for (i = 0; i < obj->arity; i++) {
		lv [0] = Tcl_NewIntObj((int) obj->children[i]->depth);
		lv [1] = Tcl_NewIntObj((int) obj->children[i]->logical_index);

		Tcl_ListObjAppendElement(interp, resultPtr,
					 Tcl_NewListObj (2, lv));
	    }
	    Tcl_SetObjResult(interp, resultPtr);
	    break;
	}
    case ELEM_PARENT:
	obj = obj->parent;
	goto return_obj;
    case ELEM_NEXT_COUSIN:
	obj = obj->next_cousin;
	goto return_obj;
    case ELEM_PREV_COUSIN:
	obj = obj->prev_cousin;
	goto return_obj;
    case ELEM_NEXT_SIBLING:
	obj = obj->next_sibling;
	goto return_obj;
    case ELEM_PREV_SIBLING:
	obj = obj->prev_sibling;
	goto return_obj;
    case ELEM_FIRST_CHILD:
	obj = obj->first_child;
	goto return_obj;
    case ELEM_LAST_CHILD:
	obj = obj->last_child;
	goto return_obj;
    case ELEM_TYPE:
	Tcl_SetObjResult(interp, Tcl_NewStringObj(hwloc_obj_type_string(obj->type), -1));
	break;
    case ELEM_NAME:
	Tcl_SetObjResult(interp, Tcl_NewStringObj (obj->name, -1));
	break;
    case ELEM_DEPTH:
	Tcl_SetObjResult(interp, Tcl_NewIntObj (obj->depth));
	break;
    case ELEM_LOGICAL_INDEX:
	Tcl_SetObjResult(interp, Tcl_NewIntObj (obj->logical_index));
	break;
    case ELEM_SIBLING_RANK:
	Tcl_SetObjResult(interp, Tcl_NewIntObj (obj->sibling_rank));
	break;
    case ELEM_ARITY:
	Tcl_SetObjResult(interp, Tcl_NewIntObj (obj->arity));
	break;
    case ELEM_ATTRIBUTES:
	{
	    int len = hwloc_obj_attr_snprintf(NULL, 0, obj, NULL, 1);
	    char* buffer = ckalloc (len+2);

	    hwloc_obj_attr_snprintf(buffer, len+1, obj, " ", 1);

	    Tcl_SetObjResult(interp, Tcl_NewStringObj(buffer, -1));
	    ckfree (buffer);
	    break;
	}
    case ELEM_CPUSET:
	{
	    /* XXX There can be an element array here. */
	    int len = hwloc_obj_cpuset_snprintf(NULL, 0, 1, &obj);
	    char* buffer = ckalloc (len+2);

	    hwloc_obj_cpuset_snprintf(buffer, len+1, 1, &obj);

	    Tcl_SetObjResult(interp, Tcl_NewStringObj(buffer, -1));
	    ckfree (buffer);
	    break;
	}
    case ELEM_INFO:
	{
	    Tcl_Obj *resultPtr = Tcl_NewListObj(0, NULL);
	    int i;

	    for (i = 0; i < obj->infos_count; i++) {
		lv [0] = Tcl_NewStringObj(obj->infos[i].name, -1);
		lv [1] = Tcl_NewStringObj(obj->infos[i].value, -1);

		Tcl_ListObjAppendElement(interp, resultPtr,
					 Tcl_NewListObj (2, lv));
	    }
	    Tcl_SetObjResult(interp, resultPtr);
	    break;
	}
    }

    return TCL_OK;

 return_obj:
    if (obj == NULL) {
        Tcl_SetResult(interp, "requested element does not exist", TCL_STATIC);
        return TCL_ERROR;
    }

    lv [0] = Tcl_NewIntObj((int) obj->depth);
    lv [1] = Tcl_NewIntObj((int) obj->logical_index);

    Tcl_SetObjResult(interp, Tcl_NewListObj(2, lv));
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
