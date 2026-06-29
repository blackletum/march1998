/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	Copyright (c) 2015, FREEZE.
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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"

enum minigun_e
{
	MINIGUN_IDLE1,
	MINIGUN_IDLE2,
	MINIGUN_IDLE3,
	MINIGUN_IDLE4,
	MINIGUN_DRAW,
	MINIGUN_HOLSTER,
	MINIGUN_SPINUP,
	MINIGUN_SPINDOWN,
	MINIGUN_FIRE,
};

class CMINIGUN : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int iItemSlot(void) { return 3; }
	int GetItemInfo(ItemInfo* p);
	int AddToPlayer(CBasePlayer* pPlayer);
	void Holster(void);
	void PrimaryAttack(void);
	void Shoot(void);
	BOOL Deploy(void);
	void WeaponIdle(void);
	float m_flNextAnimTime;
	int m_iShell;
private:
	//		int m_iShell;
	int g_sModelIndexGunSmoke;
	int bulletsFired;
};

LINK_ENTITY_TO_CLASS( weapon_minigun, CMINIGUN );


//=========================================================
//=========================================================

void CMINIGUN::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_minigun"); // hack to allow for old names
	Precache( );
	SET_MODEL(ENT(pev), "models/w_minigun.mdl");
	m_iId = WEAPON_MINIGUN;

	m_iDefaultAmmo = 100;

	FallInit();// get ready to fall down.
}


void CMINIGUN::Precache( void )
{
	PRECACHE_MODEL("models/v_minigun.mdl");
	PRECACHE_MODEL("models/w_minigun.mdl");
	PRECACHE_MODEL("models/p_9mmAR.mdl");

	m_iShell = PRECACHE_MODEL ("models/shell.mdl");// brass shellTE_MODEL

	g_sModelIndexGunSmoke = PRECACHE_MODEL ("sprites/xsmoke1.spr");// smoke

	PRECACHE_MODEL("models/w_9mmARclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");              


	PRECACHE_SOUND("hassault/hw_shoot1.wav");
	PRECACHE_SOUND("hassault/hw_shoot2.wav");
	PRECACHE_SOUND("hassault/hw_shoot3.wav");
	PRECACHE_SOUND("hassault/hw_spin.wav");
	PRECACHE_SOUND("hassault/hw_spindown.wav");
	PRECACHE_SOUND("hassault/hw_spinup.wav");
	PRECACHE_SOUND("hassault/hw_gun4.wav");

	PRECACHE_SOUND ("weapons/357_cock1.wav");

}

int CMINIGUN::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "minigun";
	p->iMaxAmmo1 = 200;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 3;
	p->iPosition = 4;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_MINIGUN;
	p->iWeight = 15;

	return 1;
}

int CMINIGUN::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

BOOL CMINIGUN::Deploy( )
{
	bulletsFired = 0;
	m_fInAttack = 0;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.5;

	if (m_pPlayer->m_rgItems[ITEM_IVANSUIT])
		pev->body = 1;
	else
		pev->body = 0;


	return DefaultDeploy("models/v_minigun.mdl", "models/p_9mmAR.mdl", MINIGUN_DRAW, "egon");
}

void CMINIGUN::Holster(void)
{
	m_fInReload = FALSE;// cancel any reload in progress.
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.5;
	SendWeaponAnim(MINIGUN_HOLSTER);
}

void CMINIGUN::PrimaryAttack()
{
	if ( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] == 0 && m_fInAttack!=0)
	{
		SendWeaponAnim(MINIGUN_SPINDOWN);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "hassault/hw_spindown.wav", 1, ATTN_NORM, 0, 100);
		WeaponIdle();
		return;
	}
	if ( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] == 0)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.5;
		return;
	}
	if ( m_fInAttack == 0)
	{
		// don't fire underwater
		if (m_pPlayer->pev->waterlevel == 3 && m_pPlayer->pev->watertype > CONTENT_FLYFIELD)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = 0.15;
			return;
		}

		// spin up
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "hassault/hw_spinup.wav", 1, ATTN_NORM, 0, 100);
		SendWeaponAnim(MINIGUN_SPINUP);
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.9;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.9;
		m_fInAttack = 1;
	}
	else if (m_fInAttack == 1)
	{
		if ( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] == 0 )
		{
			WeaponIdle();
			return;
		}
		if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
			m_fInAttack = 2;
	}
	else if (m_fInAttack == 2)
	{
		if ( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] == 0 )
		{
			WeaponIdle();
			return;
		}

//===============================================================================
//Magic Nipples - Server Side Weapons
//===============================================================================

	if ( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] == 0 || ( m_pPlayer->pev->waterlevel == 3 && m_pPlayer->pev->watertype > CONTENT_FLYFIELD ))
	{
		SendWeaponAnim(MINIGUN_SPINDOWN);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "hassault/hw_spindown.wav", 1, ATTN_NORM, 0, 100);
		WeaponIdle();
		return;
	}

	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

	bulletsFired ++;
	SendWeaponAnim( MINIGUN_FIRE );

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	m_flNextAnimTime = gpGlobals->time + 0.2;

	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "hassault/hw_gun4.wav", 1, ATTN_NORM, 0, 94 + RANDOM_LONG(0, 0xf));

	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

	Vector	vecShellVelocity = m_pPlayer->pev->velocity 
							 + gpGlobals->v_right * RANDOM_FLOAT(50,70) 
							 + gpGlobals->v_up * RANDOM_FLOAT(100,150) 
							 + gpGlobals->v_forward * 25;
	EjectBrass ( pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_up * -12 + gpGlobals->v_forward * 17 + gpGlobals->v_right * 6 , vecShellVelocity, pev->angles.y, m_iShell, TE_BOUNCE_SHELL ); 
	
	Vector vecSrc = m_pPlayer->GetGunPosition( );

	Vector vecSorc = pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_up * -14 + gpGlobals->v_forward * 57 + gpGlobals->v_right * 6;
	
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if (bulletsFired > 5)
	{
		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecSorc );
			WRITE_BYTE( TE_SPRITE );
			WRITE_COORD( vecSorc.x );
			WRITE_COORD( vecSorc.y );
			WRITE_COORD( vecSorc.z );
			WRITE_SHORT( g_sModelIndexGunSmoke );
			WRITE_BYTE( 0.1 ); // scale * 10
			WRITE_BYTE( 15 ); // brightness
		MESSAGE_END();
	}

	float flZVel = m_pPlayer->pev->velocity.z;
	m_pPlayer->pev->velocity = m_pPlayer->pev->velocity - gpGlobals->v_forward * 25;
	m_pPlayer->pev->velocity.z = flZVel;


	m_pPlayer->FireBullets( 1, vecSrc, vecAiming, VECTOR_CONE_6DEGREES, 8192, BULLET_PLAYER_MINIGUN, 1 );

	m_pPlayer->pev->punchangle.x = RANDOM_FLOAT( -2, 2 );
	m_pPlayer->pev->punchangle.y = RANDOM_FLOAT( -2, 2 );

	if ( !m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )

		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.08;

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.08;
	}
}

void CMINIGUN::WeaponIdle( void )
{
	ResetEmptySound( );

	if ( m_flTimeWeaponIdle > gpGlobals->time )
		return;

	if (m_fInAttack!=0)
	{
		if ( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] > 0)
		{
			SendWeaponAnim(MINIGUN_SPINDOWN);
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "hassault/hw_spindown.wav", 1, ATTN_NORM, 0, 100);
			m_fInAttack = 0;
			m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.9;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0;
			return;
		}
		else
		{
			SendWeaponAnim(MINIGUN_SPINDOWN);
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "hassault/hw_spindown.wav", 1, ATTN_NORM, 0, 100);
			m_fInAttack = 0;
			m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.9;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0;
			return;
		}
	}
	bulletsFired = 0;
	int iAnim;
	switch ( RANDOM_LONG( 0, 2 ) )
	{
	case 0:	
		iAnim = MINIGUN_IDLE1;	
		break;
	case 1:	
		iAnim = MINIGUN_IDLE2;	
		break;
	
	default:
	case 2:
		iAnim = MINIGUN_IDLE3;
		break;
	}
	SendWeaponAnim( iAnim );
	m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT (10, 20);
	
}




class CMINIGUNChainammo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_chainammo.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_chainammo.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( AMMO_CHAINBOX_GIVE, "minigun", 300) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_chainbox, CMINIGUNChainammo );