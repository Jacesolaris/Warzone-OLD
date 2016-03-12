textures/impfact/shadowgrate
{
	qer_editorimage	textures/impfact/shadowgrate
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
        map textures/impfact/shadowgrate
        alphaFunc GE128
        blendFunc GL_DST_COLOR GL_ZERO
        depthWrite
    }
}

textures/impfact/floor01
{
    {
        map textures/impfact/floor01
    }
}

textures/impfact/wallpanel_b
{
    {
        map $lightmap
    }
    {
        map textures/impfact/wallpanel_b
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/impfact/wallpanel_b_glow
        blendFunc GL_ONE GL_ONE
        glow
    }
}

textures/impfact/bluefog_dense
{
	qer_editorimage	textures/impfact/ase_blueknob
	qer_nocarve
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	fog
	surfaceparm	trans
	q3map_nolightmap
	fogparms	( 0 0.03 0.15 ) 1000.0
}

textures/impfact/bluefog_thin
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

textures/impfact/codeblock
{
	qer_editorimage	textures/impfact/bluecode
	qer_nocarve
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        animMap 1 textures/impfact/bluecode textures/impfact/purplecode textures/impfact/redcode 
        alphaFunc GE128
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_DST_ALPHA
    }
}

textures/impfact/ramplight_yellow
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

textures/impfact/ramplight_white
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

textures/impfact/panelwall1
{
    {
        map $lightmap
    }
    {
        map textures/impfact/panelwall1
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/impfact/panelwall1_glw
        blendFunc GL_ONE GL_ONE
        glow
        rgbGen wave square 0 1 0 0.5
    }
}

textures/impfact/wallpanel07
{
	q3map_lightimage	textures/impfact/wallpanel07
	q3map_surfacelight	200
	q3map_lightStyle 1
    {
        map $lightmap
    }
    {
        map textures/impfact/wallpanel07_glw
        blendFunc GL_ONE GL_ONE_MINUS_SRC_COLOR
        glow
        rgbGen wave noise 0.5 0.05 1 100
    }
    {
        map textures/impfact/wallpanel07
        blendFunc GL_DST_COLOR GL_ZERO
    }
}

