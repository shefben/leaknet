//====== Copyright � 1996-2003, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef CLIENT_TEXTMESSAGE_H
#define CLIENT_TEXTMESSAGE_H
#ifdef _WIN32
#pragma once
#endif

struct client_textmessage_t
{
	int		effect;
	byte	r1, g1, b1, a1;		// 2 colors for effects
	byte	r2, g2, b2, a2;
	float	x;
	float	y;
	float	fadein;
	float	fadeout;
	float	holdtime;
	float	fxtime;
	const char *pVGuiSchemeFontName; // If null, use default font for messages
	const char *pName;
	const char *pMessage;
	bool    bRoundedRectBackdropBox;	// Source 2007+ - rounded rect backdrop
	float	flBoxSize;					// as a function of font height
	byte	boxcolor[4];				// RGBA color for backdrop box
	char const *pClearMessage;			// message to clear when this one appears
};

#endif // CLIENT_TEXTMESSAGE_H
