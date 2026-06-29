//=========================================================
// chumtoad.cpp - jumpy alien frog
//=========================================================

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"schedule.h"
#include	"game.h"
#include	"weapons.h"
#include	"player.h"
#include	"soundent.h"
#include	"gamerules.h"
#include	"nodes.h"


#define		CHUB_CROAK_RANGE		384



// Chub Toad monster begins here

class CChub : public CBaseMonster
{
public:
	void Spawn( void );
	void Precache( void );
	int  Classify ( void );
	int  IRelationship( CBaseEntity *pTarget );
	void IdleSound ( void );
	virtual int	ObjectCaps( void ) 
	{ 
		int flags = CBaseToggle :: ObjectCaps() & (~FCAP_ACROSS_TRANSITION); 
		return flags | FCAP_IMPULSE_USE;
	}

	void RecalculateWaterlevel( void );
	void StartTask ( Task_t *pTask );
	void RunTask ( Task_t *pTask );
	Schedule_t  *GetScheduleOfType ( int Type );
	Schedule_t *GetSchedule ( void );

	void EXPORT ChubUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void GibMonster( void );
	int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	void SetYawSpeed( void );

	BOOL FCanCheckAttacks ( void ) { return FALSE; } // chubby is passive
	BOOL IsBarnaclePrey ( void ) { return TRUE; }

	void EXPORT MonsterThink ( void );
	void EXPORT WaterDeathThink ( void );
	void Swim( void );
	int RandomChub = RANDOM_FLOAT(0, 4);

	CUSTOM_SCHEDULES;

	float	m_flBlink;
	BOOL	m_fPlayingDead;
	float	m_flPlayDeadTime;
	float	m_flNextCroak; // don't save
	BOOL	m_fInWater;
};

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_CHUB_PLAYDEAD = LAST_COMMON_SCHEDULE + 1,
	SCHED_CHUB_STOP_PLAYDEAD,
};

//=========================================================
// monster-specific tasks
//=========================================================
enum 
{
	TASK_CHUB_IDLE = LAST_COMMON_TASK + 1,
	TASK_CHUB_START_PLAYDEAD,
	TASK_CHUB_PLAYDEAD,
	TASK_CHUB_STOP_PLAYDEAD,
};

//=========================================================
//	Idle Schedules
//=========================================================
Task_t	tlChubIdleStand1[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_CHUB_IDLE,			0				},
	{ TASK_WAIT,				(float)5		},// repick IDLESTAND every five seconds. gives us a chance to pick an active idle, fidget, etc.
};

Schedule_t	slChubIdleStand[] =
{
	{ 
		tlChubIdleStand1,
		ARRAYSIZE ( tlChubIdleStand1 ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_SEE_FEAR		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND	|
		bits_COND_SMELL_FOOD	|
		bits_COND_SMELL			|
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT		|// sound flags
		bits_SOUND_WORLD		|
		bits_SOUND_PLAYER		|
		bits_SOUND_DANGER		|

		bits_SOUND_MEAT			|// scents
		bits_SOUND_CARCASS		|
		bits_SOUND_GARBAGE,
		"IdleStand"
	},
};

Task_t	tlChubStand[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_STAND			},
	{ TASK_CHUB_STOP_PLAYDEAD,		(float)0					},
};

Schedule_t	slChubStand[] =
{
	{ 
		tlChubStand,
		ARRAYSIZE ( tlChubStand ), 
		bits_COND_NEW_ENEMY,
		0,
		"Chub Fear"
	},
};

Task_t	tlChubPlayDead[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_CHUB_START_PLAYDEAD,		(float)0					},
	{ TASK_CHUB_PLAYDEAD,			(float)0					},
	{ TASK_CHUB_STOP_PLAYDEAD,		(float)0					},
};

Schedule_t	slChubPlayDead[] =
{
	{ 
		tlChubPlayDead,
		ARRAYSIZE ( tlChubPlayDead ), 
		0,
		0,
		"Chub Playdead"
	},
};

Task_t	tlChubStopPlayDead[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_CHUB_STOP_PLAYDEAD,		(float)0					},
};

Schedule_t	slChubStopPlayDead[] =
{
	{ 
		tlChubStopPlayDead,
		ARRAYSIZE ( tlChubStopPlayDead ), 
		0,
		0,
		"Chub Stop Playdead"
	},
};

DEFINE_CUSTOM_SCHEDULES( CChub )
{
	slChubIdleStand,
	slChubStand,
	slChubPlayDead,
	slChubStopPlayDead,
};

IMPLEMENT_CUSTOM_SCHEDULES( CChub, CBaseMonster );

LINK_ENTITY_TO_CLASS( monster_chumtoad, CChub );
LINK_ENTITY_TO_CLASS( monster_chubtoad, CChub );	// both spellings are right

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CChub :: SetYawSpeed ( void )
{
	int ys;

	switch ( m_Activity )
	{
	case ACT_STAND:
	case ACT_TWITCH:
	case ACT_FEAR_DISPLAY:
		ys = 0;
		break;
	case ACT_SWIM:
		ys = 150;
		break;
	default:
		ys = 70;
		break;
	}

	pev->yaw_speed = ys;
}

void CChub :: Spawn (void)
{
	Precache();

	SET_MODEL(ENT(pev), "models/chumtoad.mdl");
	UTIL_SetSize(pev, Vector(-12, -12, 0), Vector(12, 12, 24));	

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_GREEN;
	pev->effects		= 0;
	pev->health			= gSkillData.chubtoadHealth;
	pev->view_ofs		= Vector ( 0, 0, 16 );
	m_flFieldOfView		= VIEW_FIELD_FULL;
	m_MonsterState		= MONSTERSTATE_NONE;

	if (RandomChub == 0)
		pev->skin = 0;
	else if (RandomChub == 1)
		pev->skin = 1;
	else if (RandomChub == 2)
		pev->skin = 2;
	else if (RandomChub == 3)
		pev->skin = 3;
	else if (RandomChub == 4)
		pev->skin = 4;


	MonsterInit();

//	SetThink (&CChub::AmphibiousThink);
	SetUse (&CChub::ChubUse);
}

void CChub :: Precache (void)
{
	PRECACHE_SOUND("chub/frog.wav");
	PRECACHE_MODEL("models/chumtoad.mdl");
}

int	CChub :: Classify ( void )
{
	return	CLASS_PLAYER_ALLY;
//	return	CLASS_ALIEN_PREY;
}

int CChub::IRelationship( CBaseEntity *pTarget )
{
	if ( pTarget->IsPlayer() && HasMemory(bits_MEMORY_PROVOKED) )
		return R_FR;
	return CBaseMonster::IRelationship( pTarget );
}

void CChub :: IdleSound (void)
{
}

//=========================================================
// StartTask
//=========================================================
void CChub :: StartTask ( Task_t *pTask )
{
	m_iTaskStatus = TASKSTATUS_RUNNING;

	switch ( pTask->iTask )
	{
	case TASK_SET_ACTIVITY:
	{
		Activity activity = (Activity)(int)pTask->flData;

		if ( activity == ACT_IDLE )
		{
			if ( !pev->waterlevel )
			{
				activity = ACT_IDLE;
			}
			else
			{
				activity = ACT_SWIM;
			}
		}

		m_IdealActivity = activity;
		TaskComplete();
	}
	break;
	case TASK_CHUB_IDLE:
	{
		if ( m_fPlayingDead )
		{
			m_IdealActivity = ACT_TWITCH;
		}
		else if ( !pev->waterlevel )
		{
			m_IdealActivity = ACT_IDLE;
			pev->movetype = MOVETYPE_STEP;
		//	ALERT( at_console, "Chub: Not in water!\n");
		}
		else
		{
			m_IdealActivity = ACT_SWIM;
		//	pev->movetype = MOVETYPE_FLY;
		}
		TaskComplete();
	}
	break;
	case TASK_CHUB_START_PLAYDEAD:
		{
			ALERT( at_console, "Chub: Playing dead!\n" );
			m_fPlayingDead = TRUE;
			m_IdealActivity = ACT_FEAR_DISPLAY;
			break;
		}
	case TASK_CHUB_PLAYDEAD:
		{
			m_IdealActivity = ACT_TWITCH;
			break;
		}
	case TASK_CHUB_STOP_PLAYDEAD:
		{
			ALERT( at_console, "Chub: Stopped playdead!\n" );
			Forget ( bits_MEMORY_INCOVER );
			m_fPlayingDead = FALSE; 
			pev->deadflag = DEAD_NO;
			m_flPlayDeadTime = NULL;
			TaskComplete();
			break;
		}
	default:
		CBaseMonster :: StartTask( pTask );
		break;
	}
}

//=========================================================
// RunTask
//=========================================================
void CChub :: RunTask ( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_CHUB_START_PLAYDEAD:
		{
			if (m_fSequenceFinished)
			{
				pev->deadflag = DEAD_DEAD;
				m_flPlayDeadTime = gpGlobals->time + RANDOM_FLOAT(5,10);
			//	ALERT( at_console, "Chub: Finished playdead2!\n");
				TaskComplete();
			}
			break;
		}
	case TASK_CHUB_PLAYDEAD:
		{
			if ( m_flPlayDeadTime < gpGlobals->time )
			{
				TaskComplete();	
			}
			break;
		}
	case TASK_DIE:
		{
			if ( m_fSequenceFinished || pev->frame >= 255 )
			{	
				SetUse(NULL);
				CBaseMonster :: RunTask( pTask );
			}
		}
	default:
		{
			CBaseMonster :: RunTask( pTask );
			break;
		}
	}
}

void EXPORT CChub :: ChubUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// if it's not a player, ignore
	if ( !pActivator->IsPlayer() )
		return;

	if ( HasMemory(bits_MEMORY_PROVOKED))
		return;	// im not gonna cooperate with you!

	// do not revive dead toads!!!
	if ( pev->deadflag != DEAD_NO && !m_fPlayingDead )
		return;

	CBasePlayer *pPlayer = (CBasePlayer *)pActivator;

	// can I have this?
	if ( !g_pGameRules->CanHaveAmmo( pPlayer, "Chubtoads", SNARK_MAX_CARRY) )
	{
		return;
	}

	pPlayer->GiveNamedItem( "weapon_chub" );
	EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

	CBaseEntity *pOwner = CBaseEntity::Instance(pev->owner);
	if ( pOwner != NULL )	// tell the monstermaker that this thing is gone
	{
		pOwner->DeathNotice( pev );
	}

	UTIL_Remove(this);
}

// overriden for Chubbie. see Classify() for why
void CChub :: GibMonster( void )
{
	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "common/bodysplat.wav", 1, ATTN_NORM);
	CGib::SpawnRandomGibs(pev, 1, 1);
	SetThink ( &CBaseMonster::SUB_Remove );
	pev->nextthink = gpGlobals->time;
}

int CChub :: TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	int iTakeDamage;

	if ( bitsDamageType & ( DMG_RADIATION ) ) flDamage = 0;

	if ( flDamage > 0 )
	{
		Forget(bits_MEMORY_INCOVER);
		if ( m_flPlayDeadTime )
			m_flPlayDeadTime = NULL;
	}

	if ( pevInflictor && pevInflictor->flags & FL_CLIENT )
	{
		Remember( bits_MEMORY_PROVOKED );
	}

	// HACK: change deadflag so we can pass IsAlive check
	if ( m_fPlayingDead )
		pev->deadflag = DEAD_NO;

	iTakeDamage = CBaseMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );

	if ( m_fPlayingDead )
		pev->deadflag = DEAD_DEAD;

	return iTakeDamage;
}

//=========================================================
// GetScheduleOfType - returns a pointer to one of the 
// monster's available schedules of the indicated type.
//=========================================================
Schedule_t* CChub :: GetScheduleOfType ( int Type ) 
{
	switch	( Type )
	{
	case SCHED_IDLE_STAND:
		{
			return &slChubIdleStand[ 0 ];
		}
	case SCHED_CHUB_PLAYDEAD:
		{
			return &slChubPlayDead[ 0 ];
		}
	case SCHED_CHUB_STOP_PLAYDEAD:
		{
			return &slChubStopPlayDead[ 0 ];
		}
	default:
		{
			return CBaseMonster :: GetScheduleOfType ( Type );
		}
	}
}

Schedule_t *CChub :: GetSchedule ( void )
{
	switch( m_MonsterState )
	{
		case MONSTERSTATE_IDLE:
		{
			// hop around randomly
			if ( RANDOM_LONG(0,99) <= 50 )
				return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ORIGIN);

			return CBaseMonster::GetSchedule();
		break;
		}
		case MONSTERSTATE_COMBAT:
		{
		//	ALERT( at_console, "Chub: MONSTERSTATE_COMBAT!\n" );
			if ( HasConditions( bits_COND_ENEMY_DEAD ) )
			{
				if ( m_fPlayingDead ) 
					return GetScheduleOfType( SCHED_CHUB_STOP_PLAYDEAD );

				return CBaseMonster::GetSchedule();
			}

			if ( HasConditions(bits_COND_NEW_ENEMY) )
			{
				return GetScheduleOfType ( SCHED_WAKE_ANGRY );
			}

			if ( HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE) )
			{
				if ( !m_fPlayingDead )
				{
					return CBaseMonster::GetSchedule();
				}
				else return GetScheduleOfType( SCHED_CHUB_STOP_PLAYDEAD );
			}

			if ( HasMemory(bits_MEMORY_INCOVER) )
			{
				if ( !m_fPlayingDead )
				{
					return GetScheduleOfType( SCHED_CHUB_PLAYDEAD );
				}
			}

			if ( HasConditions( bits_COND_SEE_ENEMY ) )
			{
				if ( !m_fPlayingDead )
				{
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
				}

				return GetScheduleOfType( SCHED_IDLE_STAND );
			}
			else
			{
				return GetScheduleOfType( SCHED_COMBAT_FACE );
			}
		break;
		}
		default:
		{
			return CBaseMonster::GetSchedule();
		}
	}
}

#define	CHUB_CHECK_DIST		72
#define CHUB_SWIM_SPEED		50
#define CHUB_SWIM_ACCEL		90
#define CHUB_TURN_RATE		70

// UNDONE: implement proper obstacle avoidance behavior
void CChub :: Swim ( void )
{
	TraceResult		tr;
	Vector			vecTest;
	BOOL			fStuck = FALSE;

	if ( m_Activity != ACT_SWIM )
		SetActivity(ACT_SWIM);
	
	Vector tmp = pev->angles;
	tmp.x = -tmp.x;
	UTIL_MakeVectors ( tmp );

	vecTest = pev->origin + gpGlobals->v_forward * RANDOM_LONG(CHUB_CHECK_DIST/2, CHUB_CHECK_DIST);

	UTIL_TraceLine(pev->origin, vecTest, missile, edict(), &tr);

	if ( tr.flFraction <= 0.97 )
		fStuck = TRUE;

	TRACE_MONSTER_HULL(edict(), tr.vecEndPos, tr.vecEndPos, missile, edict(), &tr);

	if ( tr.fStartSolid )
		fStuck = TRUE;

	if ( !fStuck )
	{
	//	ALERT(at_console, "VICTORY %f\n", tr.flFraction);
		pev->speed = UTIL_Approach( CHUB_SWIM_SPEED, pev->speed, CHUB_SWIM_ACCEL * .1 );
		pev->velocity = gpGlobals->v_forward * pev->speed;
		pev->avelocity = g_vecZero;
		pev->angles.y += 2.5 * RANDOM_LONG(-1, 1);

		UTIL_ParticleEffect(pev->origin, Vector(0,0,0), 224, 16);

		pev->nextthink = gpGlobals->time + .1;
	}
	else
	{
	//	ALERT(at_console, "%f\n", tr.flFraction);

		pev->velocity = g_vecZero;
		pev->angles.y += 5;

		pev->nextthink = gpGlobals->time + .05;
	}
}

void CChub :: MonsterThink ( void )
{
	if ( m_fInWater )
	{
		if ( pev->health <= 0 )
		{
		//	ALERT(at_console, "dead\n");
			SetActivity(ACT_DIESIMPLE);
			UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

			SetThink(&CChub::WaterDeathThink);
			pev->nextthink = gpGlobals->time + .1;
			pev->gravity = 0.1;
		}
		else if ( pev->waterlevel != 3 )
		{
			pev->deadflag = DEAD_NO;
			pev->movetype = MOVETYPE_STEP;
			pev->velocity = pev->avelocity = g_vecZero;
			m_fInWater = m_fPlayingDead = FALSE;
		}
		else Swim(); //	just dumbly swim around
		
		return;
	}
	else if ( pev->waterlevel == 3 )
	{
		pev->deadflag = DEAD_DEAD;	// enemies ignore chub when it swims
		pev->movetype = MOVETYPE_FLY;
		pev->velocity = pev->avelocity = g_vecZero;
		m_fInWater = m_fPlayingDead = TRUE;
	}

	if (!m_flNextCroak)
		m_flNextCroak = gpGlobals->time + RANDOM_FLOAT(1.5, 5.0);
	else if (gpGlobals->time >= m_flNextCroak)
	{
		BOOL fShouldCroak = !m_fPlayingDead && !pev->waterlevel;

		if ( fShouldCroak )
		{
			EMIT_SOUND_DYN( edict(), CHAN_VOICE, "chub/frog.wav", 1.0, ATTN_NORM, 0, RANDOM_LONG(90,110) );
			// TEMP soundent
			CSoundEnt::InsertSound ( bits_SOUND_PLAYER, pev->origin, CHUB_CROAK_RANGE, 0.5 );
		}

		m_flNextCroak = NULL;
	}

	CBaseMonster::MonsterThink();
}

void CChub :: WaterDeathThink( void )
{
	if ( pev->waterlevel == 3 )
	{
		pev->movetype = MOVETYPE_FLY;
		pev->velocity = pev->velocity * 0.5;
		pev->avelocity = pev->avelocity * 0.9;
		pev->velocity.z += 4;
	}
	else pev->movetype = MOVETYPE_BOUNCE;

	pev->nextthink = gpGlobals->time + .1;
}

// Chub Toad monster ends here


// Chub Toad Weapon starts here

class CChubGrenade : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int iItemSlot() { return 5; }
	int AddToPlayer(CBasePlayer* pPlayer);
	int GetItemInfo(ItemInfo* p);

	void PrimaryAttack(void);
	BOOL Deploy(void);
	void Holster(void);
	void WeaponIdle(void);

	void ItemPostFrame(void);

	int RandomChub = RANDOM_FLOAT(0, 4);

	int		m_fChubJustThrown;
	float	m_flAttackDelay;

	virtual BOOL UseDecrement(void)
	{
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	void Throw(void);
	BOOL TraceChub(void);

	unsigned short m_usChubFire;

};

#define		CHUBGRENADE_THROW_DELAY		1.0
#define		CHUBGRENADE_THROW_RANGE		48

enum chub_e {
	CHUB_IDLE1 = 0,
	CHUB_FIDGETLICK,
	CHUB_FIDGETCROAK,
	CHUB_DOWN,
	CHUB_UP,
	CHUB_THROW
};

LINK_ENTITY_TO_CLASS(weapon_chub, CChubGrenade);

void CChubGrenade::Spawn(void)
{
	Precache();
	m_iId = WEAPON_CHUB;
	SET_MODEL(ENT(pev), "models/chumtoad.mdl");
	pev->effects = EF_NODRAW;
	FallInit();

	m_iDefaultAmmo = 1;
}

void CChubGrenade::Precache(void)
{
	UTIL_PrecacheOther("monster_chumtoad");
	PRECACHE_MODEL("models/v_chub.mdl");
}


int CChubGrenade::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Chubtoads";
	p->iMaxAmmo1 = SNARK_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_CHUB;
	p->iWeight = 5;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

	return 1;
}

int CChubGrenade::AddToPlayer(CBasePlayer* pPlayer)
{
	if (CBasePlayerWeapon::AddToPlayer(pPlayer))
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
		WRITE_BYTE(m_iId);
		MESSAGE_END();
		m_pPlayer->SetSuitUpdate("!HEV_SQUEEK", FALSE, SUIT_NEXT_IN_30MIN);
		return TRUE;
	}
	return FALSE;
}

BOOL CChubGrenade::Deploy()
{
	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
	
	if (m_pPlayer->m_rgItems[ITEM_IVANSUIT])
		pev->body = 1;
	else
		pev->body = 0;

	return DefaultDeploy("models/v_chub.mdl", "models/p_squeak.mdl", CHUB_UP, "chub");
}

void CChubGrenade::Holster()
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.1;
	m_flAttackDelay = NULL;

	if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		m_pPlayer->pev->weapons &= ~(1 << WEAPON_CHUB);
		SetThink(&CChubGrenade::DestroyItem);
		pev->nextthink = gpGlobals->time + 0.1;
		return;
	}

	SendWeaponAnim(CHUB_DOWN);
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM);
}

BOOL CChubGrenade::TraceChub(void)
{
#ifndef CLIENT_DLL
	TraceResult tr;
	Vector trace_origin;

	trace_origin = m_pPlayer->EyePosition();

	// pull out of the player bounding box
	trace_origin = trace_origin + gpGlobals->v_forward * VEC_HUMAN_HULL_MAX.x;

	if (m_pPlayer->pev->flags & FL_DUCKING)
	{
		trace_origin = trace_origin - (VEC_HULL_MIN - VEC_DUCK_HULL_MIN);
	}

	// FIXME: magic number
	UTIL_TraceLine(trace_origin + gpGlobals->v_forward * 24, trace_origin + gpGlobals->v_forward * 64, dont_ignore_monsters, NULL, &tr);

	if (tr.fAllSolid || tr.fStartSolid || tr.flFraction <= 0.25)
		return FALSE;

	UTIL_TraceHull(tr.vecEndPos, tr.vecEndPos, dont_ignore_monsters, head_hull, NULL, &tr);

	if (tr.pHit)
		return FALSE;

#endif

	return TRUE;
}

void CChubGrenade::Throw()
{
#ifndef CLIENT_DLL
	edict_t* pChub;
	entvars_t* pevChub;
	Vector			vecSrc;

	pChub = CREATE_NAMED_ENTITY(MAKE_STRING("monster_chumtoad"));
	if (!FNullEnt(pChub))
	{
		pevChub = VARS(pChub);
		pevChub->origin = m_pPlayer->pev->origin + gpGlobals->v_forward * CHUBGRENADE_THROW_RANGE;
		pevChub->angles = m_pPlayer->pev->angles;
		DispatchSpawn(ENT(pevChub));

		pevChub->velocity = gpGlobals->v_forward * 500 + m_pPlayer->pev->velocity;
		pevChub->velocity.z += gpGlobals->v_up.z * 300;
		pevChub->angles.x = 0;
		pevChub->angles.z = 0;
	}


	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

	m_fChubJustThrown = 1;
#endif
}

void CChubGrenade::PrimaryAttack()
{
	if (!(m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]))
		return;

	ALERT(at_console, "chub\n");
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

#ifndef CLIENT_DLL
	if (TraceChub())
	{
		// player "shoot" animation
		//m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		SendWeaponAnim( CHUB_THROW | 0 );

		m_flAttackDelay = gpGlobals->time + CHUBGRENADE_THROW_DELAY;
		m_flNextPrimaryAttack = gpGlobals->time + 3.0;
	}
#endif


}

void CChubGrenade::WeaponIdle(void)
{
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase() || m_flAttackDelay)
		return;

	if (m_fChubJustThrown)
	{
		m_fChubJustThrown = 0;

		if (!m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()])
		{
			RetireWeapon();
			return;
		}

		SendWeaponAnim(CHUB_UP);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
		return;
	}

	int iAnim;
	float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 1);
	if (flRand <= 0.75)
	{
		iAnim = CHUB_IDLE1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 30.0 / 16 * (2);
	}
	else if (flRand <= 0.875)
	{
		iAnim = CHUB_FIDGETLICK;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 70.0 / 16.0;
	}
	else
	{
		iAnim = CHUB_FIDGETCROAK;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 80.0 / 16.0;
	}
	SendWeaponAnim(iAnim);
}

void CChubGrenade::ItemPostFrame(void)
{
	if (m_flAttackDelay)
	{
		if (m_flAttackDelay <= gpGlobals->time)
		{
			m_flAttackDelay = NULL;

			if (TraceChub())
				Throw();

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;
		}
	}

	CBasePlayerWeapon::ItemPostFrame();
}