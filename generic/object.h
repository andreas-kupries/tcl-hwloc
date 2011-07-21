#ifndef _OBJECT_H_
#define _OBJECT_H_

#include <hwloc.h>
#include <tcl.h>

int parse_object_args(struct topo_data *data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[], hwloc_obj_t obj);

#endif /* _OBJECT_H_ */
