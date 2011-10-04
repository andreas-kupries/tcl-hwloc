#ifndef _TOPOLOGY_H_
#define _TOPOLOGY_H_

#include <tcl.h>
#include <hwloc.h>

/*
 * Data managed by the topology class command.
 */

typedef struct topo_class {
    Tcl_Obj* types; /* List of valid element types */

    /* Allocated array of allocated C strings of the valid element types,
     * terminated by NULL, for use by Tcl_GetIndexFromObj.
     */

    char** typestr;
} topo_class;

/*
 * Data managed by each topology object/instance command.
 */

typedef struct topo_data {
    topo_class*      class;     /* Shared class information */
    hwloc_topology_t topology;  /* The topology token handled by Hwloc */
    Tcl_Interp*      interp;    /* The interpreter the command is in  */
    Tcl_Command      cmdtoken;  /* The command handle itself */
} topo_data;

void TopologyCmd_CleanUp (ClientData clientData);
int  TopologyCmd (ClientData clientData, Tcl_Interp *interp,
		  int objc, Tcl_Obj *CONST objv[]);

/*
 * Extract an element type from a Tcl_Obj*. Not quite a Tcl_ObjType however.
 */

hwloc_obj_type_t thwl_get_etype (Tcl_Interp* interp, Tcl_Obj* obj, topo_data* topology);

#endif /* _TOPOLOGY_H_ */

/* vim: set sts=4 sw=4 tw=80 et ft=c: */
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
