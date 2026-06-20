/*
==============================================================================

shared info between barney and construction

==============================================================================
*/

// 21 hit box(es)
$hbox 3 "Bip01 Pelvis" -9.890000 -5.660000 -8.600000 2.940000 6.520000 6.790000
$hbox 6 "Bip01 L Leg" 0.000000 -5.920000 -4.000000 19.370001 3.780000 3.730000
$hbox 6 "Bip01 L Leg1" 0.000000 -5.690000 -3.620000 18.830000 3.070000 2.940000
$hbox 6 "Bip01 L Foot" 0.000000 -2.990000 -4.040000 6.420000 8.680000 1.610000
$hbox 7 "Bip01 R Leg" 0.000000 -5.930000 -3.870000 19.270000 3.750000 3.790000
$hbox 7 "Bip01 R Leg1" -0.330000 -5.730000 -2.980000 18.950001 2.960000 3.190000
$hbox 7 "Bip01 R Foot" 0.000000 -3.240000 -2.320000 6.210000 8.440000 3.280000
$hbox 3 "Bip01 Spine" 0.000000 -4.890000 -6.970000 8.370000 8.000000 6.730000
$hbox 2 "Bip01 Spine2" 0.000000 -3.460000 -8.320000 6.110000 8.000000 7.530000
$hbox 2 "Bip01 Spine3" -1.000000 -5.990000 -8.220000 5.960000 8.000000 8.480000
$hbox 4 "Bip01 L Arm" -0.400000 -4.180000 0.000000 7.200000 4.750000 4.200000
$hbox 4 "Bip01 L Arm1" -2.520000 -4.570000 -3.920000 9.860000 2.280000 3.430000
$hbox 4 "Bip01 L Arm2" 0.000000 -2.850000 -2.080000 11.900000 2.850000 2.750000
$hbox 4 "Bip01 L Hand" 0.000000 -1.320000 -2.290000 4.660000 2.600000 1.650000
$hbox 5 "Bip01 R Arm" 0.000000 -4.150000 -4.190000 7.530000 4.120000 4.240000
$hbox 5 "Bip01 R Arm1" -2.690000 -4.650000 -3.420000 9.470000 2.280000 3.760000
$hbox 5 "Bip01 R Arm2" 0.000000 -2.730000 -2.560000 11.000000 3.140000 2.190000
$hbox 5 "Bip01 R Hand" 0.000000 -1.320000 -2.910000 10.380000 3.230000 4.000000
$hbox 1 "Bip01 Head" -0.660000 2.250000 -4.240000 6.000000 7.970000 3.630000
$hbox 10 "Bip01 Head" -0.660000 -4.250000 -5.240000 6.000000 2.250000 4.630000
$hbox 10 "Bip01 Head" 6.000000 -4.250000 -5.240000 11.190000 7.970000 4.630000

// 98 animation sequence(s)
$sequence "idle1" "idle1" fps 15 loop ACT_IDLE 50 
$sequence "idle2" "idle2" fps 15 ACT_IDLE 1 
$sequence "idle3" "idle3" fps 15 ACT_IDLE 1 
$sequence "idle4" "idle4" fps 12 ACT_IDLE 1 
$sequence "idle5" "idle5" fps 15 ACT_IDLE 1 
$sequence "idle6" "idle6" fps 15 ACT_IDLE 1 
$sequence "walk" "walk" LX fps 30 loop ACT_WALK 1 { event 1004 0 "common/npc_step1.wav" } { event 1004 11 "common/npc_step3.wav" } 
$sequence "run" "run" LX fps 30 loop ACT_RUN 1 { event 1004 4 "common/npc_step2.wav" } { event 1004 12 "common/npc_step4.wav" } 
$sequence "shootgun" "shootgun" fps 10 ACT_RANGE_ATTACK1 1 { event 5001 0 "21" } { event 3 0 } 
$sequence "draw" "draw" fps 15 ACT_ARM 1 { event 2 4 } 
$sequence "disarm" "disarm" fps 10 ACT_DISARM 1 { event 4 15 } 
$sequence "reload" "reload" fps 15 ACT_RELOAD 1 { event 5 8 } 
$sequence "turnleft" "turnleft" fps 15 ACT_TURN_LEFT 1 
$sequence "turnright" "turnright" fps 15 ACT_TURN_RIGHT 1 
$sequence "smlflinch" "smlflinch" fps 30 ACT_SMALL_FLINCH 1 
$sequence "biglflinch" "biglflinch" fps 30 ACT_BIG_FLINCH 1 
$sequence "cower_stand" "cower_stand" fps 14 
$sequence "locked_door" "locked_door" fps 28 loop 
$sequence "fall_loop" "fall_loop" fps 28 loop 
$sequence "barn_wave" "barn_wave" fps 22 
$sequence "beat_grunt" "beat_grunt" fps 14 
$sequence "beat_gruntidle" "beat_gruntidle" fps 14 loop 
$sequence "flashlight" "flashlight" fps 14 loop { event 1004 5 "*buttons/latchunlocked2.wav" } 
$sequence "diesimple" "diesimple" X fps 15 ACT_DIESIMPLE 1 
$sequence "dieforward" "dieforward" X fps 15 ACT_DIEFORWARD 1 
$sequence "diebackward" "diebackward" LY fps 15 ACT_DIEBACKWARD 1 
$sequence "dieviolent" "dieviolent" X fps 15 ACT_DIEVIOLENT 1 
$sequence "diecrump" "diecrump" X fps 15 ACT_DIE_GUTSHOT 1 
$sequence "barnaclehit" "barnaclehit" fps 15 ACT_BARNACLE_HIT 1 
$sequence "barnaclepull" "barnaclepull" fps 45 loop ACT_BARNACLE_PULL 1 
$sequence "barnaclecrunch" "barnaclecrunch" fps 5 ACT_BARNACLE_CHOMP 1 
$sequence "barnaclechew" "barnaclechew" fps 5 loop ACT_BARNACLE_CHEW 1 
$sequence "lying_on_back" "lying_on_back" fps 1 
$sequence "lying_on_side" "lying_on_side" fps 1 
$sequence "lying_on_stomach" "lying_on_stomach" fps 1 
$sequence "stuffed_in_vent" "stuffed_in_vent" fps 1 
$sequence "action" "action" fps 15 
$sequence "halting_player" "halting_player" fps 15 loop 
$sequence "standing_idle" "standing_idle" fps 15 loop 
$sequence "standing_looking" "standing_looking" fps 5 loop 
$sequence "cprbarney" "cprbarney" fps 20 loop 
$sequence "cprbarneyrevive" "cprbarneyrevive" fps 20 
$sequence "kibbitz_idle" "kibbitz_idle" fps 15 loop 
$sequence "c1a4_top" "c1a4_top" fps 15 
$sequence "inspect_floor" "inspect_floor" fps 15 loop 
$sequence "c1a4_bottom" "c1a4_bottom" fps 15 
$sequence "c1a4_barney_flung" "c1a4_barney_flung" AX AY AZR fps 36 { event 1000 1 } 
$sequence "barney_dragvent" "barney_dragvent" fps 15 { event 1000 1 } 
$sequence "barney_fight" "barney_fight" AX AY AZR fps 17 {
  { event 1000 1 } 
  { event 1004 131 "weapons/cbar_hitbod1.wav" } 
  { event 1004 147 "weapons/cbar_hitbod2.wav" } 
  { event 1004 159 "weapons/cbar_hitbod3.wav" } 
  { event 1004 169 "weapons/cbar_hitbod1.wav" } 
  { event 1004 216 "weapons/xbow_hitbod1.wav" } 
  { event 1003 216 "splatbz" } 
 }
$sequence "dying_barney" "dying_barney" fps 18 { event 1000 1 } 
$sequence "dying_barneyidle" "dying_barneyidle" fps 12 loop 
$sequence "dying_friend" "dying_friend" fps 18 
$sequence "dying_friendidle" "dying_friendidle" fps 12 loop 
$sequence "c1a3_bidle" "c1a3_bidle" AX AY AZR fps 3 
$sequence "c1a3_ventb" "c1a3_ventb" AX AY AZR fps 18 { event 1000 1 } 
$sequence "c1a3_emergeidle" "c1a3_emergeidle" AX AY AZR fps 16 loop 
$sequence "c1a3_emerge" "c1a3_emerge" AX AY AZR fps 16 { event 1000 1 } 
$sequence "haulbarney" "haulbarney" AX AY AZR fps 18 { event 1000 1 } 
$sequence "intropush" "intropush" fps 16 {
  { event 1004 15 "*buttons/blip1.wav" } 
  { event 1004 24 "*buttons/blip1.wav" } 
  { event 1004 34 "*buttons/blip1.wav" } 
  { event 1004 44 "*buttons/blip1.wav" } 
 }
$sequence "truckdieidle" "truckdieidle" AX AY AZR fps 15 
$sequence "truckdie" "truckdie" AX AY AZR fps 15 { event 1000 1 } { event 1008 70 "*barney/ba_die3.wav" } 
$sequence "fence" "fence" AX AY AZR fps 24 {
  { event 1000 70 } 
  { event 1003 70 "lightning" } 
  { event 1001 70 } 
  { event 1004 72 "weapons/electro4.wav" } 
  { event 1008 100 "barney/ba_pain2.wav" } 
 }
$sequence "sit1" "sit1" fps 20 loop { event 1003 50 "introchair" } { event 1003 102 "introchair" } 
$sequence "stuka_warn_1" "stuka_warn_1" fps 17 
$sequence "lean" "lean" fps 17 { event 1004 15 "*buttons/blip1.wav" } 
$sequence "leanidle" "leanidle" fps 14 
$sequence "almostidle" "almostidle" fps 16 loop 
$sequence "almost" "almost" fps 26 { event 1000 1 } 
$sequence "laseridle" "laseridle" fps 6 
$sequence "laser_top" "laser_top" fps 6 { event 1000 0 } 
$sequence "laser_bottom" "laser_bottom" fps 6 { event 1000 0 } 
$sequence "barneyfallidle" "barneyfallidle" fps 33 loop 
$sequence "barneyfall" "barneyfall" fps 32 {
  { event 1000 1 } 
  { event 1008 2 "barney/ba_pain1.wav" } 
  { event 1008 24 "barney/ba_pain2.wav" } 
  { event 1003 55 "bsplat" } 
  { event 1008 55 "debris/bustflesh1.wav" } 
 }
$sequence "c3a2_draw_1" "c3a2_draw_1" fps 16 { event 2 13 } 
$sequence "unlatch_1" "unlatch_1" fps 12 { event 1003 7 "barney_door_lock" } 
$sequence "retina_1" "retina_1" fps 20 
$sequence "relaxstand_1" "relaxstand_1" fps 10 loop 
$sequence "assassinated_1" "assassinated_1" X fps 18 {
  { event 1000 1 } 
  { event 1003 1 "blood1" } 
  { event 1003 2 "blood2" } 
 }
$sequence "sterilizer_idle" "sterilizer_idle" AX AY AZR fps 17 loop 
$sequence "sterilizer" "sterilizer" fps 17 
$sequence "c1a0_lean_idle" "c1a0_lean_idle" AX AY AZR fps 17 loop 
$sequence "c1a0_lean" "c1a0_lean" AX AY AZR fps 17 
$sequence "stuka_warn_2" "stuka_warn_2" AX AY AZR fps 17 
$sequence "train_assist_idle" "train_assist_idle" AX AY AZR fps 17 loop 
$sequence "train_assist" "train_assist" AX AY AZR fps 17 
$sequence "train_flung" "train_flung" X AX AY AZR fps 36 { event 1000 0 } 
$sequence "pepsiswing_1" "pepsiswing_1" fps 25 { event 1003 1 "die" } { event 1003 8 "swing" } 
$sequence "pepsipush_1" "pepsipush_1" X fps 18 { event 1003 51 "topple" } { event 1003 90 "horror" } 
$sequence "hambone" "hambone" fps 30 
$sequence "c3a2_draw_2" "c3a2_draw_2" fps 16 { event 2 13 } 
$sequence "unlatch_2" "unlatch_2" fps 12 { event 1003 7 "barney_door_lock" } 
$sequence "retina_2" "retina_2" fps 20 
$sequence "relaxstand_2" "relaxstand_2" fps 10 loop 
$sequence "assassinated_2" "assassinated_2" X fps 18 {
  { event 1000 1 } 
  { event 1003 1 "blood1" } 
  { event 1003 2 "blood2" } 
 }
$sequence "trackswitch" "trackswitch" fps 22 { event 1003 14 "trainarmswitch" } 
$sequence "pepsiswing_2" "pepsiswing_2" fps 25 { event 1003 1 "die" } { event 1003 8 "swing" } 
$sequence "pepsipush_2" "pepsipush_2" fps 18 { event 1003 51 "topple" } { event 1003 90 "horror" } 
$sequence "buttonpush" "buttonpush" fps 20 { event 1003 7 "button" } 
