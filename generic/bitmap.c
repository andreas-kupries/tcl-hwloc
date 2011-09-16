#include <tcl.h>
#include <hwloc.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "bitmap.h"

#define CHECK_FOR_ARG(num, name)			\
    if (objc != num) {					\
        Tcl_WrongNumArgs(interp, 3, objv, name);	\
        return TCL_ERROR;				\
    }

hwloc_bitmap_t
thwl_get_bitmap (Tcl_Interp* interp, Tcl_Obj* obj)
{
    const char*    str    = Tcl_GetString (obj);
    hwloc_bitmap_t bitmap = hwloc_bitmap_alloc();

    if (hwloc_bitmap_list_sscanf(bitmap, str) == -1) {
	Tcl_ResetResult (interp);
	Tcl_AppendResult(interp, "Expected bitmap but got \"", str, "\"", NULL);
	hwloc_bitmap_free(bitmap);
	return NULL;
    }

    return bitmap;
}

static int
get_bitnum (Tcl_Interp* interp, Tcl_Obj* obj, int* value)
{
    int res = Tcl_GetIntFromObj (interp, obj, value);

    if (res != TCL_OK) {
	return res;
    } else if (*value < 0) {
	Tcl_AppendResult(interp, "Expected integer >= 0 but got \"",
			 Tcl_GetString (obj), "\"", NULL);
	return TCL_ERROR;
    }
    return res;
}

int
thwl_set_result_bitmap (Tcl_Interp* interp, hwloc_bitmap_t bitmap) {
    char* res;

    if (hwloc_bitmap_list_asprintf(&res, bitmap) == -1) {
        Tcl_SetResult(interp, "hwloc_bitmap_list_asprintf() failed", TCL_STATIC);

	hwloc_bitmap_free(bitmap);
        return TCL_ERROR;
    }

    hwloc_bitmap_free(bitmap);

    Tcl_SetObjResult(interp, Tcl_NewStringObj(res, -1));
    free(res);
    return TCL_OK;
}

int
thwl_set_result_cbitmap (Tcl_Interp* interp, hwloc_const_bitmap_t bitmap) {
    char* res;

    if (hwloc_bitmap_list_asprintf(&res, bitmap) == -1) {
        Tcl_SetResult(interp, "hwloc_bitmap_list_asprintf() failed", TCL_STATIC);
        return TCL_ERROR;
    }

    Tcl_SetObjResult(interp, Tcl_NewStringObj(res, -1));
    free(res);
    return TCL_OK;
}

/* hwloc bitmap option... */
int parse_bitmap_args(Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    static const char* cmds[] = {
        "allbut",       "and",       "andnot",        "clear",
	"clear_range", "compare",    "compare_first", "empty",
	"first",       "from_ulong", "full",          "intersects",
	"is_empty",    "is_equal",   "is_full",       "is_included",
	"is_set",      "last",       "next",           "not",
	"only",        "or",         "set",            "set_range",
	"singlify",    "to_ulong",   "weight",         "xor",
        NULL
    };

    enum options {
	BITMAP_ALLBUT,      BITMAP_AND,        BITMAP_ANDNOT,        BITMAP_CLEAR,
	BITMAP_CLEAR_RANGE, BITMAP_COMPARE,    BITMAP_COMPARE_FIRST, BITMAP_EMPTY,
	BITMAP_FIRST,       BITMAP_FROM_ULONG, BITMAP_FULL,          BITMAP_INTERSECTS,
	BITMAP_IS_EMPTY,    BITMAP_IS_EQUAL,   BITMAP_IS_FULL,       BITMAP_IS_INCLUDED,
	BITMAP_IS_SET,      BITMAP_LAST,       BITMAP_NEXT,          BITMAP_NOT,
	BITMAP_ONLY,        BITMAP_OR,         BITMAP_SET,           BITMAP_SET_RANGE,
	BITMAP_SINGLIFY,    BITMAP_TO_ULONG,   BITMAP_WEIGHT,        BITMAP_XOR
    };

    hwloc_bitmap_t bitmap  = NULL;
    hwloc_bitmap_t bitmap1 = NULL;
    hwloc_bitmap_t bitmap2 = NULL;

    unsigned long res;
    int mask, id, index, flag = 0, prev, begin, end;

    if (Tcl_GetIndexFromObj(interp, objv[2], cmds, "option", 2, &index) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (index) {
    case BITMAP_EMPTY:
	CHECK_FOR_ARG(3,NULL);

	bitmap = hwloc_bitmap_alloc();
	hwloc_bitmap_zero(bitmap);

	return thwl_set_result_bitmap(interp, bitmap);

    case BITMAP_FULL:
	CHECK_FOR_ARG(3,NULL);

	bitmap = hwloc_bitmap_alloc();
	hwloc_bitmap_fill(bitmap);

	return thwl_set_result_bitmap(interp, bitmap);

    case BITMAP_ONLY:
	CHECK_FOR_ARG(4, "id");

	if (get_bitnum (interp, objv[3], &id) == TCL_ERROR) goto error;

	bitmap = hwloc_bitmap_alloc();
	hwloc_bitmap_only(bitmap, (unsigned) id);

	return thwl_set_result_bitmap(interp, bitmap);

    case BITMAP_ALLBUT:
	CHECK_FOR_ARG(4, "id");

	if (get_bitnum (interp, objv[3], &id) == TCL_ERROR) goto error;

	bitmap = hwloc_bitmap_alloc();
	hwloc_bitmap_allbut(bitmap, (unsigned) id);

	return thwl_set_result_bitmap(interp, bitmap);

    case BITMAP_FROM_ULONG:
	CHECK_FOR_ARG(4, "mask");

	if (Tcl_GetIntFromObj(interp, objv[3], &mask) == TCL_ERROR) goto error;

	bitmap = hwloc_bitmap_alloc();
	hwloc_bitmap_from_ulong(bitmap, (unsigned long) mask);

	return thwl_set_result_bitmap(interp, bitmap);

    case BITMAP_TO_ULONG:
	CHECK_FOR_ARG(4, "bitmap");

	bitmap = thwl_get_bitmap (interp, objv[3]);
	if (bitmap == NULL) goto error;

	res = hwloc_bitmap_to_ulong(bitmap);
	hwloc_bitmap_free(bitmap);

	if (res == -1) {
	    Tcl_SetResult(interp, "Internal hwloc_bitmap_to_ulong error", TCL_STATIC);
	    goto error;
	}

	Tcl_SetObjResult(interp, Tcl_NewIntObj (res));
	break;

    case BITMAP_SET:
	CHECK_FOR_ARG(5, "bitmap id");

	bitmap = thwl_get_bitmap (interp, objv[3]);
	if (bitmap == NULL) goto error;

	if (get_bitnum (interp, objv[4], &id) == TCL_ERROR) goto cleanup;

	hwloc_bitmap_set(bitmap, (unsigned) id);

	return thwl_set_result_bitmap(interp, bitmap);

    case BITMAP_SET_RANGE:
	CHECK_FOR_ARG(6, "bitmap begin end");

	bitmap = thwl_get_bitmap (interp, objv[3]);
	if (bitmap == NULL) goto error;

	if (get_bitnum (interp, objv[4], &begin) == TCL_ERROR) goto cleanup;
	if (get_bitnum (interp, objv[5], &end)   == TCL_ERROR) goto cleanup;

	hwloc_bitmap_set_range(bitmap, (unsigned) begin, (int) end);

	return thwl_set_result_bitmap(interp, bitmap);

    case BITMAP_CLEAR:
	CHECK_FOR_ARG(5, "bitmap id");

	bitmap = thwl_get_bitmap (interp, objv[3]);
	if (bitmap == NULL) goto error;

	if (get_bitnum (interp, objv[4], &id) == TCL_ERROR) goto cleanup;

	hwloc_bitmap_clr(bitmap, (unsigned) id);

	return thwl_set_result_bitmap(interp, bitmap);

    case BITMAP_CLEAR_RANGE:
	CHECK_FOR_ARG(6, "bitmap begin end");

	bitmap = thwl_get_bitmap (interp, objv[3]);
	if (bitmap == NULL) goto error;

	if (get_bitnum (interp, objv[4], &begin) == TCL_ERROR) goto cleanup;
	if (get_bitnum (interp, objv[5], &end)   == TCL_ERROR) goto cleanup;

	hwloc_bitmap_clr_range(bitmap, (unsigned) begin, (int) end);

	return thwl_set_result_bitmap(interp, bitmap);

    case BITMAP_SINGLIFY:
	CHECK_FOR_ARG(4, "bitmap");

	bitmap = thwl_get_bitmap (interp, objv[3]);
	if (bitmap == NULL) goto error;

	hwloc_bitmap_singlify(bitmap);

	return thwl_set_result_bitmap(interp, bitmap);

    case BITMAP_IS_SET:
	CHECK_FOR_ARG(5, "bitmap id");

	bitmap = thwl_get_bitmap (interp, objv[3]);
	if (bitmap == NULL) goto error;

	if (get_bitnum (interp, objv[4], &id) == TCL_ERROR) goto cleanup;

	flag = hwloc_bitmap_isset(bitmap, (unsigned) id);
	hwloc_bitmap_free(bitmap);

	if (flag == -1) {
	    Tcl_SetResult(interp, "Internal hwloc_bitmap_isset error", TCL_STATIC);
	    goto error;
	}

	Tcl_SetObjResult(interp, Tcl_NewIntObj (flag));
	break;

    case BITMAP_IS_EMPTY:
	CHECK_FOR_ARG(4, "bitmap");

	bitmap = thwl_get_bitmap (interp, objv[3]);
	if (bitmap == NULL) goto error;

	flag = hwloc_bitmap_iszero(bitmap);
	hwloc_bitmap_free(bitmap);

	if (flag == -1) {
	    Tcl_SetResult(interp, "Internal hwloc_bitmap_zero error", TCL_STATIC);
	    goto error;
	}

	Tcl_SetObjResult(interp, Tcl_NewIntObj (flag));
	break;

    case BITMAP_IS_FULL:
	CHECK_FOR_ARG(4, "bitmap");

	bitmap = thwl_get_bitmap (interp, objv[3]);
	if (bitmap == NULL) goto error;

	flag = hwloc_bitmap_isfull(bitmap);
	hwloc_bitmap_free(bitmap);

	if (flag == -1) {
	    Tcl_SetResult(interp, "Internal hwloc_bitmap_full error", TCL_STATIC);
	    goto error;
	}

	Tcl_SetObjResult(interp, Tcl_NewIntObj (flag));
	break;

    case BITMAP_FIRST:
	CHECK_FOR_ARG(4, "bitmap");

	bitmap = thwl_get_bitmap (interp, objv[3]);
	if (bitmap == NULL) goto error;

	flag = hwloc_bitmap_first(bitmap);
	hwloc_bitmap_free(bitmap);

	if (flag == -1) {
	    Tcl_SetResult(interp, "first is undefined for empty bitmap", TCL_STATIC);
	    goto error;
	}

	Tcl_SetObjResult(interp, Tcl_NewIntObj (flag));
	break;

    case BITMAP_NEXT:
	CHECK_FOR_ARG(5, "bitmap prev");

	bitmap = thwl_get_bitmap (interp, objv[3]);
	if (bitmap == NULL) goto error;

	if (hwloc_bitmap_iszero(bitmap)) {
	    Tcl_SetResult(interp, "next is undefined for empty bitmap", TCL_STATIC);
	    goto cleanup;
	}

	/*
	 * Note: This one place allows a negative number as well, making the
	 * operation equivalent to 'first'. Hence no use of 'get_bitnum' here.
	 */
	if (Tcl_GetIntFromObj(interp, objv[4], &prev) == TCL_ERROR) goto cleanup;

	flag = hwloc_bitmap_next(bitmap, (int) prev);
	hwloc_bitmap_free(bitmap);

	if (flag == -1) {
	    Tcl_ResetResult (interp);
	    Tcl_AppendResult(interp, "next is undefined for a bitmap empty after bit \"",
			     Tcl_GetString (objv[4]), "\"", NULL);
	    goto error;
	}

	Tcl_SetObjResult(interp, Tcl_NewIntObj (flag));
	break;

    case BITMAP_LAST:
	CHECK_FOR_ARG(4, "bitmap");

	bitmap = thwl_get_bitmap (interp, objv[3]);
	if (bitmap == NULL) goto error;

	flag = hwloc_bitmap_last(bitmap);
	if (flag == -1) {
	    if (hwloc_bitmap_iszero(bitmap)) {
		Tcl_SetResult(interp, "last is undefined for empty bitmap", TCL_STATIC);
	    } else {
		Tcl_SetResult(interp, "last is undefined for infinite bitmap", TCL_STATIC);
	    }
	    hwloc_bitmap_free(bitmap);
	    goto error;
	}

	hwloc_bitmap_free(bitmap);

	Tcl_SetObjResult(interp, Tcl_NewIntObj (flag));
	break;

    case BITMAP_WEIGHT:
	CHECK_FOR_ARG(4, "bitmap");

	bitmap = thwl_get_bitmap (interp, objv[3]);
	if (bitmap == NULL) goto error;

	flag = hwloc_bitmap_weight(bitmap);
	hwloc_bitmap_free(bitmap);

	if (flag == -1) {
	    Tcl_SetResult(interp, "hwloc_bitmap_weight error", TCL_STATIC);
	    goto error;
	}

	Tcl_SetObjResult(interp, Tcl_NewIntObj (flag));
	break;

    case BITMAP_AND:
    case BITMAP_ANDNOT:
    case BITMAP_OR:
    case BITMAP_XOR:
	CHECK_FOR_ARG(5, "bitmapA bitmapB");

	bitmap1 = thwl_get_bitmap (interp, objv[3]);
	if (bitmap1 == NULL) goto error;

	bitmap2 = thwl_get_bitmap (interp, objv[4]);
	if (bitmap2 == NULL) goto cleanup;

	bitmap = hwloc_bitmap_alloc();

	if (index == BITMAP_AND) 
	    hwloc_bitmap_and(bitmap, bitmap1, bitmap2);
	else if (index == BITMAP_ANDNOT) 
	    hwloc_bitmap_andnot(bitmap, bitmap1, bitmap2);
	else if (index == BITMAP_OR) 
	    hwloc_bitmap_or(bitmap, bitmap1, bitmap2);
	else if (index == BITMAP_XOR) 
	    hwloc_bitmap_xor(bitmap, bitmap1, bitmap2);

	hwloc_bitmap_free(bitmap1);
	hwloc_bitmap_free(bitmap2);

	return thwl_set_result_bitmap(interp, bitmap);

    case BITMAP_NOT:
	CHECK_FOR_ARG(4, "bitmap");

	bitmap = thwl_get_bitmap (interp, objv[3]);
	if (bitmap == NULL) goto error;

	bitmap1 = hwloc_bitmap_alloc();

	hwloc_bitmap_not(bitmap1, bitmap);
	hwloc_bitmap_free(bitmap);

	return thwl_set_result_bitmap(interp, bitmap1);

    case BITMAP_INTERSECTS:
    case BITMAP_IS_INCLUDED:
    case BITMAP_IS_EQUAL:
    case BITMAP_COMPARE:
    case BITMAP_COMPARE_FIRST:
	CHECK_FOR_ARG(5, "bitmapA bitmapB");

	bitmap1 = thwl_get_bitmap (interp, objv[3]);
	if (bitmap1 == NULL) goto error;

	bitmap2 = thwl_get_bitmap (interp, objv[4]);
	if (bitmap2 == NULL) goto cleanup;


	if (index == BITMAP_INTERSECTS) 
	    flag = hwloc_bitmap_intersects(bitmap1, bitmap2);
	else if (index == BITMAP_IS_INCLUDED) 
	    flag = hwloc_bitmap_isincluded(bitmap1, bitmap2);
	else if (index == BITMAP_IS_EQUAL) 
	    flag = hwloc_bitmap_isequal(bitmap1, bitmap2);
	else if (index == BITMAP_COMPARE) 
	    flag = hwloc_bitmap_compare(bitmap1, bitmap2);
	else if (index == BITMAP_COMPARE_FIRST) 
	    flag = hwloc_bitmap_compare_first(bitmap1, bitmap2);

	hwloc_bitmap_free(bitmap1);
	hwloc_bitmap_free(bitmap2);

	Tcl_SetObjResult(interp, Tcl_NewIntObj (flag));
	break;

    default:
	return TCL_ERROR;
    }

    return TCL_OK;
 cleanup:
    if (bitmap  != NULL) hwloc_bitmap_free (bitmap);
    if (bitmap1 != NULL) hwloc_bitmap_free (bitmap1);
    if (bitmap2 != NULL) hwloc_bitmap_free (bitmap2);
 error:
    return TCL_ERROR;
}

/* vim: set sts=4 sw=4 tw=80 et ft=c: */
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
