Implementation of a Concurrent Merkle Tree

Todo List:
* Test insert/delete when using multiple threads
    * insert may be working, correctness testing will be needed.
* Work on what the verify() function will look like
* Add Get function
* If Get/Contains need to verify the hashes along the node path look at ideas on how to do that. If other operations are occuring this may lead to inconsistant results.
    * look at existing implementations for get/contains to see if they recompute the hashes for these operations. If so then I will likely have to follow suite. I may create an unverified_contains which would be incredibly fast, but not have the verification guarentee.
