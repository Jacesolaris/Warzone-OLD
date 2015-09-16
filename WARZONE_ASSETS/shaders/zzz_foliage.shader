textures/yavin/foliage_base
{
	q3map_material	GreenLeaves
	
	//surfaceparm pointlight
	surfaceparm trans
	surfaceparm	noimpact
	surfaceparm	nomarks
	
	distanceCull 480 1280 0.49
	sort seethrough
	
	cull disable

	q3map_foliage models/map_objects/yavin/grass_b.md3 0.7 256 0.5 12
	q3map_foliage models/map_objects/yavin/grass_tall_b.md3 0.6 256 0.25 11
	q3map_foliage models/pop/foliages/alp_weedy_c.md3 1.5 256 0.15 10
	q3map_foliage models/pop/foliages/sch_weed_a.md3 1.5 256 0.15 9
	q3map_foliage models/pop/foliages/sch_weed_b.md3 1.5 256 0.15 8
	q3map_foliage models/pop/foliages/pop_flower_a.md3 0.4 256 0.1 7
	q3map_foliage models/pop/foliages/pop_flower_b.md3 0.4 256 0.1 6
	q3map_foliage models/pop/foliages/pop_flower_c.md3 0.4 256 0.1 5
	
	//q3map_foliage models/map_objects/yavin/fern3_b.md3 1.0 384 0.1 4
	
	
	q3map_foliage models/map_objects/yavin/tree06_b.md3 0.9 256 0.01 3
	q3map_foliage models/map_objects/yavin/tree08_b.md3 0.9 256 0.01 2
	q3map_foliage models/map_objects/yavin/tree09_b.md3 0.7 256 0.01 1
	//q3map_foliage models/map_objects/yavin/tree10_b.md3 1.1 128 0.01 10
}

textures/yavin/groundfoliage
{
	q3map_baseshader textures/yavin/foliage_base
	qer_editorimage	textures/yavin/ground
	q3map_material	ShortGrass

  {
    map $lightmap
  }
  {
    map textures/yavin/ground
    blendFunc GL_DST_COLOR GL_ZERO
    rgbGen vertex
  }
}
