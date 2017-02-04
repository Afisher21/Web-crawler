Not sure why this is happening, but when unpacking the project 2 things need to be changed:
1) Remove all parser lib objects from the additional dependencies tab, and then re-add them. For some reason it is locking them to their file location on my personal computer.
2) Add the additional dependency “winmm.lib” to the linker tab. Again, this is not an issue when I run the project locally, but must have been lost somewhere in removing files
