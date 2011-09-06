#ifndef _CPUBIND_H_
#define _CPUBIND_H_

#include <hwloc.h>
#include <tcl.h>

int parse_cpubind_args (topo_data *data, Tcl_Interp *interp,
			int objc, Tcl_Obj *CONST objv[]);

#endif /* _CPUBIND_H_ */

/* vim: set sts=4 sw=4 tw=80 et ft=c: */
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
