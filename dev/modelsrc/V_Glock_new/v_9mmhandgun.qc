/*
==============================================================================

	DermokerE398 - Glock Viewmodel (Early)

==============================================================================
*/

$modelname v_glock.mdl
$cd "./SMD"
$cdtexture "./maps_8bit"
$scale 1.0

// whole body
$body studio "glock_reference(gun)"
$body studio "glock_reference(clip)"

$bodygroup silencer
{
blank
studio "glock_reference(silencer)"
}

$bodygroup hands
{
	studio "glock_reference(beta_hands)"
	studio "glock_reference(ivan_hands)"
}

$origin 0 0 64

$sequence idle1 "glock_idle1" fps 15
$sequence idle2 "glock_idle2" fps 15
$sequence idle3 "glock_idle3" fps 15
$sequence shoot "glock_shoot" fps 25
$sequence shoot_empty "glock_shoot_slideback" fps 25

$sequence reload "glock_reload" fps 12 {
{ event 5004 19 "items/9mmclip2.wav" }
{ event 5004 24 "items/9mmclip1.wav" }
{ event 5004 35 "items/guncock1.wav" }
}

$sequence reload_noshot "glock_reload_from_idle" fps 18 {
{ event 5004 4 "items/9mmclip2.wav" }
{ event 5004 23 "items/9mmclip1.wav" }
}
$sequence draw "glock_draw" fps 16
$sequence holster "glock_holster" fps 16
$sequence add_silencer1 "glock_add_silencer1" fps 18
$sequence add_silencer2 "glock_add_silencer2" fps 18 { event 5004 8 "weapons/glockSilencerOn.wav" }
$sequence remove_silencer1 "glock_remove_silencer1" fps 18 
$sequence remove_silencer2 "glock_remove_silencer2" fps 18 { event 5004 12 "weapons/glockSilencerOff.wav" }