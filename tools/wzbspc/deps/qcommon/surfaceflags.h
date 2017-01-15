/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// This file must be identical in the quake and utils directories

// contents flags are seperate bits
// a given brush can contribute multiple content bits

// these definitions also need to be in q_shared.h!
#define __JA__

#ifndef __JA__
#define	CONTENTS_SOLID			1		// an eye is never valid in a solid
#define	CONTENTS_LAVA			8
#define	CONTENTS_SLIME			16
#define	CONTENTS_WATER			32
#define	CONTENTS_FOG			64

#define CONTENTS_NOTTEAM1		0x0080
#define CONTENTS_NOTTEAM2		0x0100
#define CONTENTS_NOBOTCLIP		0x0200

#define	CONTENTS_AREAPORTAL		0x8000

#define	CONTENTS_PLAYERCLIP		0x10000
#define	CONTENTS_MONSTERCLIP	0x20000
//bot specific contents types
#define	CONTENTS_TELEPORTER		0x40000
#define	CONTENTS_JUMPPAD		0x80000
#define CONTENTS_CLUSTERPORTAL	0x100000
#define CONTENTS_DONOTENTER		0x200000
#define CONTENTS_BOTCLIP		0x400000
#define CONTENTS_MOVER			0x800000

#define	CONTENTS_ORIGIN			0x1000000	// removed before bsping an entity

#define	CONTENTS_BODY			0x2000000	// should never be on a brush, only in game
#define	CONTENTS_CORPSE			0x4000000
#define	CONTENTS_DETAIL			0x8000000	// brushes not used for the bsp
#define	CONTENTS_STRUCTURAL		0x10000000	// brushes used for the bsp
#define	CONTENTS_TRANSLUCENT	0x20000000	// don't consume surface fragments inside
#define	CONTENTS_TRIGGER		0x40000000
#define	CONTENTS_NODROP			0x80000000	// don't leave bodies or items (death fog, lava)

#define	SURF_NODAMAGE			0x1		// never give falling damage
#define	SURF_SLICK				0x2		// effects game physics
#define	SURF_SKY				0x4		// lighting from environment map
#define	SURF_LADDER				0x8
#define	SURF_NOIMPACT			0x10	// don't make missile explosions
#define	SURF_NOMARKS			0x20	// don't leave missile marks
#define	SURF_FLESH				0x40	// make flesh sounds and effects
#define	SURF_NODRAW				0x80	// don't generate a drawsurface at all
#define	SURF_HINT				0x100	// make a primary bsp splitter
#define	SURF_SKIP				0x200	// completely ignore, allowing non-closed brushes
#define	SURF_NOLIGHTMAP			0x400	// surface doesn't need a lightmap
#define	SURF_POINTLIGHT			0x800	// generate lighting info at vertexes
#define	SURF_METALSTEPS			0x1000	// clanking footsteps
#define	SURF_NOSTEPS			0x2000	// no footstep sounds
#define	SURF_NONSOLID			0x4000	// don't collide against curves with this set
#define	SURF_LIGHTFILTER		0x8000	// act as a light filter during q3map -light
#define	SURF_ALPHASHADOW		0x10000	// do per-pixel light shadow casting in q3map
#define	SURF_NODLIGHT			0x20000	// don't dlight even if solid (solid lava, skies)
#define SURF_DUST				0x40000 // leave a dust trail when walking on this surface
#else //__JA__
#define CONTENTS_NONE			(0x00000000u)
#define	CONTENTS_SOLID			(0x00000001u) // Default setting. An eye is never valid in a solid
#define	CONTENTS_LAVA			(0x00000002u) //
#define	CONTENTS_WATER			(0x00000004u) //
#define	CONTENTS_FOG			(0x00000008u) //

#define	LAST_VISIBLE_CONTENTS	CONTENTS_FOG

#define	CONTENTS_PLAYERCLIP		(0x00000010u) //
#define	CONTENTS_MONSTERCLIP	(0x00000020u) // Physically block bots
#define CONTENTS_BOTCLIP		(0x00000040u) // A hint for bots - do not enter this brush by navigation (if possible)
#define CONTENTS_SHOTCLIP		(0x00000080u) //

#define CONTENTS_NOTTEAM1		CONTENTS_PLAYERCLIP
#define CONTENTS_NOTTEAM2		CONTENTS_PLAYERCLIP
#define CONTENTS_DONOTENTER		CONTENTS_PLAYERCLIP

#define	CONTENTS_BODY			(0x00000100u) // should never be on a brush, only in game
#define	CONTENTS_CORPSE			(0x00000200u) // should never be on a brush, only in game
#define	CONTENTS_MONSTER		CONTENTS_BODY	// should never be on a brush, only in game
#define	CONTENTS_DEADMONSTER	CONTENTS_CORPSE
#define	CONTENTS_ORIGIN			CONTENTS_CORPSE
#define	CONTENTS_WINDOW			CONTENTS_CORPSE
#define	CONTENTS_AUX			CONTENTS_CORPSE
#define	CONTENTS_MIST			CONTENTS_CORPSE
#define	CONTENTS_JUMPPAD		CONTENTS_CORPSE
#define CONTENTS_CLUSTERPORTAL	CONTENTS_CORPSE
#define CONTENTS_MOVER			CONTENTS_CORPSE
#define CONTENTS_NOBOTCLIP		CONTENTS_CORPSE

#define	CONTENTS_TRIGGER		(0x00000400u) //
#define	CONTENTS_NODROP			(0x00000800u) // don't leave bodies or items (death fog, lava)
#define CONTENTS_TERRAIN		(0x00001000u) // volume contains terrain data
#define CONTENTS_LADDER			(0x00002000u) //
#define CONTENTS_ABSEIL			(0x00004000u) // (SOF2) used like ladder to define where an NPC can abseil
#define CONTENTS_OPAQUE			(0x00008000u) // defaults to on, when off, solid can be seen through
#define CONTENTS_OUTSIDE		(0x00010000u) // volume is considered to be in the outside (i.e. not indoors)
#define CONTENTS_SLIME			(0x00020000u) // CHC needs this since we use same tools
#define CONTENTS_LIGHTSABER		(0x00040000u) // ""
#define CONTENTS_TELEPORTER		(0x00080000u) // ""
#define CONTENTS_ITEM			(0x00100000u) // ""
#define CONTENTS_NOSHOT			(0x00200000u) // shots pass through me
#define CONTENTS_AREAPORTAL/*CONTENTS_UNUSED00400000*/	(0x00400000u) //
#define CONTENTS_CURRENT_0/*CONTENTS_UNUSED00800000*/	(0x00800000u) //
#define CONTENTS_CURRENT_90/*CONTENTS_UNUSED01000000*/	(0x01000000u) //
#define CONTENTS_CURRENT_180/*CONTENTS_UNUSED02000000*/	(0x02000000u) //
#define CONTENTS_CURRENT_270/*CONTENTS_UNUSED04000000*/	(0x04000000u) //
#define	CONTENTS_DETAIL			(0x08000000u) // brushes not used for the bsp
#define	CONTENTS_INSIDE			(0x10000000u) // volume is considered to be inside (i.e. indoors)
#define CONTENTS_CURRENT_UP/*CONTENTS_UNUSED20000000*/	(0x20000000u) //
#define CONTENTS_CURRENT_DOWN/*CONTENTS_UNUSED40000000*/	(0x40000000u) //
#define	CONTENTS_TRANSLUCENT	(0x80000000u) // don't consume surface fragments inside
#define	CONTENTS_Q2TRANSLUCENT	CONTENTS_TRANSLUCENT
#define	CONTENTS_STRUCTURAL		CONTENTS_INSIDE//0x10000000	// brushes used for the bsp

#define CONTENTS_ALL			(0xFFFFFFFFu)


#define SURF_NONE				(0x00000000u)
#define SURF_LIGHT/*SURF_UNUSED00000001*/		(0x00000001u) //
#define SURF_WARP/*SURF_UNUSED00000002*/		(0x00000002u) //
#define SURF_TRANS33/*SURF_UNUSED00000004*/		(0x00000004u) //
#define SURF_TRANS66/*SURF_UNUSED00000008*/		(0x00000008u) //
#define SURF_FLOWING/*SURF_UNUSED00000010*/		(0x00000010u) //
#define SURF_HINT/*SURF_UNUSED00000020*/		(0x00000020u) //
#define SURF_UNUSED00000040		(0x00000040u) //
#define SURF_UNUSED00000080		(0x00000080u) //
#define SURF_UNUSED00000100		(0x00000100u) //
#define SURF_UNUSED00000200		(0x00000200u) //
#define SURF_UNUSED00000400		(0x00000400u) //
#define SURF_UNUSED00000800		(0x00000800u) //
#define SURF_UNUSED00001000		(0x00001000u) //
#define	SURF_SKY				(0x00002000u) // lighting from environment map
#define	SURF_SLICK				(0x00004000u) // affects game physics
#define	SURF_METALSTEPS			(0x00008000u) // CHC needs this since we use same tools (though this flag is temp?)
#define SURF_FORCEFIELD			(0x00010000u) // CHC ""			(but not temp)
#define SURF_UNUSED00020000		(0x00020000u) //
#define	SURF_NODAMAGE			(0x00040000u) // never give falling damage
#define	SURF_NOIMPACT			(0x00080000u) // don't make missile explosions
#define	SURF_NOMARKS			(0x00100000u) // don't leave missile marks
#define	SURF_NODRAW				(0x00200000u) // don't generate a drawsurface at all
#define SURF_SKIP				SURF_NODRAW
#define	SURF_NOSTEPS			(0x00400000u) // no footstep sounds
#define	SURF_NODLIGHT			(0x00800000u) // don't dlight even if solid (solid lava, skies)
#define	SURF_NOMISCENTS			(0x01000000u) // no client models allowed on this surface
#define	SURF_FORCESIGHT			(0x02000000u) // not visible without Force Sight
#define SURF_UNUSED04000000		(0x04000000u) //
#define SURF_UNUSED08000000		(0x08000000u) //
#define SURF_UNUSED10000000		(0x10000000u) //
#define SURF_UNUSED20000000		(0x20000000u) //
#define SURF_UNUSED40000000		(0x40000000u) //
#define SURF_UNUSED80000000		(0x80000000u) //
#endif //__JA__
