#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <hwloc.h>
#include <tcl.h>

int parse_bitmap_args (Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);

/*
 * Extract bitmap from a Tcl_Obj*. Not quite a Tcl_ObjType however.
 */
hwloc_bitmap_t thwl_get_bitmap (Tcl_Interp* interp, Tcl_Obj* obj);

/*
 * Return a bitmap. All formatting is done inside.
 */
int thwl_set_result_bitmap  (Tcl_Interp* interp, hwloc_bitmap_t bitmap);
int thwl_set_result_cbitmap (Tcl_Interp* interp, hwloc_const_bitmap_t bitmap);


#endif /* _OBJECT_H_ */

/* vim: set sts=4 sw=4 tw=80 et ft=c: */
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
