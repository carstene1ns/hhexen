//**************************************************************************
//**
//** $Id: i_sdlgl.c,v 1.11 2008-06-17 17:51:22 sezero Exp $
//**
//**************************************************************************

#include "h2stdinc.h"
#include <unistd.h>
#include <sys/time.h>
#include "SDL.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "h2def.h"
#include "r_local.h"
#include "ogl_def.h"
#include "sounds.h"
#include "i_sound.h"
#include "soundst.h"
#include "st_start.h"

// Public Data

int screenWidth  = SCREENWIDTH*2;
int screenHeight = SCREENHEIGHT*2;
int maxTexSize = 256;
int ratioLimit = 0;	// Zero if none.
int test3dfx = 0;

int DisplayTicker = 0;


// Code
extern void **lumpcache;

extern int usemouse, usejoystick;

boolean useexterndriver;
boolean mousepresent;


boolean novideo;	// if true, stay in text mode for debugging

//--------------------------------------------------------------------------
//
// PROC I_WaitVBL
//
//--------------------------------------------------------------------------

void I_WaitVBL(int vbls)
{
	if (novideo)
	{
		return;
	}
	while (vbls--)
	{
		SDL_Delay (16667 / 1000);
	}
}

//--------------------------------------------------------------------------
//
// PROC I_SetPalette
//
// Palette source must use 8 bit RGB elements.
//
//--------------------------------------------------------------------------

void I_SetPalette(byte *palette)
{
}

/*
============================================================================

							GRAPHICS MODE

============================================================================
*/

/*
==============
=
= I_Update
=
==============
*/

int UpdateState;
extern int screenblocks;

void I_Update (void)
{
	if(UpdateState == I_NOUPDATE)
		return;

	SDL_GL_SwapBuffers();
	UpdateState = I_NOUPDATE;
}

//--------------------------------------------------------------------------
//
// PROC I_InitGraphics
//
//--------------------------------------------------------------------------

void I_InitGraphics(void)
{
	int p;
	char text[20];
	Uint32 flags = SDL_OPENGL;

	if (novideo)
		return;

	if (M_CheckParm("-f") || M_CheckParm("--fullscreen"))
	{
		flags |= SDL_FULLSCREEN;
		setenv ("MESA_GLX_FX", "fullscreen", 1);
	}
	else
	{
		setenv ("MESA_GLX_FX", "disable", 1);
	}
	p = M_CheckParm ("-height");
	if (p && p < myargc - 1)
	{
		screenHeight = atoi (myargv[p+1]);
	}
	p = M_CheckParm ("-width");
	if (p && p < myargc - 1)
	{
		screenWidth = atoi(myargv[p+1]);
	}
	ST_Message("Screen size:  %dx%d\n", screenWidth, screenHeight);

	if (SDL_SetVideoMode(screenWidth, screenHeight, 8, flags) == NULL)
	{
		fprintf(stderr, "Couldn't set video mode %dx%d: %s\n",
			screenWidth, screenHeight, SDL_GetError());
		exit (3);
	}

	OGL_InitRenderer ();

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Print some OpenGL information.
	ST_Message ("I_InitGraphics: OpenGL information:\n");
	ST_Message ("  Vendor: %s\n", glGetString(GL_VENDOR));
	ST_Message ("  Renderer: %s\n", glGetString(GL_RENDERER));
	ST_Message ("  Version: %s\n", glGetString(GL_VERSION));
	ST_Message ("  GLU Version: %s\n", gluGetString((GLenum)GLU_VERSION));

	// Check the maximum texture size.
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
	ST_Message ("  Maximum texture size: %d\n", maxTexSize);
	if (maxTexSize == 256)
	{
	//	ST_Message("  Is this Voodoo? Using size ratio limit.\n");
		ratioLimit = 8;
	}

	if (M_CheckParm("-3dfxtest"))
	{
		test3dfx = 1;
		ST_Message ("  3dfx test mode.\n");
	}

	// Only grab if we want to
	if (!M_CheckParm ("--nograb") && !M_CheckParm ("-g"))
	{
		SDL_WM_GrabInput (SDL_GRAB_ON);
	}

	SDL_ShowCursor (0);
	snprintf (text, 20, "HHexen v%s", HHEXEN_VERSION);
	SDL_WM_SetCaption (text, "HHEXEN");
}

//--------------------------------------------------------------------------
//
// PROC I_ShutdownGraphics
//
//--------------------------------------------------------------------------

void I_ShutdownGraphics(void)
{
	OGL_ResetData ();
	OGL_ResetLumpTexData ();
	SDL_Quit ();
}

//===========================================================================

//
//  Translates the key
//
static int xlatekey (SDL_keysym *key)
{
	switch (key->sym)
	{
	// S.A.
	case SDLK_LEFTBRACKET:	return KEY_LEFTBRACKET;
	case SDLK_RIGHTBRACKET:	return KEY_RIGHTBRACKET;
	case SDLK_BACKQUOTE:	return KEY_BACKQUOTE;
	case SDLK_QUOTEDBL:	return KEY_QUOTEDBL;
	case SDLK_QUOTE:	return KEY_QUOTE;
	case SDLK_SEMICOLON:	return KEY_SEMICOLON;
	case SDLK_PERIOD:	return KEY_PERIOD;
	case SDLK_COMMA:	return KEY_COMMA;
	case SDLK_SLASH:	return KEY_SLASH;
	case SDLK_BACKSLASH:	return KEY_BACKSLASH;

	case SDLK_LEFT:		return KEY_LEFTARROW;
	case SDLK_RIGHT:	return KEY_RIGHTARROW;
	case SDLK_DOWN:		return KEY_DOWNARROW;
	case SDLK_UP:		return KEY_UPARROW;
	case SDLK_ESCAPE:	return KEY_ESCAPE;
	case SDLK_RETURN:	return KEY_ENTER;

	case SDLK_F1:		return KEY_F1;
	case SDLK_F2:		return KEY_F2;
	case SDLK_F3:		return KEY_F3;
	case SDLK_F4:		return KEY_F4;
	case SDLK_F5:		return KEY_F5;
	case SDLK_F6:		return KEY_F6;
	case SDLK_F7:		return KEY_F7;
	case SDLK_F8:		return KEY_F8;
	case SDLK_F9:		return KEY_F9;
	case SDLK_F10:		return KEY_F10;
	case SDLK_F11:		return KEY_F11;
	case SDLK_F12:		return KEY_F12;

	case SDLK_INSERT:	return KEY_INS;
	case SDLK_DELETE:	return KEY_DEL;
	case SDLK_PAGEUP:	return KEY_PGUP;
	case SDLK_PAGEDOWN:	return KEY_PGDN;
	case SDLK_HOME:		return KEY_HOME;
	case SDLK_END:		return KEY_END;
	case SDLK_BACKSPACE:	return KEY_BACKSPACE;
	case SDLK_PAUSE:	return KEY_PAUSE;
	case SDLK_EQUALS:	return KEY_EQUALS;
	case SDLK_MINUS:	return KEY_MINUS;

	case SDLK_LSHIFT:
	case SDLK_RSHIFT:
		return KEY_RSHIFT;

	case SDLK_LCTRL:
	case SDLK_RCTRL:
		return KEY_RCTRL;

	case SDLK_LALT:
	case SDLK_LMETA:
	case SDLK_RALT:
	case SDLK_RMETA:
		return KEY_RALT;

	case SDLK_KP0:
		if (key->mod & KMOD_NUM)
			return SDLK_0;
		else
			return KEY_INS;
	case SDLK_KP1:
		if (key->mod & KMOD_NUM)
			return SDLK_1;
		else
			return KEY_END;
	case SDLK_KP2:
		if (key->mod & KMOD_NUM)
			return SDLK_2;
		else
			return KEY_DOWNARROW;
	case SDLK_KP3:
		if (key->mod & KMOD_NUM)
			return SDLK_3;
		else
			return KEY_PGDN;
	case SDLK_KP4:
		if (key->mod & KMOD_NUM)
			return SDLK_4;
		else
			return KEY_LEFTARROW;
	case SDLK_KP5:
		return SDLK_5;
	case SDLK_KP6:
		if (key->mod & KMOD_NUM)
			return SDLK_6;
		else
			return KEY_RIGHTARROW;
	case SDLK_KP7:
		if (key->mod & KMOD_NUM)
			return SDLK_7;
		else
			return KEY_HOME;
	case SDLK_KP8:
		if (key->mod & KMOD_NUM)
			return SDLK_8;
		else
			return KEY_UPARROW;
	case SDLK_KP9:
		if (key->mod & KMOD_NUM)
			return SDLK_9;
		else
			return KEY_PGUP;

	case SDLK_KP_PERIOD:
		if (key->mod & KMOD_NUM)
			return SDLK_PERIOD;
		else
			return KEY_DEL;
	case SDLK_KP_DIVIDE:	return SDLK_SLASH;
	case SDLK_KP_MULTIPLY:	return SDLK_ASTERISK;
	case SDLK_KP_MINUS:	return KEY_MINUS;
	case SDLK_KP_PLUS:	return SDLK_PLUS;
	case SDLK_KP_ENTER:	return KEY_ENTER;
	case SDLK_KP_EQUALS:	return KEY_EQUALS;

	default:
		return key->sym;
	}
}

/* This processes SDL events */
void I_GetEvent(SDL_Event *Event)
{
	Uint8 buttonstate;
	event_t event;
	SDLMod mod;

	switch (Event->type)
	{
	case SDL_KEYDOWN:
		mod = SDL_GetModState ();
		if (mod & (KMOD_RCTRL|KMOD_LCTRL))
		{
			if (Event->key.keysym.sym == 'g')
			{
				if (SDL_WM_GrabInput (SDL_GRAB_QUERY) == SDL_GRAB_OFF)
					SDL_WM_GrabInput (SDL_GRAB_ON);
				else
					SDL_WM_GrabInput (SDL_GRAB_OFF);
				break;
			}
		}
		else if (mod & (KMOD_RALT|KMOD_LALT))
		{
			if (Event->key.keysym.sym == SDLK_RETURN)
			{
				SDL_WM_ToggleFullScreen(SDL_GetVideoSurface());
				break;
			}
		}
		event.type = ev_keydown;
		event.data1 = xlatekey(&Event->key.keysym);
		H2_PostEvent(&event);
		break;

	case SDL_KEYUP:
		event.type = ev_keyup;
		event.data1 = xlatekey(&Event->key.keysym);
		H2_PostEvent(&event);
		break;

	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		buttonstate = SDL_GetMouseState(NULL, NULL);
		event.type = ev_mouse;
		event.data1 = 0	| (buttonstate & SDL_BUTTON(1) ? 1 : 0)
				| (buttonstate & SDL_BUTTON(2) ? 2 : 0)
				| (buttonstate & SDL_BUTTON(3) ? 4 : 0);
		event.data2 = event.data3 = 0;
		H2_PostEvent(&event);
		break;

	case SDL_MOUSEMOTION:
		/* Ignore mouse warp events */
		if ((Event->motion.x != SCREENWIDTH/2) ||
		    (Event->motion.y != SCREENHEIGHT/2) )
		{
		/* Warp the mouse back to the center */
			event.type = ev_mouse;
			event.data1 = 0	| (Event->motion.state & SDL_BUTTON(1) ? 1 : 0)
					| (Event->motion.state & SDL_BUTTON(2) ? 2 : 0)
					| (Event->motion.state & SDL_BUTTON(3) ? 4 : 0);
			event.data2 = Event->motion.xrel << 3;
			event.data3 = -Event->motion.yrel << 3;
			H2_PostEvent(&event);
		}
		break;

	case SDL_QUIT:
		I_Quit();
	}
}

//
// I_StartTic
//
void I_StartTic (void)
{
	SDL_Event Event;
	while ( SDL_PollEvent(&Event) )
		I_GetEvent(&Event);
}


/*
============================================================================

					TIMER INTERRUPT

============================================================================
*/


/*
================
=
= I_TimerISR
=
================
*/

int I_TimerISR (void)
{
	ticcount++;
	return 0;
}

/*
============================================================================

							MOUSE

============================================================================
*/

/*
================
=
= StartupMouse
=
================
*/

void I_StartupMouse (void)
{
	mousepresent = 1;
}

