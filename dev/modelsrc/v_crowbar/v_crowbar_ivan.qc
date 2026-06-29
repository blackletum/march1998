// Created by Crowbar 0.74

$modelname "v_crowbar.mdl"
$cd "."
$cdtexture "."
$cliptotextures
$scale 1.0

$bodygroup "studio"
{
	studio "crowbar_reference(Just_the_Crowbar)"
}

$bodygroup "studio"
{
	studio "ivan_hands"
}



$flags 0


$cbox 0 0 0 0 0 0

$bbox 0 0 0 0 0 0

$hbox 0 "Bip01 R Arm2" 0 -2.807855 -4.008998 11.368423 3.025234 1.736644
$hbox 0 "Bip01 R Hand" 0 -0.87054 -5.937769 7.852933 2.943182 19.957901

$sequence "idle1" {
	"v_crowbar_anims\idle1"
	fps 13
}
$sequence "draw" {
	"v_crowbar_anims\draw"
	fps 24
}
$sequence "holster" {
	"v_crowbar_anims\holster"
	fps 24
}
$sequence "attack1" {
	"v_crowbar_anims\attack1"
	fps 22
}
$sequence "attack1miss" {
	"v_crowbar_anims\attack1miss"
	fps 22
}
$sequence "attack2" {
	"v_crowbar_anims\attack2"
	fps 22
}
$sequence "attack2hit" {
	"v_crowbar_anims\attack2hit"
	fps 22
}
$sequence "attack3" {
	"v_crowbar_anims\attack3"
	fps 24
}
$sequence "attack3hit" {
	"v_crowbar_anims\attack3hit"
	fps 24
}
$sequence "idle2" {
	"v_crowbar_anims\idle2"
	fps 15
	loop
}
$sequence "idle3" {
	"v_crowbar_anims\idle3"
	fps 15
	loop
}
