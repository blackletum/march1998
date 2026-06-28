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
// battery.cpp
//
// implementation of CHudBattery class
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>

DECLARE_MESSAGE(m_Battery, Battery)

int CHudBattery::Init(void)
{
	m_iBat = 0;
	m_fFade = 0;
	m_iFlags = 0;

	HOOK_MESSAGE(Battery);

	gHUD.AddHudElem(this);

	return 1;
};


int CHudBattery::VidInit(void)
{
	int HUD_suit_empty = gHUD.GetSpriteIndex( "suit_empty" );
	int HUD_suit_full = gHUD.GetSpriteIndex( "suit_full" );

	m_HLSPRITE1 = m_HLSPRITE2 = 0;  // delaying get sprite handles until we know the sprites are loaded
	m_prc1 = &gHUD.GetSpriteRect( HUD_suit_empty );
	m_prc2 = &gHUD.GetSpriteRect( HUD_suit_full );
	m_iHeight = m_prc2->bottom - m_prc1->top;
	m_fFade = 0;
	return 1;
};

int CHudBattery:: MsgFunc_Battery(const char *pszName,  int iSize, void *pbuf )
{
	m_iFlags |= HUD_ACTIVE;

	
	BEGIN_READ( pbuf, iSize );
	int x = READ_SHORT();

	if (x != m_iBat)
	{
		m_fFade = FADE_TIME;
		m_iBat = x;
	}

	return 1;
}


int CHudBattery::Draw(float flTime)
{
	int r, g, b, x, y, a;
	wrect_t zrc;

	if (!(gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT))))
		return 1;

	if (gHUD.m_bAlphaSuit == TRUE)
	{
		int hud_bat = gHUD.GetSpriteIndex("alpha_bat");

		wrect_t alrc = gHUD.GetSpriteRect(gHUD.GetSpriteIndex("alpha_batbar"));
		alrc.right = 20; //because of how wrects work you need to start with the offset from hud.txt
		alrc.right += m_iBat * 240 / 100;

		SPR_Set(gHUD.GetSprite(gHUD.GetSpriteIndex("alpha_batbar")), 255, 255, 255);
		SPR_DrawHoles(0, ScreenWidth - 262, ScreenHeight - 55, &alrc);

		gHUD.DrawSHudNumber(ScreenWidth - 86, ScreenHeight - 42, DHN_3DIGITS | DHN_DRAWZERO, m_iBat, 15, 111, 231);
	}
	else
	{
		gHUD.m_Health.GetPainColor(r, g, b);

		// Has health changed? Flash the health #
		if (m_fFade)
		{
			if (m_fFade > FADE_TIME)
				m_fFade = FADE_TIME;

			m_fFade -= (gHUD.m_flTimeDelta * 20);
			if (m_fFade <= 0)
			{
				a = 128;
				m_fFade = 0;
			}

			// Fade the health number back to dim
			a = MIN_ALPHA_A + (m_fFade / FADE_TIME) * 200;
		}
		else
			a = MIN_ALPHA_A;



		ScaleColors(r, g, b, a);

		x = 15;
		y = ScreenHeight - 72 - 26;

		SPR_Set(gHUD.GetSprite(gHUD.GetSpriteIndex("battery")), r, g, b);
		SPR_DrawAdditiveHud(0, x, y, &gHUD.GetSpriteRect(gHUD.GetSpriteIndex("battery")));


		x += 4;
		y += 5;

		wrect_t rc = gHUD.GetSpriteRect(gHUD.GetSpriteIndex("powerz"));
		rc.right = 87; //because of how wrects work you need to start with the offset from hud.txt
		rc.right += m_iBat * 54 / 100;

		SPR_Set(gHUD.GetSprite(gHUD.GetSpriteIndex("powerz")), r, g, b);
		SPR_DrawAdditiveHud(0, x, y, &rc);
	}
	return 1;
}