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
//
// Ammo.cpp
//
// implementation of CHudAmmo class
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "pm_shared.h"

#include <string.h>
#include <stdio.h>

#include "ammohistory.h"
#include "vgui_TeamFortressViewport.h"

WEAPON *gpActiveSel;	// NULL means off, 1 means just the menu bar, otherwise
						// this points to the active weapon menu item
WEAPON *gpLastSel;		// Last weapon menu selection 

client_sprite_t *GetSpriteList(client_sprite_t *pList, const char *psz, int iRes, int iCount);

WeaponsResource gWR;

int g_weaponselect = 0;
int slotstart = 0;

void WeaponsResource :: LoadAllWeaponSprites( void )
{
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		if ( rgWeapons[i].iId )
			LoadWeaponSprites( &rgWeapons[i] );
	}
}

int WeaponsResource :: CountAmmo( int iId ) 
{ 
	if ( iId < 0 )
		return 0;

	return riAmmo[iId];
}

int WeaponsResource :: HasAmmo( WEAPON *p )
{
	if ( !p )
		return FALSE;

	// weapons with no max ammo can always be selected
	if ( p->iMax1 == -1 )
		return TRUE;

	return (p->iAmmoType == -1) || p->iClip > 0 || CountAmmo(p->iAmmoType) 
		|| CountAmmo(p->iAmmo2Type) || ( p->iFlags & WEAPON_FLAGS_SELECTONEMPTY );
}


void WeaponsResource :: LoadWeaponSprites( WEAPON *pWeapon )
{
	int i, iRes;

	if (ScreenWidth < 640)
		iRes = 320;
	else
		iRes = 640;

	char sz[128];

	if ( !pWeapon )
		return;

	memset( &pWeapon->rcActive, 0, sizeof(wrect_t) );
	memset( &pWeapon->rcInactive, 0, sizeof(wrect_t) );
	memset( &pWeapon->rcAmmo, 0, sizeof(wrect_t) );
	memset( &pWeapon->rcAmmo2, 0, sizeof(wrect_t) );
	pWeapon->hInactive = 0;
	pWeapon->hActive = 0;
	pWeapon->hAmmo = 0;
	pWeapon->hAmmo2 = 0;

	sprintf(sz, "sprites/%s.txt", pWeapon->szName);
	client_sprite_t *pList = SPR_GetList(sz, &i);

	if (!pList)
		return;

	client_sprite_t *p;
	
	p = GetSpriteList( pList, "crosshair", iRes, i );
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hCrosshair = SPR_Load(sz);
		pWeapon->rcCrosshair = p->rc;
	}
	else
		pWeapon->hCrosshair = NULL;

	p = GetSpriteList(pList, "autoaim", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hAutoaim = SPR_Load(sz);
		pWeapon->rcAutoaim = p->rc;
	}
	else
		pWeapon->hAutoaim = 0;

	p = GetSpriteList( pList, "zoom", iRes, i );
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hZoomedCrosshair = SPR_Load(sz);
		pWeapon->rcZoomedCrosshair = p->rc;
	}
	else
	{
		pWeapon->hZoomedCrosshair = pWeapon->hCrosshair; //default to non-zoomed crosshair
		pWeapon->rcZoomedCrosshair = pWeapon->rcCrosshair;
	}

	p = GetSpriteList(pList, "zoom_autoaim", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hZoomedAutoaim = SPR_Load(sz);
		pWeapon->rcZoomedAutoaim = p->rc;
	}
	else
	{
		pWeapon->hZoomedAutoaim = pWeapon->hZoomedCrosshair;  //default to zoomed crosshair
		pWeapon->rcZoomedAutoaim = pWeapon->rcZoomedCrosshair;
	}

	p = GetSpriteList(pList, "weapon", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hInactive = SPR_Load(sz);
		pWeapon->rcInactive = p->rc;

		gHR.iHistoryGap = max( gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top );
	}
	else
		pWeapon->hInactive = 0;

	p = GetSpriteList(pList, "weapon_s", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hActive = SPR_Load(sz);
		pWeapon->rcActive = p->rc;
	}
	else
		pWeapon->hActive = 0;

	p = GetSpriteList(pList, "ammo", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hAmmo = SPR_Load(sz);
		pWeapon->rcAmmo = p->rc;

		gHR.iHistoryGap = max( gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top );
	}
	else
		pWeapon->hAmmo = 0;

	p = GetSpriteList(pList, "ammo2", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hAmmo2 = SPR_Load(sz);
		pWeapon->rcAmmo2 = p->rc;

		gHR.iHistoryGap = max( gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top );
	}
	else
		pWeapon->hAmmo2 = 0;

}

// Returns the first weapon for a given slot.
WEAPON *WeaponsResource :: GetFirstPos( int iSlot, int hud )
{
	WEAPON *pret = NULL;

	
		for (int i = 0; i < MAX_WEAPON_POSITIONS; i++)
		{
			if (rgSlots[iSlot][i] && HasAmmo(rgSlots[iSlot][i]))
			{
				pret = rgSlots[iSlot][i];
				break;
			}
		}
	return pret;
}


WEAPON* WeaponsResource :: GetNextActivePos( int iSlot, int iSlotPos )
{
	if ( iSlotPos >= MAX_WEAPON_POSITIONS || iSlot >= MAX_WEAPON_SLOTS )
		return NULL;

	WEAPON *p = gWR.rgSlots[ iSlot ][ iSlotPos+1 ];
	
	if ( !p || !gWR.HasAmmo(p) )
		return GetNextActivePos( iSlot, iSlotPos + 1 );

	return p;
}


int giBucketHeight, giBucketWidth, giABHeight, giABWidth; // Ammo Bar width and height

HLSPRITE ghsprBuckets;					// Sprite for top row of weapons menu

DECLARE_MESSAGE(m_Ammo, CurWeapon );	// Current weapon and clip
DECLARE_MESSAGE(m_Ammo, WeaponList);	// new weapon type
DECLARE_MESSAGE(m_Ammo, AmmoX);			// update known ammo type's count
DECLARE_MESSAGE(m_Ammo, AmmoPickup);	// flashes an ammo pickup record
DECLARE_MESSAGE(m_Ammo, WeapPickup);    // flashes a weapon pickup record
DECLARE_MESSAGE(m_Ammo, HideWeapon);	// hides the weapon, ammo, and crosshair displays temporarily
DECLARE_MESSAGE(m_Ammo, ItemPickup);

//inventory
DECLARE_MESSAGE(m_Ammo, AntidoteV);
DECLARE_MESSAGE(m_Ammo, RadiationV);
DECLARE_MESSAGE(m_Ammo, OxygenV);
DECLARE_MESSAGE(m_Ammo, FlashlightV);
DECLARE_MESSAGE(m_Ammo, AdrenalineV);
DECLARE_MESSAGE(m_Ammo, LonJumBat);
DECLARE_MESSAGE(m_Ammo, IvanSuitV);
DECLARE_MESSAGE(m_Ammo, DefaultSuitV);

DECLARE_COMMAND(m_Ammo, Slot1);
DECLARE_COMMAND(m_Ammo, Slot2);
DECLARE_COMMAND(m_Ammo, Slot3);
DECLARE_COMMAND(m_Ammo, Slot4);
DECLARE_COMMAND(m_Ammo, Slot5);
DECLARE_COMMAND(m_Ammo, Slot6);
DECLARE_COMMAND(m_Ammo, Slot7);
DECLARE_COMMAND(m_Ammo, Slot8);
DECLARE_COMMAND(m_Ammo, Slot9);
DECLARE_COMMAND(m_Ammo, Slot10);
DECLARE_COMMAND(m_Ammo, Close);
DECLARE_COMMAND(m_Ammo, NextWeapon);
DECLARE_COMMAND(m_Ammo, PrevWeapon);

// width of ammo fonts
#define AMMO_SMALL_WIDTH 10
#define AMMO_LARGE_WIDTH 20

#define HISTORY_DRAW_TIME	"5"

int CHudAmmo::Init(void)
{
	gHUD.AddHudElem(this);

	HOOK_MESSAGE(CurWeapon);
	HOOK_MESSAGE(WeaponList);
	HOOK_MESSAGE(AmmoPickup);
	HOOK_MESSAGE(WeapPickup);
	HOOK_MESSAGE(ItemPickup);
	HOOK_MESSAGE(HideWeapon);
	HOOK_MESSAGE(AmmoX);

	//inventory
	HOOK_MESSAGE(AntidoteV);
	HOOK_MESSAGE(RadiationV);
	HOOK_MESSAGE(LonJumBat);
	HOOK_MESSAGE(OxygenV);
	HOOK_MESSAGE(FlashlightV);
	HOOK_MESSAGE(AdrenalineV);

	// Suit variants
	HOOK_MESSAGE(IvanSuitV);
	HOOK_MESSAGE(DefaultSuitV);

	HOOK_COMMAND("slot1", Slot1);
	HOOK_COMMAND("slot2", Slot2);
	HOOK_COMMAND("slot3", Slot3);
	HOOK_COMMAND("slot4", Slot4);
	HOOK_COMMAND("slot5", Slot5);
	HOOK_COMMAND("slot6", Slot6);
	HOOK_COMMAND("slot7", Slot7);
	HOOK_COMMAND("slot8", Slot8);
	HOOK_COMMAND("slot9", Slot9);
	HOOK_COMMAND("slot10", Slot10);
	HOOK_COMMAND("cancelselect", Close);
	HOOK_COMMAND("invnext", NextWeapon);
	HOOK_COMMAND("invprev", PrevWeapon);

	Reset();

	CVAR_CREATE( "hud_drawhistory_time", HISTORY_DRAW_TIME, 0 );
	CVAR_CREATE( "hud_fastswitch", "0", FCVAR_ARCHIVE );		// controls whether or not weapons can be selected in one keypress

	m_iFlags |= HUD_ACTIVE; //!!!

	gWR.Init();
	gHR.Init();

	return 1;
};

void CHudAmmo::Reset(void)
{
	//inventory
	m_iAntidote = 0;
	m_iRadiation = 0;
	m_iLongJumpBat = 0;
	m_iOxygen = 0;
	m_flashOn = 0;
	m_iAdrenaline = 0;
	gHUD.m_fAlphaSuit = 0;

	m_fFade = 0;
	m_iFlags |= HUD_ACTIVE; //!!!

	gpActiveSel = NULL;
	gHUD.m_iHideHUDDisplay = 0;

	gWR.Reset();
	gHR.Reset();

	//	VidInit();

}

int CHudAmmo::VidInit(void)
{
	// Load sprites for buckets (top row of weapon menu)

	//magic nipples - get shape of inventory squares
	m_prc2 = &gHUD.GetSpriteRect(gHUD.GetSpriteIndex("antidote_1"));
	m_iWidth = m_prc2->right - m_prc2->left;
	m_iHeight = m_prc2->top - m_prc2->bottom;

	int HUD_airtank = gHUD.GetSpriteIndex("oxygen_off");
	int HUD_spring = gHUD.GetSpriteIndex("longjump_off");

	int HUD_airtank_full = gHUD.GetSpriteIndex("oxygen_on");
	int HUD_spring_full = gHUD.GetSpriteIndex("longjump_on");

	m_HUD_bucket0 = gHUD.GetSpriteIndex( "bucket1" );
	m_HUD_selection = gHUD.GetSpriteIndex( "selection" );

	m_hSprite4 = gHUD.GetSprite(HUD_spring);
	m_hSprite3 = gHUD.GetSprite(HUD_airtank);
	m_hSprite8 = gHUD.GetSprite(HUD_airtank_full);
	m_hSprite9 = gHUD.GetSprite(HUD_spring_full);

	ghsprBuckets = gHUD.GetSprite(m_HUD_bucket0);
	giBucketWidth = gHUD.GetSpriteRect(m_HUD_bucket0).right - gHUD.GetSpriteRect(m_HUD_bucket0).left;
	giBucketHeight = gHUD.GetSpriteRect(m_HUD_bucket0).bottom - gHUD.GetSpriteRect(m_HUD_bucket0).top;

	gHR.iHistoryGap = max( gHR.iHistoryGap, gHUD.GetSpriteRect(m_HUD_bucket0).bottom - gHUD.GetSpriteRect(m_HUD_bucket0).top);

	// If we've already loaded weapons, let's get new sprites
	gWR.LoadAllWeaponSprites();

	if (ScreenWidth >= 640)
	{
		giABWidth = 20;
		giABHeight = 4;
	}
	else
	{
		giABWidth = 10;
		giABHeight = 2;
	}

	return 1;
}

//
// Think:
//  Used for selection of weapon menu item.
//
void CHudAmmo::Think(void)
{
	if ( gHUD.m_fPlayerDead )
		return;

	if ( gHUD.m_iWeaponBits != gWR.iOldWeaponBits )
	{
		gWR.iOldWeaponBits = gHUD.m_iWeaponBits;

		for (int i = MAX_WEAPONS-1; i > 0; i-- )
		{
			WEAPON *p = gWR.GetWeapon(i);

			if ( p )
			{
				if ( gHUD.m_iWeaponBits & ( 1 << p->iId ) )
					gWR.PickupWeapon( p );
				else
					gWR.DropWeapon( p );
			}
		}
	}

	if (!gpActiveSel)
		return;

	// has the player selected one?
	if (gHUD.m_iKeyBits & IN_ATTACK)
	{
		if (gpActiveSel != (WEAPON *)1)
		{
			ServerCmd(gpActiveSel->szName);
			g_weaponselect = gpActiveSel->iId;
		}

		gpLastSel = gpActiveSel;
		gpActiveSel = NULL;
		gHUD.m_iKeyBits &= ~IN_ATTACK;

		PlaySound("common/wpn_select.wav", 1);
	}

}

//
// Helper function to return a Ammo pointer from id
//

HLSPRITE* WeaponsResource :: GetAmmoPicFromWeapon( int iAmmoId, wrect_t& rect )
{
	for ( int i = 0; i < MAX_WEAPONS; i++ )
	{
		if ( rgWeapons[i].iAmmoType == iAmmoId )
		{
			rect = rgWeapons[i].rcAmmo;
			return &rgWeapons[i].hAmmo;
		}
		else if ( rgWeapons[i].iAmmo2Type == iAmmoId )
		{
			rect = rgWeapons[i].rcAmmo2;
			return &rgWeapons[i].hAmmo2;
		}
	}

	return NULL;
}


// Menu Selection Code

void WeaponsResource :: SelectSlot( int iSlot, int fAdvance, int iDirection )
{
	if ( gHUD.m_Menu.m_fMenuDisplayed && (fAdvance == FALSE) && (iDirection == 1) )	
	{ // menu is overriding slot use commands
		gHUD.m_Menu.SelectMenuItem( iSlot + 1 );  // slots are one off the key numbers
		return;
	}

	if ( iSlot > MAX_WEAPON_SLOTS )
		return;

	if ( gHUD.m_fPlayerDead || gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL ) )
		return;

	if (!(gHUD.m_iWeaponBits & (1<<(WEAPON_SUIT)) ))
		return;

	if ( ! ( gHUD.m_iWeaponBits & ~(1<<(WEAPON_SUIT)) ))
		return;

	WEAPON *p = NULL;
	bool fastSwitch = CVAR_GET_FLOAT( "hud_fastswitch" ) != 0;

	if ( (gpActiveSel == NULL) || (gpActiveSel == (WEAPON *)1) || (iSlot != gpActiveSel->iSlot) )
	{
		PlaySound( "common/wpn_hudon.wav", 1 );
		p = GetFirstPos( iSlot, 0 );

		if ( p && fastSwitch ) // check for fast weapon switch mode
		{
			// if fast weapon switch is on, then weapons can be selected in a single keypress
			// but only if there is only one item in the bucket
			WEAPON *p2 = GetNextActivePos( p->iSlot, p->iSlotPos );
			if ( !p2 )
			{	// only one active item in bucket, so change directly to weapon
				ServerCmd( p->szName );
				g_weaponselect = p->iId;
				return;
			}
		}
	}
	else
	{
		PlaySound("common/wpn_moveselect.wav", 1);
		if ( gpActiveSel )
			p = GetNextActivePos( gpActiveSel->iSlot, gpActiveSel->iSlotPos );
		if ( !p )
			p = GetFirstPos( iSlot, 0 );
	}

	
	if ( !p )  // no selection found
	{
		// just display the weapon list, unless fastswitch is on just ignore it
		if ( !fastSwitch )
			gpActiveSel = (WEAPON *)1;
		else
			gpActiveSel = NULL;
	}
	else 
		gpActiveSel = p;
}

//------------------------------------------------------------------------
// Message Handlers
//------------------------------------------------------------------------

//inventory
int CHudAmmo::MsgFunc_AntidoteV(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int x = READ_BYTE();

	if (x != m_iAntidote)
	{
		m_fFade = FADE_TIME;
		m_iAntidote = x;
	}

	return 1;
}

int CHudAmmo::MsgFunc_RadiationV(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int x = READ_BYTE();

	if (x != m_iRadiation)
	{
		m_fFade = FADE_TIME;
		m_iRadiation = x;
	}

	return 1;
}

int CHudAmmo::MsgFunc_LonJumBat(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int x = READ_SHORT();

	m_iLongJumpBat = x;

	return 1;
}

int CHudAmmo::MsgFunc_FlashlightV(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	m_flashOn = READ_BYTE();
	int x = READ_BYTE();

	return 1;
}

int CHudAmmo::MsgFunc_OxygenV(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int x = READ_BYTE();

	if (x != m_iOxygen)
	{
		m_fFade = FADE_TIME;
		m_iOxygen = x;
		m_fOxygen = x;
	}

	return 1;
}

int CHudAmmo::MsgFunc_AdrenalineV(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int x = READ_BYTE();

	if (x != m_iAdrenaline)
	{
		m_fFade = FADE_TIME;
		m_iAdrenaline = x;
	}

	return 1;
}

int CHudAmmo::MsgFunc_IvanSuitV(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int x = READ_BYTE();

	if (x != gHUD.m_fAlphaSuit)
	{
		m_fFade = FADE_TIME;
		gHUD.m_fAlphaSuit = x;
	}

	return 1;
}

int CHudAmmo::MsgFunc_DefaultSuitV(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int x = READ_BYTE();

	if (x != gHUD.m_fDefaultSuit)
	{
		m_fFade = FADE_TIME;
		gHUD.m_fDefaultSuit = x;
	}

	return 1;
}

//
// AmmoX  -- Update the count of a known type of ammo
// 
int CHudAmmo::MsgFunc_AmmoX(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	int iIndex = READ_BYTE();
	int iCount = READ_BYTE();

	gWR.SetAmmo( iIndex, abs(iCount) );

	return 1;
}

int CHudAmmo::MsgFunc_AmmoPickup( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	int iIndex = READ_BYTE();
	int iCount = READ_BYTE();

	// Add ammo to the history
	gHR.AddToHistory( HISTSLOT_AMMO, iIndex, abs(iCount) );

	return 1;
}

int CHudAmmo::MsgFunc_WeapPickup( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	int iIndex = READ_BYTE();

	// Add the weapon to the history
	gHR.AddToHistory( HISTSLOT_WEAP, iIndex );

	return 1;
}

int CHudAmmo::MsgFunc_ItemPickup( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	const char *szName = READ_STRING();

	// Add the weapon to the history
	gHR.AddToHistory( HISTSLOT_ITEM, szName );

	return 1;
}


int CHudAmmo::MsgFunc_HideWeapon( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	
	gHUD.m_iHideHUDDisplay = READ_BYTE();

	if (gEngfuncs.IsSpectateOnly())
		return 1;

	if ( gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL ) )
	{
		static wrect_t nullrc;
		gpActiveSel = NULL;
		SetCrosshair( 0, nullrc, 0, 0, 0 );
	}
	else
	{
		if ( m_pWeapon )
			SetCrosshair( m_pWeapon->hCrosshair, m_pWeapon->rcCrosshair, 255, 255, 255 );
	}

	return 1;
}

// 
//  CurWeapon: Update hud state with the current weapon and clip count. Ammo
//  counts are updated with AmmoX. Server assures that the Weapon ammo type 
//  numbers match a real ammo type.
//
int CHudAmmo::MsgFunc_CurWeapon(const char *pszName, int iSize, void *pbuf )
{
	static wrect_t nullrc;
	int fOnTarget = FALSE;

	BEGIN_READ( pbuf, iSize );

	int iState = READ_BYTE();
	int iId = READ_CHAR();
	int iClip = READ_CHAR();
	int iMaxClip = READ_BYTE();

	// detect if we're also on target
	if ( iState > 1 )
	{
		fOnTarget = TRUE;
	}

	if ( iId < 1 )
	{
		SetCrosshair(0, nullrc, 0, 0, 0);
		return 0;
	}

	if ( g_iUser1 != OBS_IN_EYE )
	{
		// Is player dead???
		if ((iId == -1) && (iClip == -1))
		{
			gHUD.m_fPlayerDead = TRUE;
			gpActiveSel = NULL;
			return 1;
		}
		gHUD.m_fPlayerDead = FALSE;
	}

	WEAPON *pWeapon = gWR.GetWeapon( iId );

	if ( !pWeapon )
		return 0;

	if (iClip < -1)
	{
		pWeapon->iClip = abs(iClip);
	}
	else
	{
		pWeapon->iClip = iClip;
	}	


	if ( iState == 0 )	// we're not the current weapon, so update no more
		return 1;

	m_pWeapon = pWeapon;

	if ( gHUD.m_iFOV >= 90 )
	{ // normal crosshairs
		if (fOnTarget && m_pWeapon->hAutoaim)
			SetCrosshair(m_pWeapon->hAutoaim, m_pWeapon->rcAutoaim, 255, 255, 255);
		else
			SetCrosshair(m_pWeapon->hCrosshair, m_pWeapon->rcCrosshair, 255, 255, 255);
	}
	else
	{ // zoomed crosshairs
		if (fOnTarget && m_pWeapon->hZoomedAutoaim)
			SetCrosshair(m_pWeapon->hZoomedAutoaim, m_pWeapon->rcZoomedAutoaim, 255, 255, 255);
		else
			SetCrosshair(m_pWeapon->hZoomedCrosshair, m_pWeapon->rcZoomedCrosshair, 255, 255, 255);

	}

	m_fFade = 250.0f; //!!!
	m_iFlags |= HUD_ACTIVE;
	
	return 1;
}

//
// WeaponList -- Tells the hud about a new weapon type.
//
int CHudAmmo::MsgFunc_WeaponList(const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	
	WEAPON Weapon;

	strcpy( Weapon.szName, READ_STRING() );
	Weapon.iAmmoType = (int)READ_CHAR();	
	
	Weapon.iMax1 = READ_BYTE();
	if (Weapon.iMax1 == 255)
		Weapon.iMax1 = -1;

	Weapon.iAmmo2Type = READ_CHAR();
	Weapon.iMax2 = READ_BYTE();
	if (Weapon.iMax2 == 255)
		Weapon.iMax2 = -1;

	Weapon.iSlot = READ_CHAR();
	Weapon.iSlotPos = READ_CHAR();
	Weapon.iId = READ_CHAR();
	Weapon.iFlags = READ_BYTE();
	Weapon.iMaxClip = READ_BYTE();
	Weapon.iClip = 0;

	gWR.AddWeapon( &Weapon );

	return 1;

}

//------------------------------------------------------------------------
// Command Handlers
//------------------------------------------------------------------------
// Slot button pressed
void CHudAmmo::SlotInput( int iSlot )
{
	// Let the Viewport use it first, for menus
	if ( gViewPort && gViewPort->SlotInput( iSlot ) )
		return;

	gWR.SelectSlot(iSlot, FALSE, 1);
}

void CHudAmmo::UserCmd_Slot1(void)
{
	SlotInput( 0 );
}

void CHudAmmo::UserCmd_Slot2(void)
{
	SlotInput( 1 );
}

void CHudAmmo::UserCmd_Slot3(void)
{
	SlotInput( 2 );
}

void CHudAmmo::UserCmd_Slot4(void)
{
	SlotInput( 3 );
}

void CHudAmmo::UserCmd_Slot5(void)
{
	SlotInput( 4 );
}

void CHudAmmo::UserCmd_Slot6(void)
{
	SlotInput( 5 );
}

void CHudAmmo::UserCmd_Slot7(void)
{
	SlotInput( 6 );
}

void CHudAmmo::UserCmd_Slot8(void)
{
	SlotInput( 7 );
}

void CHudAmmo::UserCmd_Slot9(void)
{
	SlotInput( 8 );
}

void CHudAmmo::UserCmd_Slot10(void)
{
	SlotInput( 9 );
}

void CHudAmmo::UserCmd_Close(void)
{
	if (gpActiveSel)
	{
		gpLastSel = gpActiveSel;
		gpActiveSel = NULL;
		PlaySound("common/wpn_hudoff.wav", 1);
	}
	else
		ClientCmd("escape");
}


// Selects the next item in the weapon menu
void CHudAmmo::UserCmd_NextWeapon(void)
{
	if ( gHUD.m_fPlayerDead || (gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL)) )
		return;

	if ( !gpActiveSel || gpActiveSel == (WEAPON*)1 )
		gpActiveSel = m_pWeapon;

	int pos = 0;
	int slot = 0;
	if ( gpActiveSel )
	{
		pos = gpActiveSel->iSlotPos + 1;
		slot = gpActiveSel->iSlot;
	}

	for ( int loop = 0; loop <= 1; loop++ )
	{
		for ( ; slot < MAX_WEAPON_SLOTS; slot++ )
		{
			for ( ; pos < MAX_WEAPON_POSITIONS; pos++ )
			{
				WEAPON *wsp = gWR.GetWeaponSlot( slot, pos );

				if ( wsp && gWR.HasAmmo(wsp) )
				{
					gpActiveSel = wsp;
					return;
				}
			}

			pos = 0;
		}

		slot = 0;  // start looking from the first slot again
	}

	gpActiveSel = NULL;
}

// Selects the previous item in the menu
void CHudAmmo::UserCmd_PrevWeapon(void)
{
	if ( gHUD.m_fPlayerDead || (gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL)) )
		return;

	if ( !gpActiveSel || gpActiveSel == (WEAPON*)1 )
		gpActiveSel = m_pWeapon;

	int pos = MAX_WEAPON_POSITIONS-1;
	int slot = MAX_WEAPON_SLOTS-1;
	if ( gpActiveSel )
	{
		pos = gpActiveSel->iSlotPos - 1;
		slot = gpActiveSel->iSlot;
	}
	
	for ( int loop = 0; loop <= 1; loop++ )
	{
		for ( ; slot >= 0; slot-- )
		{
			for ( ; pos >= 0; pos-- )
			{
				WEAPON *wsp = gWR.GetWeaponSlot( slot, pos );

				if ( wsp && gWR.HasAmmo(wsp) )
				{
					gpActiveSel = wsp;
					return;
				}
			}

			pos = MAX_WEAPON_POSITIONS-1;
		}
		
		slot = MAX_WEAPON_SLOTS-1;
	}

	gpActiveSel = NULL;
}



//-------------------------------------------------------------------------
// Drawing code
//-------------------------------------------------------------------------

int CHudAmmo::Draw(float flTime)
{
	int a, x, y, r, g, b;
	int AmmoWidth;

	if (gpActiveSel)
		DrawInventory(flTime);

	// Draw Weapon Menu
	DrawWList(flTime);

	if (gHUD.m_fAlphaSuit == TRUE)
	{
		// darkkrysteq: 0.52 HUD

		int hud_ammo = gHUD.GetSpriteIndex("alpha_ammo");
		int noweapon = gHUD.GetSpriteIndex("alpha_nowep");

		// Draw Weapon Menu

		WEAPON* w = m_pWeapon;

		SPR_Set(gHUD.GetSprite(hud_ammo), 255, 255, 255);
		SPR_DrawHoles(0, ScreenWidth - 280, ScreenHeight - 327, &gHUD.GetSpriteRect(hud_ammo));

		if (!m_pWeapon || w->iAmmoType == NULL && w->iAmmo2Type == NULL && w->iClip == NULL) 
		{
			return 1;
		}

		FillRGBA(ScreenWidth - 44, ScreenHeight - 61, 10, gWR.CountAmmo(w->iAmmoType) * -244 / w->iMax1 * 2, 200, 0, 255, 255); // primarys

		FillRGBA(ScreenWidth - 55, ScreenHeight - 61, 10, w->iClip * -244 / w->iMaxClip * 2, 0, 255, 0, 255); // clip 

		// p1llowguy - game crashes here (Integer division by zero. (????))
		FillRGBA(ScreenWidth - 33, ScreenHeight - 61, 10, gWR.CountAmmo(w->iAmmo2Type) * -244 / w->iMax2 * 2, 255, 0, 0, 255); // secondarys

	}
	else
	{
		int iFlags = DHN_DRAWZERO; // draw 0 values

		UnpackRGB(r, g, b, RGB_GREENISH);

		a = (int)max(MIN_ALPHA_B, m_fFade);

		if (m_fFade > 0)
			m_fFade -= (gHUD.m_flTimeDelta * 45);

		ScaleColors(r, g, b, a);

		x = ScreenWidth - 128;//512;
		y = ScreenHeight - 56;

		int iWidth = gHUD.GetSpriteRect(gHUD.m_HUD_snumber_0).right - gHUD.GetSpriteRect(gHUD.m_HUD_snumber_0).left;

		if (!(gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT))))
		{
			x += iWidth * 4;
			x = gHUD.DrawSHudNumber(x, y, DHN_DRAWZERO | DHN_3DIGITS, 255, r, g, b);
			return 1;
		}

		WEAPON* pw = m_pWeapon; // shorthand

		if (pw != NULL)
		{
			if (m_pWeapon->iAmmoType > 0)
			{
				if (pw->iClip >= 0)
				{
					x = gHUD.DrawSHudNumber(x, y, iFlags | DHN_3DIGITS, pw->iClip, r, g, b);

					SPR_Set(gHUD.GetSprite(gHUD.GetSpriteIndex("sslash")), r, g, b);
					SPR_DrawAdditiveHud(0, x, y, &gHUD.GetSpriteRect(gHUD.GetSpriteIndex("sslash")));

					x += iWidth;
					x = gHUD.DrawSHudNumber(x, y, iFlags | DHN_3DIGITS, gWR.CountAmmo(pw->iAmmoType), r, g, b);
				}
				else
				{
					x = ScreenWidth - 79;//561;
					y = ScreenHeight - 29;
					x = gHUD.DrawSHudNumber(x, y, iFlags | DHN_3DIGITS, gWR.CountAmmo(pw->iAmmoType), r, g, b);
				}
			}
			else //for crowbar
			{
				x += iWidth * 4;
				x = gHUD.DrawSHudNumber(x, y, iFlags | DHN_3DIGITS, 255, r, g, b);
			}
			if (pw->iAmmo2Type > 0)
			{
				x = ScreenWidth - 79;//561;
				y = ScreenHeight - 29;
				x = gHUD.DrawSHudNumber(x, y, iFlags | DHN_3DIGITS, gWR.CountAmmo(pw->iAmmo2Type), r, g, b);
			}
		}
		else
		{
			x += iWidth * 4;
			x = gHUD.DrawSHudNumber(x, y, iFlags | DHN_3DIGITS, 255, r, g, b);
		}
	}	
	return 1;
}


//
// Draws the ammo bar on the hud
//
int DrawBar(int x, int y, int width, int height, float f)
{
	int r, g, b;

	if (f < 0)
		f = 0;
	if (f > 1)
		f = 1;

	if (f)
	{
		int w = f * width;

		// Always show at least one pixel if we have ammo.
		if (w <= 0)
			w = 1;
		UnpackRGB(r, g, b, RGB_GREENISH);
		FillRGBA(x, y, w, height, r, g, b, 255);
		x += w;
		width -= w;
	}

	UnpackRGB(r, g, b, RGB_YELLOWISH);

	FillRGBA(x, y, width, height, r, g, b, 128);

	return (x + width);
}



void DrawAmmoBar(WEAPON *p, int x, int y, int width, int height)
{
	if ( !p )
		return;
	
	if (p->iAmmoType != -1)
	{
		if (!gWR.CountAmmo(p->iAmmoType))
			return;

		float f = (float)gWR.CountAmmo(p->iAmmoType)/(float)p->iMax1;
		
		x = DrawBar(x, y, width, height, f);


		// Do we have secondary ammo too?

		if (p->iAmmo2Type != -1)
		{
			f = (float)gWR.CountAmmo(p->iAmmo2Type)/(float)p->iMax2;

			x += 5; //!!!

			DrawBar(x, y, width, height, f);
		}
	}
}



int WeaponsResource::HasWeapon(int slot)
{
	int i;
	WEAPON* p;

	for (i = 0; i < 10; i++)
	{
		if (i == 9 && !p)
		{
			return FALSE;
		}
		p = gWR.GetWeaponSlot(slot, i);

		if (p)
		{
			slotstart = i;
			return TRUE;
			break;
		}
	}

	// jay - return FALSE anyways to prevent warning
	return FALSE;
}

/****
*
* WHATS IN EACH SLOT?
*
****/
#define SLOT0 0 // Hands
#define SLOT1 1 // Crowbar
#define SLOT2 2 // Knife
#define SLOT3 3 // Axe
#define SLOT4 4 // Glock
#define SLOT5 5 // 357
#define SLOT6 6 // Shotgun
#define SLOT7 7 // MP5
#define SLOT8 8 //  RPG
#define SLOT9 9 // Flamethrower


//
// Draw Weapon Menu
//
int CHudAmmo::DrawWList(float flTime)
{
	int x, x2, y, i, r, g, b, a;

	if (!gpActiveSel)
		return 0;

		static wrect_t nullrc;
		SetCrosshair(0, nullrc, 0, 0, 0);

		int iActiveSlot;

		if (gpActiveSel == (WEAPON*)1)
			iActiveSlot = -1;	// current slot has no weapons
		else
			iActiveSlot = gpActiveSel->iSlot;

		//wasn't here before...
		if (iActiveSlot > 0)
		{
			if (!gWR.GetFirstPos(iActiveSlot, 0))
			{
				gpActiveSel = (WEAPON*)1;
				iActiveSlot = -1;
			}
		}

		int boxWidth = 32;
		int boxHeight = 32;
		int space = 8;

		y = gHUD.m_scrinfo.iHeight * 0.5; //!!!
		y -= boxHeight * 0.5;

		x = x2 = gHUD.m_scrinfo.iWidth * 0.5; //!!!

		int i_UseSlots = 0;
		for (i = 0; i < MAX_WEAPON_SLOTS; i++)
		{
			WEAPON* p = gWR.GetFirstPos(i, 0);

			p = gWR.GetWeaponSlot(i, 0);

			if (p)
				i_UseSlots += 1;
		}
		//gEngfuncs.Con_Printf("Usable Slots: %i\n", i_UseSlots);



		for (i = 0; i < MAX_WEAPON_SLOTS; i++)
		{
			//y = giBucketHeight + 10;

			if (i == iActiveSlot)
			{
				WEAPON* p = gWR.GetFirstPos(i, 0);

				for (int iPos = 0; iPos < MAX_WEAPON_POSITIONS; iPos++)
				{
					x2 = x = gHUD.m_scrinfo.iWidth * 0.5; //!!!
					x -= boxWidth * 0.5;
					x2 -= boxWidth * 0.5;


					p = gWR.GetWeaponSlot(i, iPos);

					if (!p || !p->iId)
						continue;

					if (gWR.GetFirstPos(i, 0) == p, 0)
					{
						x = (gHUD.m_scrinfo.iWidth * 0.5) - (boxWidth * 0.5);
						y = (gHUD.m_scrinfo.iHeight * 0.5) - (boxHeight * 0.5);


					}

					if (gpActiveSel == p)
					{

						FillRGBA(x, y, 32, 32, 255, 200, 0, 100);
						SPR_Set(p->hActive, 255, 255, 255);
						SPR_DrawHoles(0, x, y, &p->rcActive);
					}
					else
					{

						// Draw Weapon if Red if no ammo

						FillRGBA(x2, y, 32, 32, 0, 160, 0, 100);

						SPR_Set(p->hInactive, 255, 255, 255);
						SPR_DrawHoles(0, x2, y, &p->rcInactive);
					}
					y += p->rcActive.bottom - p->rcActive.top + 5;

				}
			}
		}

		UnpackRGB(r, g, b, RGB_GREENISH);

		//LEFT SIDE
		int slotL1;
		for (i = 0; i < 1; i++)
		{
			slotL1 = iActiveSlot - 1;
			WEAPON* p;

		startL1:
			if (gWR.HasWeapon(slotL1))
			{
				p = gWR.GetWeaponSlot(slotL1, 0);

				//gEngfuncs.Con_Printf("valid slot: %i\n", slotL1);
			}
			else
			{
				if (slotL1 <= 0)
				{
					slotL1 = 9;
					goto startL1;
				}
				else
				{
					slotL1--;
					goto startL1;
				}
				p = gWR.GetWeaponSlot(slotL1, 0);
			}
			y = (gHUD.m_scrinfo.iHeight * 0.5) - (boxHeight * 0.5);
			for (int iPos = 0; iPos < MAX_WEAPON_POSITIONS; iPos++)
			{
				p = gWR.GetWeaponSlot(slotL1, iPos);

				if (!p || !p->iId)
					continue;
				if (gWR.GetFirstPos(slotL1, 1) == p)
				{
					x2 -= boxWidth + space;
				}

				FillRGBA(x2, y, 32, 32, r, g, b, 100);

				SPR_Set(p->hInactive, 255, 255, 255);
				SPR_DrawHoles(0, x2, y, &p->rcInactive);

				y -= p->rcActive.bottom - p->rcActive.top + 5;

			}
/*
			if (!p || !p->iId)
				continue;

			x2 -= boxWidth + space;
			y = (gHUD.m_scrinfo.iHeight * 0.5) - (boxHeight * 0.5);

			FillRGBA(x2, y, 32, 32, r, g, b, 100);

			SPR_Set(p->hInactive, 255, 255, 255);
			SPR_DrawHoles(0, x2, y, &p->rcInactive);*/
		}

		int slotL2;
		for (i = 0; i < 1; i++)
		{
			slotL2 = slotL1 - 1;
			WEAPON* p;

		startL2:
			if (gWR.HasWeapon(slotL2))
			{
				p = gWR.GetWeaponSlot(slotL2, 0);

				//gEngfuncs.Con_Printf("valid slot: %i\n", slotL2);
			}
			else
			{
				if (slotL2 <= 0)
				{
					slotL2 = 9;
					goto startL2;
				}
				else
				{
					slotL2--;
					goto startL2;
				}
				p = gWR.GetWeaponSlot(slotL2, 0);
			}
			y = (gHUD.m_scrinfo.iHeight * 0.5) - (boxHeight * 0.5);
			for (int iPos = 0; iPos < MAX_WEAPON_POSITIONS; iPos++)
			{
				p = gWR.GetWeaponSlot(slotL2, iPos);

				if (!p || !p->iId)
					continue;
				if (gWR.GetFirstPos(slotL2, 1) == p)
				{
					x2 -= boxWidth + space;
				}

				FillRGBA(x2, y, 32, 32, r, g, b, 100);

				SPR_Set(p->hInactive, 255, 255, 255);
				SPR_DrawHoles(0, x2, y, &p->rcInactive);

				y -= p->rcActive.bottom - p->rcActive.top + 5;

			}
/*
			if (!p || !p->iId)
				continue;

			x2 -= boxWidth + space;
			y = (gHUD.m_scrinfo.iHeight * 0.5) - (boxHeight * 0.5);

			FillRGBA(x2, y, 32, 32, r, g, b, 100);

			SPR_Set(p->hInactive, 255, 255, 255);
			SPR_DrawHoles(0, x2, y, &p->rcInactive);*/
		}

		//RIGHT SIDE
		int slotR1;
		for (i = 0; i < 1; i++)
		{
			slotR1 = iActiveSlot + 1;
			WEAPON* p;

		label:
			if (gWR.HasWeapon(slotR1))
			{
				p = gWR.GetWeaponSlot(slotR1, 0);

				//gEngfuncs.Con_Printf("valid slot: %i\n", slotR1);
			}
			else
			{
				if (slotR1 >= MAX_WEAPON_SLOTS)
				{
					slotR1 = 0;
					goto label;
				}
				else
				{
					slotR1++;
					goto label;
				}
				p = gWR.GetWeaponSlot(slotR1, 0);
			}

			y = (gHUD.m_scrinfo.iHeight * 0.5) - (boxHeight * 0.5);
			for (int iPos = 0; iPos < MAX_WEAPON_POSITIONS; iPos++)
			{
				p = gWR.GetWeaponSlot(slotR1, iPos);

				if (!p || !p->iId)
					continue;
				if (gWR.GetFirstPos(slotR1, 1) == p)
				{
					x += boxWidth + space;
				}

				FillRGBA(x, y, 32, 32, r, g, b, 100);

				SPR_Set(p->hInactive, 255, 255, 255);
				SPR_DrawHoles(0, x, y, &p->rcInactive);

				y -= p->rcActive.bottom - p->rcActive.top + 5;

			}
		}
	return 1;
}

int CHudAmmo::DrawInventory(float flTime) //magic nipples - INVENTORY
{
	int r, g, b, a, y, x;
	int apparatusIconWidth = 60;

	if (!gpActiveSel)
	{
		return 0;
	}

	a = 255;

	UnpackRGB(r, g, b, RGB_GREENISH);

	ScaleColors(r, g, b, a);

	x = ScreenWidth - m_iWidth - 15;


	// OXYGEN
	y = ((ScreenHeight / ScreenHeight) - (m_iHeight * .2));
	if (m_iOxygen == 1)
	{
		SPR_Set(m_hSprite8, r, g, b);
		SPR_DrawAdditive(0, x, y, m_prc2);
	}
	else
	{
		SPR_Set(m_hSprite3, r, g, b);
		SPR_DrawAdditive(0, x, y, m_prc2);
	}

	//p1llowguy - Make the airtank icow show how much oxygen is left in it.
	/*
	if (m_fOxygen < 15)
	{
		SPR_Set(m_hSprite3, r, g, b);
		SPR_DrawAdditive(0, x, y, m_prc2);
		if (m_fOxygen > 0)
		{
			int iOffset = 60 - (5 * m_fOxygen);
			wrect_t rc;
			rc = *m_prc2;
			rc.top += iOffset;
			//gEngfuncs.pfnConsolePrint( "Airtank offset\n" );
			SPR_Set(m_hSprite8, r, g, b);
			SPR_DrawAdditive(0, x, y + iOffset, &rc);
		}

		y += apparatusIconWidth + 8;
	}
	*/
	// LONGJUMP
	y = ((ScreenHeight / ScreenHeight) - (m_iHeight * 1.35));

	if ((m_iLongJumpBat < 5 && m_iLongJumpBat > -1))
	{
		SPR_Set(m_hSprite4, r, g, b);
		SPR_DrawAdditive(0, x, y, m_prc2);
		if (m_iLongJumpBat > 0)
		{
			int iOffset = 50 - (12 * m_iLongJumpBat);
			wrect_t rc;
			rc = *m_prc2;
			rc.top += iOffset;
			SPR_Set(m_hSprite9, r, g, b);
			SPR_DrawAdditive(0, x, y + iOffset, &rc);
		}

		y += apparatusIconWidth + 8;
	}
	/*
	//P1llowguy - for some reason, when the longjump battery is full, the icon disappears, so I just made this code
	*/
	else if (m_iLongJumpBat > -1)
	{
		SPR_Set(m_hSprite9, r, g, b);
		SPR_DrawAdditive(0, x, y, m_prc2);
	}
	else
	{
		SPR_Set(m_hSprite4, r, g, b);
		SPR_DrawAdditive(0, x, y, m_prc2);
	}

	// RADIATION
	y = ((ScreenHeight / ScreenHeight) - (m_iHeight * 2.48));
	if (m_iRadiation == 5)
	{
		SPR_Set(gHUD.GetSprite(gHUD.GetSpriteIndex("radiation_5")), r, g, b);
		SPR_DrawAdditive(0, x, y, m_prc2);
	}
	else if (m_iRadiation == 4)
	{
		SPR_Set(gHUD.GetSprite(gHUD.GetSpriteIndex("radiation_4")), r, g, b);
		SPR_DrawAdditive(0, x, y, m_prc2);
	}
	else if (m_iRadiation == 3)
	{
		SPR_Set(gHUD.GetSprite(gHUD.GetSpriteIndex("radiation_3")), r, g, b);
		SPR_DrawAdditive(0, x, y, m_prc2);
	}
	else if (m_iRadiation == 2)
	{
		SPR_Set(gHUD.GetSprite(gHUD.GetSpriteIndex("radiation_2")), r, g, b);
		SPR_DrawAdditive(0, x, y, m_prc2);
	}
	else if (m_iRadiation == 1)
	{
		SPR_Set(gHUD.GetSprite(gHUD.GetSpriteIndex("radiation_1")), r, g, b);
		SPR_DrawAdditive(0, x, y, m_prc2);
	}
	else
	{
		SPR_Set(gHUD.GetSprite(gHUD.GetSpriteIndex("radiation_0")), r, g, b);
		SPR_DrawAdditive(0, x, y, m_prc2);
	}

	// ANTIDOTE
	y = ((ScreenHeight / ScreenHeight) - (m_iHeight * 3.64));
	if (m_iAntidote == 5)
	{
		SPR_Set(gHUD.GetSprite(gHUD.GetSpriteIndex("antidote_5")), r, g, b);
		SPR_DrawAdditive(0, x, y, m_prc2);
	}
	else if (m_iAntidote == 4)
	{
		SPR_Set(gHUD.GetSprite(gHUD.GetSpriteIndex("antidote_4")), r, g, b);
		SPR_DrawAdditive(0, x, y, m_prc2);
	}
	else if (m_iAntidote == 3)
	{
		SPR_Set(gHUD.GetSprite(gHUD.GetSpriteIndex("antidote_3")), r, g, b);
		SPR_DrawAdditive(0, x, y, m_prc2);
	}
	else if (m_iAntidote == 2)
	{
		SPR_Set(gHUD.GetSprite(gHUD.GetSpriteIndex("antidote_2")), r, g, b);
		SPR_DrawAdditive(0, x, y, m_prc2);
	}
	else if (m_iAntidote == 1)
	{
		SPR_Set(gHUD.GetSprite(gHUD.GetSpriteIndex("antidote_1")), r, g, b);
		SPR_DrawAdditive(0, x, y, m_prc2);
	}
	else
	{
		SPR_Set(gHUD.GetSprite(gHUD.GetSpriteIndex("antidote_0")), r, g, b);
		SPR_DrawAdditive(0, x, y, m_prc2);
	}

	// ADRENALINE
	y = ((ScreenHeight / ScreenHeight) - (m_iHeight * 4.8));

	if (m_iAdrenaline == 1)
	{
		SPR_Set(gHUD.GetSprite(gHUD.GetSpriteIndex("adrenaline_on")), r, g, b);
		SPR_DrawAdditive(0, x, y, m_prc2);
	}
	else
	{
		SPR_Set(gHUD.GetSprite(gHUD.GetSpriteIndex("adrenaline_off")), r, g, b);
		SPR_DrawAdditive(0, x, y, m_prc2);
	}

	// FLASHLIGHT
	y = ((ScreenHeight / ScreenHeight) - (m_iHeight * 5.96));

	if (m_flashOn == 1)
	{
		SPR_Set(gHUD.GetSprite(gHUD.GetSpriteIndex("flash_on")), r, g, b);
		SPR_DrawAdditive(0, x, y, m_prc2);
	}
	else
	{
		SPR_Set(gHUD.GetSprite(gHUD.GetSpriteIndex("flash_off")), r, g, b);
		SPR_DrawAdditive(0, x, y, m_prc2);
	}
	return 1;
}

/* =================================
	GetSpriteList

Finds and returns the matching 
sprite name 'psz' and resolution 'iRes'
in the given sprite list 'pList'
iCount is the number of items in the pList
================================= */
client_sprite_t *GetSpriteList(client_sprite_t *pList, const char *psz, int iRes, int iCount)
{
	if (!pList)
		return NULL;

	int i = iCount;
	client_sprite_t *p = pList;

	while(i--)
	{
		if ((!strcmp(psz, p->szName)) && (p->iRes == iRes))
			return p;
		p++;
	}

	return NULL;
}
