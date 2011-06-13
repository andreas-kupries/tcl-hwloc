#include <hwloc.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "topology.h"

int TopologyCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    static const char* cmds[] = {
        "destory",
        "export",
        "depth",
        "type",
        "width",
        "local",
        NULL
    };
    int index;

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "option ?arg? ...");
        return TCL_ERROR;
    }

    if (Tcl_GetIndexFromObj(interp, objv[1], cmds, "option", 0, &index) != TCL_OK)
        return TCL_ERROR;

    switch (index) {
        case 0: /* destory */
        {

             break;
        }
        case 1: /* export */
        {

             break;
        }
        case 2: /* depth */
        {

             break;
        }
        case 3: /* type */
        {

             break;
        }
        case 4: /* width */
        {

             break;
        }
        case 5: /* local */
        {

             break;
        }
    }

    return TCL_OK;
}

/* This is also executed automatically when the interpreter the command resides in, is deleted. */
void TopologyCmd_CleanUp(ClientData clientData) {
    struct topo_data *data = (struct topo_data *) clientData;
    hwloc_topology_destroy(data->topology);
    Tcl_DecrRefCount(data->name);
    ckfree((char *) data);
}
