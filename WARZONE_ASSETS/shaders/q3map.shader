//
// q3map debug shaders 
//

//
// q3map lightgrid bounds
//
// the min/max bounds of brushes with this shader in a map
// will define the bounds of the map's lightgrid (model lighting)
// note: make it as small as possible around player space
// to minimize bsp size and compile time
//

textures/common/lightgrid
{
	qer_trans 0.5
	surfaceparm nodraw
	surfaceparm nolightmap
	surfaceparm nonsolid
	surfaceparm detail
	surfaceparm nomarks
	surfaceparm trans
	surfaceparm lightgrid
}

// enable with -debugsurfaces switch
debugsurfaces
{
	surfaceparm trans
	surfaceparm nolightmap
	surfaceparm nonsolid
	surfaceparm noimpact
	surfaceparm nomarks
	sort opaque
	{
		map *default
		rgbGen vertex
	}
}

// enable with -debugportals switch
debugportals
{
	surfaceparm trans
	surfaceparm nolightmap
	surfaceparm nonsolid
	surfaceparm noimpact
	surfaceparm nomarks
	sort additive
	cull none
	{
		map $whiteimage
		blendFunc GL_SRC_ALPHA GL_ONE
		rgbGen vertex
	}
}

