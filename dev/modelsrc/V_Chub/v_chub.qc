

$modelname models/v_chub.mdl
$cd .
$cdtexture ./maps_8bit

$scale 1.0

$origin 0 0 64

$hbox 0 "Bip01 L Arm2" 0.290000 -2.800000 -1.730000 11.680000 3.020000 4.000000
$hbox 0 "Bip01 L Hand" 0.130000 -0.870000 -1.940000 3.890000 1.030000 1.790000
$hbox 0 "Bip01 R Arm2" 2.400000 -1.570000 -3.220000 11.368421 4.270000 2.510000
$hbox 0 "Bip01 R Hand" -0.190000 -0.870000 -1.790000 3.890000 3.006830 1.810000
$hbox 0 "Bip01 R Finger0" 1.120000 -0.600000 -0.430000 2.520000 0.440000 0.880000
$hbox 0 "Bip02 L Leg" -1.340000 -1.640000 -0.340000 2.030000 0.990000 0.850000
$hbox 0 "Bip02 R Leg" -1.140000 -1.620000 -0.850000 2.090000 0.870000 0.360000
$hbox 0 "Bip02 Spine" -1.520000 -2.010000 -2.540000 1.793749 1.570000 2.330000
$hbox 0 "Bip02 Spine1" 0.180000 -1.950000 -3.020000 2.598145 2.120000 3.030000
$hbox 0 "Bip02 Neck" -1.430381 -1.620000 -1.586158 0.300000 1.117694 1.863878
$hbox 0 "Bip02 Head" 0.080000 -1.140000 -2.010000 1.610000 0.380000 2.120000


// whole body
$body body "vchub_Template(toad)" 

$bodygroup "hands"
{
    studio "vchub_Template(beta_hands)"
    studio "vchub_Template(ivan_hands)"
}

$texturegroup skinfamilies 
{
{ "CHB_Side1.bmp" "CHB_Eye1.bmp"}
{ "CHB_Skin1.bmp" "CHB_SKINEye1.bmp"}
{ "CHB_Skin2.bmp" "CHB_SKINEye4.bmp"}
{ "CHB_Skin3.bmp" "CHB_SKINEye3.bmp"}
{ "CHB_Skin4.bmp" "CHB_SKINEye2.bmp"}
}

$sequence idle1 "vchub_Idle1(Subtle)" loop fps 16
$sequence fidgetLick "vchub_Fidget1(lick)" fps 16
$sequence fidgetCroak "vchub_Fidget1(Croak)" fps 16
$sequence down "vchub_Down1" fps 16
$sequence up "vchub_Up1" fps 30
$sequence Throw "vchub_Throw1" fps 24 