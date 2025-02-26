Dennis Karpenko (dk1111), Suk Jin Hong (sh1949)


use ./memtest to run memtest (for the correctness test)

Test Plan: 
malloc a bunch, free a bunch. run memgrind -- see if that works. 
We can intentionally try and break malloc and free, see if the code makes things work that are not supposed to.
Ie., we can malloc 5000 bytes, or try and free an array of null pointers. 
we can iterate through the heap, and see if somehow iterate out of it


Test Program Descriptions: 

Test 1-3 are as written in (4.0) of the project writeup.
We have test 3 iterate through the array in both cases, test 6 does it fully random. 

test 4:
This randomly either sequentiually allocates or sequentially deallocates a ptr in an array (does a 50/50 flip with rand()).
It's basically test 6, but test 6 is truly random in what entry it allocates or deallocates so it fails a bunch (more later).

test 5:
We malloc every entry in a size 100 ptr array.
then we free the even entries.
then we free the rest
This is a test of our leak_detector and coalesce() functions, because in the last free loop, it must coalesce with every free.

test 6: 
This randomly chooses an entry of a 120 size array, and does a 50/50 flip on whether to free or malloc it. 
This test whether our badPointer() function in free() works. The max allocation is 14,400 bytes (?), in theory.
This is forces malloc to fail, so there should be a bunch of errors.


If you're reading this, and it causes issues with grading, comment out the function.



Design Notes: 

Of note I suppose is the bit arithmetic in malloc and free. In particular, we forgot to account for the header when freeing and mallocing initially.
Ie., in free would not adjust the size of the new (big) free chunk to include the second chunks header, so we were 8 bits off.
this took Suk Jin 2 hours to debug after we sat together for another 2.

Also, we forgot to add 8 bytes to the header location of the malloced free chunk (if we split a free block into a active and free block),
again, we were 8 bits off. This took like 3 hours to debug together (－‸ლ).

That's pretty much, everything else is standard (we didn't do anything crazy with the headers or our malloc and free).

