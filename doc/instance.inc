
After the creation of a topology instance via [cmd {hwloc create}] (see section
[sectref {Class API}]) the instance command provides the following API and
methods:

[list_begin definitions]

[call [cmd topologyCmd] [method convert] [arg ...]]
[call [cmd topologyCmd] [method cpubind] [arg ...]]
[call [cmd topologyCmd] [method cpuset] [arg ...]]
[call [cmd topologyCmd] [method depth] [opt "[option -type] [arg type]"]]

[call [cmd topologyCmd] [method destroy]]

This method destroys the topology instance and underlying structures.
The result of the method is the empty string.


[call [cmd topologyCmd] [method export] [arg path]]

This method converts the topology into a XML description and stores it in the
file [arg path]. This description can be loaded into a new topology via

[example {
    hwloc create X -set_xml path
}]

The result of the method is the empty string.


[call [cmd topologyCmd] [method local]]

This method checks if the instance is a representation of the local system
and returns a boolean value, [const true] if the test suceeded, and [const false]
otherwise. Related parts in the class API are the option [option -set_flags]
with flag [const this_system], option [option -set_fsroot] and option
[option -set_xml].

[call [cmd topologyCmd] [method membind] [arg ...]]

[call [cmd topologyCmd] [method nodeset] [arg ...]]
[call [cmd topologyCmd] [method object] [arg id] [arg ...]]
[call [cmd topologyCmd] [method object_by] [option -depth] [arg depth] [arg index]]
[call [cmd topologyCmd] [method object_by] [option -type] [arg type] [arg index]]

[call [cmd topologyCmd] [method root]]

This method returns the identifier for the root object of the topology tree.

[call [cmd topologyCmd] [method type] [opt "[option -depth] [arg depth]"]]
[call [cmd topologyCmd] [method width] [opt "[option -type] [arg type]"]]
[call [cmd topologyCmd] [method width] [opt "[option -depth] [arg depth]"]]

[list_end]
