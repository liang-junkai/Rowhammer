secret is stored as a global variable in line39 of edd.c as s_hex

line64 read s_hex to sec, I suppose we should the flip should wait after this read finish

line 71 is simulation bit_flip, we can add flip here directly to sec2. (sec2 is copied from sec in line 68.
