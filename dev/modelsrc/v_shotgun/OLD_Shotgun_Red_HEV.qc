/*
==============================================================================

shotgun viewmodel by drooga

==============================================================================
*/

$modelname v_shotgun.mdl
$cd ./
$cdtexture maps_8bit
$scale 1.0 

// whole body
$body studio "v_shotgun_reference"
$body studio "max_shell"

//$origin       SIDE-SIDE IN-OUT  UP-DOWN

$origin 0 0 64

$sequence sm_idle "small_idle" fps 9 loop
$sequence shoot1 "shoot_reload" fps 20 { event 5001 0 "31" }
$sequence shoot2 "new_shoot" fps 20 { event 5001 0 "31" }
$sequence shoot_big "shoot_bigammo" fps 13 { event 5001 0 "41" }
$sequence reload "reload_short" fps 16
$sequence pump "pump_after_reload_full" frame 0 8 fps 15
$sequence start_reload "weird_reload" frame 0 8 fps 12
$sequence draw "draw_weapon" fps 24
$sequence reholster "reholster" fps 30
$sequence idle4 "idle4" fps 9 loop
$sequence deepidle "deep_idle" fps 12 loop
