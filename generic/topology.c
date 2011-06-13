#include <hwloc.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "topology.h"
#include "convert.h"

int TopologyCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    struct topo_data *data = (struct topo_data *) clientData;

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
            Tcl_DeleteCommandFromToken(interp, data->cmdtoken);
            hwloc_topology_destroy(data->topology);
            Tcl_DecrRefCount(data->name);
            ckfree((char *) data);
            break;
        }
        case 1: /* export */
        {
            hwloc_topology_export_xml (data->topology, Tcl_GetString(objv[2]));
            break;
        }
        case 2: /* depth ?-type type? */
        {
            if (objc == 2) {
                Tcl_Obj *objPtr = Tcl_NewIntObj((int) hwloc_topology_get_depth(data->topology));
                Tcl_SetObjResult(interp, objPtr);
            } else if (objc == 4 && strcmp(Tcl_GetString(objv[2]), "-type") == 0) {
                hwloc_obj_type_t type = convert_obj2obj_type(interp, objv[3]);
                if (type == TCL_ERROR) /* Error message is set */
                    return TCL_ERROR;

                Tcl_Obj *objPtr = Tcl_NewIntObj((int) hwloc_get_type_depth(data->topology, type));
                Tcl_SetObjResult(interp, objPtr);
            } else {
                Tcl_WrongNumArgs(interp, 2, objv, "-type arg");
                return TCL_ERROR;
            }
            break;
        }
        case 3: /* type -depth depth */
        {
            if (objc != 4 || strcmp(Tcl_GetString(objv[2]), "-depth")) {
                Tcl_WrongNumArgs(interp, 2, objv, "-depth arg");
                return TCL_ERROR;
            } else {
                int depth = 0;
                if (Tcl_GetIntFromObj(interp, objv[3], &depth) == TCL_ERROR) 
                    return TCL_ERROR;

                Tcl_Obj *objPtr = Tcl_NewIntObj((int) hwloc_get_depth_type(data->topology, (unsigned int) depth));
                Tcl_SetObjResult(interp, objPtr);
            }
            break;
        }
        case 4: /* width {-depth depth | -type type} */
        {
            if (objc == 4 && strcmp(Tcl_GetString(objv[2]), "-depth") == 0) {
                int depth = 0;
                if (Tcl_GetIntFromObj(interp, objv[3], &depth) == TCL_ERROR) 
                    return TCL_ERROR;

                Tcl_Obj *objPtr = Tcl_NewIntObj((int) hwloc_get_nbobjs_by_depth(data->topology, (unsigned int) depth));
                Tcl_SetObjResult(interp, objPtr);
            } else if (objc == 4 && strcmp(Tcl_GetString(objv[2]), "-type") == 0) {
                hwloc_obj_type_t type = convert_obj2obj_type(interp, objv[3]);
                if (type == TCL_ERROR) /* Error message is set */
                    return TCL_ERROR;

                Tcl_Obj *objPtr = Tcl_NewIntObj((int) hwloc_get_nbobjs_by_type(data->topology, type));
                Tcl_SetObjResult(interp, objPtr);
            } else {
                Tcl_WrongNumArgs(interp, 2, objv, "-depth arg");
                return TCL_ERROR;
            }
            break;
        }
        case 5: /* local */
        {
            Tcl_Obj *objPtr = Tcl_NewIntObj((int) hwloc_topology_is_thissystem(data->topology));
            Tcl_SetObjResult(interp, objPtr);
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
