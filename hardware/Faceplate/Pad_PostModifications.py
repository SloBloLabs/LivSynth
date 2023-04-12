from pcbnew import *

b = GetBoard()

# set a number on speaker grill pads
i = 0
for p in b.GetPads():
    if(p.GetDrillSize().x < 2000000):
        i += 1
        p.SetNumber("P%03d" % i)

# remove B.Mask on speaker grill pads to darken back side of the panel
for p in b.GetPads():
    if(p.GetDrillSize().x < 2000000):
        p.SetLayerSet(p.GetLayerSet().RemoveLayer(B_Mask))