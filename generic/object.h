#ifndef _OBJECT_H_
#define _OBJECT_H_

#include <hwloc.h>
#include <tcl.h>

int parse_object_args (topo_data *data, Tcl_Interp *interp,
		       int objc, Tcl_Obj *CONST objv[],
		       hwloc_obj_t obj);

#endif /* _OBJECT_H_ */

/* vim: set sts=4 sw=4 tw=80 et ft=c: */
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
