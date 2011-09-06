#ifndef _TOPOLOGY_H_
#define _TOPOLOGY_H_

#include <tcl.h>
#include <hwloc.h>

/*
 * Data managed by each topology object/instance command.
 */

typedef struct topo_data {
    hwloc_topology_t topology;  /* The topology token handled by Hwloc */
    Tcl_Obj*         name;      /* The command name */
    Tcl_Interp*      interp;    /* The interpreter the command is in  */
    Tcl_Command      cmdtoken;  /* The command handle itself */
} topo_data;

void TopologyCmd_CleanUp (ClientData clientData);
int  TopologyCmd (ClientData clientData, Tcl_Interp *interp,
		  int objc, Tcl_Obj *CONST objv[]);

#endif /* _TOPOLOGY_H_ */

/* vim: set sts=4 sw=4 tw=80 et ft=c: */
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
