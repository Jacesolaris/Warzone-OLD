textures/rbettenbergtest/t1_danger_sky
{
	qer_editorimage	textures/skies/sky.tga
	//q3map_surfacelight	75
	//q3map_lightsubdivide	512
	//sun 0.75 0.79 1 40 315 40
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/bespin 512 -
}

textures/rbettenbergtest/t1_danger_phong
{
	qer_editorimage textures/danger/sand
	q3map_nonplanar
	q3map_shadeangle 120
	q3map_splotchfix
	q3map_lightsubdivide	32
    {
        map textures/danger/sand
        //blendFunc GL_ONE GL_ZERO
	//tcMod scale 0.7 0.7
	rgbGen identity
    }
    //{
        //map textures/danger/sand
        //blendFunc GL_DST_COLOR GL_ZERO
	//rgbGen identity
    //}
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
	rgbGen identity
    }
}

textures/rbettenbergtest/t1_danger_streaked
{
	qer_editorimage textures/danger/sand_rough
	q3map_nonplanar
	q3map_shadeangle 120
	q3map_splotchfix
	q3map_lightsubdivide	32
    {
        map textures/danger/sand_rough
        //blendFunc GL_ONE GL_ZERO
	//tcMod scale 0.3 1
	rgbGen identity
    }
    //{
        //map textures/danger/sand
        //blendFunc GL_DST_COLOR GL_ZERO
	//rgbGen identity
    //}
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
	rgbGen identity
    }
}

textures/rbettenbergtest/korriban1_phong
{
	qer_editorimage textures/h_evil/evil_rock12
	q3map_nonplanar
	q3map_shadeangle 120
	q3map_splotchfix
	//q3map_lightsubdivide	32
    {
        map textures/h_evil/evil_rock12
        //blendFunc GL_ONE GL_ZERO
        tcMod scale 0.5 0.5
	rgbGen identity
    }
    {
        map textures/h_evil/evil_rock13
        blendFunc GL_DST_COLOR GL_ZERO
	rgbGen identity
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_ZERO
	rgbGen identity
    }
}

textures/rbettenbergtest/rbettenbergtest_water1
{
	qer_editorimage	textures/yavin/water1.tga
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	water
	q3map_material	Water
	q3map_nolightmap
	q3map_novertexshadows
	cull	disable
    {
        map textures/yavin/water1
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod scroll 0.05 0.1
    }
    {
        map textures/yavin/water1
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod scroll 0.15 -0.05
    }
}

textures/rbettenbergtest/rbettenbergtest_light1
{
	qer_editorimage	textures/colors/white.tga
	q3map_surfacelight	2400
	q3map_lightsubdivide	128
	surfaceparm	metalsteps
	//surfaceparm	monlightmap

    {
        map textures/colors/white.tga
    }
}

textures/rbettenbergtest/rbettenbergtest_glass1
{
	qer_editorimage	textures/rbettenbergtest/glass2
	q3map_tesssize	48
	qer_trans	0.5
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	cull	disable
    {
        map textures/rbettenbergtest/glass2
        blendFunc GL_ONE GL_ONE
        tcGen environment
    }
}

textures/rbettenbergtest/rbettenbergtest_waternodraw
{
	qer_editorimage	textures/rbettenbergtest/animspec0
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nodraw
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	water
	q3map_material	Water
	q3map_nolightmap
	q3map_novertexshadows
	cull	disable
}

textures/rbettenbergtest/rbettenbergtest_waterfall1
{
	qer_editorimage	textures/yavin/water1.tga
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	water
	q3map_material	Water
	q3map_nolightmap
	q3map_novertexshadows
	cull	disable
    {
        map textures/yavin/water1
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod scroll 0 3
    }
    {
        map textures/yavin/water1
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod scroll 0 0.5
    }
}

textures/rbettenbergtest/rbettenbergtest_bluefog
{
// fogparms ( Red, Green, Blue=Color of opacity layer) Number.0=depth of transparency/opacity border.

	qer_editorimage	textures/rbettenbergtest/Ford_bluefog.tga
	qer_nocarve
	qer_trans	0.5
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0 0 0.176471 ) 500.0
	cull	disable
	deformvertexes	move	0 0 0 sin 0 1 0 1
}

textures/rbettenbergtest/rbettenbergtest_waterfall2
{
	qer_editorimage	textures/yavin/water1.tga
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	water
	q3map_material	Water
	q3map_nolightmap
	q3map_novertexshadows
	cull	disable
    {
        map textures/yavin/water1
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod scroll 0 -3
    }
    {
        map textures/yavin/water1
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod scroll 0 -0.5
    }
}

textures/rbettenbergtest/rbettenbergtest_sky1
{
	qer_editorimage	textures/skies/sky.tga
	q3map_surfacelight	700
	q3map_lightsubdivide	512
	sun 1 0.85 0.85 350 300 60
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/bespin 512 -
}

textures/rbettenbergtest/rbettenbergtest_waterfall3
{
	qer_editorimage	textures/yavin/water1.tga
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	water
	q3map_material	Water
	q3map_nolightmap
	q3map_novertexshadows
	cull	disable
    {
        map textures/yavin/water1
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod scroll 0 -1
    }
    {
        map textures/yavin/water1
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod scroll 0 -0.5
    }
}

textures/rbettenbergtest/tcfoghull
{
	qer_editorimage	textures/skies/sky.tga
	q3map_surfacelight	700
	q3map_lightsubdivide	512
	sun 1 0.85 0.85 350 300 60
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/rbettenbergtest/rbettenbergtestfoghull/tc 512 -
}

textures/rbettenbergtest/tcfog
{
// fogparms ( Red, Green, Blue=Color of opacity layer) Number.0=depth of transparency/opacity border.

	qer_editorimage	textures/rbettenbergtest/Ford_bluefog.tga
	qer_nocarve
	qer_trans	0.5
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0.627451 0.137255 0 ) 5600.0
	cull	disable
	deformvertexes	move	0 0 0 sin 0 1 0 1
}

textures/rbettenbergtest/tc_lavatest
{
	qer_editorimage	textures/imp_mine/lava2
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	lava
	q3map_nolightmap
	q3map_novertexshadows
	cull	disable
    {
        map textures/imp_mine/lava2
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod rotate 1
        tcMod turb 0 0.1 0.5 0.1
    }
    {
        map textures/imp_mine/lava2
        blendFunc GL_ONE GL_ONE
        rgbGen identity
        tcMod rotate -1
        tcMod turb 0 0.1 0 0.1
    }
}

textures/rbettenbergtest/rbettenbergtest_sky_dpredicament
{
	q3map_lightimage	textures/skies/yavin_up.tga
	qer_editorimage	textures/skies/yavin_up.tga
	q3map_surfacelight	800
	q3map_lightsubdivide	512
	sun 0.898039 0.898039 1 400 300 50
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/yavin 512 -
}

textures/rbettenbergtest/rbettenbergtest_light1_dpredicament
{
	q3map_lightimage	textures/impdetention/metalpanel1f1dark
	lightcolor	( 0.9 0.9 1 )
	qer_editorimage	textures/impdetention/metalpanel1f1dark
	q3map_surfacelight	3200
	q3map_lightsubdivide	64
    {
        map $lightmap
    }
    {
        map textures/impdetention/metalpanel1f1dark
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/impdetention/metalpanel1f1darkglow
        blendFunc GL_ONE GL_ONE
    }
}

textures/rbettenbergtest/hevil_phongrock
{
	qer_editorimage textures/h_evil/evil_rock12.tga

	q3map_nonplanar
	q3map_shadeAngle 60

    {
	map textures/h_evil/evil_rock12.tga
	tcMod scale 0.25 0.25
	rgbGen identity
    }
    {
	map $lightmap
	blendFunc GL_DST_COLOR GL_ZERO
	rgbGen identity
    }
}

textures/rbettenbergtest/rbettenbergtest_sky_hevil
{
	q3map_lightimage	textures/colors/white.tga
	qer_editorimage	textures/skies/yavin_up.tga
	q3map_surfacelight	800
	q3map_lightsubdivide	512
	sun 1 1 1 400 300 50
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/yavin 512 -
}

textures/rbettenbergtest/hevil_phongrock2
{

	q3map_nonplanar
	q3map_shadeAngle 60

    {
        map textures/h_evil/evil_rock12
        blendFunc GL_ONE GL_ZERO
        tcMod scale 0.1 0.1
    }
    {
        map textures/h_evil/evil_rock13
        blendFunc GL_DST_COLOR GL_ZERO
        tcMod scale 0.5 0.5
    }
    {
        map $lightmap
        blendFunc GL_ZERO GL_ONE
    }
}

textures/rbettenbergtest/taspir_phongrock
{
	qer_editorimage textures/h_evil/evil_rock12.tga

	q3map_lightmapsamplesize 32
	q3map_nonplanar
	q3map_shadeAngle 60

    {
	map textures/h_evil/evil_rock12.tga
	tcMod scale 0.25 0.25
	rgbGen identity
    }
    {
	map $lightmap
	blendFunc GL_DST_COLOR GL_ZERO
	rgbGen identity
    }
}

textures/rbettenbergtest/taspir1_fog
{

	qer_editorimage	textures/colors/white.tga
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	// (  Red          Green        Blue     ) Distance
	fogparms	( 0.8 0.7 0.4 ) 4800.0
}

// t1_danger

textures/danger/terrain_0
{
	q3map_nonplanar
	q3map_splotchfix
	q3map_shadeangle	150
	q3map_lightmapaxis z
	q3map_tcGen ivector ( 384 0 0 ) ( 0 384 0 )
	{
		map textures/danger/sanddunes
	}
	{
		map $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		tcGen lightmap
	}
}

textures/danger/terrain_1
{
	q3map_nonplanar
	q3map_splotchfix
	q3map_shadeangle	150
	q3map_lightmapaxis z
	q3map_tcGen ivector ( 384 0 0 ) ( 0 384 0 )
	{
		map textures/danger/sand_blend
	}
	{
		map $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		tcGen lightmap
	}
}

textures/danger/terrain_2
{
	q3map_nonplanar
	q3map_splotchfix
	q3map_shadeangle	150
	q3map_lightmapaxis z
	q3map_tcGen ivector ( 384 0 0 ) ( 0 384 0 )
	{
		map textures/danger/sand_rough
	}
	{
		map $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		tcGen lightmap
	}
}

textures/danger/terrain_0to1
{
	q3map_nonplanar
	q3map_splotchfix
	q3map_shadeangle	150
	q3map_lightmapaxis z
	q3map_tcGen ivector ( 384 0 0 ) ( 0 384 0 )
	
	{
		map textures/danger/sanddunes
	}
	{
		map textures/danger/sand_blend
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
	{
		map $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		tcGen lightmap
	}
}

textures/danger/terrain_0to2
{
	q3map_nonplanar
	q3map_splotchfix
	q3map_shadeangle	150
	q3map_lightmapaxis z
	q3map_tcGen ivector ( 384 0 0 ) ( 0 384 0 )
	
	{
		map textures/danger/sanddunes
	}
	{
		map textures/danger/sand_rough
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
	{
		map $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		tcGen lightmap
	}
}

textures/danger/terrain_1to2
{
	q3map_nonplanar
	q3map_splotchfix
	q3map_shadeangle	150
	q3map_lightmapaxis z
	q3map_tcGen ivector ( 384 0 0 ) ( 0 384 0 )
	
	{
		map textures/danger/sand_blend
	}
	{
		map textures/danger/sand_rough
		alphaGen vertex
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
	{
		map $lightmap
		blendFunc GL_DST_COLOR GL_ZERO
		tcGen lightmap
	}
}

textures/danger/terrain_vertex
{
	{
		map textures/danger/sanddunes
		rgbGen vertex
	}
}