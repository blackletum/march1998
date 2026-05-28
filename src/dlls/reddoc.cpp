/***
*
*	Copyright (c) 2022, Magic Nipples.
*
*	Use and modification of this code is allowed as long
*	as credit is provided! Enjoy!
*
****/
//=========================================================
// red doctor (run...)
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "customentity.h"
#include "effects.h"
#include "decals.h"
#include "shake.h"
#include "player.h"
#include "schedule.h"
#include "weapons.h"
#include "gamerules.h"
#include "client.h"

class CRedDoc : public CBaseAnimating
{
	void Spawn(void);
	void Precache(void);

	void EXPORT Think(void);
	void EXPORT Touch(CBaseEntity* pOther);

	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);

	CBaseEntity* pPlayer;
};

LINK_ENTITY_TO_CLASS(monster_reddoc, CRedDoc);

void CRedDoc::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/reddoc.mdl");

	UTIL_SetOrigin(pev, pev->origin);
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid = SOLID_BBOX;
	pev->movetype = MOVETYPE_NOCLIP;

	pev->flags |= FL_MONSTER;
	pev->takedamage = DAMAGE_YES;
	pev->health = 100;

	pev->nextthink = gpGlobals->time + 0.01;

	ALERT(at_console, "Half-Life by Valve LLC\n");
	pPlayer = NULL;
	pPlayer = UTIL_PlayerByIndex(1);

	pev->sequence = LookupSequence("walk");
	pev->frame = 0;
	pev->framerate = 1.0;
	ResetSequenceInfo();

	EMIT_SOUND_DYN(ENT(pev), CHAN_AUTO, "reddoc/drone.wav", 0.5, ATTN_NORM, SND_SPAWNING, PITCH_NORM);
}

void CRedDoc::Precache(void)
{
	PRECACHE_MODEL("models/reddoc.mdl");
}

void CRedDoc::Think(void)
{
	StudioFrameAdvance();

	/*if (pev->sequence != LookupSequence("walk"))
	{
		pev->sequence = LookupSequence("walk");
		pev->frame = 0;
		pev->framerate = 1.0;
		ResetSequenceInfo();
	}*/

	pev->effects = EF_NOINTERP;

	if(RANDOM_LONG(0,20) == 20)
		pev->sequence = LookupSequence("die1");
	else
		pev->sequence = LookupSequence("walk");

	if((!pPlayer) || (pPlayer == NULL))
		pPlayer = UTIL_PlayerByIndex(1);

	if (pPlayer)
	{
		if (pPlayer->pev->health > 0)
		{
			pev->angles.y = UTIL_VecToYaw(pPlayer->pev->origin - pev->origin);

			UTIL_MakeVectors(pev->angles);
			float enemydist = fabs(pPlayer->pev->origin.x - pev->origin.x);
			pev->velocity = gpGlobals->v_forward * (90 + (enemydist / 5));
			pev->velocity.z = 0;

			if (FBitSet(pPlayer->pev->flags, FL_ONGROUND))
			{
				int m_hEnemyHeight;
				if (FBitSet(pPlayer->pev->flags, FL_DUCKING))
					m_hEnemyHeight = pPlayer->pev->origin.z - 18;
				else
					m_hEnemyHeight = pPlayer->pev->origin.z - 36;

				TraceResult tr;
				UTIL_TraceLine(pev->origin, pPlayer->pev->origin + Vector(0, 0, -36), ignore_monsters, ignore_glass, ENT(pev), &tr);

				//ALERT(at_console, "%f\n", (pev->origin - pPlayer->pev->origin).Length());
				if (tr.flFraction == 1)
					pev->origin.z = m_hEnemyHeight;
			}
		}
		else
		{
			CLIENT_PRINTF(ENT(pPlayer->pev), print_console, "Fatal Error: Entity is Not a Player: 2\n");
			SERVER_COMMAND("disconnect\n");
			pev->velocity = g_vecZero;
			pev->framerate = 0.0;
		}
	}

	pev->nextthink = gpGlobals->time + 0.01;

	if (RANDOM_LONG(0, 4) >= 4 || pPlayer->pev->health <= 0)
		pev->skin = 1;
	else
		pev->skin = 0;

	if(pev->skin == 1)
		EMIT_SOUND_DYN(ENT(pev), CHAN_AUTO, "reddoc/scream.wav", 0.8, 2, 0, RANDOM_LONG(90, 110));
	else
		STOP_SOUND(ENT(pev), CHAN_AUTO, "reddoc/scream.wav");
}

void CRedDoc::Touch(CBaseEntity* pOther)
{
	if ( !pOther->IsPlayer())
		return;

	pOther->TakeDamage(pOther->pev, pev, 200, 0);
}

//=========================================================
// Override all damage
//=========================================================
int CRedDoc::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	pev->health = pev->max_health / 2; // always trigger the 50% damage aitrigger
	return TRUE;
}


void CRedDoc::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
	UTIL_Ricochet(ptr->vecEndPos, 1.0);
	AddMultiDamage(pevAttacker, this, flDamage, bitsDamageType);
}