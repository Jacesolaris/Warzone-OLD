textures/custom/shadowgrate
{
	qer_editorimage	textures/custom/shadowgrate
	qer_trans	1
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_alphashadow
	cull	twosided
    {
// * blendFunc GL_DST-COLOR GL_ZERO

        map $lightmap
        depthFunc equal
        rgbGen identity
    }
    {
        map textures/custom/shadowgrate
        alphaFunc GE128
        blendFunc GL_DST_COLOR GL_ZERO
        depthWrite
    }
}

textures/custom/floor01
{
    {
        map textures/custom/floor01
    }
}

textures/custom/wallpanel_b
{
    {
        map $lightmap
    }
    {
        map textures/custom/wallpanel_b
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/custom/wallpanel_b_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

textures/custom/bluefog_dense
{
	qer_editorimage	textures/custom/ase_blueknob
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0 0.03 0.15 ) 1000.0
}

textures/custom/bluefog_thin
{
	qer_editorimage	textures/common/gradient
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0 0.03 0.15 ) 3000.0
}

textures/custom/codeblock
{
	qer_editorimage	textures/custom/bluecode
	qer_nocarve
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        animMap 1 textures/custom/bluecode textures/custom/purplecode textures/custom/redcode 
        alphaFunc GE128
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_DST_ALPHA
    }
}

textures/custom/ramplight_yellow
{
	q3map_lightimage	textures/vjun/light_vjunamber_glow
	qer_editorimage	textures/vjun/light_vjunamber
	q3map_surfacelight	1000
	q3map_lightsubdivide	32
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/vjun/light_vjunamber
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/vjun/light_vjunamber_glow
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/custom/ramplight_white
{
	q3map_lightimage	textures/vjun/light_vjun_glw.tga
	qer_editorimage	textures/vjun/light_vjun.tga
	q3map_surfacelight	1250
	q3map_lightsubdivide	32
    {
        map $lightmap
        rgbGen identity
    }
    {
        map textures/vjun/light_vjun
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/vjun/light_vjun_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen identity
    }
}

textures/custom/panelwall1
{
    {
        map $lightmap
    }
    {
        map textures/custom/panelwall1
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/custom/panelwall1_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave square 0 1 0 0.5
    }
}

textures/custom/wallpanel07
{
	q3map_lightimage	textures/custom/wallpanel07
	q3map_surfacelight	200
	q3map_lightStyle 1
    {
        map $lightmap
    }
    {
        map textures/custom/wallpanel07_glw
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        glow
        rgbGen wave noise 0.5 0.05 1 100
    }
    {
        map textures/custom/wallpanel07
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

