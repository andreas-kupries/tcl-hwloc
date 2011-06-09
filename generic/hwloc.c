#include <tcl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hwloc.h>
#include "info.h"

/*
 * Function Prototypes
 */

struct topo_data {
    hwloc_topology_t *topology;
    Tcl_Obj *name;
    Tcl_Interp *interp;
    Tcl_Command cmdtoken;
};

static int HwlocCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
static int TopologyCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
static void TopologyCmd_CleanUp(ClientData clientData);

/*
 * Function Bodies
 */

int Hwloc_Init(Tcl_Interp *interp) {
    if (Tcl_InitStubs(interp, "8.4", 0) == NULL) {
        return TCL_ERROR;
    }

    Tcl_CreateObjCommand(interp, "hwloc", HwlocCmd, (ClientData) NULL, NULL);

    Tcl_PkgProvide(interp, "hwloc", "1.0");
    return TCL_OK;
}

static int HwlocCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    static const char* cmds[] = {
        "version",
        "create",
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
        case 0: /* version */
        {
            if (objc > 3) {
                Tcl_WrongNumArgs(interp, 2, objv, NULL);
                return TCL_ERROR;
            }
            
            Tcl_Obj *objPtr = Tcl_NewIntObj((int) hwloc_get_api_version());
            Tcl_SetObjResult(interp, objPtr);
        }
        case 1: /* create */
        {
            if (objc < 4) {
                Tcl_WrongNumArgs(interp, 2, objv, "name ?arg? ...");
                return TCL_ERROR;
            }
            
            hwloc_topology_t *topo;
            if (hwloc_topology_init(topo) == -1) {
                    Tcl_SetResult(interp, "failed to initialize topology", TCL_STATIC);
                    return TCL_ERROR;
            }

            if (hwloc_topology_load(topo) == -1) {
                    Tcl_SetResult(interp, "failed to populate topology", TCL_STATIC);
                    return TCL_ERROR;
            }

            if (hwloc_topology_check(topo) == -1) {
                    Tcl_SetResult(interp, "topology failed internal consistency check", TCL_STATIC);
                    return TCL_ERROR;
            }

            struct topo_data *data = (struct topo_data *) ckalloc(sizeof(struct topo_data));
            data->topology = topo;
            data->name = objv[2]
            Tcl_IncrRefCount(data->name);
            data->interp = interp;
            data->cmdtoken = Tcl_CreateObjCommand(interp, data->name, TopologyCmd, (ClientData) data, TopologyCmd_CleanUp);
        }
    }

    return TCL_OK;
}

static int TopologyCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {

    return TCL_OK;
}

/* This is also executed automatically when the interpreter the command resides in, is deleted. */
static void TopologyCmd_CleanUp(ClientData clientData) {
    struct topo_data *data = (struct topo_data *) clientData;
    Tcl_DecrRefCount(data->name);
    ckfree((char *) data);
}
