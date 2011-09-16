
After the creation of a topology instance via [cmd {hwloc create}] (see section
[sectref {Class API}]) the instance command provides the following API and
methods:

[list_begin definitions]

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method convert] [opt [option -strict]] [option -to_cpuset]  [arg set]]
[call [cmd topologyCmd] [method convert] [opt [option -strict]] [option -to_nodeset] [arg set]]

This method converts from bitmaps representing node sets to cpu sets,
and vice versa. The returned result is again a bitmap, in the other
domain.

[para] If the [option -strict] option is specified the conversion will
punt if the topology describes a non-NUMA system without NUMA nodes,
and return an empty CPU or node, respectively.

[para] Otherwise, the default, the conversion will treat the system as
a single memory node and use the following heuristics:

[list_begin enumerated]
[enum] If the input set is empty, the result will be emptied as well.
[enum] Otherwise the result will be entirely filled.
[list_end]

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method cpubind] [arg ...]]

@@@ TODO @@@ cpubind

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method cpuset] [option -allowed]]
[call [cmd topologyCmd] [method cpuset] [option -complete]]
[call [cmd topologyCmd] [method cpuset] [option -online]]
[call [cmd topologyCmd] [method cpuset] [option -topology]]

This method returns a bitmap describing various types of CPU sets,
i.e. subsets of the whole set of logical processors found in the
topology.

[list_begin options]
[opt_def -allowed]  All [term allowed] processors.
[opt_def -complete] All processors.
[opt_def -online]   All [term online] (aka [term active] processors
[opt_def -topology] All processors of the system.
[list_end]

@@@ TODO @@@ cpuset - Question - How does a cpu set relate to object types ?

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method depth] [opt "[option -type] [arg type]"]]

This method returns the depth of the topology as an integer value, or
the depth at which the elements of the specified [arg type] are found.

An error is thrown if an invalid [arg type] is specified. A negative
value, [const -1] to be precise, is returned if the [arg type] is
valid but the topology does not contain elements of that type.

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

@@@ TODO @@@ membind

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method nodeset] [option -allowed]]
[call [cmd topologyCmd] [method nodeset] [option -complete]]
[call [cmd topologyCmd] [method nodeset] [option -topology]]

This method returns a bitmap describing various types of memory sets,
i.e. subsets of the whole set of logical memory found in the topology.

[list_begin options]
[opt_def -allowed]  All [term allowed] memory nodes.
[opt_def -complete] All memory nodes.
[opt_def -topology] All memory of the system.
[list_end]

@@@ TODO @@@ nodeset - Question - How does a node set relate to object types ?

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [arg ...]]

This method provides an ensemble of sub-commands enabling navigation
in the tree of elements contained in the topology, relative to the
element specified by the [arg id]. See also method [method root].

Please see section [sectref {Element Identifiers}] for details of the
syntax used here. Errors are thrown for identifiers not following this
syntax, or refering to elements not existing in the topology.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method arity]]

This method returns an integer number, the number of children for the
element.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method attributes]]

This method, and [method info] below return similar information, this
one more human-readable, the other machine-readable, and with a few
keys excluded. The result here is a string shell variable assignments,
separated by a single space.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method children]]

This method returns a list of element identifiers, the children of the
element. If the element hs no children the result is the empty list.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method cpuset]]

This method returns the CPU set associated with the element.

-- Determine exactly what this means from the hwloc documentation --

The result is an integer number formatted hexadecimally

-- Might it not be better to get this as bitmap at least ? --

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method depth]]

This method returns the depth the element is at. This is, in essence,
the first element of its element identifier.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method first_child]]

This method returns the element identifier of the first, leftmost
child of the element. An error is thrown if the element has no
children.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method info]]

This method, and [method attributes] above return similar information,
this one machine-readable, the other more human-readable, and with a
few keys more. The result here is a list of pairs, with each pair
containing key and vaue, in this order.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method last_child]]

This method returns the element identifier of the last, rightmost
child of the element. An error is thrown if the element has no
children.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method logical_index]]

This method returns the index of the element in its layer. This is, in
essence, the second element of its element identifier.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method name]]

This method returns a string, the name of the element.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method next_cousin]]

This method returns the identifier of the right sibling of the
element, if there is any, or the first child in the next sibling of
its parent element. In essence it navigates to the right in its layer
without being bound to the children of the current parent.

The method throws an error if and only if the element is the last
element in its layer.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method next_sibling]]

This method returns the identifier of the right sibling of the
element.  The method throws an error if the element is the last child
in its parent, i.e. does not have a right sibling.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method parent]]

This method returns the identifier of the parent of the element.  The
method throws an error if the element is the root of the topology.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method prev_cousin]]

This method returns the identifier of the left sibling of the element,
if there is any, or the last child in the previous sibling of its
parent element. In essence it navigates to the left in its layer
without being bound to the children of the current parent.

The method throws an error if and only if the element is the first
element in its layer.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method prev_sibling]]

This method returns the identifier of the left sibling of the element.
The method throws an error if the element is the first child in its
parent, i.e. does not have a left sibling.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method sibling_rank]]

This method returns an integer number, the index of the element among
the children of its parent.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object] [arg id] [method type]]

This method returns a string, the type of the element.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method object_by] [option -depth] [arg depth] [arg index]]
[call [cmd topologyCmd] [method object_by] [option -type] [arg type] [arg index]]

This method is a constructor for element identifiers. Either directly
from [arg depth] and [arg index], or the depth specified indirectly
through the element [arg type].

An error is thrown if the intended element does not exist in the
topology.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method root]]

This method returns the identifier for the root element of the
topology tree.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method type] [opt "[option -depth] [arg depth]"]]

This method returns a string, the type of the elements at the
specified [arg depth] of the topology. An error is thrown if an
invalid depth is specified, i.e. either a value less than zero, or
greater than the value returned by

[example {
    topology depth
}]

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method width] [opt "[option -type] [arg type]"]]
[call [cmd topologyCmd] [method width] [opt "[option -depth] [arg depth]"]]

This method returns an integer value, the number of elements contained
in the topology which are of either the specified [arg type], or found
at the specified [arg depth].

[para] An error is thrown if an invalid depth is specified, i.e. either
a value less than zero, or greater than the value returned by

[example {
    topology depth
}]

[para] Alternatively, an error is also thrown if an invalid [arg type]
is specified. The value [const 0] is returned if the [arg type] is
valid but the topology does not contain elements of that type.

[list_end]