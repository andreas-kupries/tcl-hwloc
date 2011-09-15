
The objects in a topology instance are organized in a combination of
tree and matrix, with the tree structure providing the information
about nesting and connectivity between them, and the matrix providing
direct addressability.

[para] The object identifiers used by the package are based on the
latter, identifying each object by depth and index in that layer. A
valid object identifier follows the rules below:

[list_begin enumerated]
[enum] It is a list containing exactly two elements.

[enum] Each element is an integer number.

[enum] The first element is the depth of the object in the topology,
       counted from [const 0], with [const 0] refering to the topmost
       layer, the [term root] layer, and increasing as we move deeper
       down. The maximum allowed value is one less than the result of
       the instance method [method depth] (without arguments).

[enum] The second element is the index of the object in its layer,
       counted from [const 0], with [const 0] refering to the leftmost
       object, and increasing as we move to the right. The maximum
       allowed value is one less than the result of the instance method
       [method width] for the specified depth.
[list_end]
