textures/bespin/sky
{
	qer_editorimage	textures/skies/sky.tga
	q3map_surfacelight	450
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/bespin 512 -
}

textures/bespin/sky_platform
{
	qer_editorimage	textures/skies/sky.tga
	q3map_surfacelight	250
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	textures/skies/bespin 512 -
}

textures/bespin/windowblue
{
	surfaceparm	metalsteps
    {
        map $lightmap
    }
    {
        map textures/bespin/windowblue
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/windowblue_glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/u_light01
{
    {
        map $lightmap
    }
    {
        map textures/bespin/u_light01
        blendFunc GL_DST_COLOR GL_ZERO
        rgbGen identity
    }
    {
        map textures/bespin/u_lightglow01
        blendFunc GL_ONE GL_ONE
        rgbGen identity
    }
}

textures/bespin/u_light02
{
    {
        map $lightmap
    }
    {
        map textures/bespin/u_light02
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/u_light02glow
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.1 0 0.5
    }
}

textures/bespin/u_light02a
{
	surfaceparm	metalsteps
    {
        map $lightmap
    }
    {
        map textures/bespin/u_light02a
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/u_light02aglow
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.1 0 0.5
    }
}

textures/bespin/u_light03
{
	qer_editorimage	textures/bespin/u_light03
	surfaceparm	metalsteps
    {
        map $lightmap
    }
    {
        map textures/bespin/u_light03
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/u_light03glow
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.1 0 0.5
    }
}

textures/bespin/u_light04
{
    {
        map $lightmap
    }
    {
        map textures/bespin/u_light04
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/u_light04glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/botton_on
{
    {
        map $lightmap
    }
    {
        map textures/bespin/botton
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/botton_on2
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/clouds
{
	surfaceparm	noimpact
	surfaceparm	nomarks
	surfaceparm	nonsolid
	q3map_nolightmap
    {
        map textures/bespin/clouds
        blendFunc GL_ONE GL_ZERO
        tcMod scroll 0.02 0.04
        tcMod turb 0 0.1 0 0.03
    }
}

textures/bespin/n_win01
{
// q3map_surfacelight	500

	surfaceparm	nomarks
	polygonOffset
	q3map_nolightmap
    {
        map textures/bespin/n_win01
        alphaFunc GT0
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        rgbGen identity
    }
    {
        map textures/bespin/n_win01glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/n_win02
{
// q3map_surfacelight	500

	surfaceparm	nomarks
	polygonOffset
	q3map_nolightmap
    {
        map textures/bespin/n_win02
        alphaFunc GT0
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
    {
        map textures/bespin/n_win02glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/n_win03
{
// q3map_surfacelight	500

	polygonOffset
	q3map_nolightmap
    {
        map textures/bespin/n_win03
        alphaFunc GT0
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
    {
        map textures/bespin/n_win03glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/mp_r_light04
{
	surfaceparm	metalsteps
    {
        map $lightmap
    }
    {
        map textures/bespin/mp_r_light04
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/mp_r_light04_glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/mp_b_light04
{
	surfaceparm	metalsteps
    {
        map $lightmap
    }
    {
        map textures/bespin/mp_b_light04
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/mp_b_light04_glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/mp_r_light02a
{
	surfaceparm	metalsteps
    {
        map $lightmap
    }
    {
        map textures/bespin/mp_r_light02a
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/mp_r_light02a_glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/mp_b_light02a
{
	surfaceparm	metalsteps
    {
        map $lightmap
    }
    {
        map textures/bespin/mp_b_light02a
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/mp_b_light02a_glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/botton2
{
    {
        map $lightmap
    }
    {
        map textures/bespin/botton2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/botton2glow
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/botton2on
{
	qer_editorimage	textures/bespin/botton2
	surfaceparm	metalsteps
    {
        map $lightmap
    }
    {
        map textures/bespin/botton2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/botton2glow2
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/bottondroidopen
{
	qer_editorimage	textures/bespin/bottondroid
    {
        map $lightmap
    }
    {
        map textures/bespin/bottondroid
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/bottondroidglow
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/n_light01
{
	surfaceparm	metalsteps
	polygonOffset
    {
        map $lightmap
    }
    {
        map textures/bespin/n_light01
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/n_light01_glw
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/bcameraswitch
{
	qer_editorimage	textures/bespin/bcameraswitch
	surfaceparm	metalsteps
    {
        map gfx/effects/decoystatic
        blendFunc GL_ONE GL_ZERO
        rgbGen wave noise 1 0.5 0 5
        tcMod scroll 5 9
        tcMod scale 7 2
    }
    {
        map gfx/effects/decoystatic
        blendFunc GL_ONE GL_ONE
        rgbGen wave noise 1 0.25 -0.5 3
        tcMod scroll -2 -2
        tcMod scale 7 9
    }
    {
        map textures/bespin/bcameraswitch
        blendFunc GL_ONE GL_SRC_ALPHA
    }
    {
        map textures/bespin/bcameraswitchglow
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 0 1
    }
    {
        map $lightmap
        blendFunc GL_DST_COLOR GL_SRC_COLOR
    }
}

textures/bespin/bswitch
{
    {
        map $lightmap
    }
    {
        map textures/bespin/bswitch
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/bswitcha
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 0 1
    }
    {
        map textures/bespin/bswitchx
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.1 0 5
    }
}

textures/bespin/bswitchon
{
	qer_editorimage	textures/bespin/bswitch
	surfaceparm	metalsteps
    {
        map $lightmap
    }
    {
        map textures/bespin/bswitch2
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/bswitchb
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 1 0.1 0 5
    }
}

textures/bespin/botton_off
{
	qer_editorimage	textures/bespin/botton
	surfaceparm	metalsteps
    {
        map $lightmap
    }
    {
        map textures/bespin/botton
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/botton_on
        blendFunc GL_ONE GL_ONE
        rgbGen wave sin 0 1 0 1
    }
}

textures/bespin/clouds_bottom
{
	surfaceparm	metalsteps
	q3map_nolightmap
    {
        map textures/bespin/clouds_bottom
        blendFunc GL_ONE GL_ZERO
        tcMod scroll 0.15 0
    }
    {
        map textures/bespin/clouds_mid
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcMod scroll 0.25 0
    }
    {
        map textures/bespin/clouds_top
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        detail
        tcMod scroll 0.55 0
    }
}

textures/bespin/control01
{
	surfaceparm	metalsteps
    {
        map $lightmap
    }
    {
        map textures/bespin/control01
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/control01_glw
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/control02
{
	qer_editorimage	textures/bespin/control02
    {
        map $lightmap
    }
    {
        map textures/bespin/control02
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/control02_glw
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/control03
{
	qer_editorimage	textures/bespin/control03
    {
        map $lightmap
    }
    {
        map textures/bespin/control03
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/control03_glw
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/control04
{
	qer_editorimage	textures/bespin/control04
	surfaceparm	metalsteps
    {
        map $lightmap
    }
    {
        map textures/bespin/control04
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/bespin/control04_glw
        blendFunc GL_ONE GL_ONE
    }
}

textures/bespin/water1
{
	surfaceparm	metalsteps
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	water
	surfaceparm	fog
	surfaceparm	trans
	q3map_material	Water
	q3map_nolightmap
	q3map_onlyvertexlighting
	fogparms	( 0.14902 0.184314 0.494118 ) 512.0
    {
        map textures/bespin/water1
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        rgbGen exactVertex
        alphaGen const 0.2
        tcMod scroll 0.005 0.01
        tcMod turb 1 0.01 0 0.1
    }
    {
        map textures/bespin/water1
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        alphaGen vertex
    }
    {
        map textures/bespin/water1c
        blendFunc GL_DST_COLOR GL_ONE
        tcMod scroll -0.005 -0.01
        tcMod turb 1 0.01 0 0.1
    }
}

textures/bespin/water2
{
	qer_editorimage	textures/bespin/water1
	surfaceparm	metalsteps
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	water
	surfaceparm	fog
	surfaceparm	trans
	q3map_material	Water
	q3map_nolightmap
	q3map_onlyvertexlighting
	fogparms	( 0.113725 0.137255 0.380392 ) 1024.0
    {
        map textures/bespin/water1
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        rgbGen exactVertex
        alphaGen const 0.2
        tcMod scroll 0.005 0.01
        tcMod turb 1 0.03 0 0.3
    }
    {
        map textures/bespin/water1
        blendFunc GL_ONE GL_SRC_ALPHA
        rgbGen exactVertex
        alphaGen const 0.2
        tcMod scroll -0.005 -0.01
        tcMod turb 0 -0.03 0.5 -0.3
    }
}
