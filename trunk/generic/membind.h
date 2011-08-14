#ifndef _MEMBIND_H_
#define _MEMBIND_H_

#include <hwloc.h>
#include <tcl.h>

int parse_membind_args(struct topo_data *data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);

#endif /* _MEMBIND_H_ */
