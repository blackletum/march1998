/***
*
*	Copyright (c) 2021, Magic Nipples.
*
*	Use and modification of this code is allowed as long
*	as credit is provided! Enjoy!
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "customentity.h"
#include "effects.h"
#include "weapons.h"
#include "decals.h"
#include "func_break.h"
#include "shake.h"
#include "player.h"

#include "animation.h"
#include "scriptevent.h"

//=================================================================
// env_model: like env_sprite, except you can specify a sequence.
//=================================================================
#define SF_ENVMODEL_OFF			1
#define SF_ENVMODEL_SOLID		2
#define SF_ENVMODEL_NOSHADOW	4
#define SF_ENVMODEL_BREAKABLE	32

class CEnvModel : public CBaseAnimating
{
	void Spawn(void);
	void Precache(void);
	void HandleAnimEvent(MonsterEvent_t* pEvent);
	void EXPORT Think(void);
	void KeyValue(KeyValueData* pkvd);
	//STATE GetState(void);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	virtual int	ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	//int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	void BeenFlamed(entvars_t* pevAttacker, TraceResult& ptr, Vector angle); //yotd
	void BeenCharged(entvars_t* pevAttacker, TraceResult& ptr); //yotd
	void Killed(entvars_t* pevAttacker, int iGib);

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

	void SetSequence(void);

	string_t m_iszSequence_On;
	string_t m_iszSequence_Off;
	int m_iAction_On;
	int m_iAction_Off;

	int		m_iBodyGibs;

	int			m_Parent; //PARENTING
	CBaseEntity* m_cParent;
};

TYPEDESCRIPTION CEnvModel::m_SaveData[] =
{
	DEFINE_FIELD(CEnvModel, m_iszSequence_On, FIELD_STRING),
	DEFINE_FIELD(CEnvModel, m_iszSequence_Off, FIELD_STRING),
	DEFINE_FIELD(CEnvModel, m_iAction_On, FIELD_INTEGER),
	DEFINE_FIELD(CEnvModel, m_iAction_Off, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CEnvModel, CBaseAnimating);
LINK_ENTITY_TO_CLASS(env_model, CEnvModel);

void CEnvModel::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszSequence_On"))
	{
		m_iszSequence_On = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszSequence_Off"))
	{
		m_iszSequence_Off = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iAction_On"))
	{
		m_iAction_On = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iAction_Off"))
	{
		m_iAction_Off = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "parent")) //PARENTING
	{
		m_Parent = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
	{
		CBaseAnimating::KeyValue(pkvd);
	}
}

void CEnvModel::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), STRING(pev->model));
	UTIL_SetOrigin(pev, pev->origin);

	m_cParent = NULL;

	if (pev->spawnflags & SF_ENVMODEL_NOSHADOW)
		pev->effects |= EF_NOSHADOW;

	if (pev->spawnflags & SF_ENVMODEL_SOLID)
	{
		//pev->origin.z += 1;
		//DROP_TO_FLOOR(ENT(pev));
		pev->solid = SOLID_BBOX;

		Vector min, max;
		ExtractBbox(pev->sequence, min, max);
		UTIL_SetSize(pev, min, max);
	}
	SetBoneController(0, 0);
	SetBoneController(1, 0);

	SetSequence();

	//SetNextThink(0.1);
	pev->nextthink = gpGlobals->time + 0.1;


	if (pev->spawnflags & SF_ENVMODEL_BREAKABLE)
	{
		pev->flags |= FL_MONSTER;
		pev->takedamage = DAMAGE_YES;
	}

	//pev->fuser1 = 12;
	//ALERT(at_console, "%f is the scale\n", pev->scale);
}

void CEnvModel::Precache(void)
{
	PRECACHE_MODEL((char*)STRING(pev->model));
	m_iBodyGibs = PRECACHE_MODEL("models/fleshgibs.mdl");
}

/*STATE CEnvModel::GetState(void)
{
	if (pev->spawnflags & SF_ENVMODEL_OFF)
		return STATE_OFF;
	else
		return STATE_ON;
}*/

void CEnvModel::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (ShouldToggle(useType, !(pev->spawnflags & SF_ENVMODEL_OFF)))
	{
		if (pev->spawnflags & SF_ENVMODEL_OFF)
			pev->spawnflags &= ~SF_ENVMODEL_OFF;
		else
			pev->spawnflags |= SF_ENVMODEL_OFF;

		SetSequence();
		//SetNextThink(0.1);
		pev->nextthink = gpGlobals->time + 0.1;
	}
}

void CEnvModel::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{

	case SCRIPT_EVENT_SOUND:			// Play a named wave file
		EMIT_SOUND(edict(), CHAN_BODY, pEvent->options, 1.0, ATTN_IDLE);
		break;

	case SCRIPT_EVENT_SOUND_VOICE:
		EMIT_SOUND(edict(), CHAN_VOICE, pEvent->options, 1.0, ATTN_IDLE);
		break;

	case SCRIPT_EVENT_FIREEVENT:		// Fire a trigger
		FireTargets(pEvent->options, this, this, USE_TOGGLE, 0);
		break;

	case MONSTER_EVENT_BODYDROP_HEAVY:
		if (pev->flags & FL_ONGROUND)
		{
			if (RANDOM_LONG(0, 1) == 0)
			{
				EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "common/bodydrop3.wav", 1, ATTN_NORM, 0, 90);
			}
			else
			{
				EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "common/bodydrop4.wav", 1, ATTN_NORM, 0, 90);
			}
		}
		break;

	case MONSTER_EVENT_BODYDROP_LIGHT:
		if (pev->flags & FL_ONGROUND)
		{
			if (RANDOM_LONG(0, 1) == 0)
			{
				EMIT_SOUND(ENT(pev), CHAN_BODY, "common/bodydrop3.wav", 1, ATTN_NORM);
			}
			else
			{
				EMIT_SOUND(ENT(pev), CHAN_BODY, "common/bodydrop4.wav", 1, ATTN_NORM);
			}
		}
		break;

	case MONSTER_EVENT_SWISHSOUND:
	{
		// NO MONSTER may use this anim event unless that monster's precache precaches this sound!!!
		EMIT_SOUND(ENT(pev), CHAN_BODY, "zombie/claw_miss2.wav", 1, ATTN_NORM);
		break;
	}

	default:
		CBaseAnimating::HandleAnimEvent(pEvent);
		break;

	}
}

void CEnvModel::Think(void)
{
	int iTemp;

	SequencePrecache(GET_MODEL_PTR(ENT(pev)), STRING(m_iszSequence_Off));
	SequencePrecache(GET_MODEL_PTR(ENT(pev)), STRING(m_iszSequence_On));

	//	ALERT(at_console, "env_model Think fr=%f\n", pev->framerate);

	//magic nipples - animation events for player
	float flInterval = pev->frame;
	DispatchAnimEvents(flInterval);

	StudioFrameAdvance(); // set m_fSequenceFinished if necessary

	if ((m_Parent) && (!m_cParent))
	{
		pev->movetype = MOVETYPE_COMPOUND;
		m_cParent = UTIL_FindEntityByTargetname(NULL, STRING(m_Parent));
		pev->aiment = ENT(m_cParent->pev);
	}

//	if (m_fSequenceLoops)
//	{
//		SetNextThink( 1E6 );
//		return; // our work here is done.
//	}
	if (m_fSequenceFinished && !m_fSequenceLoops)
	{
		if (pev->spawnflags & SF_ENVMODEL_OFF)
			iTemp = m_iAction_Off;
		else
			iTemp = m_iAction_On;

		switch (iTemp)
		{
			//		case 1: // loop
			//			pev->animtime = gpGlobals->time;
			//			m_fSequenceFinished = FALSE;
			//			m_flLastEventCheck = gpGlobals->time;
			//			pev->frame = 0;
			//			break;
		case 2: // change state
			if (pev->spawnflags & SF_ENVMODEL_OFF)
				pev->spawnflags &= ~SF_ENVMODEL_OFF;
			else
				pev->spawnflags |= SF_ENVMODEL_OFF;
			SetSequence();
			break;
		default: //remain frozen
			return;
		}
	}

	//SetNextThink(0.1);
	pev->nextthink = gpGlobals->time + 0.1;
}

void CEnvModel::SetSequence(void)
{
	int iszSeq;

	if (pev->spawnflags & SF_ENVMODEL_OFF)
		iszSeq = m_iszSequence_Off;
	else
		iszSeq = m_iszSequence_On;

	if (!iszSeq)
		return;

	pev->sequence = LookupSequence(STRING(iszSeq));

	if (pev->sequence == -1)
	{
		if (pev->targetname)
			ALERT(at_error, "env_model %s: unknown sequence \"%s\"\n", STRING(pev->targetname), STRING(iszSeq));
		else
			ALERT(at_error, "env_model: unknown sequence \"%s\"\n", STRING(pev->targetname), STRING(iszSeq));
		pev->sequence = 0;
	}

	pev->frame = 0;
	ResetSequenceInfo();

	if (pev->spawnflags & SF_ENVMODEL_OFF)
	{
		if (m_iAction_Off == 1)
			m_fSequenceLoops = 1;
		else
			m_fSequenceLoops = 0;
	}
	else
	{
		if (m_iAction_On == 1)
			m_fSequenceLoops = 1;
		else
			m_fSequenceLoops = 0;
	}
}

/*int CEnvModel::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	if (pev->health <= 0)
	{
		pev->solid = SOLID_NOT;
		pev->body = 1;
	}

	// ALERT( at_console, "%.0f\n", flDamage );
	return CBaseEntity::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}*/

void CEnvModel::Killed(entvars_t* pevAttacker, int iGib)
{
	pev->owner = ENT(pevAttacker);
	pev->solid = SOLID_NOT;
	pev->body = 1;

	Vector vecSpot = pev->origin + (pev->mins + pev->maxs) * 0.5;
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecSpot);
	WRITE_BYTE(TE_BREAKMODEL);

	// position
	WRITE_COORD(vecSpot.x);
	WRITE_COORD(vecSpot.y);
	WRITE_COORD(vecSpot.z);

	// size
	WRITE_COORD(pev->size.x);
	WRITE_COORD(pev->size.y);
	WRITE_COORD(pev->size.z);

	// velocity
	WRITE_COORD(0);
	WRITE_COORD(0);
	WRITE_COORD(0);

	// randomization
	WRITE_BYTE(10);
	// Model
	WRITE_SHORT(m_iBodyGibs);
	// # of shards
	WRITE_BYTE(0);	// let client decide
	// duration
	WRITE_BYTE(25);// 2.5 seconds
	// flags
	WRITE_BYTE(BREAK_FLESH);
	MESSAGE_END();
}