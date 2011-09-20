
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
[call [cmd topologyCmd] [method {cpubind get}] [opt options...]]

This method returns the currently set CPU bindings for the current
process.

The behaviour can be modified by specifying one or more of the options
below:

[list_begin options]
[opt_def -nomembind]

This option is accepted, but ignored. It has no meaning for the
method in question.

[opt_def -pid [arg pid]]
Return the information for the referenced process instead.

[opt_def -process]
Return the information for all threads of the (possibly multithreaded)
process.

[opt_def -strict]
When specified the system checks whether all threads of the process
actually have the same binding. By default, the binding of each thread
will be accumulated, i.e. the union of all bindings returned.

[para] [emph Note] that this flag is meaningless when retrieving the
binding of a thread.

[opt_def -thread]
Return the binding of the current thread of the current process. This
option cannot be used together with [option -pid]. I.e. when querying
a different process we cannot query that process' threads in detail.

[list_end]

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method {cpubind last}] [opt options...]]

This method returns the CPU where the current thread or process was
run last time it executed.  Note that the may be outdated, i.e. not be
the CPU it is currently running on, as the OS may have shifted it
around already according to their binding, as it saw fit.

The behaviour can be modified by specifying one or more of the options
below:

[list_begin options]
[opt_def -nomembind]

This option is accepted, but ignored. It has no meaning for the
method in question.

[opt_def -pid [arg pid]]
Return the information for the referenced process instead.

[opt_def -process]
Return the information for all threads of the (possibly multithreaded)
process.

[opt_def -strict]

This option is accepted, but ignored. It has no meaning for the
method in question.

[opt_def -thread]

Return the location of the current thread of the current process. This
option cannot be used together with [option -pid]. I.e. when querying
a different process we cannot query that process' threads in detail.

[list_end]

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method {cpubind set}] [opt options...] [arg cpuset]]

By default this method binds the current process, assumed to be
single-threaded, to the specified [arg cpuset]. This is the most
portable way of using this method.

The behaviour can be modified by specifying one or more of the options
below:

[list_begin options]
[opt_def -nomembind]

Instructs the system to not change memory bindings. I.e there are
operating systems there binding a process/thread to a (set of) CPU(s)
will also bind its memory to the associated NUMA node, by default. If
this is a problem for the process then using this flag causes the
package to avoid the OS functions which would bind memory as well.

[para] Note however that depending on system configuration and OS this
may fail.

[opt_def -pid [arg pid]]
Bind the referenced process instead.

[opt_def -process]
Bind all threads of the (possibly multithreaded) process.

[opt_def -strict]
Request a strict binding. By default, the OS may shift the process
and/or thread to CPUs it was not strictly bound to, if the designated
CPUs are busy and others are idle, to ensure progress. A strict
binding prevents this, i.e. the process or thread will never go
anywhere but the designated CPUs.

[para] Note such a request may fail, as the OS may not implement this
functionality, or it may have been be disabled by the system's
administrator.

[opt_def -thread]
Bind just the current thread of the current process. This option
cannot be used together with [option -pid]. I.e. when binding a
different process we have no control over how that process' threads
are bound in detail.

[list_end]

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method {cpuset allowed}]]
[call [cmd topologyCmd] [method {cpuset complete}]]
[call [cmd topologyCmd] [method {cpuset online}]]
[call [cmd topologyCmd] [method {cpuset topology}]]

This method returns a bitmap describing various types of CPU sets,
i.e. subsets of the whole set of logical processors found in the
topology.

[list_begin definitions]
[def allowed]  All [term allowed] processors.
[def complete] All processors.
[def online]   All [term online] (aka [term active] processors.
[def topology] All processors of the system.
[list_end]

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
[call [cmd topologyCmd] [method {membind get}] [opt options...]]

This method returns a 2-element list containing the currently set
default NUMA bindings and policies for a process and/or thread, in
this order. The bindings are represented by a bitmap representing
either a nodeset (default) or a cpuset.

Its behaviour can be modified by specifying one or more of the options
below:

[list_begin options]
[opt_def -migrate]
[opt_def -nocpubind]

These two options are relevant only for [method {membind set}].
While this method accepts them, they are ignored.

[comment {--- TODO --- Rework the implementation to not accept them}]

[opt_def -cpu]

By default the bitmap returned by the method represents a nodeset.

When this option is specified a cpuset is returned instead, with the
method internally converting the nodeset result to it.

For convenience sake's all descriptions below talk only about
nodesets, with the conversion implied where needed.

[opt_def -pid [arg pid]]

By default this method queries the current process or its threads.

When this option is specified the referenced process is queried
instead, and we lose the ability for a finegrained query of
threads. I.e. usage of option [option -thread] is forbidden, and
[option -process] is implied.

[opt_def -process]
[opt_def -thread]

These two options exclude each other, i.e. when using one use of the
other is forbidden.

[para] By default, i.e. when neither of the options is present, the
process is assumed to be single threaded. The package will then use
either process-based OS functions or thread-based OS functions,
depending on which are available and fitting. This makes this the most
portable method.

[para] Passing option [option -process] causes the return of the
current policies and nodesets for all the threads in the specified
process (current, or referenced by option [option -pid]).

The exact nature of the result depends on the presence of option
[option -strict], and will be explained there.

[para] Passing option [option -thread] specifies to return the current
policy and nodeset for the current thread invoking the method, in the
current process.

Use of this option is forbidden when option [option -pid] is present.

[opt_def -strict]
The (non-)presence of this option guides the handling of multiple
threads.

In the default case of either a single-threaded process (neither
[option -process], nor [option -thread] present) or when querying a
single thread (via [option -thread]) we have only a single nodeset and
policy, these are returned, and this option is ignored when specified.

[para] Now when [option -process] or [option -pid] are used we are
querying (potentially) multiple threads and have to merge the nodesets
and policies in some way.

By default that is exactly what happens. The union of all found
nodesets is computed and returned. For the policies, if all are
identical this one policy is returned, and otherwise "mixed" is
returned.

[para] By specifying this option we disable the above merging process
and replace it with a check. If the nodesets and policies of all the
queried threads are identical, then this information is returned as
usual. If there are differences however, then an error is thrown
instead.

[list_end]

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method {membind set}] [opt options...] [arg nodeset] [arg policy]]

This method sets the default memory binding [arg policy] and to prefer
the NUMA node(s) specified by the [arg nodeset]. This determines which
NUMA modes to use when allocating memory.

Its behaviour can be modified by specifying one or more of the options
below:

[list_begin options]

[opt_def -migrate]

If specified the package will attempt to migrate existing allocated
memory to the specified NUMA nodes.

By default failure to perform the migration is not reported. However
if the option [option -strict] is specified also, an error will be
thrown.

[opt_def -nocpubind]

Specifying this option instructs the package to not change cpu
bindings. I.e there are operating systems where binding a process
and/or thread to a (set of) NUMA node(s) will also bind its execution
to the associated CPUs, by default. If this is a problem for the
process then using this flag causes the package to avoid the OS
functions which would bind cpus as well.

[para] Note however that depending on system configuration and OS this
may fail.

[opt_def -cpu]

By default the bitmap taken by the method represents a [arg nodeset].

When this option is specified a cpuset is taken instead, with the
method internally converting it to a nodeset.

For convenience sake's all descriptions below talk only about
nodesets, with the conversion implied where needed.

[opt_def -pid [arg pid]]

By default this method operates on the current process or its threads.

When this option is specified the referenced process is operated on
instead, and we lose the ability for a finegrained handling of
threads. I.e. usage of option [option -thread] is forbidden, and
[option -process] is implied.

[opt_def -thread]
[opt_def -process]

These two options exclude each other, i.e. when using one use of the
other is forbidden.

[para] By default, i.e. when neither of the options is present, the
process is assumed to be single threaded. The package will then use
either process-based OS functions or thread-based OS functions,
depending on which are available and fitting. This makes this the most
portable form.

[para] Passing [option -process] sets the policy for all threads of
the specified (possibly multithreaded) process.

[para] Passing [option -thread] sets the policy for the current thread
of the current process.

[opt_def -strict]

Presence of this option requests a strict binding from the OS. The
function will fail if the binding can not be guaranteed / completely
enforced.

This flag has slightly different meanings depending on the presence of
the options [option -pid] and [option -cpu].

Hwloc's documentation doesn't go into further detail.

[list_end]

The following policies are accepted as legal. Remember however that
not all systems support all policies, and attempting an unsupported
policy causes errors to be thrown.

[list_begin definitions]
[def [const bind]]
Allocate memory on the specified NUMA nodes.

[def [const default]]
Reset the memory allocation policy to the system default. This is the
only policy which is fully portable across systems.

[def [const firsttouch]]
Allocated memory is not immediately bound to a specific locality. Each
page in the allocation is individually bound to the local NUMA node of
the first thread that touches it, at the time of this touch.

[def [const interleave]]
Allocate the memory on the specified nodes in an interleaved /
round-robin manner. The precise layout of the memory across multiple
NUMA nodes is OS/system specific. Interleaving can be useful when
threads distributed across the specified NUMA nodes will all be
accessing the whole memory range concurrently, since the interleave
will then balance the memory references.

[def [const mixed]]
This policy is returned when multiple threads or parts of a memory
area have differing memory binding policies. It is not usable for
setting a policy.

[def [const nexttouch]]
For each page bound with this policy, the next time it is touched (and
next time only), it is moved from its current location to the local
NUMA node of the thread where the memory reference occurred (if it
needs to be moved at all).

[def [const replicate]]

Replicate memory on the given nodes; reads from this memory will
attempt to be serviced from the NUMA node local to the reading
thread. Replicating can be useful when multiple threads from the
specified NUMA nodes will be sharing the same read-only data.

[list_end]

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method {nodeset allowed}]]
[call [cmd topologyCmd] [method {nodeset complete}]]
[call [cmd topologyCmd] [method {nodeset topology}]]

This method returns a bitmap describing various types of numa node
sets, i.e. subsets of the whole set of logical memory found in the
topology.

[list_begin definitions]
[def allowed]  All [term allowed] numa nodes.
[def complete] All numa nodes.
[def topology] All numa nodes of the system.
[list_end]

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method element] [arg id] [arg ...]]

This method provides an ensemble of sub-commands enabling navigation
in the tree of elements contained in the topology, relative to the
element specified by the [arg id]. See also method [method root].

Please see section [sectref {Element Identifiers}] for details of the
syntax used here. Errors are thrown for identifiers not following this
syntax, or refering to elements not existing in the topology.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method element] [arg id] [method arity]]

This method returns an integer number, the number of children for the
element.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method element] [arg id] [method attributes]]

This method, and [method info] below return similar information, this
one more human-readable, the other machine-readable, and with a few
keys excluded. The result here is a string shell variable assignments,
separated by a single space.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method element] [arg id] [method children]]

This method returns a list of element identifiers, the children of the
element. If the element hs no children the result is the empty list.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method element] [arg id] [method {cpuset allowed}]]
[call [cmd topologyCmd] [method element] [arg id] [method {cpuset complete}]]
[call [cmd topologyCmd] [method element] [arg id] [method {cpuset online}]]
[call [cmd topologyCmd] [method element] [arg id] [method {cpuset topology}]]

This method returns a bitmap, one of several CPU sets associated with
the element.

[list_begin definitions]
[def allowed]  All [term allowed] processors.
[def complete] All processors.
[def online]   All [term online] (aka [term active] processors.
[def topology] All processors of the system.
[list_end]

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method element] [arg id] [method depth]]

This method returns the depth the element is at. This is, in essence,
the first element of its element identifier.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method element] [arg id] [method first_child]]

This method returns the element identifier of the first, leftmost
child of the element. An error is thrown if the element has no
children.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method element] [arg id] [method info]]

This method, and [method attributes] above return similar information,
this one machine-readable, the other more human-readable, and with a
few keys more. The result here is a list of pairs, with each pair
containing key and vaue, in this order.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method element] [arg id] [method last_child]]

This method returns the element identifier of the last, rightmost
child of the element. An error is thrown if the element has no
children.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method element] [arg id] [method logical_index]]

This method returns the index of the element in its layer. This is, in
essence, the second element of its element identifier.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method element] [arg id] [method name]]

This method returns a string, the name of the element.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method element] [arg id] [method next_cousin]]

This method returns the identifier of the right sibling of the
element, if there is any, or the first child in the next sibling of
its parent element. In essence it navigates to the right in its layer
without being bound to the children of the current parent.

The method throws an error if and only if the element is the last
element in its layer.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method element] [arg id] [method next_sibling]]

This method returns the identifier of the right sibling of the
element.  The method throws an error if the element is the last child
in its parent, i.e. does not have a right sibling.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method element] [arg id] [method {nodeset allowed}]]
[call [cmd topologyCmd] [method element] [arg id] [method {nodeset complete}]]
[call [cmd topologyCmd] [method element] [arg id] [method {nodeset topology}]]

This method returns a bitmap, one of several NUMA node sets associated
with the element.

[list_begin definitions]
[def allowed]  All [term allowed] numa nodes.
[def complete] All numa nodes.
[def topology] All numa nodes of the system.
[list_end]

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method element] [arg id] [method parent]]

This method returns the identifier of the parent of the element.  The
method throws an error if the element is the root of the topology.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method element] [arg id] [method prev_cousin]]

This method returns the identifier of the left sibling of the element,
if there is any, or the last child in the previous sibling of its
parent element. In essence it navigates to the left in its layer
without being bound to the children of the current parent.

The method throws an error if and only if the element is the first
element in its layer.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method element] [arg id] [method prev_sibling]]

This method returns the identifier of the left sibling of the element.
The method throws an error if the element is the first child in its
parent, i.e. does not have a left sibling.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method element] [arg id] [method sibling_rank]]

This method returns an integer number, the index of the element among
the children of its parent.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method element] [arg id] [method type]]

This method returns a string, the type of the element.

[comment ---------------------------------------------------------------------]
[call [cmd topologyCmd] [method element_by] [option -depth] [arg depth] [arg index]]
[call [cmd topologyCmd] [method element_by] [option -type] [arg type] [arg index]]

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
