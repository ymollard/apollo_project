# apollo_project
C++ hack to convert raw binaries from the NASA's Apollo 15/16 mission (generated IBM 360) to modern files

# Manual alignments
Some corrupted files have been manually aligned to dump a valid cyclic file:
* DD019717_F1.DAT: Removed incomplete records between 0x1AEED4 and 0x1AF036
