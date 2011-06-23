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
static int parse_create_args(struct topo_data *data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);

/*
 * Function Bodies
 */

int Hwloc_Init(Tcl_Interp *interp) {
    char buffer[100];
    if (Tcl_InitStubs(interp, "8.5", 0) == NULL) {
        return TCL_ERROR;
    }

    Tcl_CreateObjCommand(interp, "hwloc", HwlocCmd, (ClientData) NULL, NULL);

    sprintf(buffer, "%ld", (long) HWLOC_TOPOLOGY_FLAG_WHOLE_SYSTEM);
    Tcl_SetVar(interp, "HWLOC_TOPOLOGY_FLAG_WHOLE_SYSTEM", buffer, TCL_GLOBAL_ONLY);

    sprintf(buffer, "%ld", (long) HWLOC_TOPOLOGY_FLAG_IS_THISSYSTEM);
    Tcl_SetVar(interp, "HWLOC_TOPOLOGY_FLAG_IS_THISSYSTEM", buffer, TCL_GLOBAL_ONLY);

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

            if (objc > 3) {
                    if (parse_create_args(data, interp, objc, objv) == TCL_ERROR)
                        return TCL_ERROR;
            }

            if (hwloc_topology_load(data->topology) == -1) {
                    Tcl_SetResult(interp, "failed to load topology", TCL_STATIC);
                    ckfree((char *) data);
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
static int parse_create_args(struct topo_data *data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    static const char* cmds[] = {
        "-ignore_type",
        "-ignore_type_keep_structure",
        "-ignore_all_keep_structure",
        "-set_flags",
        "-set_fsroot",
        "-set_pid",
        "-set_synthetic",
        "-set_xml",
        NULL
    };
    int index;

    if (Tcl_GetIndexFromObj(interp, objv[3], cmds, "option", 2, &index) != TCL_OK)
        return TCL_ERROR;

    switch (index) {
        case 0: /* -ignore_type type */
        {
            if (objc != 5) {
                Tcl_WrongNumArgs(interp, 4, objv, "type");
                goto on_error;
            }

            hwloc_obj_type_t type = hwloc_obj_type_of_string(Tcl_GetString(objv[4]));
            if (type == -1) {
                Tcl_SetResult(interp, "unrecognized object type", TCL_STATIC);
                goto on_error;
            }

            if (hwloc_topology_ignore_type(data->topology, type) == -1) {
                Tcl_SetResult(interp, "hwloc_topology_ignore_type() failed", TCL_STATIC);
                goto on_error;
            }
            break;
        }
        case 1: /* -ignore_type_keep_structure type */
	{
            if (objc != 5) {
                Tcl_WrongNumArgs(interp, 4, objv, "type");
                goto on_error;
            }

            hwloc_obj_type_t type = hwloc_obj_type_of_string(Tcl_GetString(objv[4]));
            if (type == -1) {
                Tcl_SetResult(interp, "unrecognized object type", TCL_STATIC);
                goto on_error;
            }

            if (hwloc_topology_ignore_type_keep_structure(data->topology, type) == -1) {
                Tcl_SetResult(interp, "hwloc_topology_ignore_type_keep_structure() failed", TCL_STATIC);
                goto on_error;
            }
            break;
        }
        case 2: /* -ignore_all_keep_structure */
	{
            if (objc != 4) {
                Tcl_WrongNumArgs(interp, 3, objv, NULL);
                goto on_error;
            }
            if (hwloc_topology_ignore_all_keep_structure(data->topology) == -1) {
                Tcl_SetResult(interp, "hwloc_topology_ignore_all_keep_structure() failed", TCL_STATIC);
                goto on_error;
            }
            break;
	}
        case 3: /* -set_flags flags */
	{
            if (objc != 5) {
                Tcl_WrongNumArgs(interp, 4, objv, "flags");
                goto on_error;
            }

            long flags = 0;
            if (Tcl_GetLongFromObj(interp, objv[4], &flags) == TCL_ERROR) goto on_error;

            if (hwloc_topology_set_flags(data->topology, flags) == -1) {
                Tcl_SetResult(interp, "hwloc_topology_set_flags() failed", TCL_STATIC);
                goto on_error;
            }

            break;
        }
        case 4: /* -set_fsroot path */
	{
            if (objc != 5) {
                Tcl_WrongNumArgs(interp, 4, objv, "path");
                goto on_error;
            }

            if (hwloc_topology_set_fsroot(data->topology, Tcl_GetString(objv[4])) == -1) {
                Tcl_SetResult(interp, "hwloc_topology_set_fsroot() failed", TCL_STATIC);
                goto on_error;
            }

            break;
        }
        case 5: /* -set_pid pid */
	{
            if (objc != 5) {
                Tcl_WrongNumArgs(interp, 4, objv, "pid");
                goto on_error;
            }

            int pid = 0;
            if (Tcl_GetIntFromObj(interp, objv[4], &pid) == TCL_ERROR) goto on_error;

            if (hwloc_topology_set_pid(data->topology, (hwloc_pid_t) pid) == -1) {
                Tcl_SetResult(interp, "hwloc_topology_set_pid() failed", TCL_STATIC);
                goto on_error;
            }
            
            break;
        }
        case 6: /* -set_synthetic value */
	{
            if (objc != 5) {
                Tcl_WrongNumArgs(interp, 4, objv, "value");
                goto on_error;
            }

            if (hwloc_topology_set_synthetic(data->topology, Tcl_GetString(objv[4])) == -1) {
                Tcl_SetResult(interp, "hwloc_topology_set_synthetic() failed", TCL_STATIC);
                goto on_error;
            }

            break;
        }
        case 7: /* -set_xml value */
	{
            if (objc != 5) {
                Tcl_WrongNumArgs(interp, 4, objv, "value");
                goto on_error;
            }

            if (hwloc_topology_set_xml(data->topology, Tcl_GetString(objv[4])) == -1) {
                Tcl_SetResult(interp, "hwloc_topology_set_xml() failed", TCL_STATIC);
                goto on_error;
            }

            break;
        }
    }

    return TCL_OK;

on_error:
    ckfree((char *) data);
    return TCL_ERROR;
}
