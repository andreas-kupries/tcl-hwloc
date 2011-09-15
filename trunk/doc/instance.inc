
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

This method provides an ensemble of sub-commands enabling navigation
in the tree of objects contained in the topology, relative to the
object specified by [arg id]. See also method [method root].

Please see section [sectref {Object Identifiers}] for details of the
syntax used here. Errors are thrown for ids not following this syntax,
or refering to objects not existing in the topology.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method arity]]

This method returns an integer number, the number of children the
object has.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method attributes]]

This method, and [method info] below return similar information, this
one more human-readable, the other machine-readable, and with a few
keys excluded. The result here is a string shell variable assignments,
separated by a single space.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method children]]

This method returns a list of object identifiers, the children of the
object.  If the object hs no children the result is the empty list.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method cpuset]]

This method returns the CPU set associated with the object.

-- Determine exactly what this means from the hwloc documentation --

The result is an integer number formatted hexadecimally

-- Might it not be better to get this as bitmap at least ? --

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method depth]]

This method returns the depth the object is at. This is, in essence,
the first element of its object identifier.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method first_child]]

This method returns the object identifier of the first, leftmost child
of the object. An error is thrown if the object has no children.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method info]]

This method, and [method attributes] above return similar information,
this one machine-readable, the other more human-readable, and with a
few keys more. The result here is a list of pairs, with each pair
containing key and vaue, in this order.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method last_child]]

This method returns the object identifier of the last, rightmost child
of the object. An error is thrown if the object has no children.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method logical_index]]

This method returns the index of the object in its layer. This is, in
essence, the second element of its object identifier.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method name]]

This method returns a string, the name of the object.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method next_cousin]]

This method returns the identifier of the right sibling of the object,
if there is any, or the first child in the next sibling of its parent
object. In essence it navigates to the right in its layer without
being bound to the children of the current parent.

The method throws an error if and only if the object is the last
object in its layer.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method next_sibling]]

This method returns the identifier of the right sibling of the object.
The method throws an error if the object is the last child in its
parent, i.e. does not have a right sibling.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method parent]]

This method returns the identifier of the parent  of the object.
The method throws an error if the object is the root of the topology.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method prev_cousin]]

This method returns the identifier of the left sibling of the object,
if there is any, or the last child in the previous sibling of its
parent object. In essence it navigates to the left in its layer
without being bound to the children of the current parent.

The method throws an error if and only if the object is the first
object in its layer.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method prev_sibling]]

This method returns the identifier of the left sibling of the object.
The method throws an error if the object is the first child in its
parent, i.e. does not have a left sibling.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method sibling_rank]]

This method returns the index of the object among the children of its
parent.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method type]]

This method returns a string, the type of the object.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object_by] [option -depth] [arg depth] [arg index]]
[call [cmd topologyCmd] [method object_by] [option -type] [arg type] [arg index]]

This method is a constructor for object identifiers. Either directly
from [arg depth] and [arg index], or the depth specified indirectly
through the object [arg type].

An error is thrown if the intended object does not exist in the topology.

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
