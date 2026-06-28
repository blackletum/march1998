/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
/*

===== items.cpp ========================================================

  functions governing the selection/use of weapons for players

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "player.h"
#include "skill.h"
#include "items.h"
#include "gamerules.h"

extern int gmsgItemPickup;

class CWorldItem : public CBaseEntity
{
public:
	void	KeyValue(KeyValueData *pkvd ); 
	void	Spawn( void );
	int		m_iType;
};

LINK_ENTITY_TO_CLASS(world_items, CWorldItem);

void CWorldItem::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "type"))
	{
		m_iType = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}

void CWorldItem::Spawn( void )
{
	CBaseEntity *pEntity = NULL;

	switch (m_iType) 
	{
	case 44: // ITEM_BATTERY:
		pEntity = CBaseEntity::Create( "item_battery", pev->origin, pev->angles );
		break;
	case 42: // ITEM_ANTIDOTE:
		pEntity = CBaseEntity::Create( "item_antidote", pev->origin, pev->angles );
		break;
	case 43: // ITEM_SECURITY:
		pEntity = CBaseEntity::Create( "item_security", pev->origin, pev->angles );
		break;
	case 45: // ITEM_SUIT:
		pEntity = CBaseEntity::Create( "item_suit", pev->origin, pev->angles );
		break;
	}

	if (!pEntity)
	{
		ALERT( at_console, "unable to create world_item %d\n", m_iType );
	}
	else
	{
		pEntity->pev->target = pev->target;
		pEntity->pev->targetname = pev->targetname;
		pEntity->pev->spawnflags = pev->spawnflags;
	}

	REMOVE_ENTITY(edict());
}


void CItem::Spawn(void)
{
	if (!FBitSet(pev->spawnflags, SF_AMMO_NOFALL))
	{
		pev->movetype = MOVETYPE_TOSS;
	}
	else
	{
		pev->movetype = MOVETYPE_NONE;
	}

	if (FBitSet(pev->spawnflags, SF_AMMO_INDRAWER))
	{
		UTIL_SetSize(pev, Vector(-4, -4, 0), Vector(4, 4, 4));
		SetThink(&CItem::DrawerThink);
		pev->nextthink = gpGlobals->time + 0.1;
	}
	else
	{
		UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));
	}

	pev->solid = SOLID_TRIGGER;
	UTIL_SetOrigin(pev, pev->origin);
	SetTouch(&CItem::ItemTouch);

	if (!FBitSet(pev->spawnflags, SF_AMMO_NOFALL))
	{
		if (DROP_TO_FLOOR(ENT(pev)) == 0)	// this engine function makes item fall even if it has NOFALL spawnflag set
		{
			ALERT(at_error, "Item %s fell out of level at %f,%f,%f", STRING(pev->classname), pev->origin.x, pev->origin.y, pev->origin.z);
			UTIL_Remove(this);
			return;
		}
	}
}

void CItem::DrawerThink(void)
{
	CBaseEntity* pEntity = NULL;
	Vector vecStart, vecEnd;
	TraceResult tr;

	vecStart = pev->origin;
	vecStart.z += 2;

	pEntity = UTIL_FindEntityInSphere(pEntity, vecStart, 12.0);

	if (pEntity->IsPlayer())
	{
		//	ALERT ( at_console, "sphere\n" );
		this->ItemTouch(pEntity);
		return;
	}

	vecEnd = pev->origin;
	vecEnd.z += 24;

	UTIL_TraceLine(pev->origin, vecEnd, dont_ignore_monsters, edict(), &tr);

	pEntity = CBaseEntity::Instance(tr.pHit);

	if (pEntity->IsPlayer())
	{
		//	ALERT ( at_console, "trace\n" );
		this->ItemTouch(pEntity);
		return;
	}

	pev->nextthink = gpGlobals->time + 0.1;
}

extern int gEvilImpulse101;

void CItem::ItemTouch( CBaseEntity *pOther )
{
	// if it's not a player, ignore
	if ( !pOther->IsPlayer() )
	{
		return;
	}

	CBasePlayer *pPlayer = (CBasePlayer *)pOther;

	// ok, a player is touching this item, but can he have it?
	if ( !g_pGameRules->CanHaveItem( pPlayer, this ) )
	{
		// no? Ignore the touch.
		return;
	}

	if (MyTouch( pPlayer ))
	{
		SUB_UseTargets( pOther, USE_TOGGLE, 0 );
		SetTouch( NULL );
		
		// player grabbed the item. 
		g_pGameRules->PlayerGotItem( pPlayer, this );
		if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_YES )
		{
			Respawn(); 
		}
		else
		{
			UTIL_Remove( this );
		}
	}
	else if (gEvilImpulse101)
	{
		UTIL_Remove( this );
	}
}

CBaseEntity* CItem::Respawn( void )
{
	SetTouch( NULL );
	pev->effects |= EF_NODRAW;

	UTIL_SetOrigin( pev, g_pGameRules->VecItemRespawnSpot( this ) );// blip to whereever you should respawn.

	SetThink ( &CItem::Materialize );
	pev->nextthink = g_pGameRules->FlItemRespawnTime( this ); 
	return this;
}

void CItem::Materialize( void )
{
	if ( pev->effects & EF_NODRAW )
	{
		// changing from invisible state to visible.
		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", 1, ATTN_NORM, 0, 150 );
		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	SetTouch( &CItem::ItemTouch );
}

#define SF_SUIT_SHORTLOGON		0x0001

class CItemSuit : public CItem
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_suit.mdl");
		CItem::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_suit.mdl");
	}
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		if ( pPlayer->pev->weapons & (1<<WEAPON_SUIT) )
			return FALSE;

		if ( pev->spawnflags & SF_SUIT_SHORTLOGON )
			EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_A0");		// short version of suit logon,
		else
			EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_AAx");	// long version of suit logon

		pPlayer->m_bDefaultSuit = TRUE;

		pPlayer->pev->weapons |= (1<<WEAPON_SUIT);
		return TRUE;
	}
};

LINK_ENTITY_TO_CLASS(item_suit, CItemSuit);

class CItemAlphaSuit: public CItem
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_suit.mdl");
		pev->body = 1;
		CItem::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_suit.mdl");
	}
	BOOL MyTouch(CBasePlayer* pPlayer)
	{
		if (pPlayer->pev->weapons & (1 << WEAPON_SUIT))
			return FALSE;

		if (pev->spawnflags & SF_SUIT_SHORTLOGON)
			EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_A0");		// short version of suit logon,
		else
			EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_AAx");	// long version of suit logon

		pPlayer->m_bAlphaSuit = TRUE;

		ALERT(at_console, "picked up alpha suit");

		pPlayer->pev->weapons |= (1 << WEAPON_SUIT);
		return TRUE;
	}
};

LINK_ENTITY_TO_CLASS(item_ivansuit, CItemAlphaSuit);

class CItemBattery : public CItem
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_battery.mdl");
		CItem::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_battery.mdl");
		PRECACHE_SOUND( "items/gunpickup2.wav" );
	}
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		if ( pPlayer->pev->deadflag != DEAD_NO )
			return FALSE;

		if ((pPlayer->pev->armorvalue < MAX_NORMAL_BATTERY) &&
			(pPlayer->pev->weapons & (1<<WEAPON_SUIT)))
		{
			int pct;
			char szcharge[64];

			pPlayer->pev->armorvalue += gSkillData.batteryCapacity;
			pPlayer->pev->armorvalue = min(pPlayer->pev->armorvalue, MAX_NORMAL_BATTERY);

			EMIT_SOUND( pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM );

			MESSAGE_BEGIN( MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev );
				WRITE_STRING( STRING(pev->classname) );
			MESSAGE_END();

			
			// Suit reports new power level
			// For some reason this wasn't working in release build -- round it.
			pct = (int)( (float)(pPlayer->pev->armorvalue * 100.0) * (1.0/MAX_NORMAL_BATTERY) + 0.5);
			pct = (pct / 5);
			if (pct > 0)
				pct--;
		
			sprintf( szcharge,"!HEV_%1dP", pct );
			
			//EMIT_SOUND_SUIT(ENT(pev), szcharge);
			pPlayer->SetSuitUpdate(szcharge, FALSE, SUIT_NEXT_IN_30SEC);
			return TRUE;		
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS(item_battery, CItemBattery);


class CItemSecurity : public CItem
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_security.mdl");
		CItem::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_security.mdl");
	}
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		pPlayer->m_rgItems[ITEM_SECURITY] += 1;
		return TRUE;
	}
};

LINK_ENTITY_TO_CLASS(item_security, CItemSecurity);

class CItemLongJump : public CItem
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_longjump.mdl");
		CItem::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_longjump.mdl");
	}
	BOOL MyTouch(CBasePlayer* pPlayer)
	{
		if (pPlayer->m_fLongJump)
		{
			return FALSE;
		}

		if ((pPlayer->pev->weapons & (1 << WEAPON_SUIT)))
		{
			pPlayer->m_fLongJump = TRUE;// player now has longjump module

			g_engfuncs.pfnSetPhysicsKeyValue(pPlayer->edict(), "slj", "1");

			MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev);
			WRITE_STRING(STRING(pev->classname));
			MESSAGE_END();

			EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_A1");	// Play the longjump sound UNDONE: Kelly? correct sound?
			pPlayer->m_iLongJumpBattery = 5;
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS(item_longjump, CItemLongJump);

//	Antidote - syringe which will protect you from poisons
class CItemAntidote : public CItem
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_antidote.mdl");
		CItem::Spawn();
	}


	void Precache(void)
	{
		PRECACHE_MODEL("models/w_antidote.mdl");
		PRECACHE_SOUND("items/gunpickup3.wav");
		PRECACHE_SOUND("items/gunpickup4.wav");
	}

	BOOL MyTouch(CBasePlayer* pPlayer)
	{

		if (g_iSkillLevel == SKILL_EASY && pPlayer->m_rgItems[ITEM_ANTIDOTE] >= 5 && pPlayer->pev->weapons & (1 << WEAPON_SUIT))
			return FALSE;

		if (g_iSkillLevel == SKILL_MEDIUM && pPlayer->m_rgItems[ITEM_ANTIDOTE] >= 3 && pPlayer->pev->weapons & (1 << WEAPON_SUIT))
			return FALSE;

		if (g_iSkillLevel == SKILL_HARD && pPlayer->m_rgItems[ITEM_ANTIDOTE] >= 1 && pPlayer->pev->weapons & (1 << WEAPON_SUIT))
			return FALSE;

		pPlayer->m_rgItems[ITEM_ANTIDOTE]++;

		switch (RANDOM_LONG(0, 1))
		{
		case 0: EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup3.wav", 1, ATTN_NORM); break;
		case 1: EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup4.wav", 1, ATTN_NORM); break;
		}

		return TRUE;
	}
};

LINK_ENTITY_TO_CLASS(item_antidote, CItemAntidote);

//	Radiation - syringe which will protect you from radiation
class CItemRadiation : public CItem
{
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_rad.mdl");
		PRECACHE_SOUND("items/gunpickup3.wav");
		PRECACHE_SOUND("items/gunpickup4.wav");
	}
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_rad.mdl");
		CItem::Spawn();
	}

	BOOL MyTouch(CBasePlayer* pPlayer)
	{
		if (g_iSkillLevel == SKILL_EASY && pPlayer->m_rgItems[ITEM_RADIATION] >= 5 && pPlayer->pev->weapons & (1 << WEAPON_SUIT))
			return FALSE;

		if (g_iSkillLevel == SKILL_MEDIUM && pPlayer->m_rgItems[ITEM_RADIATION] >= 3 && pPlayer->pev->weapons & (1 << WEAPON_SUIT))
			return FALSE;

		if (g_iSkillLevel == SKILL_HARD && pPlayer->m_rgItems[ITEM_RADIATION] >= 1 && pPlayer->pev->weapons & (1 << WEAPON_SUIT))
			return FALSE;

		pPlayer->m_rgItems[ITEM_RADIATION]++;


		switch (RANDOM_LONG(0, 1))
		{
		case 0: EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup3.wav", 1, ATTN_NORM); break;
		case 1: EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup4.wav", 1, ATTN_NORM); break;
		}

		return TRUE;
	}
};

LINK_ENTITY_TO_CLASS(item_rad, CItemRadiation);
LINK_ENTITY_TO_CLASS(item_radiation, CItemRadiation);

// airtank - item which will able to be underwater for 2 mins
class CItemOxygen : public CItem
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_oxygen.mdl");
		CItem::Spawn();
	}

	void Precache(void)
	{
		PRECACHE_MODEL("models/w_oxygen.mdl");
		PRECACHE_SOUND("items/airtank1.wav");
		PRECACHE_SOUND("items/gunpickup3.wav");
		PRECACHE_SOUND("items/gunpickup4.wav");
	}
	BOOL MyTouch(CBasePlayer* pPlayer)
	{
		if (pPlayer->m_rgItems[ITEM_OXYGEN] >= 1)
			return FALSE;

		pPlayer->m_rgItems[ITEM_OXYGEN]++;
		pPlayer->m_fOxygen = 15;

		EMIT_SOUND(ENT(pev), CHAN_VOICE, "items/airtank1.wav", 1.0, ATTN_NORM);

		return TRUE;
	}
};

LINK_ENTITY_TO_CLASS(item_airtank, CItemOxygen);

// Adrenaline - syringe which will revive you after death
class CItemAdrenaline : public CItem
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_adrenaline.mdl");
		CItem::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_adrenaline.mdl");
		PRECACHE_SOUND("items/gunpickup3.wav");
		PRECACHE_SOUND("items/gunpickup4.wav");
	}
	BOOL MyTouch(CBasePlayer* pPlayer)
	{
		if (pPlayer->m_rgItems[ITEM_ADRENALINE] <= 0 && pPlayer->pev->weapons & (1 << WEAPON_SUIT))
		{
			switch (RANDOM_LONG(0, 1))
			{
			case 0: EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup3.wav", 1, ATTN_NORM); break;
			case 1: EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup4.wav", 1, ATTN_NORM); break;
			}
			pPlayer->m_rgItems[ITEM_ADRENALINE] += 1;
			return TRUE;
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS(item_adrenaline, CItemAdrenaline);


// Scope - item which will able to put Scope on crossbow
class CItemScope : public CItem
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_crossbow.mdl");
		CItem::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_crossbow.mdl");
	}
	BOOL MyTouch(CBasePlayer* pPlayer)
	{
		pPlayer->m_fHasCrossbowScope = 1;
		return TRUE;
	}
};

LINK_ENTITY_TO_CLASS(item_scope, CItemScope);

// Silencer - item which will able to put silencer on glock
class CItemSilencer : public CItem
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_silencer.mdl");
		CItem::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_silencer.mdl");
	}
	BOOL MyTouch(CBasePlayer* pPlayer)
	{
		pPlayer->m_fHasSilencer = 1;
		return TRUE;
	}
};

LINK_ENTITY_TO_CLASS(item_silencer, CItemSilencer);

//	Alien shield - Reduces incoming damage if you have armor.
class CItemShield : public CItem
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_shield.mdl");
		pev->netname = MAKE_STRING("Alien Shield");
		CItem::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_shield.mdl");
	}
	BOOL MyTouch(CBasePlayer* pPlayer)
	{
		if (!pPlayer->m_rgItems[ITEM_SHIELD] && pPlayer->pev->weapons & (1 << WEAPON_SUIT))
		{
			pPlayer->SetSuitUpdate("!HEV_A2", FALSE, SUIT_REPEAT_OK);
			EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);
			pPlayer->m_rgItems[ITEM_SHIELD] = 1;

			MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev);
			WRITE_STRING(STRING(pev->netname));
			MESSAGE_END();

			return TRUE;
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS(item_shield, CItemShield);


//	Tourniquet, item which will stop bleeding
class CItemTourniquet : public CItem
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_tourniquet.mdl");
		CItem::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_tourniquet.mdl");
	}
	BOOL MyTouch(CBasePlayer* pPlayer)
	{
		if (!pPlayer->m_rgItems[ITEM_TOURNIQUET])
		{
			pPlayer->SetSuitUpdate("!HEV_TOURNIQUETTE", FALSE, SUIT_REPEAT_OK);
			EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);
			pPlayer->m_rgItems[ITEM_TOURNIQUET] = 1;

			MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev);
			WRITE_STRING(STRING(pev->netname));
			MESSAGE_END();

			return TRUE;
		}
	}
};

LINK_ENTITY_TO_CLASS(item_tourniquet, CItemTourniquet);