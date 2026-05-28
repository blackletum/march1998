/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// Control Sphere - A vort's best friend
//=========================================================

// UNDONE: Don't flinch every time you get hit

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"schedule.h"
#include	"flyingmonster.h"
#include	"effects.h"

//=========================================================
// monster-specific tasks
//=========================================================
enum
{
	TASK_SPHR_FINDISLV = LAST_COMMON_TASK + 1
};

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_CNTRL_FINDISLV = LAST_COMMON_SCHEDULE + 1,
};

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define	ZOMBIE_AE_ATTACK_RIGHT		0x01
#define	ZOMBIE_AE_ATTACK_LEFT		0x02
#define	ZOMBIE_AE_ATTACK_BOTH		0x03

#define ZOMBIE_FLINCH_DELAY			2		// at most one flinch every n secs

class CControlsphere : public CFlyingMonster
{
public:
	void Spawn(void);
	void Precache(void);
	void SetYawSpeed(void);
	int  Classify(void);
	void HandleAnimEvent(MonsterEvent_t* pEvent);
	int IgnoreConditions(void);
	Schedule_t* GetScheduleOfType(int Type);
	Schedule_t* GetSchedule(void);
	void Move(float flInterval);
	void StartTask(Task_t* pTask);
	int  CheckLocalMove(const Vector& vecStart, const Vector& vecEnd, CBaseEntity* pTarget, float* pflDist);
	void MoveExecute(CBaseEntity* pTargetEnt, const Vector& vecDir, float flInterval);
	//void SetActivity(Activity NewActivity);
	BOOL ShouldAdvanceRoute(float flWaypointDist);

	float m_flNextFlinch;

	void PainSound(void);
	void AlertSound(void);
	void IdleSound(void);
	void AttackSound(void);

	static const char* pAttackSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];

	CUSTOM_SCHEDULES;

	// No range attacks
	BOOL CheckRangeAttack1(float flDot, float flDist) { return FALSE; }
	BOOL CheckRangeAttack2(float flDot, float flDist) { return FALSE; }
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	Vector m_velocity;
};

LINK_ENTITY_TO_CLASS(monster_controlsphere, CControlsphere);

//=========================================================
// custom tasks
//=========================================================
Task_t tlCntrlsphrfindislv[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_CHASE_ENEMY_FAILED	},
	//{ TASK_STOP_MOVING,		(float)0					},
	{ TASK_SPHR_FINDISLV,	(float)0		},
	//{ TASK_RUN_PATH,			(float)0		},
	//{ TASK_WAIT_FOR_MOVEMENT,	(float)0		},
};

Schedule_t slCntrlsphrfindislv[] =
{
	{
		tlCntrlsphrfindislv,
		ARRAYSIZE(tlCntrlsphrfindislv),
		0,
		0,
		"Cntrlsphrfindislv"
	},
};

Task_t tlContrlchase[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_CHASE_ENEMY_FAILED	},
	{ TASK_STOP_MOVING,		(float)0					},
	{ TASK_GET_PATH_TO_ENEMY,	(float)0		},
	{ TASK_RUN_PATH,			(float)0		},
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0		},
};

Schedule_t slContrlchase[] =
{
	{
		tlContrlchase,
		ARRAYSIZE(tlContrlchase),
		bits_COND_ENEMY_DEAD |
		bits_COND_NEW_ENEMY |
		bits_COND_CAN_RANGE_ATTACK1 |
		bits_COND_CAN_MELEE_ATTACK1 |
		bits_COND_TASK_FAILED |
		bits_COND_HEAR_SOUND,

		0,
		"Chase Enemy"
	},
};

DEFINE_CUSTOM_SCHEDULES(CControlsphere)
{
	slCntrlsphrfindislv,
	slContrlchase,
};
IMPLEMENT_CUSTOM_SCHEDULES(CControlsphere, CFlyingMonster);

const char* CControlsphere::pAttackHitSounds[] =
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char* CControlsphere::pAttackMissSounds[] =
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

const char* CControlsphere::pAttackSounds[] =
{
	"zombie/zo_attack1.wav",
	"zombie/zo_attack2.wav",
};

const char* CControlsphere::pIdleSounds[] =
{
	"zombie/zo_idle1.wav",
	"zombie/zo_idle2.wav",
	"zombie/zo_idle3.wav",
	"zombie/zo_idle4.wav",
};

const char* CControlsphere::pAlertSounds[] =
{
	"zombie/zo_alert10.wav",
	"zombie/zo_alert20.wav",
	"zombie/zo_alert30.wav",
};

const char* CControlsphere::pPainSounds[] =
{
	"zombie/zo_pain1.wav",
	"zombie/zo_pain2.wav",
};

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CControlsphere::Classify(void)
{
	return	CLASS_ALIEN_MONSTER;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CControlsphere::SetYawSpeed(void)
{
	int ys;

	ys = 120;

#if 0
	switch (m_Activity)
	{
	}
#endif

	pev->yaw_speed = ys;
}

int CControlsphere::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	// Take 30% damage from bullets
	if (bitsDamageType == 4098/*DMG_BULLET*/)
	{
		//ALERT(at_console, "THIS CODE IS RUNNING!\n");
		Vector vecDir = pev->origin - (pevInflictor->absmin + pevInflictor->absmax) * 0.5;
		vecDir = vecDir.Normalize();
		float flForce = DamageForce(flDamage);
		pev->velocity = pev->velocity + vecDir * flForce;
		flDamage *= 0.3;
	}

	// HACK HACK -- until we fix this.
	if (IsAlive())
		PainSound();
	return CBaseMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

void CControlsphere::PainSound(void)
{
	int pitch = 95 + RANDOM_LONG(0, 9);

	if (RANDOM_LONG(0, 5) < 2)
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pPainSounds[RANDOM_LONG(0, ARRAYSIZE(pPainSounds) - 1)], 1.0, ATTN_NORM, 0, pitch);
}

void CControlsphere::AlertSound(void)
{
	int pitch = 95 + RANDOM_LONG(0, 9);

	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pAlertSounds[RANDOM_LONG(0, ARRAYSIZE(pAlertSounds) - 1)], 1.0, ATTN_NORM, 0, pitch);
}

void CControlsphere::IdleSound(void)
{
	int pitch = 95 + RANDOM_LONG(0, 9);

	// Play a random idle sound
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pIdleSounds[RANDOM_LONG(0, ARRAYSIZE(pIdleSounds) - 1)], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
}

void CControlsphere::AttackSound(void)
{
	// Play a random attack sound
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pAttackSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackSounds) - 1)], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
}


//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CControlsphere::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case ZOMBIE_AE_ATTACK_RIGHT:
	{
		// do stuff for this event.
//		ALERT( at_console, "Slash right!\n" );
		CBaseEntity* pHurt = CheckTraceHullAttack(70, gSkillData.zombieDmgOneSlash, DMG_SLASH);
		if (pHurt)
		{
			if (pHurt->pev->flags & (FL_MONSTER | FL_CLIENT))
			{
				pHurt->pev->punchangle.z = -18;
				pHurt->pev->punchangle.x = 5;
				pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_right * 100;
			}
			// Play a random attack hit sound
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackHitSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackHitSounds) - 1)], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		}
		else // Play a random attack miss sound
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackMissSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackMissSounds) - 1)], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));

		if (RANDOM_LONG(0, 1))
			AttackSound();
	}
	break;

	case ZOMBIE_AE_ATTACK_LEFT:
	{
		// do stuff for this event.
//		ALERT( at_console, "Slash left!\n" );
		CBaseEntity* pHurt = CheckTraceHullAttack(70, gSkillData.zombieDmgOneSlash, DMG_SLASH);
		if (pHurt)
		{
			if (pHurt->pev->flags & (FL_MONSTER | FL_CLIENT))
			{
				pHurt->pev->punchangle.z = 18;
				pHurt->pev->punchangle.x = 5;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_right * 100;
			}
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackHitSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackHitSounds) - 1)], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		}
		else
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackMissSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackMissSounds) - 1)], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));

		if (RANDOM_LONG(0, 1))
			AttackSound();
	}
	break;

	case ZOMBIE_AE_ATTACK_BOTH:
	{
		// do stuff for this event.
		CBaseEntity* pHurt = CheckTraceHullAttack(70, gSkillData.zombieDmgBothSlash, DMG_SLASH);
		if (pHurt)
		{
			if (pHurt->pev->flags & (FL_MONSTER | FL_CLIENT))
			{
				pHurt->pev->punchangle.x = 5;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * -100;
			}
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackHitSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackHitSounds) - 1)], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		}
		else
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackMissSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackMissSounds) - 1)], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));

		if (RANDOM_LONG(0, 1))
			AttackSound();
	}
	break;

	default:
		CBaseMonster::HandleAnimEvent(pEvent);
		break;
	}
}

//=========================================================
// Spawn
//=========================================================
void CControlsphere::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), "models/sphere.mdl");
	UTIL_SetSize(pev, Vector(-1, -1, 0), Vector(1, 1, 2));

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_FLY;
	m_bloodColor = BLOOD_COLOR_GREEN;
	pev->health = gSkillData.zombieHealth;
	pev->view_ofs = Vector(0, 0, 5);// position of the eyes relative to monster's origin.
	m_flFieldOfView = VIEW_FIELD_FULL;
	m_MonsterState = MONSTERSTATE_NONE;
	m_afCapability = bits_CAP_DOORS_GROUP;
	pev->flags |= FL_FLY;
	pev->flags &= ~FL_ONGROUND;

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CControlsphere::Precache()
{
	int i;

	PRECACHE_MODEL("models/sphere.mdl");

	for (i = 0; i < ARRAYSIZE(pAttackHitSounds); i++)
		PRECACHE_SOUND((char*)pAttackHitSounds[i]);

	for (i = 0; i < ARRAYSIZE(pAttackMissSounds); i++)
		PRECACHE_SOUND((char*)pAttackMissSounds[i]);

	for (i = 0; i < ARRAYSIZE(pAttackSounds); i++)
		PRECACHE_SOUND((char*)pAttackSounds[i]);

	for (i = 0; i < ARRAYSIZE(pIdleSounds); i++)
		PRECACHE_SOUND((char*)pIdleSounds[i]);

	for (i = 0; i < ARRAYSIZE(pAlertSounds); i++)
		PRECACHE_SOUND((char*)pAlertSounds[i]);

	for (i = 0; i < ARRAYSIZE(pPainSounds); i++)
		PRECACHE_SOUND((char*)pPainSounds[i]);
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================



int CControlsphere::IgnoreConditions(void)
{
	int iIgnore = CBaseMonster::IgnoreConditions();

	if ((m_Activity == ACT_MELEE_ATTACK1) || (m_Activity == ACT_MELEE_ATTACK1))
	{
#if 0
		if (pev->health < 20)
			iIgnore |= (bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE);
		else
#endif			
			if (m_flNextFlinch >= gpGlobals->time)
				iIgnore |= (bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE);
	}

	if ((m_Activity == ACT_SMALL_FLINCH) || (m_Activity == ACT_BIG_FLINCH))
	{
		if (m_flNextFlinch < gpGlobals->time)
			m_flNextFlinch = gpGlobals->time + ZOMBIE_FLINCH_DELAY;
	}

	return iIgnore;

}

//=========================================================
// Any custom shedules?
//=========================================================
Schedule_t* CControlsphere::GetScheduleOfType(int Type)
{
	switch (Type)
	{
	case SCHED_CNTRL_FINDISLV:
	{
		return slCntrlsphrfindislv;
		break;
	}
	case SCHED_CHASE_ENEMY:
	{
		return slContrlchase;
		break;
	}
	
	}
	return CFlyingMonster::GetScheduleOfType(Type);
}

//=========================================================
// Load up the schedules so ai isn't dumb
//=========================================================
Schedule_t* CControlsphere::GetSchedule(void)
{
	switch (m_MonsterState)
	{
	case MONSTERSTATE_COMBAT:
	{
		
			if ((pev->origin - m_hEnemy->pev->origin).Length() < 256)
			{
				ALERT(at_console, "FUCK!\n");
				return GetScheduleOfType(SCHED_CNTRL_FINDISLV);
				break;
			}


	}
	}
	return CFlyingMonster::GetSchedule();
}

//=========================================================
// StartTask - hacks so the stuka hovers instead of hanging
//=========================================================
void CControlsphere::StartTask(Task_t* pTask)
{
	m_iTaskStatus = TASKSTATUS_RUNNING;

	switch (pTask->iTask)
	{
	case TASK_SPHR_FINDISLV:
	{
		CBaseEntity* pEnemy = m_hEnemy;
		
		MakeIdealYaw(pEnemy->pev->origin);
		ChangeYaw(pev->yaw_speed);
		if ((pev->origin - m_hEnemy->pev->origin).Length() > 150)
		{
			pev->velocity = (gpGlobals->v_forward * 240);
		}
		else
		{
			pev->velocity = (gpGlobals->v_right * 100 + gpGlobals->v_forward * 40 + pEnemy->pev->velocity);
		}

		CBeam* pBeam = CBeam::BeamCreate("sprites/lgtning.spr", 30);

		pBeam->PointEntInit(pEnemy->pev->origin, entindex());
		//pBeam->SetEndAttachment(pEnemy->pev->origin);
		pBeam->SetColor(180, 255, 96);
		//m_pBeam[m_iBeams]->SetColor( 96, 128, 16 );
		pBeam->SetBrightness(64);
		pBeam->SetNoise(80);
		pBeam->SetThink(&CBeam::SUB_Remove);
		pBeam->pev->nextthink = gpGlobals->time + 0.1;
		//m_iBeams++;

		pev->origin.z = pEnemy->pev->origin.z + 60;
		
		TaskComplete();
		break;
	}
	case TASK_STOP_MOVING:
	{

			pev->velocity = pev->velocity * 0;
	

		TaskComplete();
		break;
	}

	break;
	default:
	{
		CFlyingMonster::StartTask(pTask);
	}
	}
}

//=========================================================
// Move - take a step towards the next ROUTE location
//=========================================================
#define DIST_TO_CHECK	512
void CControlsphere::Move(float flInterval)
{
	float		flWaypointDist;
	float		flCheckDist;
	float		flDist;// how far the lookahead check got before hitting an object.
	float		flMoveDist;
	Vector		vecDir;
	Vector		vecApex;
	CBaseEntity* pTargetEnt;

	// Don't move if no valid route
	if (FRouteClear())
	{
		ALERT(at_aiconsole, "Tried to move with no route!\n");
		TaskFail();
		return;
	}

	if (m_flMoveWaitFinished > gpGlobals->time)
		return;

	// if the monster is moving directly towards an entity (enemy for instance), we'll set this pointer
	// to that entity for the CheckLocalMove and Triangulate functions.
	pTargetEnt = NULL;

	m_flGroundSpeed = 200;

	if (m_flGroundSpeed == 0)
		m_flGroundSpeed = 200;

	flMoveDist = m_flGroundSpeed * flInterval;

	do
	{
		// local move to waypoint.
		vecDir = (m_Route[m_iRouteIndex].vecLocation - pev->origin).Normalize();
		flWaypointDist = (m_Route[m_iRouteIndex].vecLocation - pev->origin).Length();
		pev->origin.z = m_hEnemy->pev->origin.z + 60;
		MakeIdealYaw(m_Route[m_iRouteIndex].vecLocation);
		ChangeYaw(pev->yaw_speed);

		if ((pev->origin - m_hEnemy->pev->origin).Length() < 256)
		{
			RouteClear();
		}
		
		// if the waypoint is closer than CheckDist, CheckDist is the dist to waypoint
		if (flWaypointDist < DIST_TO_CHECK)
		{
			flCheckDist = flWaypointDist;
		}
		else
		{
			flCheckDist = DIST_TO_CHECK;
		}

		if ((m_Route[m_iRouteIndex].iType & (~bits_MF_NOT_TO_MASK)) == bits_MF_TO_ENEMY)
		{
			// only on a PURE move to enemy ( i.e., ONLY MF_TO_ENEMY set, not MF_TO_ENEMY and DETOUR )
			pTargetEnt = m_hEnemy;
		}
		else if ((m_Route[m_iRouteIndex].iType & ~bits_MF_NOT_TO_MASK) == bits_MF_TO_TARGETENT)
		{
			pTargetEnt = m_hTargetEnt;
		}

		// !!!BUGBUG - CheckDist should be derived from ground speed.
		// If this fails, it should be because of some dynamic entity blocking this guy.
		// We've already checked this path, so we should wait and time out if the entity doesn't move
		flDist = 0;
		if (CheckLocalMove(pev->origin, pev->origin + vecDir * flCheckDist, pTargetEnt, &flDist) != LOCALMOVE_VALID)
		{
			CBaseEntity* pBlocker;

			// Can't move, stop
			Stop();
			// Blocking entity is in global trace_ent
			pBlocker = CBaseEntity::Instance(gpGlobals->trace_ent);
			if (pBlocker)
			{
				DispatchBlocked(edict(), pBlocker->edict());
			}
			if (pBlocker && m_moveWaitTime > 0 && pBlocker->IsMoving() && !pBlocker->IsPlayer() && (gpGlobals->time - m_flMoveWaitFinished) > 3.0)
			{
				// Can we still move toward our target?
				if (flDist < m_flGroundSpeed)
				{
					// Wait for a second
					m_flMoveWaitFinished = gpGlobals->time + m_moveWaitTime;
					ALERT(at_console, "Move %s!!!\n", STRING(pBlocker->pev->classname));
					return;
				}
			}
			else
			{
				// try to triangulate around whatever is in the way.
				if (FTriangulate(pev->origin, m_Route[m_iRouteIndex].vecLocation, flDist, pTargetEnt, &vecApex))
				{
					InsertWaypoint(vecApex, bits_MF_TO_DETOUR);
					RouteSimplify(pTargetEnt);
				}
				else
				{
					//ALERT ( at_console, "Couldn't Triangulate\n" );
					Stop();
					if (m_moveWaitTime > 0)
					{
						FRefreshRoute();
						m_flMoveWaitFinished = gpGlobals->time + m_moveWaitTime * 0.1;
					}
					else
					{
						TaskFail();
						//ALERT( at_console, "Failed to move!\n" );
					}
					return;
				}
			}
		}

		// UNDONE: this is a hack to quit moving farther than it has looked ahead.
		if (flCheckDist < flMoveDist)
		{
			MoveExecute(pTargetEnt, vecDir, flCheckDist / m_flGroundSpeed);

			 ALERT( at_console, "%.02f\n", flInterval );
			AdvanceRoute(flWaypointDist);
			//flMoveDist -= flCheckDist;
		}
		else
		{
			MoveExecute(pTargetEnt, vecDir, flMoveDist / m_flGroundSpeed);

			if (ShouldAdvanceRoute(flWaypointDist - flMoveDist))
			{
				AdvanceRoute(flWaypointDist);
			}
			flMoveDist = 0;
		}

		if (MovementIsComplete())
		{
			Stop();
			RouteClear();
		}
	} while (flMoveDist > 0 && flCheckDist > 0);

	// cut corner?
	if (flWaypointDist < 128)
	{
		if (m_movementGoal == MOVEGOAL_ENEMY)
			RouteSimplify(m_hEnemy);
		else
			RouteSimplify(m_hTargetEnt);
		FRefreshRoute();

		if (m_flGroundSpeed > 100)
			m_flGroundSpeed -= 40;
	}
	else
	{
		if (m_flGroundSpeed < 400)
			m_flGroundSpeed += 10;
	}
}

BOOL CControlsphere::ShouldAdvanceRoute(float flWaypointDist)
{
	if (flWaypointDist <= 32)
	{
		return TRUE;
	}

	return FALSE;
}


int CControlsphere::CheckLocalMove(const Vector& vecStart, const Vector& vecEnd, CBaseEntity* pTarget, float* pflDist)
{
	TraceResult tr;

	UTIL_TraceHull(vecStart + Vector(0, 0, 32), vecEnd + Vector(0, 0, 32), dont_ignore_monsters, large_hull, edict(), &tr);

	// ALERT( at_console, "%.0f %.0f %.0f : ", vecStart.x, vecStart.y, vecStart.z );
	// ALERT( at_console, "%.0f %.0f %.0f\n", vecEnd.x, vecEnd.y, vecEnd.z );

	if (pflDist)
	{
		*pflDist = ((tr.vecEndPos - Vector(0, 0, 32)) - vecStart).Length();// get the distance.
	}

	// ALERT( at_console, "check %d %d %f\n", tr.fStartSolid, tr.fAllSolid, tr.flFraction );
	if (tr.fStartSolid || tr.flFraction < 1.0)
	{
		if (pTarget && pTarget->edict() == gpGlobals->trace_ent)
			return LOCALMOVE_VALID;
		return LOCALMOVE_INVALID;
	}

	return LOCALMOVE_VALID;
}


void CControlsphere::MoveExecute(CBaseEntity* pTargetEnt, const Vector& vecDir, float flInterval)
{
	if (m_IdealActivity != m_movementActivity)
		m_IdealActivity = m_movementActivity;

	// ALERT( at_console, "move %.4f %.4f %.4f : %f\n", vecDir.x, vecDir.y, vecDir.z, flInterval );

	// float flTotal = m_flGroundSpeed * pev->framerate * flInterval;
	// UTIL_MoveToOrigin ( ENT(pev), m_Route[ m_iRouteIndex ].vecLocation, flTotal, MOVE_STRAFE );

	m_velocity = m_velocity * 0.8 + m_flGroundSpeed * vecDir * 0.2;

	UTIL_MoveToOrigin(ENT(pev), pev->origin + m_velocity, m_velocity.Length() * flInterval, MOVE_STRAFE);

}