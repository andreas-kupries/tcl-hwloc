
The methods in this section all deal with hwloc bitmaps, which are
used to represent sets of nodes, cpus, etc. An important thing to note
is that such bitmaps can be infinite, in the sense that a finite
header may be followed by an infinite trailer of 1-bits (with no
0-bits anymore).

[list_begin definitions]
[comment ---------------------------------------------------------------------]
[call [cmd {hwloc bitmap}] [method empty]]
[call [cmd {hwloc bitmap}] [method full]]

These two methods return the empty (No bits set) and full (All bits set,
infinite) bitmaps, respectively. 

[comment ---------------------------------------------------------------------]
[call [cmd {hwloc bitmap}] [method allbut] [arg id]]
[call [cmd {hwloc bitmap}] [method only] [arg id]]

These two methods return bitmaps with a single bit unset ([method allbut])
or set ([method only]). The result of [method allbut] is an infinite bitmap.
The methods throw an error if a negative [arg id] is specified.

[comment ---------------------------------------------------------------------]
[call [cmd {hwloc bitmap}] [method first] [arg bitmap]]

This method returns the id of the first set bit found in the [arg bitmap],
counting up from bit 0. The method will throw an error if the [arg bitmap]
is empty, i.e. has no bit set at all.

[call [cmd {hwloc bitmap}] [method last] [arg bitmap]]

This method returns the id of the last set bit found in the [arg bitmap],
counting up from bit 0. The method will throw an error if the [arg bitmap]
is either empty, i.e. has no bit set at all, or infinite, i.e. has no
defined "last" bit.

[call [cmd {hwloc bitmap}] [method next] [arg bitmap] [arg prev]]

This method, like [method first], returns the id of the first set bit found
in the [arg bitmap], however all bits up to and including bit [arg prev] are
ignored. Negative values for [arg prev] are allowed.

[para] The method will throw an error if the [arg bitmap] is empty, i.e.
has no bit set at all, or if it is empty after bit [arg prev].

[para] The method [method first] is, in essence, the same as [method next]
called with "prev == [const -1]", i.e.

[example {
    [first bitmap] == [next bitmap -1]
}]

[comment ---------------------------------------------------------------------]
[call [cmd {hwloc bitmap}] [method from_ulong] [arg mask]]

This method takes an unsigned integer and converts it into a bitmap, based on
the integer's binary representation. The LSB is mapped to bit 0 of the bitmap.

[call [cmd {hwloc bitmap}] [method to_ulong] [arg bitmap]]

This method is the complement to [method from_ulong], converting the [arg bitmap]
into an unsigned integer value represented by the bits of the map.

[comment ---------------------------------------------------------------------]
[call [cmd {hwloc bitmap}] [method is_empty] [arg bitmap]]
[call [cmd {hwloc bitmap}] [method is_full] [arg bitmap]]

These two methods test if the specified [arg bitmap] is either the empty or the
full bitmap, i.e. no bits sets vs. all bits set, respectively. The result of the
methods is a boolean value, [const true] if the test succeeds, and [const false]
otherwise.

[comment ---------------------------------------------------------------------]
[call [cmd {hwloc bitmap}] [method not] [arg bitmap]]

This method takes the [arg bitmap], inverts all bits, and returns this as its
result.

[comment ---------------------------------------------------------------------]
[call [cmd {hwloc bitmap}] [method singlify] [arg bitmap]]

This method is similar to [method first], except that its result is a bitmap,
not a bit number, and it accepts the empty bitmap. In the resulting bitmap all
bits but the first set bit of the input are unset. If the input is empty the
output is simply empty as well.

[comment ---------------------------------------------------------------------]
[call [cmd {hwloc bitmap}] [method weight] [arg bitmap]]

This method counts the number of set bits in the specified [arg bitmap] and
returns this as its result. The method will throw an error if the [arg bitmap]
is infinite, i.e. has not a bounded weight.

[comment ---------------------------------------------------------------------]
[call [cmd {hwloc bitmap}] [method and] [arg bitmap1] [arg bitmap2]]
[call [cmd {hwloc bitmap}] [method andnot] [arg bitmap1] [arg bitmap2]]
[call [cmd {hwloc bitmap}] [method or] [arg bitmap1] [arg bitmap2]]
[call [cmd {hwloc bitmap}] [method xor] [arg bitmap1] [arg bitmap2]]

These four methods perform standard bitwise operations on the two bitmaps,
returning the operations' result. See the following mapping to C expressions:

[example {
    and    A B == A & B
    andnot A B == A & ~B
    or     A B == A | B
    xor    A B == A ^ B
}]

[comment ---------------------------------------------------------------------]
[call [cmd {hwloc bitmap}] [method is_equal] [arg bitmap1] [arg bitmap2]]

This method tests if the two specified bitmaps are equal in all respects.
The result of the method is a boolean value, [const true] if the bitmaps
are equal, and [const false] otherwise.

[comment ---------------------------------------------------------------------]
[call [cmd {hwloc bitmap}] [method is_included] [arg bitmapsub] [arg bitmapsuper]]

This method tests if [arg bitmapsub] is a subset of [arg bitmapsuper].
The result of the method is a boolean value, [const true] if the bitmap
is a subset, and [const false] otherwise.

[comment ---------------------------------------------------------------------]
[call [cmd {hwloc bitmap}] [method compare] [arg bitmap1] [arg bitmap2]]
[call [cmd {hwloc bitmap}] [method compare_first] [arg bitmap1] [arg bitmap2]]

These two method compare the two bitmaps by comparing the last and first set
bits respectively. The result of the methods are the difference between the
ids of these bits. The empty bitmap is treated as if it had bit [const -1] set.

[comment {
    It is unclear how 'compare' works for infinite bitmaps, with their
    undefined last bit
}]

[comment ---------------------------------------------------------------------]
[call [cmd {hwloc bitmap}] [method intersects] [arg bitmap1] [arg bitmap2]]

This method tests if the two specified bitmaps intersect with each other, i.e if
the result of [method and]'ing them together is not empty. The result of the method
is a boolean value, [const true] if the bitmaps have a non-empty intersection,
and [const false] otherwise.

[comment ---------------------------------------------------------------------]
[call [cmd {hwloc bitmap}] [method set] [arg bitmap] [arg id]]
[call [cmd {hwloc bitmap}] [method clear] [arg bitmap] [arg id]]
[call [cmd {hwloc bitmap}] [method is_set] [arg bitmap] [arg id]]

These three methods set, unset, and test the bits in a [arg bitmap]. The
first two methods return the modified bitmap as result, whereas the last
returns a boolean value, [const true] if the bit is set, and [arg false]
otherwise. The methods throw an error if a negative [arg id] is specified.

[comment ---------------------------------------------------------------------]
[call [cmd {hwloc bitmap}] [method set_range] [arg bitmap] [arg begin] [arg end]]
[call [cmd {hwloc bitmap}] [method clear_range] [arg bitmap] [arg begin] [arg end]]

These two methods are extensions of the modifiers in the previous group from
single bits to bit ranges. The range of bits to set or clear is specified by two
bit ids. The methods return the modified bitmap as their result.

The methods throw an error if a negative id is specified, for either [arg begin]
or [arg end]. Specifying an [arg end] < [arg begin] is acceptable however, this
specifies an empty range and the method will return the unmodified [arg bitmap].

[list_end]
