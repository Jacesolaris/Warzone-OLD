// siege1_hoth

textures/siege/snowwallphong1
{
	q3map_shadeangle	145
	qer_editorimage	textures/hoth/h_wallchunk.tga
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/hoth/h_wallchunk
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/siege/snowwallphong2
{
	q3map_shadeangle	145
	qer_editorimage	textures/hoth/h_basicwall.tga
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/hoth/h_basicwall
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/siege/snowwallphong3
{
	q3map_shadeangle	145
	qer_editorimage	textures/hoth/h_wallchunk2.tga
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/hoth/h_wallchunk2
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/siege/snowfloorphong1
{
	q3map_shadeangle	170
	qer_editorimage	textures/hoth/h_floor.tga
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/hoth/h_floor
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/siege/skyportal_hoth
{
	q3map_shadeangle	60
	qer_editorimage	textures/hoth/h_outground_3
	q3map_lightmapsamplesize	8
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
	q3map_novertexshadows
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/hoth/h_outground_3
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
        tcMod scale 4 4
    }
}

textures/siege/cliffwall1
{
	q3map_shadeangle	170
	qer_editorimage	textures/yavin/rock3.tga
	q3map_lightmapsamplesize	128
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/yavin/rock3
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/siege/icetrans
{
	qer_editorimage	textures/hoth/ice
	surfaceparm	nonopaque
	q3map_material	Ice
	q3map_patchshadow
	q3map_novertexshadows
    {
        map textures/hoth/ice
        blendFunc GL_ONE_MINUS_DST_COLOR GL_ONE_MINUS_SRC_COLOR
        rgbGen vertex
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ONE_MINUS_SRC_COLOR
    }
}

textures/siege/outgroundlightmap
{
	q3map_shadeangle	110
	qer_editorimage	textures/hoth/h_outground_2.tga
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/hoth/h_outground_2
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/siege/blacklight
{
	qer_editorimage	textures/colors/white.tga
	q3map_surfacelight	1000
	q3map_nolightmap
    {
        map $whiteimage
        rgbGen const ( 0.000000 0.000000 0.000000 )
    }
}

// siege2_desert

textures/siege/desert
{
	qer_editorimage	textures/skies/sky.tga
	q3map_skylight	120	3
	sun 1 0.98 0.92 100 75 75
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	nopicmip
	nomipmaps
	q3map_nolightmap
	skyParms	textures/skies/bespin 512 -
}

textures/siege/stuccowall
{
	q3map_shadeangle	60
	qer_editorimage	textures/desert/stucco.tga
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/desert/stucco
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/siege/sand1
{
	q3map_shadeangle	60
	qer_editorimage	textures/desert/sandfloor2
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
    {
        map $lightmap
    }
    {
        map textures/desert/sandfloor2
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/siege/korcolumn
{
	q3map_shadeangle	160
	qer_editorimage	textures/korriban/k_wall9b
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
    {
        map $lightmap
    }
    {
        map textures/korriban/k_wall9b
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/siege/korcliffside
{
	q3map_shadeangle	160
	qer_editorimage	textures/korriban/os_rock2
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
    {
        map $lightmap
    }
    {
        map textures/korriban/os_rock2
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/siege/korrock3
{
	q3map_shadeangle	150
	qer_editorimage	textures/korriban/os_rock2
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/korriban/os_rock2
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/siege/korrock3diffuse
{
	q3map_shadeangle	150
	qer_editorimage	textures/korriban/os_rock2
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
    {
        map $lightmap
    }
    {
        map textures/korriban/os_rock2
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen lightingDiffuse
    }
}

textures/siege/osrockvertex
{
	q3map_shadeangle	200
	qer_editorimage	textures/korriban/os_rock
	q3map_nolightmap
	q3map_nonplanar
	q3map_forcemeta
	q3map_onlyvertexlighting
    {
        map textures/korriban/os_rock
        rgbGen vertex
    }
}

textures/siege/korportal
{
	q3map_shadeangle	175
	qer_editorimage	textures/korriban/os_rock3
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
    {
        map $lightmap
    }
    {
        map textures/korriban/os_rock3
        rgbGen identity
    }
}

textures/siege/ore_lightemitting_lavapool
{
	q3map_lightimage	textures/imp_mine/lava2
	qer_editorimage	textures/imp_mine/lava2
	q3map_surfacelight	1600
	q3map_lightsubdivide	1024
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	lava
	surfaceparm	trans
	q3map_material	Water
	q3map_nolightmap
	q3map_novertexshadows
	cull	twosided
    {
        map textures/imp_mine/lava2
        tcMod scroll 0.1 0.1
        tcMod turb 1 0.3 1 0.1
    }
}

textures/siege/korlava
{
	qer_editorimage	textures/imp_mine/lava2
	q3map_surfacelight	1400
	q3map_lightsubdivide	512
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	lava
	cull	twosided
    {
        map $lightmap
    }
    {
        map textures/imp_mine/lava2
        blendFunc GL_ONE GL_SRC_ALPHA
        glow
        rgbGen wave sin 0.8 0.1 0 0.3
        alphaGen const 0
        tcMod scroll 0.04 0.04
    }
    {
        map textures/imp_mine/lava2
        blendFunc GL_DST_COLOR GL_SRC_ALPHA
        tcMod scroll 0.05 0.05
        tcMod turb 1 0.3 1 0.1
    }
}


textures/siege/asteroidrock
{
	q3map_shadeangle	70
	qer_editorimage	textures/desert/t_rockwall1
	q3map_nonplanar
	q3map_splotchfix
    {
        map $lightmap
    }
    {
        map textures/desert/t_rockwall1
        blendFunc GL_DST_COLOR GL_ZERO
        tcMod scale 0.8 0.8
    }
    {
        map textures/desert/t_rockwall2
        blendFunc GL_ONE_MINUS_DST_COLOR GL_ZERO
        tcMod scale 0.6 0.6
    }
}

// desertstuff

textures/siege/desert_grime_tb
{
	qer_editorimage	textures/desert/stucco_grime_top_bottom.tga
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/desert/stucco_grime_top_bottom
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/siege/desert_stucco
{
	qer_editorimage	textures/desert/stucco.tga
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/desert/stucco
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/siege/desert_grime
{
	qer_editorimage	textures/desert/stucco_grime.tga
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/desert/stucco_grime
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

textures/siege/siege2sand
{
	q3map_shadeangle	170
	qer_editorimage	textures/desert/sandfloor2old.tga
	q3map_lightmapsamplesize	64
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/desert/sandfloor2old
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

// hoth sky

textures/siege/hoth_sky
{
	qer_editorimage	textures/skies/sky.tga
	q3map_lightsubdivide	512
	q3map_skylight	350	6
	sun 1 0.96 0.9 90 90 90
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	q3map_lightrgb 0.8 0.9 1.0
	notc
	q3map_nolightmap
	skyParms	textures/skies/hevil 128 -
    {
        map textures/skies/cloudlayer2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcMod scroll 0 0.00225
        tcMod scale 4 4
    }
}

textures/siege/hoth_cliff1
{
// qer_editorimage textures/rift/rock_color

	q3map_shadeangle	179
	qer_editorimage	textures/hoth/mp_rock
	q3map_lightmapaxis y
	q3map_nonplanar
	q3map_splotchfix
    {
// map textures/rift/rock_color

        map textures/hoth/mp_rock
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/siege/ice_phong
{
	q3map_shadeangle	145
	qer_editorimage	textures/hoth/h_basicwall.tga
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/hoth/h_basicwall
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
}

// TERRAIN BLEND SHADERS

textures/siege1/terrain_0
{
	q3map_shadeangle	170
	q3map_lightmapsamplesize	64
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
	q3map_tcGen	ivector ( 1024 0 0 ) ( 0 1024 0 )
    {
        map textures/hoth/h_outground
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/siege1/terrain_1
{
	q3map_shadeangle	170
	q3map_lightmapsamplesize	64
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
	q3map_tcGen	ivector ( 1024 0 0 ) ( 0 1024 0 )
    {
        map textures/hoth/h_outground_2
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/siege1/terrain_0to1
{
	q3map_shadeangle	170
	q3map_lightmapsamplesize	64
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
	q3map_tcGen	ivector ( 1024 0 0 ) ( 0 1024 0 )
    {
        map textures/hoth/h_outground
    }
    {
        map textures/hoth/h_outground_2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/siege1/terrain_vertex
{
	q3map_nolightmap
    {
        map textures/hoth/h_outground
        rgbGen vertex
    }
}

// Korriban

textures/siege3/terrain_0
{
	q3map_shadeangle	120
	q3map_lightmapaxis z
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
	q3map_tcGen	ivector ( 512 0 0 ) ( 0 512 0 )
    {
        map textures/korriban/os_rock3
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/siege3/terrain_1
{
	q3map_shadeangle	120
	q3map_lightmapaxis z
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
	q3map_tcGen	ivector ( 512 0 0 ) ( 0 512 0 )
    {
        map textures/korriban/os_rock2
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/siege3/terrain_0to1
{
	q3map_shadeangle	120
	q3map_lightmapaxis z
	q3map_nonplanar
	q3map_splotchfix
	q3map_forcemeta
	q3map_tcGen	ivector ( 512 0 0 ) ( 0 512 0 )
    {
        map textures/korriban/os_rock3
    }
    {
        map textures/korriban/os_rock2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/siege3/terrain_vertex
{
	q3map_nolightmap
    {
        map textures/korriban/os_rock2
        rgbGen vertex
    }
}

// kor terrain big

textures/kor/terrain_0
{
	q3map_shadeangle	120
	q3map_lightmapaxis z
	q3map_nonplanar
	q3map_splotchfix
	q3map_tcGen	ivector ( 512 0 0 ) ( 0 512 0 )
    {
        map textures/korriban/os_rock2
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/kor/terrain_1
{
	q3map_shadeangle	120
	q3map_lightmapaxis z
	q3map_nonplanar
	q3map_splotchfix
	q3map_tcGen	ivector ( 512 0 0 ) ( 0 512 0 )
    {
        map textures/korriban/os_rock3
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/kor/terrain_2
{
	q3map_shadeangle	120
	q3map_lightmapaxis z
	q3map_nonplanar
	q3map_splotchfix
	q3map_tcGen	ivector ( 512 0 0 ) ( 0 512 0 )
    {
        map textures/korriban/os_outsidebaseb_broken
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/kor/terrain_0to1
{
	q3map_shadeangle	120
	q3map_lightmapaxis z
	q3map_nonplanar
	q3map_splotchfix
	q3map_tcGen	ivector ( 512 0 0 ) ( 0 512 0 )
    {
        map textures/korriban/os_rock2
    }
    {
        map textures/korriban/os_rock3
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/kor/terrain_0to2
{
	q3map_shadeangle	120
	q3map_lightmapaxis z
	q3map_nonplanar
	q3map_splotchfix
	q3map_tcGen	ivector ( 512 0 0 ) ( 0 512 0 )
    {
        map textures/korriban/os_rock2
    }
    {
        map textures/korriban/os_outsidebaseb_broken
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/kor/terrain_1to2
{
	q3map_shadeangle	120
	q3map_lightmapaxis z
	q3map_nonplanar
	q3map_splotchfix
	q3map_tcGen	ivector ( 512 0 0 ) ( 0 512 0 )
    {
        map textures/korriban/os_rock3
    }
    {
        map textures/korriban/os_outsidebaseb_broken
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/kor/terrain_vertex
{
	q3map_nolightmap
    {
        map textures/korriban/os_rock2
        rgbGen vertex
    }
}

// siege_desert

textures/siege2airbase/terrain_0
{
	q3map_lightmapsamplesize	64
	q3map_lightmapaxis z
	q3map_tcGen	ivector ( 1024 0 0 ) ( 0 1024 0 )
    {
        map $lightmap
        tcGen lightmap
    }
    {
        map textures/desert/sandfloor2old
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/siege2airbase/terrain_1
{
	q3map_lightmapsamplesize	64
	q3map_lightmapaxis z
	q3map_tcGen	ivector ( 1024 0 0 ) ( 0 1024 0 )
    {
        map $lightmap
        tcGen lightmap
    }
    {
        map textures/desert/sandfloor2
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/siege2airbase/terrain_2
{
	q3map_lightmapsamplesize	64
	q3map_lightmapaxis z
	q3map_tcGen	ivector ( 1024 0 0 ) ( 0 1024 0 )
    {
        map $lightmap
        tcGen lightmap
    }
    {
        map textures/desert/sandfloor2old
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

textures/siege2airbase/terrain_0to1
{
	q3map_lightmapsamplesize	64
	q3map_lightmapaxis z
	q3map_tcGen	ivector ( 1024 0 0 ) ( 0 1024 0 )
    {
        map textures/desert/sandfloor2old
    }
    {
        map textures/desert/sandfloor2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/siege2airbase/terrain_0to2
{
	q3map_lightmapsamplesize	64
	q3map_lightmapaxis z
	q3map_tcGen	ivector ( 1024 0 0 ) ( 0 1024 0 )
    {
        map textures/desert/sandfloor2old
    }
    {
        map textures/desert/sandfloor2old
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/siege2airbase/terrain_1to2
{
	q3map_lightmapsamplesize	64
	q3map_lightmapaxis z
	q3map_tcGen	ivector ( 1024 0 0 ) ( 0 1024 0 )
    {
        map textures/desert/sandfloor2
    }
    {
        map textures/desert/sandfloor2old
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/siege2airbase/terrain_vertex
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/desert/sandfloor2old
        rgbGen vertex
    }
}

// korriban1terrain

textures/korriban1/terrain_0
{
	q3map_lightmapsamplesize	64
	q3map_lightmapaxis z
	q3map_nonplanar
	q3map_tcGen	ivector ( 640 0 0 ) ( 0 640 0 )
    {
// map textures/siege/korlava

        map textures/common/lav1
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/korriban1/terrain_1
{
	q3map_lightmapsamplesize	64
	q3map_lightmapaxis z
	q3map_nonplanar
	q3map_tcGen	ivector ( 640 0 0 ) ( 0 640 0 )
    {
        map textures/common/lrock2
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/korriban1/terrain_2
{
	q3map_lightmapsamplesize	64
	q3map_lightmapaxis z
	q3map_nonplanar
	q3map_tcGen	ivector ( 640 0 0 ) ( 0 640 0 )
    {
        map textures/desert/sandfloor2
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/korriban1/terrain_3
{
	q3map_lightmapsamplesize	64
	q3map_lightmapaxis z
	q3map_nonplanar
	q3map_tcGen	ivector ( 640 0 0 ) ( 0 640 0 )
    {
        map textures/desert/t_rockwall1
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/korriban1/terrain_0to1
{
	q3map_lightmapsamplesize	64
	q3map_lightmapaxis z
	q3map_nonplanar
	q3map_tcGen	ivector ( 640 0 0 ) ( 0 640 0 )
    {
// map textures/siege/korlava

        map textures/common/lav1
    }
    {
        map textures/common/lrock2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/korriban1/terrain_0to2
{
	q3map_lightmapsamplesize	64
	q3map_lightmapaxis z
	q3map_nonplanar
	q3map_tcGen	ivector ( 640 0 0 ) ( 0 640 0 )
    {
// map textures/siege/korlava

        map textures/common/lav1
    }
    {
        map textures/desert/sandfloor2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/korriban1/terrain_0to3
{
	q3map_lightmapsamplesize	64
	q3map_lightmapaxis z
	q3map_nonplanar
	q3map_tcGen	ivector ( 640 0 0 ) ( 0 640 0 )
    {
// map textures/siege/korlava

        map textures/common/lav1
    }
    {
        map textures/desert/t_rockwall1
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/korriban1/terrain_1to2
{
	q3map_lightmapsamplesize	64
	q3map_lightmapaxis z
	q3map_nonplanar
	q3map_tcGen	ivector ( 640 0 0 ) ( 0 640 0 )
    {
        map textures/common/lrock2
    }
    {
        map textures/desert/sandfloor2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/korriban1/terrain_1to3
{
	q3map_lightmapsamplesize	64
	q3map_lightmapaxis z
	q3map_nonplanar
	q3map_tcGen	ivector ( 640 0 0 ) ( 0 640 0 )
    {
        map textures/common/lrock2
    }
    {
        map textures/desert/t_rockwall1
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/korriban1/terrain_2to3
{
	q3map_lightmapsamplesize	64
	q3map_lightmapaxis z
	q3map_nonplanar
	q3map_tcGen	ivector ( 640 0 0 ) ( 0 640 0 )
    {
        map textures/desert/sandfloor2
    }
    {
        map textures/desert/t_rockwall1
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/korriban1/terrain_vertex
{
    {
// map textures/desert/sandfloor2

        map textures/common/lav1
        rgbGen vertex
    }
}

// korcrystalroom1

textures/korterra1/terrain_0
{
	q3map_lightmapaxis z
	q3map_nonplanar
	q3map_tcGen	ivector ( 256 0 0 ) ( 0 256 0 )
    {
        map textures/common/lrock2
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/korterra1/terrain_1
{
	q3map_lightmapaxis z
	q3map_nonplanar
	q3map_tcGen	ivector ( 256 0 0 ) ( 0 256 0 )
    {
        map textures/h_evil/evil_rock12
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/korterra1/terrain_2
{
	q3map_lightmapaxis z
	q3map_nonplanar
	q3map_tcGen	ivector ( 256 0 0 ) ( 0 256 0 )
    {
        map textures/common/lav1
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/korterra1/terrain_0to1
{
	q3map_lightmapaxis z
	q3map_nonplanar
	q3map_tcGen	ivector ( 256 0 0 ) ( 0 256 0 )
    {
        map textures/common/lrock2
    }
    {
        map textures/h_evil/evil_rock12
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/korterra1/terrain_0to2
{
	q3map_lightmapaxis z
	q3map_nonplanar
	q3map_tcGen	ivector ( 256 0 0 ) ( 0 256 0 )
    {
        map textures/common/lrock2
    }
    {
        map textures/common/lav1
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/korterra1/terrain_1to2
{
	q3map_lightmapaxis z
	q3map_nonplanar
	q3map_tcGen	ivector ( 256 0 0 ) ( 0 256 0 )
    {
        map textures/h_evil/evil_rock12
    }
    {
        map textures/common/lav1
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/korterra1/terrain_vertex
{
    {
        map textures/common/lrock2
        rgbGen vertex
    }
}

// crystallava korriban

textures/crystallava/terrain_0
{
	q3map_lightmapaxis z
	q3map_tcGen	ivector ( 256 0 0 ) ( 0 256 0 )
    {
        map textures/korriban/os_rock2
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/crystallava/terrain_1
{
	q3map_lightmapaxis z
	q3map_tcGen	ivector ( 256 0 0 ) ( 0 256 0 )
    {
        map textures/korriban/os_rock
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/crystallava/terrain_0to1
{
	q3map_lightmapaxis z
	q3map_tcGen	ivector ( 256 0 0 ) ( 0 256 0 )
    {
        map textures/korriban/os_rock2
    }
    {
        map textures/korriban/os_rock
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/crystallava/terrain_vertex
{
    {
        map textures/korriban/os_rock2
        rgbGen vertex
    }
}

models/map_objects/mp/mp_scepter
{
	q3map_nolightmap
    {
        map models/weapons2/sith_scepter/sith_scepter
        blendFunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
    {
        map textures/rift/env_crystal_red
        blendFunc GL_ONE GL_ONE
        glow
        tcGen environment
    }
}

textures/terradesert/terrain_0
{
	q3map_nonplanar
	
	q3map_tcGen	ivector ( 512 0 0 ) ( 0 640 0 )
    {
        map textures/desert/sandfloor2old
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/terradesert/terrain_1
{	
	q3map_nonplanar
	q3map_tcGen	ivector ( 512 0 0 ) ( 0 640 0 )
    {
        map textures/desert/t_rockwall1
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/terradesert/terrain_0to1
{
	q3map_nonplanar
	q3map_tcGen	ivector ( 512 0 0 ) ( 0 640 0 )
    {
        map textures/desert/sandfloor2old
    }
    {
        map textures/desert/t_rockwall1
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
        tcGen lightmap
    }
}

textures/terradesert/terrain_vertex
{
    {
        map textures/desert/sandfloor2old
        rgbGen vertex
    }
}

