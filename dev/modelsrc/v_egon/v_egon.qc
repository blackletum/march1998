$modelname valve/models/v_egon.mdl
$cd ".\"
$cdtexture ".\maps_8bit"

$scale 1.0

// whole body
$body body "egon_newgun"

$bodygroup "hands"
{
studio "egon_hands_beta"
studio "egon_hands_ivan"
}


$origin 2 4 64

$attachment 0 "Gauss" -10 2.5 0 X -1

$sequence idle1 "anims/egon_idle1(sway)" loop fps 30
$sequence fidget1 "anims/egon_fidget1(handle)" fps 30
$sequence altfireon "anims/egon_fire2altfire" fps 30
$sequence altfirecycle "anims/egon_altfirecycle" fps 30 loop
$sequence altfireoff "anims/egon_altfire2fire" fps 30
$sequence fire1 "anims/egon_firecycle1" fps 30 loop
$sequence fire2 "anims/egon_firecycle2" fps 30 loop
$sequence fire3 "anims/egon_firecycle3" fps 30 loop
$sequence fire4 "anims/egon_firecycle4" fps 30 loop

$sequence draw	"anims/egon_draw" fps 30
$sequence holster "anims/egon_holster" fps 30	