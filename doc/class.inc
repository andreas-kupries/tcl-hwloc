
The main API provides three methods, to retrive version information, create
topology instances, and manipulate bitmaps.

[list_begin definitions]

[call [cmd hwloc] [method bitmap] [arg ...]]

This ensemble provides a set of methods for the manipulation of hwloc bitmaps.
More details about the individual submethods are available in section
[sectref {Bitmap API}].

[call [cmd hwloc] [method create] [arg object] [arg ...]]

This method creates a new topology object whose associated instance command
has the name [arg object]. 

The creation process can be influenced by a number of options:

[list_begin options]
[opt_def -ignore_all_keep_structure]

Create a topology which ignores, i.e. excludes, all elements which do
not provide any structure to it. An element does not provide structure
if it is the single child of its parent, and/or has only a single
child of its own.

[opt_def -ignore_type [arg type]]

Create a topology which excludes elements of the specified type.  Note
that the toplevel element of a topology is never ignored, even if it
is of the specified type. Note further that elements of type "pu"
cannot be excluded.

The list of valid element types can be retrieved via [cmd {hwloc types}]. The exact
list depends on the version of Hwloc the binding is linked against (at runtime). See

[uri http://www.open-mpi.org/projects/hwloc/doc/v1.2.1/group__hwlocality__types.php \
     {Hwloc's documentation}]

for the list of types handled by a specific version.

[opt_def -ignore_type_keep_structure]

Similar to option [option -ignore_type], however the elements of the
specified type are excluded if and only if they do not provide
structure to the topology.

[opt_def -flags [arg flags]]

The argument is a list of values.
The acceptable flags, and their meaning are:

[list_begin definitions]
[def [const this_system]]

Assume that the selected backend provides the topology for the system
on which we are running. This forces the instance method
[method ...] to return [const true], i.e. makes hwloc assume that the
selected backend provides the topology for the system on which we are
running, even if it is not the OS-specific backend but the XML backend
for instance. This means making the binding functions actually call the
OS-specific system calls and really do binding, while the XML backend
would otherwise provide empty hooks just returning success.

[para] Setting the environment variable [var HWLOC_THISSYSTEM] may also
result in the same behavior.

[para] This can be used for efficiency reasons to first detect the
topology once, save it to an XML file, and quickly reload it later
through the XML backend, but still having binding functions actually
do bind.

[def [const whole_system]]

Detect the whole system, ignore reservations and offline settings.
Gather all resources, even if some were disabled by the administrator.
For instance, ignore Linux Cpusets and gather all processors and
memory nodes, and ignore the fact that some resources may be offline.

[list_end]

[opt_def -fsroot [arg path]]

Make [arg path] the filesystem root while retrieving the topology.

[para] On Linux systems, use sysfs and procfs files as if they were
mounted on the given [arg path] instead of the main file-system root.
Setting the environment variable [var HWLOC_FSROOT] may also result
in this behavior. Not using the main file-system root causes the
instance method [method local] to later return [const false].

[para] Note: For convenience, this backend provides empty binding hooks
which just return success. To have hwloc still actually call
OS-specific hooks, the flag [const this_system] has to be set (see
option [option -flags]) to assert that the loaded file is really
the underlying system.

[opt_def -pid [arg int]]

Set the process id from which to view and build the topology.

[para] On some systems, processes may have different views of the
machine, for instance the set of allowed CPUs. By default, hwloc
exposes the view from the current process. Using [option -pid]
forces it to expose the topology of the machine from the point of
view of another process.


[opt_def -synthetic [arg description]]

Create a synthetic topology from the [arg description].

This should be a space-separated string of numbers describing the
arity of each level. Each number may be prefixed with a type and
a colon to enforce the type of a level. If only some level types
are enforced, hwloc will try to choose the other types according
to usual topologies. It may fail however and you may have to
specify more level types manually.

[opt_def -xml [arg path]]

Create the topology not from from the system, but the XML-based
description found in the file [arg path].

[para] Setting the environment variable [var HWLOC_XMLFILE] may
also result in this behavior. This file may have been generated
earlier by exporting a topology instance as XML.

[list_end]

[call [cmd hwloc] [method types]]

This method returns a list containing the names of all valid element
types.

[call [cmd hwloc] [method version]]

This method returns HWloc's API version as its result.

[list_end]

