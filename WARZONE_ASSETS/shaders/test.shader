textures/tests/glass_green
{
	surfaceparm	nonsolid
	surfaceparm	trans
	cull	twosided
    {
        map textures/tests/glass_green
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map textures/tests/env_chrome_add
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
}

textures/tests/glasspartition
{
	surfaceparm	nonsolid
	surfaceparm	trans
	cull	twosided
    {
        map textures/tests/glasspartition
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
    {
        map textures/tests/env_chrome_add
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
}

textures/tests/test1
{
	qer_editorimage	textures/impgarrison/new5.tga
    {
        map $lightmap
    }
    {
        map textures/impgarrison/new5
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/tests/env_chrome_add2
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
}

textures/tests/test2
{
	qer_editorimage	textures/impgarrison/disk_side_02.tga
    {
        map $lightmap
    }
    {
        map textures/impgarrison/disk_side_02
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/tests/env_chrome_add
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
}

textures/tests/test3
{
	qer_editorimage	textures/impgarrison/floor01.tga
    {
        map $lightmap
    }
    {
        map textures/impgarrison/floor01
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/tests/glass1
        blendFunc GL_ONE GL_ONE
        tcGen environment
        tcMod scale 0.5 0.5
    }
}

// *****************************************

// *	Glass Shaders from Trek		*

// *****************************************

textures/tests/glass1_opaque
{
	qer_editorimage	textures/tests/glass1.tga
	q3map_tesssize	48
	qer_trans	0.5
	surfaceparm	trans
    {
// tcMod scale 3 3

        map textures/tests/glass1
        blendFunc GL_ONE GL_ZERO
        tcGen environment
    }
    {
        map textures/tests/glass1
        blendFunc GL_ONE GL_ONE
        detail
        tcGen environment
        tcMod scale -2 2
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/tests/glass1
{
	qer_trans	0.5
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/tests/glass2
        blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
    }
    {
// tcMod scale 3 3

        map textures/tests/glass2
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
}

textures/tests/glass_nolightmap
{
	qer_editorimage	textures/tests/glass1.tga
	qer_trans	0.5
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/tests/glass1
        blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
    }
    {
// tcMod scale 3 3

        map textures/tests/glass1
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
}

textures/tests/glass_vertexlight
{
	qer_editorimage	textures/tests/glass1.tga
	qer_trans	0.5
	surfaceparm	trans
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
// tcMod scale 3 3

        map textures/tests/glass1
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
        tcGen environment
    }
}

textures/tests/light_test
{
	qer_editorimage	textures/impgarrison/grate01.tga
	q3map_surfacelight	1200
	q3map_nolightmap
    {
        map textures/impgarrison/grate01
        rgbGen identity
    }
}

textures/tests/metal1
{
	qer_editorimage	textures/impgarrison/metal1.tga
	q3map_surfacelight	300
	q3map_nolightmap
    {
        map textures/impgarrison/metal1
        rgbGen identity
    }
}

textures/tests/light_red
{
	qer_editorimage	textures/tests/light_red.tga
	q3map_surfacelight	300
	q3map_nolightmap
    {
        map textures/tests/light_red
        rgbGen identity
    }
}

textures/tests/light_red2
{
	qer_editorimage	textures/tests/light_red2.tga
	q3map_surfacelight	300
	q3map_nolightmap
    {
        map textures/tests/light_red2
        rgbGen identity
    }
}

textures/tests/light_blue
{
	qer_editorimage	textures/tests/light_blue.tga
	q3map_surfacelight	2000
	q3map_nolightmap
    {
        map textures/tests/light_blue
        rgbGen identity
    }
}

textures/tests/light_red3
{
	qer_editorimage	textures/tests/light_red3.tga
	q3map_surfacelight	10000
	q3map_nolightmap
    {
        map textures/tests/light_red3
        rgbGen identity
    }
}

textures/tests/forcefield
{
	qer_editorimage	textures/tests/energything.tga
	surfaceparm	nomarks
	surfaceparm	forcefield
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map textures/tests/energything2
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.65 0.35 0 0.2
        tcMod scroll -1 2
    }
    {
        map textures/tests/energything2
        blendFunc GL_ONE GL_ONE
        tcMod scroll 2 1
    }
}

textures/tests/forcefield_2
{
	qer_editorimage	textures/tests/energything.tga
	surfaceparm	nomarks
	surfaceparm	forcefield
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/tests/energything
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.65 0.35 0 0.2
    }
    {
        map textures/tests/energything2
        blendFunc GL_ONE GL_ONE
        tcMod scroll 562 0
    }
}

textures/tests/temp5
{
    {
        map textures/tests/blue_glow
        blendFunc GL_DST_COLOR GL_ZERO
        depthWrite
        rgbGen wave sin 0 1 0 2
    }
    {
        map textures/tests/temp5
        blendFunc GL_ONE GL_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/tests/glass_reflective
{
	qer_editorimage	textures/tests/glass2.tga
	q3map_tesssize	48
	qer_trans	0.5
	surfaceparm	forcefield
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/tests/glass2
        blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
    }
    {
// tcMod scale 3 3

        map textures/tests/glass2
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
}

textures/tests/floor_specular
{
	qer_editorimage	textures/impgarrison/floor01.tga
    {
        map $lightmap
    }
    {
        map textures/bespin/floor2
        blendFunc GL_DST_COLOR GL_ZERO
        alphaGen lightingSpecular
    }
}

textures/tests/floor_slick
{
	qer_editorimage	textures/impgarrison/floor01.tga
	surfaceparm	slick
    {
        map $lightmap
    }
    {
        map textures/bespin/basic2
        blendFunc GL_DST_COLOR GL_ZERO
        alphaGen lightingSpecular
    }
}

textures/tests/energything3
{
	surfaceparm	nomarks
	surfaceparm	forcefield
	q3map_nolightmap
    {
        map textures/tests/energything3
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 1.1
    }
    {
        map textures/tests/energything3
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 0.9
    }
    {
        map textures/tests/energything3
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 0.4
    }
    {
        map textures/tests/energything3
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 0.1
    }
}

textures/tests/glass_portal
{
	qer_editorimage	textures/tests/glass2
	qer_trans	0.5
	surfaceparm	trans
	portal
	q3map_nolightmap
    {
        map textures/tests/glass2
        blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
        alphaGen portal 256
    }
    {
// tcMod scale 3 3

        map textures/tests/glass2
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
}

textures/tests/slick_tube
{
	qer_editorimage	textures/nar_hideout/floor5
	surfaceparm	slick
	surfaceparm	forcefield
    {
        map $lightmap
    }
    {
        map textures/nar_hideout/floor5
        blendFunc GL_DST_COLOR GL_ZERO
        alphaGen lightingSpecular
    }
}

textures/tests/waste
{
// {

// map textures/tests/shine

// blendFunc GL_DST_COLOR GL_SRC_COLOR

// detail

// tcGen environment

// tcMod scroll 0.13 0.14

// }

// {

// map textures/tests/shine

// blendFunc GL_DST_COLOR GL_SRC_COLOR

// detail

// tcGen environment

// tcMod scroll -0.17 -0.09

// }

	qer_editorimage	textures/tests/waste
	q3map_tesssize	192
	surfaceparm	nosteps
	surfaceparm	nonsolid
	surfaceparm	trans
	cull	twosided
	deformvertexes	wave	200 sin 0 2 0 0.5
    {
        map textures/tests/waste
        blendFunc GL_ONE GL_SRC_COLOR
        rgbGen identity
        tcMod scroll 0 0.1
        tcMod turb 0.1 0.1 0.1 0.1
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
    }
}

textures/tests/glass_security
{
	qer_editorimage	textures/tests/glass2
	qer_trans	0.5
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/tests/glass2
        blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
    }
    {
// tcMod scale 3 3

        map textures/tests/glass2
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
    {
        map textures/kejim/security_glass
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/tests/mirror_floor2
{
	qer_editorimage	textures/tests/qer_mirror.tga
	surfaceparm	forcefield
	portal
	q3map_nolightmap
    {
        map textures/tests/qer_mirror2b
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        depthWrite
    }
    {
        map textures/tests/floor02_alphab
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
    {
        map textures/kejim/env_newfloor_kej
        blendFunc GL_ONE GL_ONE
        rgbGen const ( 0.200000 0.200000 0.200000 )
        tcGen environment
    }
}

textures/tests/forcefield_nonsolid
{
	qer_editorimage	textures/tests/energything.tga
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	forcefield
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map textures/tests/energything2
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.65 0.35 0 0.2
        tcMod scroll -1 2
    }
    {
        map textures/tests/energything2
        blendFunc GL_ONE GL_ONE
        tcMod scroll 2 1
    }
}

textures/tests/glass_security_border
{
	qer_editorimage	textures/tests/glass2
	qer_trans	0.5
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/tests/glass2
        blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
    }
    {
// tcMod scale 3 3

        map textures/tests/glass2
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
    {
        map textures/kejim/security_glass_border
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/tests/forcefield_block
{
	qer_editorimage	textures/tests/energything.tga
	surfaceparm	nomarks
	surfaceparm	playerclip
	surfaceparm	monsterclip
	surfaceparm	botclip
	surfaceparm	shotclip
	surfaceparm	forcefield
	q3map_nolightmap
	cull	twosided
    {
        map textures/tests/energything2
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0.65 0.35 0 0.2
        tcMod scroll -1 2
    }
    {
        map textures/tests/energything2
        blendFunc GL_ONE GL_ONE
        tcMod scroll 2 1
    }
}

textures/tests/energything_block
{
	qer_editorimage	textures/tests/energything3
	surfaceparm	nomarks
	surfaceparm	playerclip
	surfaceparm	monsterclip
	surfaceparm	botclip
	surfaceparm	shotclip
	surfaceparm	forcefield
	q3map_nolightmap
    {
        map textures/tests/energything3
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 1.1
    }
    {
        map textures/tests/energything3
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 0.9
    }
    {
        map textures/tests/energything3
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 0.4
    }
    {
        map textures/tests/energything3
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 0.1
    }
}

textures/tests/mirror_floor
{
	qer_editorimage	textures/tests/qer_mirror.tga
	surfaceparm	forcefield
	portal
	q3map_nolightmap
	sort	portal
    {
        map textures/tests/floor02_alphac
        blendFunc GL_ONE GL_ONE
    }
    {
        map textures/tests/qer_mirror2b
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        alphaGen portal 256
    }
}

textures/tests/droid_shield
{
	deformvertexes	bulge	0 0.75 0
    {
        map gfx/effects/p_shield
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen entity
        tcGen environment
        tcMod rotate 200
        tcMod turb 0.6 0.3 0 0.6
    }
    {
        map gfx/effects/p_shield
        blendFunc GL_DST_COLOR GL_ONE
        rgbGen entity
        alphaGen const 0.5
        tcMod rotate -600
        tcMod scale 2 3
    }
}

textures/impdetention/elevatorup_on
{
    {
        map $lightmap
    }
    {
        map textures/impdetention/elevatorup
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/impdetention/elevatorup_on
        blendFunc GL_ONE GL_ONE
    }
}

textures/impdetention/elevatordown_on
{
    {
        map $lightmap
    }
    {
        map textures/impdetention/elevatordown
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/impdetention/elevatordown_on
        blendFunc GL_ONE GL_ONE
    }
}

textures/impdetention/control_ff_on
{
	sort	opaque
    {
        map textures/tests/energything2
        tcMod scroll -5 1
    }
    {
        map textures/tests/energything2
        blendFunc GL_ONE GL_ONE
        tcMod scroll 1 2
    }
    {
        map textures/impdetention/control_ff_on
        blendFunc GL_ONE GL_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/impdetention/elevatorcall
{
    {
        map $lightmap
    }
    {
        map textures/impdetention/elevatorcall
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/impdetention/elevatorcall_on
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 0.5 1
    }
    {
        map textures/impdetention/elevatorcall_on2
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 0 1
    }
}

textures/impdetention/ff_edge
{
	q3map_nolightmap
    {
        map textures/impdetention/edge1
    }
    {
        map textures/impdetention/ff_edge
        blendFunc GL_ONE GL_ONE
        tcMod scroll 1 0
    }
    {
        map textures/impdetention/ff_edge
        blendFunc GL_ONE GL_ONE
        tcMod scroll -0.7 0
    }
}

textures/impdetention/light_red
{
	qer_editorimage	textures/impdetention/light_red
	q3map_surfacelight	2000
	q3map_nolightmap
    {
        map textures/impdetention/light_red
        rgbGen identity
    }
}

textures/impdetention/light_white
{
	qer_editorimage	textures/impdetention/light_white
	q3map_surfacelight	2000
	q3map_nolightmap
    {
        map textures/impdetention/light_white
        rgbGen identity
    }
}

textures/impdetention/elevatorup
{
    {
        map $lightmap
    }
    {
        map textures/impdetention/elevatorup
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/impdetention/elevator_glow
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
    }
}

textures/impdetention/elevatordown
{
    {
        map $lightmap
    }
    {
        map textures/impdetention/elevatordown
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/impdetention/elevator_glow
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
    }
}

textures/impdetention/crate
{
    {
        map textures/impdetention/red
        blendFunc GL_ONE GL_ZERO
        rgbGen wave sin 1.5 1 0 0.5
    }
    {
        map textures/impdetention/crate
        blendFunc GL_ONE GL_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/impdetention/512_512lights
{
    {
        map textures/impdetention/light_fx
        tcMod rotate 5
        tcMod scroll 0.1 0.3
    }
    {
        map textures/impdetention/512_512lights
        blendFunc GL_ONE GL_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/impdetention/deathcon1a
{
    {
        map textures/tests/energything2
        tcMod scroll 0 0.1
        tcMod scale 2 2
        tcMod turb 0 0.1 0 2
    }
    {
        map textures/impdetention/deathcon1a
        blendFunc GL_ONE GL_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/impdetention/deathcon1
{
    {
        map textures/tests/energything2
        tcMod scale 4 4
        tcMod turb 0 0.4 0 1
    }
    {
        map textures/impdetention/deathcon1
        blendFunc GL_ONE GL_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/impdetention/deathcon2a
{
    {
        map textures/tests/energything2
        tcMod stretch sin 0 0.2 0 0.3
    }
    {
        map textures/impdetention/deathcon2a
        blendFunc GL_ONE GL_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/impdetention/deathcon2
{
    {
        map textures/tests/energything2
        tcMod scroll 0 2
    }
    {
        map textures/impdetention/deathcon2
        blendFunc GL_ONE GL_SRC_ALPHA
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/impdetention/light_blue
{
	qer_editorimage	textures/impdetention/light_blue
	q3map_surfacelight	2000
	q3map_nolightmap
    {
        map textures/impdetention/light_blue
        rgbGen identity
    }
}

textures/impdetention/light_white_flare
{
	qer_editorimage	textures/impdetention/light_white
	q3map_surfacelight	2000
	q3map_flare	textures/flares/flare_white
	q3map_nolightmap
    {
        map textures/impdetention/light_white
        rgbGen identity
    }
}

textures/yavin/map
{
	qer_trans	0.5
	surfaceparm	forcefield
	surfaceparm	trans
	q3map_nolightmap
    {
        map textures/yavin/map
        blendFunc GL_ONE GL_ONE
    }
}

textures/yavin/slipdirt
{
	qer_editorimage	textures/yavin/dugdirt
	surfaceparm	slick
	surfaceparm	nodamage
    {
        map $lightmap
    }
    {
        map textures/yavin/dugdirt
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/yavin/mrj_water
{
	qer_editorimage	textures/yavin/water1
	q3map_tesssize	128
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	water
	surfaceparm	trans
	q3map_material	Water
	q3map_nolightmap
	q3map_onlyvertexlighting
	q3map_novertexshadows
	deformvertexes	wave	100 sin 0 1 0 1
    {
        map textures/yavin/water1
        tcMod scroll 0.1 0.1
        tcMod turb 1 0.3 1 0.1
    }
}

textures/yavin/coolant_test
{
	qer_editorimage	textures/kejim/coolant
	q3map_tesssize	128
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	water
	surfaceparm	trans
	q3map_material	Water
	q3map_nolightmap
	q3map_onlyvertexlighting
	q3map_novertexshadows
	sort	seeThrough
	deformvertexes	wave	100 sin 0 1 0 1
    {
        map textures/kejim/coolant
        blendFunc GL_ZERO GL_SRC_COLOR
        tcMod scroll 0.1 0.1
        tcMod turb 1 0.3 1 0.1
    }
}

textures/yavin/smooth_rock2_rain
{
	qer_editorimage	textures/yavin/smooth_rock2
    {
        map $lightmap
    }
    {
        map textures/yavin/smooth_rock2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        clampmap gfx/sprites/rainhit
            surfaceSprites effect 2.5 2.5 30 1200
            ssVariance 1 0.75
            ssWind 1.5
            ssFXDuration 135
            ssFXGrow 6 6
            ssFXAlphaRange 0.35 0
            ssFXWeather
        blendFunc GL_ONE GL_ONE
    }
    {
        clampmap gfx/sprites/rainring
            surfaceSprites effect 2 2 30 350
            ssVariance 2 1
            ssFaceup
            ssFXDuration 220
            ssFXGrow 2.5 2.5
            ssFXAlphaRange 0.5 0
            ssFXWeather
        blendFunc GL_ONE GL_ONE
    }
    {
        clampmap gfx/sprites/fog
            surfaceSprites effect 3 3 30 1200
            ssVariance 1 0.75
            ssWind 1.5
            ssFXDuration 135
            ssFXGrow 6 6
            ssFXAlphaRange 0.25 0
            ssFXWeather
        blendFunc GL_ONE GL_ONE
    }
}

textures/shapes
{
	qer_editorimage	textures/kejim/puzzle01.tga
	cull	twosided
    {
        animMap 4 textures/kejim/puzzle01.tga textures/kejim/puzzle04.tga textures/kejim/puzzle07.tga textures/kejim/puzzle10.tga 
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

textures/lines
{
	qer_editorimage	textures/kejim/puzzle02.tga
	cull	twosided
    {
        animMap 4 textures/kejim/puzzle02.tga textures/kejim/puzzle05.tga textures/kejim/puzzle08.tga textures/kejim/puzzle11.tga 
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

textures/dots
{
	qer_editorimage	textures/kejim/puzzle03.tga
	cull	twosided
    {
        animMap 4 textures/kejim/puzzle03.tga textures/kejim/puzzle06.tga textures/kejim/puzzle09.tga textures/kejim/puzzle12.tga 
        blendFunc GL_ONE GL_ONE
        rgbGen vertex
    }
}

textures/impgarrison/blastdoor_lock
{
    {
        animMap 1 textures/impgarrison/blastdoor_lock textures/impgarrison/blastdoor_01 
        blendFunc GL_ONE GL_ONE
    }
}

textures/fogs/rain
{
	qer_editorimage	textures/fogs/fog.tga
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0 0.03 0.15 ) 1000.0
}

textures/system/rainsurf
{
	qer_trans	0.7
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
    {
        clampmap gfx/sprites/rainhit
            surfaceSprites effect 2.5 2.5 30 1200
            ssVariance 1 0.75
            ssWind 1.5
            ssFXDuration 135
            ssFXGrow 6 6
            ssFXAlphaRange 0.35 0
            ssFXWeather
        blendFunc GL_ONE GL_ONE
    }
    {
        clampmap gfx/sprites/rainring
            surfaceSprites effect 2 2 30 350
            ssVariance 2 1
            ssFaceup
            ssFXDuration 220
            ssFXGrow 2.5 2.5
            ssFXAlphaRange 0.5 0
            ssFXWeather
        blendFunc GL_ONE GL_ONE
    }
    {
        clampmap gfx/sprites/fog
            surfaceSprites effect 3 3 30 1200
            ssVariance 1 0.75
            ssWind 1.5
            ssFXDuration 135
            ssFXGrow 6 6
            ssFXAlphaRange 0.25 0
            ssFXWeather
        blendFunc GL_ONE GL_ONE
    }
}

