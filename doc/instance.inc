
After the creation of a topology instance via [cmd {hwloc create}] (see section
[sectref {Class API}]) the instance command provides the following API and
methods:

[list_begin definitions]

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method convert] [arg ...]]

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method cpubind] [arg ...]]

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method cpuset] [arg ...]]

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method depth] [opt "[option -type] [arg type]"]]

This method returns the depth of the topology as an integer value, or
the depth at which the objects of the specified [arg type] are
found.

An error is thrown if an invalid [arg type] is specified. A negative
value, [const -1] to be precise, is returned if the [arg type] is
valid but the topology does not contain objects of that type.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method destroy]]

This method destroys the topology instance and underlying structures.
The result of the method is the empty string.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method export] [arg path]]

This method converts the topology into a XML description and stores it
in the file [arg path]. This description can be loaded into a new
topology via

[example {
    hwloc create X -set_xml path
}]

The result of the method is the empty string.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method local]]

This method checks if the instance is a representation of the local
system and returns a boolean value, [const true] if the test suceeded,
and [const false] otherwise. Related parts in the class API are the
option [option -set_flags] with flag [const this_system], option
[option -set_fsroot] and option [option -set_xml].

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method membind] [arg ...]]

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method nodeset] [arg ...]]

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [arg ...]]

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object_by] [option -depth] [arg depth] [arg index]]
[call [cmd topologyCmd] [method object_by] [option -type] [arg type] [arg index]]

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method root]]

This method returns the identifier for the root object of the topology
tree.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method type] [opt "[option -depth] [arg depth]"]]

This method returns a string, the type of the objects at the specified
[arg depth] of the topology. An error is thrown if an invalid depth is
specified, i.e. either a value less than zero, or greater than the
value returned by

[example {
    topology depth
}]

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method width] [opt "[option -type] [arg type]"]]
[call [cmd topologyCmd] [method width] [opt "[option -depth] [arg depth]"]]

This method returns an integer value, the number of objects contained
in the topology which are of either the specified [arg type], or found
at the specified [arg depth].

[para] An error is thrown if an invalid depth is specified,
i.e. either a value less than zero, or greater than the value returned
by

[example {
    topology depth
}]

[para] Alternatively, an error is also thrown if an invalid [arg type]
is specified. The value [const 0] is returned if the [arg type] is
valid but the topology does not contain objects of that type.

[list_end]
