/*
	This file is part of Warzone 2100.
	Copyright (C) 1999-2004  Eidos Interactive
	Copyright (C) 2005-2007  Warzone Resurrection Project

	Warzone 2100 is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	Warzone 2100 is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Warzone 2100; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/
/***************************************************************************/
/*
 * pieState.c
 *
 * renderer setup and state control routines for 3D rendering
 *
 */
/***************************************************************************/

#include "lib/framework/frame.h"

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include "lib/ivis_common/piestate.h"
#include "lib/ivis_common/piedef.h"
#include "lib/ivis_common/tex.h"

/***************************************************************************/
/*
 *	Global Variables
 */
/***************************************************************************/

extern RENDER_STATE	rendStates;

/***************************************************************************/
/*
 *	Source
 */
/***************************************************************************/

void pie_SetDepthBufferStatus(DEPTH_MODE depthMode) {
	switch(depthMode) {
		case DEPTH_CMP_LEQ_WRT_ON:
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
			glDepthMask(GL_TRUE);
			break;
		case DEPTH_CMP_ALWAYS_WRT_ON:
			glDisable(GL_DEPTH_TEST);
			glDepthMask(GL_TRUE);
			break;
		case DEPTH_CMP_LEQ_WRT_OFF:
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
			glDepthMask(GL_FALSE);
			break;
		case DEPTH_CMP_ALWAYS_WRT_OFF:
			glDisable(GL_DEPTH_TEST);
			glDepthMask(GL_FALSE);
			break;
	}
}

/// Set the depth (z) offset
/// Negative values are closer to the screen
void pie_SetDepthOffset(float offset)
{
	if(offset == 0.0f)
	{
		glDisable (GL_POLYGON_OFFSET_FILL);
	}
	else
	{
		glPolygonOffset(offset, offset);
		glEnable (GL_POLYGON_OFFSET_FILL);
	}
}

/// Set the OpenGL fog start and end
void pie_UpdateFogDistance(float begin, float end)
{
	glFogf(GL_FOG_START, begin);
	glFogf(GL_FOG_END, end);
}

//***************************************************************************
//
// pie_SetFogStatus(BOOL val)
//
// Toggle fog on and off for rendering objects inside or outside the 3D world
//
//***************************************************************************

void pie_SetFogStatus(BOOL val)
{
	PIELIGHT fog;
	float fog_colour[4];

	if (rendStates.fogEnabled)
	{
		//fog enabled so toggle if required
		if (rendStates.fog != val)
		{
			rendStates.fog = val;
			if (rendStates.fog) {
				fog.argb = pie_GetFogColour();
				fog_colour[0] = fog.byte.r/255.0f;
				fog_colour[1] = fog.byte.g/255.0f;
				fog_colour[2] = fog.byte.b/255.0f;
				fog_colour[3] = fog.byte.a/255.0f;

				glFogi(GL_FOG_MODE, GL_LINEAR);
				glFogfv(GL_FOG_COLOR, fog_colour);
				glFogf(GL_FOG_DENSITY, 0.35f);
				glHint(GL_FOG_HINT, GL_DONT_CARE);
				glEnable(GL_FOG);
			} else {
				glDisable(GL_FOG);
			}
		}
	}
	else
	{
		//fog disabled so turn it off if not off already
		if (rendStates.fog != FALSE)
		{
			rendStates.fog = FALSE;
		}
	}
}

/***************************************************************************/
void pie_SetTexturePage(SDWORD num)
{
	if (num != rendStates.texPage) {
		rendStates.texPage = num;
		if (num < 0) {
			glDisable(GL_TEXTURE_2D);
		} else {
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, _TEX_PAGE[num].textPage3dfx);
		}
	}
}

/***************************************************************************/

void pie_SetColourKeyedBlack(BOOL keyingOn)
{
	if (keyingOn != rendStates.keyingOn)
	{
		rendStates.keyingOn = keyingOn;
		pieStateCount++;

		if (keyingOn == TRUE) {
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0.1f);
		} else {
			glDisable(GL_ALPHA_TEST);
		}
	}
}

/***************************************************************************/
void pie_SetColourCombine(COLOUR_MODE colCombMode)
{
	if (colCombMode != rendStates.colourCombine) {
		rendStates.colourCombine = colCombMode;
		pieStateCount++;
		switch (colCombMode) {
			case COLOUR_FLAT_CONSTANT:
			case COLOUR_FLAT_ITERATED:
				pie_SetTexturePage(-1);
				break;
			case COLOUR_TEX_CONSTANT:
			case COLOUR_TEX_ITERATED:
			default:
				break;
		}
	}
}

/***************************************************************************/
void pie_SetTranslucencyMode(TRANSLUCENCY_MODE transMode)
{
	if (transMode != rendStates.transMode) {
		rendStates.transMode = transMode;
		switch (transMode) {
			case TRANS_ALPHA:
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case TRANS_ADDITIVE:
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				break;
			case TRANS_FILTER:
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_SRC_COLOR);
				break;
			default:
				rendStates.transMode = TRANS_DECAL;
				glDisable(GL_BLEND);
				break;
		}
	}
}

/***************************************************************************/
// set the constant colour used in text and flat render modes
/***************************************************************************/
void pie_SetColour(UDWORD colour)
{
	if (colour != rendStates.colour) {
		PIELIGHT c;

		rendStates.colour = colour;
		pieStateCount++;

		c.argb = colour;
		glColor4ub(c.byte.r, c.byte.g, c.byte.b, c.byte.a);
	}
}

/***************************************************************************/
void pie_SetGammaValue(float val)
{
	debug(LOG_VIDEO, "%s(%f)", __FUNCTION__, val);
	SDL_SetGamma(val, val, val);
}
