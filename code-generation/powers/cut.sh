# Darksun one
dd bs=1 skip=270192 count=6272 if=DSUN.EXE of=dsun1.dat
# Darksun 2
dd bs=1 skip=73781 count=23360 if=RESOURCE.GFF of=dsun2.dat
# Darksun Online
dd bs=1 skip=2361268 count=23360 if=MDARK.EXE of=dso.dat
# Darksun Online othe rmonster attacks
dd bs=1 skip=1643318 count=1825 if=MDARK.EXE of=dso-extra.dat && xxd dso-extra.dat 
