#ifndef _MEMBIND_H_
#define _MEMBIND_H_

#include <hwloc.h>
#include <tcl.h>

int parse_membind_args (topo_data *data, Tcl_Interp *interp,
			int objc, Tcl_Obj *CONST objv[]);

#endif /* _MEMBIND_H_ */

/* vim: set sts=4 sw=4 tw=80 et ft=c: */
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
