#ifndef _CPUBIND_H_
#define _CPUBIND_H_

#include <hwloc.h>
#include <tcl.h>

int parse_cpubind_args(struct topo_data *data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);

#endif /* _CPUBIND_H_ */
