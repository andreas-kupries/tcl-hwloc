#ifndef _TOPOLOGY_H_
#define _TOPOLOGY_H_

#include <tcl.h>
#include <hwloc.h>

struct topo_data {
    hwloc_topology_t topology;
    Tcl_Obj *name;
    Tcl_Interp *interp;
    Tcl_Command cmdtoken;
};

int TopologyCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
void TopologyCmd_CleanUp(ClientData clientData);

#endif /* _TOPOLOGY_H_ */
