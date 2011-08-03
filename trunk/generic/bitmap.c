#include <tcl.h>
#include <hwloc.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "bitmap.h"

#define CHECK_FOR_EXTRA_ARGS(num) \
    if (objc > num) {\
        Tcl_WrongNumArgs(interp, num, objv, NULL);\
        hwloc_bitmap_free(bitmap);\
        return TCL_ERROR;\
    }

#define CHECK_FOR_ARG(num, name) \
    if (objc < num) {\
        Tcl_WrongNumArgs(interp, num - 1, objv, name);\
        return TCL_ERROR;\
    }

#define CHECK_FOR_ARG2(num, name) \
    if (objc < num) {\
        Tcl_WrongNumArgs(interp, num - 1, objv, name);\
        hwloc_bitmap_free(bitmap);\
        return TCL_ERROR;\
    }

#define CHECK_FOR_ARG3(num, name) \
    if (objc < num) {\
        Tcl_WrongNumArgs(interp, num - 1, objv, name);\
        hwloc_bitmap_free(bitmap1);\
        return TCL_ERROR;\
    }

#define ERROR_EXIT \
{ hwloc_bitmap_free(bitmap); \
    return TCL_ERROR; }

static int set_bitmap_result(Tcl_Interp *interp, hwloc_bitmap_t bitmap) {
    char *res;
    if (hwloc_bitmap_list_asprintf(&res, bitmap) == -1) {
        Tcl_SetResult(interp, "hwloc_bitmap_list_asprintf() failed", TCL_STATIC);
        return TCL_ERROR;
    }

    Tcl_Obj *objPtr = Tcl_NewStringObj(res, -1);
    free(res);
    Tcl_SetObjResult(interp, objPtr);
    return TCL_OK;
}

/* hwloc bitmap option... */
int parse_bitmap_args(Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
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

    switch (index) {
        case BITMAP_EMPTY:
            {
                hwloc_bitmap_t bitmap = hwloc_bitmap_alloc();
                CHECK_FOR_EXTRA_ARGS(3)

                hwloc_bitmap_zero(bitmap);

                if (set_bitmap_result(interp, bitmap) == TCL_ERROR) ERROR_EXIT
                    hwloc_bitmap_free(bitmap);
                break;
            }
        case BITMAP_FULL:
            {
                hwloc_bitmap_t bitmap = hwloc_bitmap_alloc();
                CHECK_FOR_EXTRA_ARGS(3)

                hwloc_bitmap_fill(bitmap);

                if (set_bitmap_result(interp, bitmap) == TCL_ERROR) ERROR_EXIT
                    hwloc_bitmap_free(bitmap);
                break;
            }
        case BITMAP_ONLY:
            {
                CHECK_FOR_ARG(4, "bitmap")
                const char *bitmap_str = Tcl_GetString(objv[3]);
                hwloc_bitmap_t bitmap = hwloc_bitmap_alloc();
                if (hwloc_bitmap_list_sscanf(bitmap, bitmap_str) == -1) {
                    Tcl_SetResult(interp, "failed to parse cpuset", TCL_STATIC);
                    ERROR_EXIT
                }

                CHECK_FOR_ARG2(5, "id")
                CHECK_FOR_EXTRA_ARGS(5)
                int id = 0;
                if (Tcl_GetIntFromObj(interp, objv[4], &id) == TCL_ERROR) ERROR_EXIT

                hwloc_bitmap_only(bitmap, (unsigned) id);

                if (set_bitmap_result(interp, bitmap) == TCL_ERROR) ERROR_EXIT
                hwloc_bitmap_free(bitmap);
                break;
            }
        case BITMAP_ALLBUT:
            {
                CHECK_FOR_ARG(4, "bitmap")
                const char *bitmap_str = Tcl_GetString(objv[3]);
                hwloc_bitmap_t bitmap = hwloc_bitmap_alloc();
                if (hwloc_bitmap_list_sscanf(bitmap, bitmap_str) == -1) {
                    Tcl_SetResult(interp, "failed to parse cpuset", TCL_STATIC);
                    ERROR_EXIT
                }

                CHECK_FOR_ARG2(5, "id")
                CHECK_FOR_EXTRA_ARGS(5)
                int id = 0;
                if (Tcl_GetIntFromObj(interp, objv[4], &id) == TCL_ERROR) ERROR_EXIT

                hwloc_bitmap_allbut(bitmap, (unsigned) id);

                if (set_bitmap_result(interp, bitmap) == TCL_ERROR) ERROR_EXIT
                hwloc_bitmap_free(bitmap);
                break;
            }
        case BITMAP_FROM_ULONG:
            {
                hwloc_bitmap_t bitmap = hwloc_bitmap_alloc();

                CHECK_FOR_ARG2(4, "mask")
                CHECK_FOR_EXTRA_ARGS(4)

                int mask = 0;
                if (Tcl_GetIntFromObj(interp, objv[3], &mask) == TCL_ERROR) ERROR_EXIT

                hwloc_bitmap_from_ulong(bitmap, (unsigned long) mask);

                if (set_bitmap_result(interp, bitmap) == TCL_ERROR) ERROR_EXIT
                hwloc_bitmap_free(bitmap);
                break;
            }
        case BITMAP_TO_ULONG:
            {
                CHECK_FOR_ARG(4, "bitmap")
                const char *bitmap_str = Tcl_GetString(objv[3]);
                hwloc_bitmap_t bitmap = hwloc_bitmap_alloc();
                if (hwloc_bitmap_list_sscanf(bitmap, bitmap_str) == -1) {
                    Tcl_SetResult(interp, "failed to parse cpuset", TCL_STATIC);
                    ERROR_EXIT
                }
                CHECK_FOR_EXTRA_ARGS(4)

                unsigned long res = hwloc_bitmap_to_ulong(bitmap);
                if (res == -1) {
                    Tcl_SetResult(interp, "hwloc_bitmap_to_ulong error", TCL_STATIC);
                    ERROR_EXIT
                }

                Tcl_Obj *objPtr = Tcl_NewIntObj((int) res);
                Tcl_SetObjResult(interp, objPtr);
                hwloc_bitmap_free(bitmap);
                break;
            }
        case BITMAP_SET:
            {
                CHECK_FOR_ARG(4, "bitmap")
                const char *bitmap_str = Tcl_GetString(objv[3]);
                hwloc_bitmap_t bitmap = hwloc_bitmap_alloc();
                if (hwloc_bitmap_list_sscanf(bitmap, bitmap_str) == -1) {
                    Tcl_SetResult(interp, "failed to parse cpuset", TCL_STATIC);
                    ERROR_EXIT
                }

                CHECK_FOR_ARG2(5, "id")
                CHECK_FOR_EXTRA_ARGS(5)
                int id = 0;
                if (Tcl_GetIntFromObj(interp, objv[4], &id) == TCL_ERROR) ERROR_EXIT

                hwloc_bitmap_set(bitmap, (unsigned) id);

                if (set_bitmap_result(interp, bitmap) == TCL_ERROR) ERROR_EXIT
                    hwloc_bitmap_free(bitmap);
                break;
            }
        case BITMAP_SET_RANGE:
            {
                CHECK_FOR_ARG(4, "bitmap")
                const char *bitmap_str = Tcl_GetString(objv[3]);
                hwloc_bitmap_t bitmap = hwloc_bitmap_alloc();
                if (hwloc_bitmap_list_sscanf(bitmap, bitmap_str) == -1) {
                    Tcl_SetResult(interp, "failed to parse cpuset", TCL_STATIC);
                    ERROR_EXIT
                }

                CHECK_FOR_ARG2(5, "begin")
                int begin = 0;
                if (Tcl_GetIntFromObj(interp, objv[4], &begin) == TCL_ERROR) ERROR_EXIT

                CHECK_FOR_ARG2(6, "end")
                CHECK_FOR_EXTRA_ARGS(6)
                int end = 0;
                if (Tcl_GetIntFromObj(interp, objv[5], &end) == TCL_ERROR) ERROR_EXIT

                hwloc_bitmap_set_range(bitmap, (unsigned) begin, (int) end);

                if (set_bitmap_result(interp, bitmap) == TCL_ERROR) ERROR_EXIT
                hwloc_bitmap_free(bitmap);
                break;
            }
        case BITMAP_CLEAR:
            {
                CHECK_FOR_ARG(4, "bitmap")
                const char *bitmap_str = Tcl_GetString(objv[3]);
                hwloc_bitmap_t bitmap = hwloc_bitmap_alloc();
                if (hwloc_bitmap_list_sscanf(bitmap, bitmap_str) == -1) {
                    Tcl_SetResult(interp, "failed to parse cpuset", TCL_STATIC);
                    ERROR_EXIT
                }

                CHECK_FOR_ARG2(5, "id")
                CHECK_FOR_EXTRA_ARGS(5)
                int id = 0;
                if (Tcl_GetIntFromObj(interp, objv[4], &id) == TCL_ERROR) ERROR_EXIT

                hwloc_bitmap_clr(bitmap, (unsigned) id);

                if (set_bitmap_result(interp, bitmap) == TCL_ERROR) ERROR_EXIT
                hwloc_bitmap_free(bitmap);
                break;
            }
        case BITMAP_CLEAR_RANGE:
            {
                CHECK_FOR_ARG(4, "bitmap")
                const char *bitmap_str = Tcl_GetString(objv[3]);
                hwloc_bitmap_t bitmap = hwloc_bitmap_alloc();
                if (hwloc_bitmap_list_sscanf(bitmap, bitmap_str) == -1) {
                    Tcl_SetResult(interp, "failed to parse cpuset", TCL_STATIC);
                    ERROR_EXIT
                }

                CHECK_FOR_ARG2(5, "begin")
                int begin = 0;
                if (Tcl_GetIntFromObj(interp, objv[4], &begin) == TCL_ERROR) ERROR_EXIT

                CHECK_FOR_ARG2(6, "end")
                CHECK_FOR_EXTRA_ARGS(6)
                int end = 0;
                if (Tcl_GetIntFromObj(interp, objv[5], &end) == TCL_ERROR) ERROR_EXIT

                hwloc_bitmap_clr_range(bitmap, (unsigned) begin, (int) end);

                if (set_bitmap_result(interp, bitmap) == TCL_ERROR) ERROR_EXIT
                hwloc_bitmap_free(bitmap);
                break;
            }
        case BITMAP_SINGLIFY:
            {
                CHECK_FOR_ARG(4, "bitmap")
                const char *bitmap_str = Tcl_GetString(objv[3]);
                hwloc_bitmap_t bitmap = hwloc_bitmap_alloc();
                if (hwloc_bitmap_list_sscanf(bitmap, bitmap_str) == -1) {
                    Tcl_SetResult(interp, "failed to parse cpuset", TCL_STATIC);
                    ERROR_EXIT
                }
                CHECK_FOR_EXTRA_ARGS(4)

                hwloc_bitmap_singlify(bitmap);

                if (set_bitmap_result(interp, bitmap) == TCL_ERROR) ERROR_EXIT
                hwloc_bitmap_free(bitmap);
                break;
            }
        case BITMAP_IS_SET:
            {
                CHECK_FOR_ARG(4, "bitmap")
                const char *bitmap_str = Tcl_GetString(objv[3]);
                hwloc_bitmap_t bitmap = hwloc_bitmap_alloc();
                if (hwloc_bitmap_list_sscanf(bitmap, bitmap_str) == -1) {
                    Tcl_SetResult(interp, "failed to parse cpuset", TCL_STATIC);
                    ERROR_EXIT
                }

                CHECK_FOR_ARG2(5, "id")
                CHECK_FOR_EXTRA_ARGS(5)
                int id = 0;
                if (Tcl_GetIntFromObj(interp, objv[4], &id) == TCL_ERROR) ERROR_EXIT

                int res = hwloc_bitmap_isset(bitmap, (unsigned) id);
                if (res == -1) {
                    Tcl_SetResult(interp, "hwloc_bitmap_isset error", TCL_STATIC);
                    ERROR_EXIT
                }

                Tcl_Obj *objPtr = Tcl_NewIntObj((int) res);
                Tcl_SetObjResult(interp, objPtr);
                hwloc_bitmap_free(bitmap);
                break;
            }
        case BITMAP_IS_ZERO:
            {
                CHECK_FOR_ARG(4, "bitmap")
                const char *bitmap_str = Tcl_GetString(objv[3]);
                hwloc_bitmap_t bitmap = hwloc_bitmap_alloc();
                if (hwloc_bitmap_list_sscanf(bitmap, bitmap_str) == -1) {
                    Tcl_SetResult(interp, "failed to parse cpuset", TCL_STATIC);
                    ERROR_EXIT
                }
                CHECK_FOR_EXTRA_ARGS(4)

                int res = hwloc_bitmap_iszero(bitmap);
                if (res == -1) {
                    Tcl_SetResult(interp, "hwloc_bitmap_zero error", TCL_STATIC);
                    ERROR_EXIT
                }

                Tcl_Obj *objPtr = Tcl_NewIntObj((int) res);
                Tcl_SetObjResult(interp, objPtr);
                hwloc_bitmap_free(bitmap);
                break;
            }
        case BITMAP_IS_FULL:
            {
                CHECK_FOR_ARG(4, "bitmap")
                const char *bitmap_str = Tcl_GetString(objv[3]);
                hwloc_bitmap_t bitmap = hwloc_bitmap_alloc();
                if (hwloc_bitmap_list_sscanf(bitmap, bitmap_str) == -1) {
                    Tcl_SetResult(interp, "failed to parse cpuset", TCL_STATIC);
                    ERROR_EXIT
                }
                CHECK_FOR_EXTRA_ARGS(4)

                int res = hwloc_bitmap_isfull(bitmap);
                if (res == -1) {
                    Tcl_SetResult(interp, "hwloc_bitmap_full error", TCL_STATIC);
                    ERROR_EXIT
                }

                Tcl_Obj *objPtr = Tcl_NewIntObj((int) res);
                Tcl_SetObjResult(interp, objPtr);
                hwloc_bitmap_free(bitmap);
                break;
            }
        case BITMAP_FIRST:
            {
                CHECK_FOR_ARG(4, "bitmap")
                const char *bitmap_str = Tcl_GetString(objv[3]);
                hwloc_bitmap_t bitmap = hwloc_bitmap_alloc();
                if (hwloc_bitmap_list_sscanf(bitmap, bitmap_str) == -1) {
                    Tcl_SetResult(interp, "failed to parse cpuset", TCL_STATIC);
                    ERROR_EXIT
                }
                CHECK_FOR_EXTRA_ARGS(4)

                int res = hwloc_bitmap_first(bitmap);
                if (res == -1) {
                    Tcl_SetResult(interp, "hwloc_bitmap_first error", TCL_STATIC);
                    ERROR_EXIT
                }

                Tcl_Obj *objPtr = Tcl_NewIntObj((int) res);
                Tcl_SetObjResult(interp, objPtr);
                hwloc_bitmap_free(bitmap);
                break;
            }
        case BITMAP_NEXT:
            {
                CHECK_FOR_ARG(4, "bitmap")
                const char *bitmap_str = Tcl_GetString(objv[3]);
                hwloc_bitmap_t bitmap = hwloc_bitmap_alloc();
                if (hwloc_bitmap_list_sscanf(bitmap, bitmap_str) == -1) {
                    Tcl_SetResult(interp, "failed to parse cpuset", TCL_STATIC);
                    ERROR_EXIT
                }

                CHECK_FOR_ARG2(5, "prev")
                CHECK_FOR_EXTRA_ARGS(5)
                int prev = 0;
                if (Tcl_GetIntFromObj(interp, objv[4], &prev) == TCL_ERROR) ERROR_EXIT

                int res = hwloc_bitmap_next(bitmap, (int) prev);
                if (res == -1) {
                    Tcl_SetResult(interp, "hwloc_bitmap_next error", TCL_STATIC);
                    ERROR_EXIT
                }

                Tcl_Obj *objPtr = Tcl_NewIntObj((int) res);
                Tcl_SetObjResult(interp, objPtr);
                hwloc_bitmap_free(bitmap);
                break;
            }
        case BITMAP_LAST:
            {
                CHECK_FOR_ARG(4, "bitmap")
                const char *bitmap_str = Tcl_GetString(objv[3]);
                hwloc_bitmap_t bitmap = hwloc_bitmap_alloc();
                if (hwloc_bitmap_list_sscanf(bitmap, bitmap_str) == -1) {
                    Tcl_SetResult(interp, "failed to parse cpuset", TCL_STATIC);
                    ERROR_EXIT
                }
                CHECK_FOR_EXTRA_ARGS(4)

                int res = hwloc_bitmap_last(bitmap);
                if (res == -1) {
                    Tcl_SetResult(interp, "hwloc_bitmap_last error", TCL_STATIC);
                    ERROR_EXIT
                }

                Tcl_Obj *objPtr = Tcl_NewIntObj((int) res);
                Tcl_SetObjResult(interp, objPtr);
                hwloc_bitmap_free(bitmap);
                break;
            }
        case BITMAP_WEIGHT:
            {
                CHECK_FOR_ARG(4, "bitmap")
                const char *bitmap_str = Tcl_GetString(objv[3]);
                hwloc_bitmap_t bitmap = hwloc_bitmap_alloc();
                if (hwloc_bitmap_list_sscanf(bitmap, bitmap_str) == -1) {
                    Tcl_SetResult(interp, "failed to parse cpuset", TCL_STATIC);
                    ERROR_EXIT
                }
                CHECK_FOR_EXTRA_ARGS(4)

                int res = hwloc_bitmap_weight(bitmap);
                if (res == -1) {
                    Tcl_SetResult(interp, "hwloc_bitmap_weight error", TCL_STATIC);
                    ERROR_EXIT
                }

                Tcl_Obj *objPtr = Tcl_NewIntObj((int) res);
                Tcl_SetObjResult(interp, objPtr);
                hwloc_bitmap_free(bitmap);
                break;
            }
        case BITMAP_AND:
        case BITMAP_ANDNOT:
        case BITMAP_OR:
        case BITMAP_XOR:
            {
                CHECK_FOR_ARG(4, "bitmap")
                const char *bitmap_str1 = Tcl_GetString(objv[3]);
                hwloc_bitmap_t bitmap1 = hwloc_bitmap_alloc();
                if (hwloc_bitmap_list_sscanf(bitmap1, bitmap_str1) == -1) {
                    Tcl_SetResult(interp, "failed to parse cpuset", TCL_STATIC);
                    hwloc_bitmap_free(bitmap1);
                    return TCL_ERROR;
                }

                CHECK_FOR_ARG3(5, "bitmap")
                const char *bitmap_str2 = Tcl_GetString(objv[4]);
                hwloc_bitmap_t bitmap2 = hwloc_bitmap_alloc();
                if (hwloc_bitmap_list_sscanf(bitmap2, bitmap_str2) == -1) {
                    Tcl_SetResult(interp, "failed to parse cpuset", TCL_STATIC);
                    hwloc_bitmap_free(bitmap1);
                    hwloc_bitmap_free(bitmap2);
                    return TCL_ERROR;
                }

                //CHECK_FOR_EXTRA_ARGS(5):
                if (objc > 5) {
                    Tcl_WrongNumArgs(interp, 5, objv, NULL);
                    hwloc_bitmap_free(bitmap1);
                    hwloc_bitmap_free(bitmap2);
                    return TCL_ERROR;
                }

                hwloc_bitmap_t bitmap = hwloc_bitmap_alloc();

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

                if (set_bitmap_result(interp, bitmap) == TCL_ERROR) { 
                    hwloc_bitmap_free(bitmap);
                    return TCL_ERROR;
                }
                hwloc_bitmap_free(bitmap);
                break;
            }
        case BITMAP_NOT:
            {
                CHECK_FOR_ARG(4, "bitmap")
                const char *bitmap_str = Tcl_GetString(objv[3]);
                hwloc_bitmap_t bitmap = hwloc_bitmap_alloc();
                if (hwloc_bitmap_list_sscanf(bitmap, bitmap_str) == -1) ERROR_EXIT
                CHECK_FOR_EXTRA_ARGS(4)

                hwloc_bitmap_t bitmap_res = hwloc_bitmap_alloc();

                hwloc_bitmap_not(bitmap_res, bitmap);

                hwloc_bitmap_free(bitmap);

                if (set_bitmap_result(interp, bitmap_res) == TCL_ERROR) { 
                    hwloc_bitmap_free(bitmap_res);
                    return TCL_ERROR;
                }
                hwloc_bitmap_free(bitmap_res);
                break;
            }
        case BITMAP_INTERSECTS:
        case BITMAP_IS_INCLUDED:
        case BITMAP_IS_EQUAL:
        case BITMAP_COMPARE:
        case BITMAP_COMPARE_FIRST:
            {
                CHECK_FOR_ARG(4, "bitmap")
                const char *bitmap_str1 = Tcl_GetString(objv[3]);
                hwloc_bitmap_t bitmap1 = hwloc_bitmap_alloc();
                if (hwloc_bitmap_list_sscanf(bitmap1, bitmap_str1) == -1) {
                    Tcl_SetResult(interp, "failed to parse cpuset", TCL_STATIC);
                    hwloc_bitmap_free(bitmap1);
                    return TCL_ERROR;
                }

                CHECK_FOR_ARG3(5, "bitmap")
                const char *bitmap_str2 = Tcl_GetString(objv[4]);
                hwloc_bitmap_t bitmap2 = hwloc_bitmap_alloc();
                if (hwloc_bitmap_list_sscanf(bitmap2, bitmap_str2) == -1) {
                    Tcl_SetResult(interp, "failed to parse cpuset", TCL_STATIC);
                    hwloc_bitmap_free(bitmap1);
                    hwloc_bitmap_free(bitmap2);
                    return TCL_ERROR;
                }
                //CHECK_FOR_EXTRA_ARGS(5):
                if (objc > 5) {
                    Tcl_WrongNumArgs(interp, 5, objv, NULL);
                    hwloc_bitmap_free(bitmap1);
                    hwloc_bitmap_free(bitmap2);
                    return TCL_ERROR;
                }

                int res = 0;

                if (index == BITMAP_INTERSECTS) 
                    res = hwloc_bitmap_intersects(bitmap1, bitmap2);
                else if (index == BITMAP_IS_INCLUDED) 
                    res = hwloc_bitmap_isincluded(bitmap1, bitmap2);
                else if (index == BITMAP_IS_EQUAL) 
                    res = hwloc_bitmap_isequal(bitmap1, bitmap2);
                else if (index == BITMAP_COMPARE) 
                    res = hwloc_bitmap_compare(bitmap1, bitmap2);
                else if (index == BITMAP_COMPARE_FIRST) 
                    res = hwloc_bitmap_compare_first(bitmap1, bitmap2);

                Tcl_Obj *objPtr = Tcl_NewIntObj((int) res);
                Tcl_SetObjResult(interp, objPtr);
                hwloc_bitmap_free(bitmap1);
                hwloc_bitmap_free(bitmap2);
                break;
            }
        default: return TCL_ERROR;
    }

    return TCL_OK;
}
