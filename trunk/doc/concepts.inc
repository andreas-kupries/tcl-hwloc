
Information about most of the concepts needed to handle hwloc can be
found at its website, in particular at

[uri http://www.open-mpi.org/projects/hwloc/doc/v1.2.1/modules.php]

and equivalent urls for other versions of the underlying package.

[para] Because of this only an abridged summary is provided here, with
the expectation that searchers for deeper knowledge will go to the
above site.

[para] At the heart of the package is the [term topology], an object
which encapsulates a whole system and its configuration, i.e. the
[term elements] it consists of, and their relationships.

The latter is modeled as a tree, with the relation of "X is child of
Y" meaning "X is (conceptually) contained in Y".

This puts the whole system at the top, as the root of the tree, and
the most basic processing elements at the bottom, contained in a
hierarchy of [term caches], [term cpus], [term sockets], and the like.

[para] This same tree can also be viewed as a [term matrix], with each
row representing a layer of the system, from the processing elements
up to the whole system. This second view is especially useful for
identifying and adressing each of the elements in the topology, as two
numbers are sufficient for this, the depth in the hierachy (counting
up while going down from the top), and an index in the layer (counting
up from the left).

[para] An important side note here is that all elements in a layer are
of the same type, which also allows addressing by type, with the depth
implied by it. At least for elements usually existing only in a single
layer. A good counter example are caches, which may occupy more than
one layer, representing the hierarchy of cache levels in the system.

[para] Beyond simple introspection of the system's configuration the
package also enables its user to pin processes and threads to specific
parts of the system, either in terms of logical processors, or in
terms of memory. The former is more relevant for multi-macore
machines, and the latter for NUMA machines.

The central concepts there are cpu and (memory) node sets, both of
which are represented by (possibly infinite) [term bitmaps].
Conversion between both types is possible, as is going from topology
elements to cpu sets.
