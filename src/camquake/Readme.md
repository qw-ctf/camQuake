# Camquake

## Workflow
enable camquake, none of the "camquake" comamnds will work without this  
```camquake_enable 1```  

enable rendering  
```camquake_render 1```  

generate a new camera setup  
```camquake setup add "my_cam"```  
you can have multiple camera setups loaded at once, you will however need to supply the name to the commands

set start/stop time in seconds  
```camquake setup start_time "my_cam" 0.000000```  
```camquake setup stop_time "my_cam" 2.000000```  

now its time to set some points  
for every setup there is a view path and a camera path points can either be set directly via coordinates or via the keyword ```current``` which will use your current position.  
for example, to set a camera point directly:  
```camquake setup add_camera_point "my_cam" 252.351837 -19.253332 -219.774139```  
to set a view point directly:  
```camquake setup add_view_point "my_cam" 249.264099 30.651236 -219.774139```  
a convinient setup is to bind these to a key:  
```bind mouse1 "camquake setup add_camera_point current; camquake setup add_view_point current"```  

the interpolation between path points is handled differently depending on the amounts of points in a path:  
- 1 Point:  Stationary
- 2 Points:  Linear interpolation
- 3 Points:  Bezier interpolation
- 4+ Points:  CatmullRom interpolation  
you will se the interpolated curve if camquake_render is set to 1  
  
once you've setup a path type ```camquake play "my_cam"```

# editing
for editing you need to select a path  
```camquake select "my_cam"``  
then you need to enter edit mode  
```camquake edit```  
in this mode you can select points and move them ```F1``` toggles a help display  
the basics are:  
hover over a point to select it, it should turn blue (if you havent changed the colros). keep mosue1 pressed and move the mouse to move the point.  
the directions the point will be moved is indicated at the top of the screen and via 2 lines: red for left/right and green for up down.  
you can change the movement directions by pressing 1|2.  Available directions are 
 "xy", "xz", "yz", "view", "x", "y" and "z"  
  pressing ```3``` will allow you to switch between moving a point and moving the whole path.
  ```4``` and ```5``` will divide/multiply the movement multiplier by 10
 




