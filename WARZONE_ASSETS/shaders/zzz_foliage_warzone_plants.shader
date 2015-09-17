//
//
// Grasses...
//
//

models/map_objects/yavin/grass
{
  qer_editorimage	models/map_objects/yavin/grass
	//q3map_nolightmap
	q3map_alphashadow
	//q3map_onlyvertexlighting
	q3map_material	GreenLeaves
	
	surfaceparm trans
	surfaceparm	noimpact
	surfaceparm	nomarks
	//surfaceparm	nonopaque
	
	sort seethrough
	
	//cull	twosided
	
  {
    map models/map_objects/yavin/grass
    blendfunc GL_ONE GL_ZERO
    alphaFunc GE128
    //rgbGen lightingDiffuse
    depthWrite // writes the alpha map for other stages to use
    rgbGen identity
  }
  {
    map $lightmap
    blendfunc GL_DST_COLOR GL_ZERO
    rgbGen identity
    depthFunc equal // sets the alpha to the same as the stages that wrote it
  }
}

models/map_objects/yavin/grass_tall
{
	qer_editorimage	models/map_objects/yavin/grass_tall
	//q3map_nolightmap
	q3map_alphashadow
	//q3map_onlyvertexlighting
	q3map_material	GreenLeaves
	
	surfaceparm trans
	surfaceparm	noimpact
	surfaceparm	nomarks
	//surfaceparm	nonopaque
	
	sort seethrough
	
	//cull	twosided
	
  {
    map models/map_objects/yavin/grass_tall
    blendfunc GL_ONE GL_ZERO
    alphaFunc GE128
    //rgbGen lightingDiffuse
    depthWrite // writes the alpha map for other stages to use
    rgbGen identity
  }
  {
    map $lightmap
    blendfunc GL_DST_COLOR GL_ZERO
    rgbGen identity
    depthFunc equal // sets the alpha to the same as the stages that wrote it
  }
}

models/map_objects/yavin/grass_b
{
	qer_editorimage	models/map_objects/yavin/grass.png
	//q3map_nolightmap
	q3map_alphashadow
	//q3map_onlyvertexlighting
	surfaceparm trans
	surfaceparm	noimpact
	surfaceparm	nomarks
	//surfaceparm	nonopaque
	
	sort seethrough
	
	//cull	twosided
	
  {
    map models/map_objects/yavin/grass.png
    blendfunc GL_ONE GL_ZERO
    alphaFunc GE128
    //rgbGen lightingDiffuse
    depthWrite // writes the alpha map for other stages to use
    rgbGen identity
  }
  {
    map $lightmap
    rgbGen identity
    blendfunc GL_DST_COLOR GL_ZERO
    depthFunc equal // sets the alpha to the same as the stages that wrote it
  }
}

models/map_objects/yavin/grass_tall_b
{
	qer_editorimage	models/map_objects/yavin/grass_tall.png
	//q3map_nolightmap
	q3map_alphashadow
	//q3map_onlyvertexlighting
	surfaceparm trans
	surfaceparm	noimpact
	surfaceparm	nomarks
	//surfaceparm	nonopaque
	
	sort seethrough
	
	//cull	twosided
	
  {
    map models/map_objects/yavin/grass_tall.png
    blendfunc GL_ONE GL_ZERO
    alphaFunc GE128
    //rgbGen lightingDiffuse
    depthWrite // writes the alpha map for other stages to use
    rgbGen identity
  }
  {
    map $lightmap
    rgbGen identity
    blendfunc GL_DST_COLOR GL_ZERO
    depthFunc equal // sets the alpha to the same as the stages that wrote it
  }
}

models/pop/foliages/alp_weedy_c
{
	qer_editorimage	models/pop/foliages/alp_weedy_c.png
	//q3map_nolightmap
	q3map_alphashadow
	//q3map_onlyvertexlighting
	q3map_material	GreenLeaves
	
	surfaceparm trans
	surfaceparm	noimpact
	surfaceparm	nomarks
	//surfaceparm	nonopaque
	
	sort seethrough
	
	//cull	twosided
	
  {
    map models/pop/foliages/alp_weedy_c.png
    blendfunc GL_ONE GL_ZERO
    alphaFunc GE128
    //rgbGen lightingDiffuse
    depthWrite // writes the alpha map for other stages to use
    rgbGen identity
  }
  {
    map $lightmap
    rgbGen identity
    blendfunc GL_DST_COLOR GL_ZERO
    depthFunc equal // sets the alpha to the same as the stages that wrote it
  }
}

models/pop/foliages/sch_weed_a
{
	qer_editorimage	models/pop/foliages/sch_weed_a.png
	//q3map_nolightmap
	q3map_alphashadow
	//q3map_onlyvertexlighting
	q3map_material	GreenLeaves
	
	surfaceparm trans
	surfaceparm	noimpact
	surfaceparm	nomarks
	//surfaceparm	nonopaque
	
	sort seethrough
	
	//cull	twosided
	
  {
    map models/pop/foliages/sch_weed_a.png
    blendfunc GL_ONE GL_ZERO
    alphaFunc GE128
    //rgbGen lightingDiffuse
    depthWrite // writes the alpha map for other stages to use
    rgbGen identity
  }
  {
    map $lightmap
    rgbGen identity
    blendfunc GL_DST_COLOR GL_ZERO
    depthFunc equal // sets the alpha to the same as the stages that wrote it
  }
}

models/pop/foliages/sch_weed_b
{
	qer_editorimage	models/pop/foliages/sch_weed_b.png
	//q3map_nolightmap
	q3map_alphashadow
	//q3map_onlyvertexlighting
	q3map_material	GreenLeaves
	
	surfaceparm trans
	surfaceparm	noimpact
	surfaceparm	nomarks
	//surfaceparm	nonopaque
	
	sort seethrough
	
	//cull	twosided
	
  {
    map models/pop/foliages/sch_weed_b.png
    blendfunc GL_ONE GL_ZERO
    alphaFunc GE128
    //rgbGen lightingDiffuse
    depthWrite // writes the alpha map for other stages to use
    rgbGen identity
  }
  {
    map $lightmap
    rgbGen identity
    blendfunc GL_DST_COLOR GL_ZERO
    depthFunc equal // sets the alpha to the same as the stages that wrote it
  }
}

models/pop/foliages/pop_flower_a
{
  qer_editorimage	models/pop/foliages/pop_flower_a.png
	//q3map_nolightmap
	q3map_alphashadow
	//q3map_onlyvertexlighting
	q3map_material	GreenLeaves
	
	surfaceparm trans
	surfaceparm	noimpact
	surfaceparm	nomarks
	//surfaceparm	nonopaque
	
	sort seethrough
	
	//cull	twosided
	
  {
    map models/pop/foliages/pop_flower_a.png
    blendfunc GL_ONE GL_ZERO
    alphaFunc GE128
    //rgbGen lightingDiffuse
    depthWrite // writes the alpha map for other stages to use
    rgbGen identity
  }
  {
    map $lightmap
    rgbGen identity
    blendfunc GL_DST_COLOR GL_ZERO
    depthFunc equal // sets the alpha to the same as the stages that wrote it
  }
}

models/pop/foliages/pop_flower_b
{
  qer_editorimage	models/pop/foliages/pop_flower_b.png
	//q3map_nolightmap
	q3map_alphashadow
	//q3map_onlyvertexlighting
	q3map_material	GreenLeaves
	
	surfaceparm trans
	surfaceparm	noimpact
	surfaceparm	nomarks
	//surfaceparm	nonopaque
	
	sort seethrough
	
	//cull	twosided
	
  {
    map models/pop/foliages/pop_flower_b.png
    blendfunc GL_ONE GL_ZERO
    alphaFunc GE128
    //rgbGen lightingDiffuse
    depthWrite // writes the alpha map for other stages to use
    rgbGen identity
  }
  {
    map $lightmap
    rgbGen identity
    blendfunc GL_DST_COLOR GL_ZERO
    depthFunc equal // sets the alpha to the same as the stages that wrote it
  }
}

models/pop/foliages/pop_flower_c
{
  qer_editorimage	models/pop/foliages/pop_flower_c.png
	//q3map_nolightmap
	q3map_alphashadow
	//q3map_onlyvertexlighting
	q3map_material	GreenLeaves
	
	surfaceparm trans
	surfaceparm	noimpact
	surfaceparm	nomarks
	//surfaceparm	nonopaque
	
	sort seethrough
	
	//cull	twosided
	
  {
    map models/pop/foliages/pop_flower_c.png
    blendfunc GL_ONE GL_ZERO
    alphaFunc GE128
    //rgbGen lightingDiffuse
    depthWrite // writes the alpha map for other stages to use
    rgbGen identity
  }
  {
    map $lightmap
    blendfunc GL_DST_COLOR GL_ZERO
    rgbGen identity
    depthFunc equal // sets the alpha to the same as the stages that wrote it
  }
}

//
//
// Ferns...
//
//

models/map_objects/yavin/fern
{
  qer_editorimage	models/map_objects/yavin/fern.png
	//q3map_nolightmap
	q3map_alphashadow
	//q3map_onlyvertexlighting
	q3map_material	GreenLeaves
	
	surfaceparm trans
	surfaceparm	noimpact
	surfaceparm	nomarks
	//surfaceparm	nonopaque
	
	sort seethrough
	
	cull	twosided
  {
    map models/map_objects/yavin/fern.png
    blendfunc GL_ONE GL_ZERO
    alphaFunc GE128
    //rgbGen lightingDiffuse
    depthWrite // writes the alpha map for other stages to use
    rgbGen identity
  }
  {
    map $lightmap
    blendfunc GL_DST_COLOR GL_ZERO
    rgbGen identity
    depthFunc equal // sets the alpha to the same as the stages that wrote it
  }
}

models/map_objects/yavin/fern2
{
  qer_editorimage	models/map_objects/yavin/fern2.png
	//q3map_nolightmap
	q3map_alphashadow
	//q3map_onlyvertexlighting
	q3map_material	GreenLeaves
	
	surfaceparm trans
	surfaceparm	noimpact
	surfaceparm	nomarks
	//surfaceparm	nonopaque
	
	sort seethrough
	
	cull	twosided
  {
    map models/map_objects/yavin/fern2.png
    blendfunc GL_ONE GL_ZERO
    alphaFunc GE128
    //rgbGen lightingDiffuse
    depthWrite // writes the alpha map for other stages to use
    rgbGen identity
  }
  {
    map $lightmap
    blendfunc GL_DST_COLOR GL_ZERO
    rgbGen identity
    depthFunc equal // sets the alpha to the same as the stages that wrote it
  }
}

models/map_objects/yavin/fern3
{
	qer_editorimage	models/map_objects/yavin/fern3.png
	//q3map_nolightmap
	q3map_alphashadow
	//q3map_onlyvertexlighting
	q3map_material	GreenLeaves
	
	surfaceparm trans
	surfaceparm	noimpact
	surfaceparm	nomarks
	//surfaceparm	nonopaque
	
	sort seethrough
	
	cull	twosided
  {
    map models/map_objects/yavin/fern3.png
    blendfunc GL_ONE GL_ZERO
    alphaFunc GE128
    //rgbGen lightingDiffuse
    depthWrite // writes the alpha map for other stages to use
    rgbGen identity
  }
  {
    map models/map_objects/yavin/fern3_spec
    blendFunc GL_SRC_ALPHA GL_ONE
    detail
    alphaGen lightingSpecular
    depthFunc equal // sets the alpha to the same as the stages that wrote it
  }
  {
    map $lightmap
    blendfunc GL_DST_COLOR GL_ZERO
    rgbGen identity
    depthFunc equal // sets the alpha to the same as the stages that wrote it
  }
}

models/map_objects/yavin/fern3_b
{
	qer_editorimage	models/map_objects/yavin/fern3b.png
	//q3map_nolightmap
	q3map_alphashadow
	//q3map_onlyvertexlighting
	q3map_material	GreenLeaves
	
	surfaceparm trans
	surfaceparm	noimpact
	surfaceparm	nomarks
	//surfaceparm	nonopaque
	
	sort seethrough
	
	cull	twosided
  {
    map models/map_objects/yavin/fern3b.png
    blendfunc GL_ONE GL_ZERO
    alphaFunc GE128
    //rgbGen lightingDiffuse
    depthWrite // writes the alpha map for other stages to use
    rgbGen identity
  }
  {
    map models/map_objects/yavin/fern3_spec
    blendFunc GL_SRC_ALPHA GL_ONE
    detail
    alphaGen lightingSpecular
    depthFunc equal // sets the alpha to the same as the stages that wrote it
  }
  {
    map $lightmap
    blendfunc GL_DST_COLOR GL_ZERO
    rgbGen identity
    depthFunc equal // sets the alpha to the same as the stages that wrote it
  }
}

//
//
// Plants...
//
//

models/map_objects/yavin/plant
{
  qer_editorimage	models/map_objects/yavin/plant.png
	//q3map_nolightmap
	q3map_alphashadow
	//q3map_onlyvertexlighting
	q3map_material	GreenLeaves
	
	surfaceparm trans
	surfaceparm	noimpact
	surfaceparm	nomarks
	//surfaceparm	nonopaque
	
	sort seethrough
	
	cull	twosided
  {
    map models/map_objects/yavin/plant.png
    blendfunc GL_ONE GL_ZERO
    alphaFunc GE128
    //rgbGen lightingDiffuse
    depthWrite // writes the alpha map for other stages to use
    rgbGen identity
  }
  {
    map $lightmap
    blendfunc GL_DST_COLOR GL_ZERO
    rgbGen identity
    depthFunc equal // sets the alpha to the same as the stages that wrote it
  }
}

models/map_objects/yavin/plant_b
{
	qer_editorimage	models/map_objects/yavin/plant.png
	//q3map_nolightmap
	q3map_alphashadow
	//q3map_onlyvertexlighting
	q3map_material	GreenLeaves
	
	surfaceparm trans
	surfaceparm	noimpact
	surfaceparm	nomarks
	//surfaceparm	nonopaque
	
	sort seethrough
	
	cull	twosided
  {
    map models/map_objects/yavin/plant.png
    blendfunc GL_ONE GL_ZERO
    alphaFunc GE128
    //rgbGen lightingDiffuse
    depthWrite // writes the alpha map for other stages to use
    rgbGen identity
  }
  {
    map $lightmap
    blendfunc GL_DST_COLOR GL_ZERO
    rgbGen identity
    depthFunc equal // sets the alpha to the same as the stages that wrote it
  }
}

//
//
// Vines...
//
//

models/map_objects/yavin/vines
{
	qer_editorimage	models/map_objects/yavin/vines.png
	//q3map_nolightmap
	q3map_alphashadow
	//q3map_onlyvertexlighting
	q3map_material	GreenLeaves
	
	surfaceparm trans
	surfaceparm	noimpact
	surfaceparm	nomarks
	//surfaceparm	nonopaque
	
	sort seethrough
	
	cull	twosided
  {
    map models/map_objects/yavin/vines.png
    blendfunc GL_ONE GL_ZERO
    alphaFunc GE128
    //rgbGen lightingDiffuse
    depthWrite // writes the alpha map for other stages to use
    rgbGen identity
  }
  {
    map $lightmap
    blendfunc GL_DST_COLOR GL_ZERO
    rgbGen identity
    depthFunc equal // sets the alpha to the same as the stages that wrote it
  }
}

models/map_objects/yavin/vines_b
{
	qer_editorimage	models/map_objects/yavin/vines.png
	//q3map_nolightmap
	q3map_alphashadow
	//q3map_onlyvertexlighting
	q3map_material	GreenLeaves
	
	surfaceparm trans
	surfaceparm	noimpact
	surfaceparm	nomarks
	//surfaceparm	nonopaque
	
	sort seethrough
	
	cull	twosided
  {
    map models/map_objects/yavin/vines.png
    blendfunc GL_ONE GL_ZERO
    alphaFunc GE128
    //rgbGen lightingDiffuse
    depthWrite // writes the alpha map for other stages to use
    rgbGen identity
  }
  {
    map $lightmap
    blendfunc GL_DST_COLOR GL_ZERO
    rgbGen identity
    depthFunc equal // sets the alpha to the same as the stages that wrote it
  }
}
