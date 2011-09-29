#include <tcl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <hwloc.h>
#include "topology.h"
#include "bitmap.h"

/*
 * Function Prototypes
 */

static int HwlocCmd (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
static int parse_create_args (topo_data* data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
static int parse_set_flags (Tcl_Interp *interp, Tcl_Obj *obj, int *result);

/*
 * Function Bodies
 */

int Tclhwloc_Init(Tcl_Interp *interp) {
    if (Tcl_InitStubs(interp, "8.5", 0) == NULL) {
        return TCL_ERROR;
    }

    Tcl_CreateObjCommand(interp, "hwloc", HwlocCmd, (ClientData) NULL, NULL);

    Tcl_PkgProvide(interp, "hwloc", "1.0");
    return TCL_OK;
}

static int HwlocCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    static const char* cmds[] = {
        "bitmap", "create", "version", NULL
    };
    enum options {
        HWLOC_BITMAP, HWLOC_CREATE, HWLOC_VERSION
    };
    int index;

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "option ?arg? ...");
        return TCL_ERROR;
    }

    if (Tcl_GetIndexFromObj(interp, objv[1], cmds, "option", 0, &index) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (index) {
    case HWLOC_VERSION:
        {
            if (objc > 2) {
                Tcl_WrongNumArgs(interp, 2, objv, NULL);
                return TCL_ERROR;
            }
            
            Tcl_SetObjResult(interp, Tcl_NewIntObj ((int) hwloc_get_api_version()));
            break;
        }
    case HWLOC_CREATE:
        {
            topo_data* data;
	    char* name;
	    Tcl_Obj* fqn;
	    Tcl_CmdInfo ci;

            if (objc < 3) {
                Tcl_WrongNumArgs(interp, 2, objv, "name ?arg? ...");
                return TCL_ERROR;
            }
            
	    data = (topo_data *) ckalloc(sizeof(topo_data));

            if (hwloc_topology_init(&data->topology) == -1) {
		Tcl_SetResult(interp, "failed to initialize topology", TCL_STATIC);
		ckfree((char *) data);
		return TCL_ERROR;
            }

            if (objc > 3) {
		if (parse_create_args(data, interp, objc, objv) == TCL_ERROR) {
		    return TCL_ERROR;
		}
            }

            if (hwloc_topology_load(data->topology) == -1) {
		Tcl_SetResult(interp, "failed to load topology", TCL_STATIC);
		ckfree((char *) data);
		return TCL_ERROR;
            }

            /* Run some internal consistency checks */
            hwloc_topology_check(data->topology);

	    /*
	     * Create and initialize the instance (topology) command.  This
	     * includes conversion from unqualified names to fully-qualified,
	     * checking for existence of the command, i.e. collision with
	     * something else (we do not overwrite existing commands, that is
	     * usually an error and source of difficult to debug problems).
	     */

	    name = Tcl_GetString (objv [2]);

	    if (!Tcl_StringMatch (name, "::*")) {
		/* Relative name. Prefix with current namespace */

		Tcl_Eval (interp, "namespace current");
		fqn = Tcl_GetObjResult (interp);
		fqn = Tcl_DuplicateObj (fqn);
		Tcl_IncrRefCount (fqn);

		if (!Tcl_StringMatch (Tcl_GetString (fqn), "::")) {
		    Tcl_AppendToObj (fqn, "::", -1);
		}
		Tcl_AppendToObj (fqn, name, -1);
	    } else {
		fqn = Tcl_NewStringObj (name, -1);
		Tcl_IncrRefCount (fqn);
	    }
	    Tcl_ResetResult (interp);

	    if (Tcl_GetCommandInfo (interp,
				    Tcl_GetString (fqn),
				    &ci)) {
		Tcl_Obj* err;

		err = Tcl_NewObj ();
		Tcl_AppendToObj    (err, "command \"", -1);
		Tcl_AppendObjToObj (err, fqn);
		Tcl_AppendToObj    (err, "\" already exists, unable to create topology", -1);

		Tcl_DecrRefCount (fqn);
		Tcl_SetObjResult (interp, err);

		TopologyCmd_CleanUp ((ClientData) data);
		return TCL_ERROR;
	    }

            data->interp = interp;
            data->cmdtoken = Tcl_CreateObjCommand(interp,
						  Tcl_GetString(fqn),
						  TopologyCmd,
						  (ClientData) data,
						  TopologyCmd_CleanUp);

	    Tcl_SetObjResult (interp, fqn);
	    Tcl_DecrRefCount (fqn);
            break;
        }
    case HWLOC_BITMAP:
	{
            if (objc < 3) {
                Tcl_WrongNumArgs(interp, 2, objv, "option ?arg? ...");
                return TCL_ERROR;
            }
            
	    if (parse_bitmap_args(interp, objc, objv) == TCL_ERROR) {
		return TCL_ERROR;
	    }
	}
    }

    return TCL_OK;
}

/* Already have processed the 3 first arguments */
static int parse_create_args(topo_data* data, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    static const char* cmds[] = {
        "-ignore_all_keep_structure",  "-ignore_type",
        "-ignore_type_keep_structure", "-flags",
        "-fsroot",                     "-pid",
        "-synthetic",                  "-xml",
        NULL
    };
    enum options {
        CREATE_IGNORE_ALL_KEEP_STRUCTURE,   CREATE_IGNORE_TYPE,
        CREATE_IGNORE_TYPE_KEEP_STRUCTURE,  CREATE_FLAGS,
        CREATE_FSROOT,                      CREATE_PID,
        CREATE_SYNTHETIC,                   CREATE_XML
    };
    int index;

    /*
     * Syntax: hwloc create NAME option...
     *         [0]   [1]    [2]  [3...]
     */

    int objc_curr = 3;

    while (objc_curr < objc) {
        if (Tcl_GetIndexFromObj(interp, objv[objc_curr], cmds, "option", objc_curr, &index) != TCL_OK) {
            return TCL_ERROR;
	}

        switch (index) {
	case CREATE_IGNORE_TYPE: /* -ignore_type type */
	    {
		hwloc_obj_type_t type;

		if (objc <= (objc_curr+1)) {
		    Tcl_WrongNumArgs(interp, 3, objv, "-ignore_type type");
		    goto on_error;
		}

		type = hwloc_obj_type_of_string(Tcl_GetString(objv[objc_curr+1]));

		if (type == -1) {
		    /* TODO: mention the bogus type, mention the expected types
		     * TODO: Expected types can be computed once, saved static
		     * TODO: (client data)
		     */
		    Tcl_SetResult(interp, "unrecognized object type", TCL_STATIC);
		    goto on_error;
		}

		if (hwloc_topology_ignore_type(data->topology, type) == -1) {
		    /* TODO Better message */
		    Tcl_SetResult(interp, "hwloc_topology_ignore_type() failed", TCL_STATIC);
		    goto on_error;
		}

		objc_curr += 2;
		break;
	    }
	case CREATE_IGNORE_TYPE_KEEP_STRUCTURE: /* -ignore_type_keep_structure type */
	    {
		hwloc_obj_type_t type;

		if (objc <= (objc_curr+1)) {
		    Tcl_WrongNumArgs(interp, 3, objv, "-ignore_type_keep_structure type");
		    goto on_error;
		}

		type = hwloc_obj_type_of_string(Tcl_GetString(objv[objc_curr+1]));
		if (type == -1) {
		    Tcl_SetResult(interp, "unrecognized object type", TCL_STATIC);
		    goto on_error;
		}

		if (hwloc_topology_ignore_type_keep_structure(data->topology, type) == -1) {
		    /* TODO Better message */
		    Tcl_SetResult(interp, "hwloc_topology_ignore_type_keep_structure() failed", TCL_STATIC);
		    goto on_error;
		}

		objc_curr += 2;
		break;
	    }
	case CREATE_IGNORE_ALL_KEEP_STRUCTURE: /* -ignore_all_keep_structure */
	    {
		if (hwloc_topology_ignore_all_keep_structure(data->topology) == -1) {
		    Tcl_SetResult(interp, "hwloc_topology_ignore_all_keep_structure() failed", TCL_STATIC);
		    goto on_error;
		}

		objc_curr += 1;
		break;
	    }
	case CREATE_FLAGS: /* -flags flags */
	    {
		int flags = 0;

		if (objc <= (objc_curr+1)) {
		    Tcl_WrongNumArgs(interp, 3, objv, "-flags flags");
		    goto on_error;
		}

		if (parse_set_flags(interp, objv[objc_curr+1], &flags) == TCL_ERROR) {
		    goto on_error;
		}

		if (hwloc_topology_set_flags(data->topology, flags) == -1) {
		    Tcl_SetResult(interp, "hwloc_topology_set_flags() failed", TCL_STATIC);
		    goto on_error;
		}

		objc_curr += 2;
		break;
	    }
	case CREATE_FSROOT: /* -fsroot path */
	    {
		if (objc <= (objc_curr+1)) {
		    Tcl_WrongNumArgs(interp, 3, objv, "-fsroot path");
		    goto on_error;
		}

		if(access(Tcl_GetString(objv[objc_curr+1]), R_OK) != 0 ) {
		    Tcl_SetResult(interp, "directory doesn't exist or lacks read permission", TCL_STATIC);
		    goto on_error;
		}

		if (hwloc_topology_set_fsroot(data->topology, Tcl_GetString(objv[objc_curr+1])) == -1) {
		    Tcl_SetResult(interp, "hwloc_topology_set_fsroot() failed", TCL_STATIC);
		    goto on_error;
		}

		objc_curr += 2;
		break;
	    }
	case CREATE_PID: /* -pid pid */
	    {
		int pid = 0;

		if (objc <= (objc_curr+1)) {
		    Tcl_WrongNumArgs(interp, 3, objv, "-pid pid");
		    goto on_error;
		}

		if (Tcl_GetIntFromObj(interp, objv[objc_curr+1], &pid) == TCL_ERROR) {
		    goto on_error;
		}

		if (hwloc_topology_set_pid(data->topology, (hwloc_pid_t) pid) == -1) {
		    Tcl_SetResult(interp, "hwloc_topology_set_pid() failed", TCL_STATIC);
		    goto on_error;
		}

		objc_curr += 2;
		break;
	    }
	case CREATE_SYNTHETIC: /* -synthetic value */
	    {
		const char* synth;

		if (objc <= (objc_curr+1)) {
		    Tcl_WrongNumArgs(interp, 3, objv, "-synthetic description");
		    goto on_error;
		}

		synth = Tcl_GetString(objv[objc_curr+1]);
		if (!strlen(synth)) {
		    Tcl_SetResult(interp, "Expected synthetic topology description, got \"\"", TCL_STATIC);
		    goto on_error;
		}

		if (hwloc_topology_set_synthetic(data->topology, synth) == -1) {
		    /* TODO: How to disable hwloc printing error data to stderr/out ? */
		    Tcl_SetResult(interp, "hwloc_topology_set_synthetic() failed", TCL_STATIC);
		    goto on_error;
		}

		objc_curr += 2;
		break;
	    }
	case CREATE_XML: /* -xml value */
	    {
		if (objc <= (objc_curr+1)) {
		    Tcl_WrongNumArgs(interp, 3, objv, "-xml path");
		    goto on_error;
		}

		/* TODO: Map to Tcl FS functions */
		/* TODO: Look over hwloc API to see
		 * if we can integrate Tcl VFS better
		 * whereever PATHs are expected/used.
		 */
		if(access(Tcl_GetString(objv[objc_curr+1]), R_OK) != 0 ) {
		    Tcl_SetResult(interp, "file doesn't exist or lacks read permission", TCL_STATIC);
		    goto on_error;
		}

		if (hwloc_topology_set_xml(data->topology, Tcl_GetString(objv[4])) == -1) {
		    Tcl_SetResult(interp, "hwloc_topology_set_xml() failed", TCL_STATIC);
		    goto on_error;
		}

		objc_curr += 2;
		break;
	    }
	default:
	    {
		return TCL_ERROR;
	    }
        }
    } /* while */

    return TCL_OK;

 on_error:
    ckfree((char *) data);
    return TCL_ERROR;
}

static int
parse_set_flags(Tcl_Interp *interp, Tcl_Obj *obj, int *result) {
    static const char* flags[] = {
	"this_system", "whole_system", NULL
    };
    enum options {
        SET_THIS_SYSTEM, SET_WHOLE_SYSTEM
    };
    static int map [] = {
	HWLOC_TOPOLOGY_FLAG_IS_THISSYSTEM,
	HWLOC_TOPOLOGY_FLAG_WHOLE_SYSTEM
    };

    int i, index;

    int       obj_objc;
    Tcl_Obj **obj_objv;

    if (Tcl_ListObjGetElements(interp, obj, &obj_objc, &obj_objv) == TCL_ERROR) {
        Tcl_SetResult(interp, "parsing flags failed", TCL_STATIC);
        return TCL_ERROR;
    }

    for (i = 0; i < obj_objc; i++) {
        if (Tcl_GetIndexFromObj(interp, obj_objv[i], flags, "flag", 0, &index) != TCL_OK) {
            return TCL_ERROR;
	}
	*result |= map [index];
    }

    return TCL_OK;
}

/* vim: set sts=4 sw=4 tw=80 et ft=c: */
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
