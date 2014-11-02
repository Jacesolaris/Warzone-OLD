textures/stevetest/bwater
{
	qer_editorimage	textures/stevetest/dest1green1
	surfaceparm	nonsolid
	surfaceparm	water
	q3map_nolightmap
    {
        map textures/stevetest/waterbase
        tcMod rotate -0.5
        tcMod stretch sin 1 0.05 0 0.1
    }
    {
        map textures/stevetest/waterlayer
        blendFunc GL_ONE_MINUS_SRC_ALPHA GL_ONE
        tcMod rotate 5
        tcMod stretch sin 1 0.1 0 0.2
    }
    {
        map textures/stevetest/watertop
        blendFunc GL_SRC_ALPHA GL_ONE
        tcMod stretch sin 1 0.1 0 0.1
        tcMod turb 0 0.3 0 0.03
    }
}

textures/stevetest/lring
{
	qer_editorimage	textures/stevetest/lring
	q3map_nolightmap
    {
        map textures/stevetest/lring
        tcMod rotate 200
    }
    {
        map textures/stevetest/spokes1
        blendFunc GL_ONE GL_ONE
        tcMod rotate -50
        tcMod stretch sin 1 0.05 0 4
    }
    {
        map textures/stevetest/ringmask1
        blendFunc GL_ONE GL_SRC_ALPHA
    }
}

textures/stevetest/wtr1
{
	qer_editorimage	textures/stevetest/wtr1
	q3map_nolightmap
    {
        map textures/stevetest/waterbase
        tcMod stretch sin 1 0.2 0 0.1
    }
    {
        animMap 5 textures/stevetest/wtr1 textures/stevetest/wtr2 textures/stevetest/wtr3 textures/stevetest/wtr4 textures/stevetest/wtr5 textures/stevetest/wtr6 textures/stevetest/wtr7 textures/stevetest/wtr8 
        blendFunc GL_ONE_MINUS_DST_COLOR GL_ZERO
        tcMod scroll 0.05 0
    }
    {
        map textures/stevetest/watertop
        blendFunc GL_SRC_ALPHA GL_ONE
        tcMod turb 1 0.05 0 0.2
    }
}

textures/stevetest/hole
{
	qer_editorimage	textures/stevetest/power18a
	q3map_nolightmap
    {
        map textures/stevetest/power18a
        tcMod scroll 0 1
    }
    {
        map textures/stevetest/power18a
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcMod stretch sin 1 0.07 0 1
        tcMod scroll 0 -0.7
    }
    {
        map textures/stevetest/llinep
        blendFunc GL_ONE GL_ONE
        tcMod stretch sawtooth 1 0.2 0 1
        tcMod scroll 3 1
    }
}

textures/stevetest/p1
{
	q3map_nolightmap
    {
        map textures/stevetest/p1
        tcMod scroll 0 4
    }
    {
        map textures/stevetest/p1a
        blendFunc GL_ONE_MINUS_SRC_ALPHA GL_SRC_COLOR
        tcMod scroll 4 0
    }
}

textures/stevetest/metal1
{
    {
        map $lightmap
    }
    {
        map textures/stevetest/metal1
        blendFunc GL_DST_COLOR GL_ZERO
        tcMod stretch sin 1 0.02 0 0.1
    }
    {
        map textures/stevetest/metal2
        blendFunc GL_SRC_ALPHA GL_SRC_ALPHA
        tcMod turb 1 0.05 0 0.02
    }
}

textures/stevetest/power8
{
	q3map_nolightmap
    {
        map textures/stevetest/power26
        tcMod scroll -1 0
    }
    {
        map textures/stevetest/power8
        blendFunc GL_SRC_ALPHA GL_SRC_COLOR
        tcMod scroll 5 0
    }
}

textures/stevetest/power23
{
	q3map_nolightmap
    {
        map textures/stevetest/power23
        tcMod scroll 0 1
    }
    {
        map textures/stevetest/power26
        blendFunc GL_ONE_MINUS_DST_COLOR GL_ONE
        tcMod scroll 0 -2
    }
}

textures/stevetest/llinep
{
	q3map_nolightmap
    {
        map textures/stevetest/llinep
        tcMod stretch sawtooth 1 2 0 20
        tcMod scroll 10 0
    }
    {
        map textures/stevetest/lline
        blendFunc GL_DST_COLOR GL_ONE
        tcMod stretch sin 1 0.5 0 20
        tcMod scroll -20 0
    }
}

textures/stevetest/bacbase
{
	surfaceparm	nomarks
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map textures/stevetest/bac_liquid
        blendFunc GL_ONE GL_SRC_ALPHA
        tcMod stretch sin 1 0.2 0 0.03
        tcMod scroll 0 0.05
    }
    {
        map textures/stevetest/bac_small
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcMod scroll 0 0.45
    }
    {
        map textures/stevetest/bac_medium
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcMod turb 1 0.05 0 0.08
        tcMod scroll 0 0.2
    }
    {
        map textures/stevetest/bac_large
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcMod turb 1 0.05 0 0.1
        tcMod scroll 0 0.1
    }
}

textures/stevetest/leadstuff
{
    {
        map textures/stevetest/e8
        tcMod turb 1 0.05 0 0.2
        tcMod stretch sin 1 0.05 0 0.03
        tcMod scroll 0.05 0
    }
    {
        map textures/stevetest/leadstuff
        blendFunc GL_ONE_MINUS_SRC_ALPHA GL_SRC_ALPHA
        tcMod turb 1 0.02 0 0.03
        tcMod stretch sin 1 0.02 0 0.08
        tcMod scroll 0 0.02
    }
}

textures/stevetest/power39
{
	q3map_nolightmap
    {
        map textures/stevetest/power39
        tcMod scroll 0 3
    }
    {
        map textures/stevetest/power37
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcMod scroll 0 -4
    }
}

textures/stevetest/power32
{
	q3map_nolightmap
    {
        map textures/stevetest/power32
        tcMod scroll 0 2
    }
    {
        map textures/stevetest/power38
        blendFunc GL_DST_COLOR GL_SRC_ALPHA
        tcMod scroll 0 4
        tcMod scale 4 1
    }
}

textures/stevetest/rmask
{
	q3map_nolightmap
    {
        map textures/stevetest/power18a
        tcMod rotate 20
        tcMod scroll 0 10
    }
    {
        map textures/stevetest/llinep
        blendFunc GL_ONE_MINUS_DST_COLOR GL_ONE
        tcMod rotate 300
        tcMod scroll 3 0
    }
    {
        map textures/stevetest/rmask
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcMod stretch sin 1 0.05 0 10
    }
}

textures/stevetest/ring
{
	qer_editorimage	textures/stevetest/power18a
	q3map_nolightmap
    {
        map textures/stevetest/power18a
        tcMod rotate 10
        tcMod scroll 0 10
    }
    {
        map textures/stevetest/llinep
        blendFunc GL_ONE_MINUS_DST_COLOR GL_ONE
        tcMod rotate 300
        tcMod scroll 3 0
    }
    {
        map textures/stevetest/llinep
        blendFunc GL_ONE_MINUS_DST_COLOR GL_ONE
        tcMod rotate 100
        tcMod scroll 3 0
    }
    {
        map textures/stevetest/rmask
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcMod stretch sin 1 0.05 0 7
    }
}

textures/stevetest/beam1
{
	qer_editorimage	textures/stevetest/power32
	surfaceparm	nonopaque
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map textures/stevetest/power32
        tcMod scroll 1 4
    }
    {
        map textures/stevetest/power38
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcMod scale 4 1
        tcMod scroll -1 1
    }
}

textures/stevetest/Hothwall
{
	qer_editorimage	textures/stevetest/wall10
    {
        map $lightmap
    }
    {
        map textures/stevetest/wall10
        blendFunc GL_DST_COLOR GL_ZERO
    }
    {
        map textures/stevetest/cloude
        blendFunc GL_DST_COLOR GL_SRC_ALPHA
        tcGen environment
        tcMod scale 1 0.05
        tcMod transform 1 0 -45 1 0 0
    }
}

textures/stevetest/vlline
{
	q3map_nolightmap
    {
        map textures/stevetest/vlline
        blendFunc GL_SRC_ALPHA GL_ONE
    }
}

textures/stevetest/Clouds
{
	qer_editorimage	textures/stevetest/cloudlayer3
	surfaceparm	nodamage
	surfaceparm	noimpact
	notc
	q3map_nolightmap
    {
        map textures/stevetest/cloudlayer3
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0.005 0
    }
    {
        map textures/stevetest/cloudlayer2
        blendFunc GL_SRC_ALPHA GL_SRC_COLOR
        tcMod scroll 0.03 0
    }
}

textures/stevetest/bolt1
{
	q3map_nolightmap
    {
        animMap 20 textures/stevetest/bolt1 textures/stevetest/bolt2 textures/stevetest/bolt3 
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 -1
    }
}

textures/stevetest/nb1
{
	q3map_nolightmap
    {
        animMap 20 textures/stevetest/nb1 textures/stevetest/nb2 textures/stevetest/nb3 textures/stevetest/nb4 
        blendFunc GL_ONE GL_ONE
    }
}

textures/stevetest/dr1
{
	q3map_nolightmap
	cull	twosided
    {
        map textures/stevetest/dr1
        tcMod scroll 0 -3
    }
}

textures/stevetest/pbake0000
{
	q3map_nolightmap
    {
        map textures/stevetest/ggoo1
        tcMod stretch sin 1 0.02 0 1
        tcMod scroll 0.1 0
    }
    {
        map textures/stevetest/ggoo
        blendFunc GL_ONE_MINUS_SRC_ALPHA GL_ONE_MINUS_SRC_COLOR
        tcMod scroll 0.3 0
    }
    {
        map textures/stevetest/pbake0000
        blendFunc GL_ONE GL_SRC_ALPHA
    }
}

textures/stevetest/colorb
{
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map textures/stevetest/colorb1
        blendFunc GL_ONE GL_ONE
        tcMod scale 4 1
        tcMod scroll 0 -2
    }
    {
        map textures/stevetest/colorb
        blendFunc GL_ONE_MINUS_SRC_ALPHA GL_SRC_ALPHA
        tcMod scale 4 1
        tcMod scroll 0 -4
    }
}

textures/stevetest/oglow1
{
	q3map_nolightmap
    {
        map textures/common/flava2
        tcMod scroll 0.01 0
    }
    {
        map textures/stevetest/f1
        blendFunc GL_SRC_ALPHA GL_ONE
        tcMod stretch sin 1 0.02 0 0.1
        tcMod scroll 0.025 0
    }
    {
        map textures/stevetest/crust2_a
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

textures/stevetest/pbake1
{
	qer_editorimage	textures/stevetest/ggoo1
	q3map_nolightmap
    {
        map textures/stevetest/ggoo1
        tcMod stretch sin 1 0.02 0 1
        tcMod scroll 0.1 0
    }
    {
        map textures/stevetest/ggoo3
        blendFunc GL_DST_COLOR GL_SRC_ALPHA
        tcMod scroll 0.3 0
    }
    {
        map textures/stevetest/pbake0000
        blendFunc GL_ONE GL_SRC_ALPHA
    }
}

textures/stevetest/w1
{
	q3map_nolightmap
    {
        map textures/stevetest/w4
        blendFunc GL_ONE GL_ONE
        tcMod turb 1 0.5 0.1 0.1
        tcMod scroll 0 -2
    }
    {
        map textures/stevetest/w5
        blendFunc GL_DST_COLOR GL_SRC_ALPHA
        tcMod scale 0.5 1
        tcMod scroll 0 -1
    }
    {
        map textures/stevetest/w2
        blendFunc GL_ONE_MINUS_SRC_ALPHA GL_SRC_COLOR
        tcMod turb 1 0.3 0.2 0.1
        tcMod scroll 0 -1.5
    }
    {
        map textures/common/water3_alpha
        blendFunc GL_ONE_MINUS_SRC_ALPHA GL_ONE
        tcMod scroll 0 -3
    }
}

textures/stevetest/W2
{
	qer_editorimage	textures/stevetest/w4
	q3map_nolightmap
    {
        map textures/stevetest/wfl
        tcMod scroll 0.02 -0.1
    }
    {
        map textures/stevetest/wfla
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcMod stretch sin 1 0.1 0 0.1
        tcMod transform 2 0 0 1 0 0
    }
    {
        map textures/stevetest/wfl
        blendFunc GL_ONE_MINUS_SRC_ALPHA GL_SRC_ALPHA
        tcMod scroll 0 -0.2
    }
}

textures/stevetest/W3
{
	qer_editorimage	textures/stevetest/wf2
	q3map_nolightmap
    {
        map textures/stevetest/wf2
        tcMod scroll 0.02 -0.1
    }
    {
        map textures/stevetest/wfla
        blendFunc GL_DST_COLOR GL_ONE_MINUS_SRC_ALPHA
        tcMod stretch sin 1 0.1 0 0.1
        tcMod transform 2 0 0 1 0 0
    }
    {
        map textures/stevetest/wf2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_COLOR
        tcMod scroll 0 -0.2
    }
}

textures/stevetest/W4
{
	qer_editorimage	textures/stevetest/wf3
	q3map_nolightmap
    {
        map textures/stevetest/wf3
        tcMod scroll 0.02 -0.1
    }
    {
        map textures/stevetest/wf2
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcMod stretch sin 1 0.1 0 0.1
        tcMod transform 2 0 0 1 0 0
    }
    {
        map textures/stevetest/wf2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_COLOR
        tcMod scroll 0 -0.2
    }
}

textures/stevetest/W3a
{
	qer_editorimage	textures/stevetest/wf2
	q3map_nolightmap
    {
        map textures/common/waterf1a
        tcMod scroll 0 -2
        tcMod scale 1 0.5
    }
    {
        map textures/common/waterf1
        blendFunc GL_DST_COLOR GL_SRC_ALPHA
        tcMod stretch sin 1 0.05 0 0.1
        tcMod scroll 0 -0.6
    }
}

textures/stevetest/W5
{
	qer_editorimage	textures/stevetest/wf3
	q3map_nolightmap
    {
        map textures/stevetest/wf3
        tcMod scroll 0.02 -0.1
    }
    {
        map textures/stevetest/wfn2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_COLOR
        tcMod scroll 0 -0.05
    }
    {
        map textures/stevetest/waterf1
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcMod scroll 0 -0.25
    }
}

textures/stevetest/hewater
{
	qer_editorimage	textures/stevetest/wf3
	q3map_nolightmap
    {
        map textures/stevetest/wf3
        tcMod scroll 0.02 -0.1
    }
    {
        map textures/stevetest/wfn2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_COLOR
        tcMod stretch sin 1 0.02 0 0.1
        tcMod turb 1 0.02 0 0.02
        tcMod scroll 0 -0.05
    }
    {
        map textures/stevetest/waterf1
        blendFunc GL_DST_COLOR GL_SRC_COLOR
        tcMod turb 1 0.01 0 0.01
        tcMod scroll 0 -0.15
    }
}

textures/stevetest/newb
{
	qer_editorimage	textures/stevetest/colorb1
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map textures/stevetest/atzera01
        tcMod scale 1 0.25
        tcMod scroll -0.5 1.5
    }
    {
        map textures/stevetest/atzera
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcMod scale 1 0.5
        tcMod scroll -0.25 2
    }
    {
        map textures/stevetest/ziflew1
        blendFunc GL_DST_COLOR GL_SRC_ALPHA
        tcMod scale 1 0.5
        tcMod scroll 0 2
    }
}

textures/stevetest/newb1
{
	qer_editorimage	textures/stevetest/colorb1
	surfaceparm	trans
	q3map_nolightmap
	cull	twosided
    {
        map textures/stevetest/ziflew2
        tcMod scroll 0 1.5
    }
    {
        map textures/stevetest/ziflew1
        blendFunc GL_DST_COLOR GL_SRC_ALPHA
        tcMod scroll 0 2
    }
}

textures/hoth/testhoth_0
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/hoth/lsnow1a
        rgbGen vertex
        tcMod scale 0.25 0.25
    }
}

textures/hoth/testhoth_1
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/hoth/snow_01
        rgbGen vertex
        tcMod scale 0.05 0.05
    }
}

textures/hoth/testhoth_2
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/hoth/h_outground_2
        rgbGen vertex
        tcMod scale 0.05 0.05
    }
}

textures/hoth/testhoth_0to1
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/hoth/lsnow1a
        rgbGen vertex
        alphaGen vertex
        tcMod scale 0.25 0.25
    }
    {
        map textures/hoth/snow_01
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
        tcMod scale 0.05 0.05
    }
}

textures/hoth/testhoth_0to2
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/hoth/lsnow1a
        rgbGen vertex
        alphaGen vertex
        tcMod scale 0.25 0.25
    }
    {
        map textures/hoth/h_outground_2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
        tcMod scale 0.05 0.05
    }
}

textures/hoth/testhoth_1to2
{
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/hoth/snow_01
        rgbGen vertex
        tcMod scale 0.05 0.05
    }
    {
        map textures/hoth/h_outground_2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        alphaGen vertex
        tcMod scale 0.05 0.05
    }
}

