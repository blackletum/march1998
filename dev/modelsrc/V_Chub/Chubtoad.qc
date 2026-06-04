$modelname models/chumtoad.mdl
$cd .
$cdtexture ./Maps_8bit
$scale 1.0

// whole body
$body studio "Chub_Template_Biped1"


$eyeposition 0 0 8
//$eyeposition 0 7 8

$texturegroup skinfamilies 
{
{ "CHB_Side1.bmp" "CHB_Eye1.bmp"}
{ "CHB_Skin1.bmp" "CHB_SKINEye1.bmp"}
{ "CHB_Skin2.bmp" "CHB_SKINEye4.bmp"}
{ "CHB_Skin3.bmp" "CHB_SKINEye3.bmp"}
{ "CHB_Skin4.bmp" "CHB_SKINEye2.bmp"}
}

$hbox 0 "Bip01 L Leg" -2.130000 -3.210000 -0.660000 2.969935 0.960000 1.680000
$hbox 0 "Bip01 L Leg1" -2.140000 -0.840000 -0.390000 6.698392 1.650000 1.200000
$hbox 0 "Bip01 L Foot" 0.440000 -3.530000 -2.630000 1.720000 2.637741 2.680000
$hbox 0 "Bip01 R Leg" -2.240000 -3.180000 -1.670000 2.969935 0.910000 1.160000
$hbox 0 "Bip01 R Leg1" -1.950000 -0.920000 -1.170000 6.698391 1.650000 0.350000
$hbox 0 "Bip01 R Foot" 0.460000 -3.100000 -1.850000 1.730000 2.637741 2.930000
$hbox 0 "Bip01 Spine" -2.990000 -3.870000 -5.320000 4.330000 4.180000 5.370000
$hbox 0 "Bip01 Spine1" 0.850000 -3.870000 -5.930000 5.320000 3.160000 5.980000
$hbox 0 "Bip01 Neck" -2.577006 0.000000 -3.293112 0.531618 2.187984 3.487256
$hbox 0 "Bip01 Head" 0.310000 -1.480000 -4.110000 3.120000 0.750000 4.160000
$hbox 0 "Bip01 L Arm2" -0.700000 -1.020000 -0.530000 2.142583 0.960000 0.870000
$hbox 0 "Bip01 L Hand" -0.220000 -0.300000 -0.820000 1.333709 0.870000 1.030000
$hbox 0 "Bip01 L Finger0" 0.620000 -0.210000 -1.120000 2.380000 0.890000 0.730000
$hbox 0 "Bip01 R Arm2" -0.560000 -1.120000 -0.980000 2.142583 0.850000 0.470000
$hbox 0 "Bip01 R Hand" -0.100000 -0.240000 -1.200000 1.380000 0.940000 0.610000
$hbox 0 "Bip01 R Finger0" -0.110000 -0.170000 -1.110000 2.430000 0.940000 0.740000

$sequence idle "Chub_idle(subtle)1" LOOP fps 16 ACT_IDLE 10
$sequence idle2 "Chub_fidget1(scratch)" fps 16 ACT_IDLE 1
$sequence idle3 "Chub_fidget2(croak)" fps 16 ACT_IDLE 1
$sequence flinch1 "Chub_flinch(big)1" fps 16 ACT_BIG_FLINCH 1
$sequence flinch2 "Chub_flinch(small)1" fps 16 ACT_SMALL_FLINCH 1
$sequence hop "Chub_hop1" LOOP fps 60 LX ACT_WALK 1
$sequence hop "Chub_BigHop1" LOOP fps 40 LX ACT_RUN 1
$sequence swim "Chub_swim1" LOOP fps 16 ACT_SWIM 1
//$sequence die1 "Chub_Die(Back)1" fps 30 LX ACT_DIESIMPLE 1
//$sequence die2 "Chub_die(backwards)1" fps 25 LX ACT_DIESIMPLE 1
$sequence die3 "Chub_Die(Flop)1" fps 30 LX ACT_DIESIMPLE 1
$sequence pdwiggle "Chub_play_dead(wiggle1)1" fps 40 ACT_DIEVIOLENT 1
$sequence playdead1 "Chub_play_dead(flip_ovr)1" fps 16 ACT_STAND 1
$sequence pdwiggle "Chub_play_dead(wiggle1)1" fps 16 ACT_TWITCH 1
$sequence playdead2 "Chub_play_dead1" fps 16 ACT_FEAR_DISPLAY 1