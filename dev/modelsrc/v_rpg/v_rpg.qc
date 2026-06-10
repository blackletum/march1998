$modelname v_rpg.mdl
$cd "."
$cdtexture old_maps_8bit

$scale 1.0

$origin 0 0 64

// whole body


$body body "rpg_rocket" 
$body body "rpg_launcher"
$body body "rpg_hand" reverse


$sequence idle "anims/rpg_idle1(simple)" loop fps 15		
$sequence idle2 "anims/rpg_idle1(simple)1" loop fps 15			
$sequence reload "anims/rpg_fire1(reload)" fps 30
$sequence fire "anims/rpg_fire2(empty)" fps 30
$sequence holster1 "anims/holster1" fps 30
$sequence draw1 "anims/draw1" fps 30
$sequence holster2 "anims/holster" fps 30
$sequence draw2 "anims/draw" fps 30

