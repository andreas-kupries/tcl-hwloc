#include <tcl.h>
#include <hwloc.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "bitmap.h"

/* hwloc bitmap option... */
int parse_bitmap_args( Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    static const char* cmds[] = {
        "empty",
        "full",
        "only",
        "allbut",
        "from_ulong",
        "to_ulong",
        "set",
        "set_range",
        "clear",
	"clear_range",
        "singlify",
        "is_set",
        "is_zero",
        "is_full",
        "first",
        "next",
        "last",
        "weight",
        "or",
        "and",
        "andnot",
        "xor",
        "not",
        "intersects",
        "is_included",
        "is_equal",
        "compare",
        "compare_first",
        NULL
    };
    enum options {
	BITMAP_EMPTY,
	BITMAP_FULL,
	BITMAP_ONLY,
	BITMAP_ALLBUT,
	BITMAP_FROM_ULONG,
	BITMAP_TO_ULONG,
	BITMAP_SET,
	BITMAP_SET_RANGE,
	BITMAP_CLEAR,
	BITMAP_CLEAR_RANGE,
	BITMAP_SINGLIFY,
	BITMAP_IS_SET,
	BITMAP_IS_ZERO,
	BITMAP_IS_FULL,
	BITMAP_FIRST,
	BITMAP_NEXT,
	BITMAP_LAST,
	BITMAP_WEIGHT,
	BITMAP_OR,
	BITMAP_AND,
	BITMAP_ANDNOT,
	BITMAP_XOR,
	BITMAP_NOT,
	BITMAP_INTERSECTS,
	BITMAP_IS_INCLUDED,
	BITMAP_IS_EQUAL,
	BITMAP_COMPARE,
	BITMAP_COMPARE_FIRST
    };
    int index;

    if (Tcl_GetIndexFromObj(interp, objv[2], cmds, "option", 2, &index) != TCL_OK)
        return TCL_ERROR;

    hwloc_bitmap_t bitmap = hwloc_bitmap_alloc();
    
    switch (index) {
        case BITMAP_EMPTY:
            {
		hwloc_bitmap_zero(bitmap);
		break;
            }
        case BITMAP_FULL:
            {
		hwloc_bitmap_fill(bitmap);
		break;
            }
	case BITMAP_ONLY:
            {
		
		break;
            }
	case BITMAP_ALLBUT:
            {
		
		break;
            }
	case BITMAP_FROM_ULONG:
            {
		
		break;
            }
	case BITMAP_TO_ULONG:
            {
		
		break;
            }
	case BITMAP_SET:
            {
		
		break;
            }
	case BITMAP_SET_RANGE:
            {
		
		break;
            }
	case BITMAP_CLEAR:
            {
		
		break;
            }
	case BITMAP_CLEAR_RANGE:
            {
		
		break;
            }
	case BITMAP_SINGLIFY:
            {
		
		break;
            }
	case BITMAP_IS_SET:
            {
		
		break;
            }
	case BITMAP_IS_ZERO:
            {
		
		break;
            }
	case BITMAP_IS_FULL:
            {
		
		break;
            }
	case BITMAP_FIRST:
            {
		
		break;
            }
	case BITMAP_NEXT:
            {
		
		break;
            }
	case BITMAP_LAST:
            {
		
		break;
            }
	case BITMAP_WEIGHT:
            {
		
		break;
            }
	case BITMAP_OR:
            {
		
		break;
            }
	case BITMAP_AND:
            {
		
		break;
            }
	case BITMAP_ANDNOT:
            {
		
		break;
            }
	case BITMAP_XOR:
            {
		
		break;
            }
	case BITMAP_NOT:
            {
		
		break;
            }
	case BITMAP_INTERSECTS:
            {
		
		break;
            }
	case BITMAP_IS_INCLUDED:
            {
		
		break;
            }
	case BITMAP_IS_EQUAL:
            {
		
		break;
            }
	case BITMAP_COMPARE:
            {
		
		break;
            }
	case BITMAP_COMPARE_FIRST:
            {
		
		break;
            }
    }

    char *list;
    if (hwloc_bitmap_list_asprintf(&list, bitmap) == -1) {
	Tcl_SetResult(interp, "hwloc_bitmap_list_asprintf() failed", TCL_STATIC);
	hwloc_bitmap_free(bitmap);
	return TCL_ERROR;
    }

    Tcl_Obj *objPtr = Tcl_NewStringObj(list, -1);
    Tcl_SetObjResult(interp, objPtr);

    hwloc_bitmap_free(bitmap);
    free(list);

    return TCL_OK;
}
