#include <tcl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hwloc.h>
#include "convert.h"
#include "topology.h"

/*
 * Function Prototypes
 */

static int HwlocCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
static int parse_create_args(struct topo_data *data, ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);

/*
 * Function Bodies
 */

int Hwloc_Init(Tcl_Interp *interp) {
    if (Tcl_InitStubs(interp, "8.5", 0) == NULL) {
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
            if (objc > 2) {
                Tcl_WrongNumArgs(interp, 2, objv, NULL);
                return TCL_ERROR;
            }
            
            Tcl_Obj *objPtr = Tcl_NewIntObj((int) hwloc_get_api_version());
            Tcl_SetObjResult(interp, objPtr);
            break;
        }
        case 1: /* create */
        {
            if (objc < 3) {
                Tcl_WrongNumArgs(interp, 2, objv, "name ?arg? ...");
                return TCL_ERROR;
            }
            
            struct topo_data *data = (struct topo_data *) ckalloc(sizeof(struct topo_data));
            if (hwloc_topology_init(&data->topology) == -1) {
                    Tcl_SetResult(interp, "failed to initialize topology", TCL_STATIC);
                    ckfree((char *) data);
                    return TCL_ERROR;
            }

            if (hwloc_topology_load(data->topology) == -1) {
                    Tcl_SetResult(interp, "failed to load topology", TCL_STATIC);
                    ckfree((char *) data);
                    return TCL_ERROR;
            }

            if (objc > 3) {
                    if (parse_create_args(data, clientData, interp, objc, objv) == TCL_ERROR)
                        return TCL_ERROR;
            }

            /* Run some internal consistency checks */
            hwloc_topology_check(data->topology);

            data->name = Tcl_DuplicateObj(objv[2]);
            Tcl_IncrRefCount(data->name);
            data->interp = interp;
            data->cmdtoken = Tcl_CreateObjCommand(interp, Tcl_GetString(data->name), TopologyCmd, (ClientData) data, TopologyCmd_CleanUp);
            break;
        }
    }

    return TCL_OK;
}

/* Already have processed the 3 first arguments */
static int parse_create_args(struct topo_data *data, ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    static const char* cmds[] = {
        "-ignore_type",
        NULL
    };
    int index;

    if (Tcl_GetIndexFromObj(interp, objv[3], cmds, "option", 2, &index) != TCL_OK)
        return TCL_ERROR;

    switch (index) {
        case 0: /* -ignore_type */
        {
            if (objc != 5) {
                Tcl_WrongNumArgs(interp, 4, objv, NULL);
                return TCL_ERROR;
            }

            int type = convert_obj2obj_type(interp, objv[4]);
            if (type == -1) {
                Tcl_SetResult(interp, "object type not recognized", TCL_STATIC);
                ckfree((char *) data);
                return TCL_ERROR;
            }

            if (hwloc_topology_ignore_type(data->topology, type) == -1) {
                Tcl_SetResult(interp, "hwloc_topology_ignore_type() failed", TCL_STATIC);
                ckfree((char *) data);
                return TCL_ERROR;
            }
            break;
        }
    }

    return TCL_OK;
}
