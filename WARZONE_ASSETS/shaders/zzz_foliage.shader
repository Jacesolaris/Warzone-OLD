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

	q3map_foliage models/pop/foliages/alp_weedy_c.md3 1.10 256 0.05 11
	q3map_foliage models/pop/foliages/sch_weed_a.md3 1.10 256 0.05 10
	q3map_foliage models/pop/foliages/sch_weed_b.md3 1.10 256 0.05 9

  q3map_foliage models/warzone/foliage/plant01.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant02.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant03.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant04.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant05.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant06.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant07.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant08.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant09.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant10.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant11.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant12.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant13.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant14.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant15.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant16.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant17.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant18.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant19.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant20.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant21.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant22.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant23.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant24.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant25.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant26.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant27.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant28.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant29.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant30.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant31.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant32.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant33.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant34.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant35.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant36.md3 1.0 256 0.005 5
	
	//q3map_foliage models/map_objects/yavin/tree06_b.md3 0.9 256 0.001 3
	q3map_foliage models/map_objects/yavin/tree08_b.md3 1.0 256 0.001 2
	q3map_foliage models/map_objects/yavin/tree08_b.md3 0.7 256 0.001 2
	q3map_foliage models/map_objects/yavin/tree08_b.md3 1.4 256 0.001 2
	//q3map_foliage models/map_objects/yavin/tree09_b.md3 0.7 256 0.01 1
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

textures/yavin/foliagedense_base
{
	q3map_material	GreenLeaves
	
	//surfaceparm pointlight
	surfaceparm trans
	surfaceparm	noimpact
	surfaceparm	nomarks
	
	distanceCull 480 1280 0.49
	sort seethrough
	
	cull disable

	q3map_foliage models/pop/foliages/alp_weedy_c.md3 1.10 256 0.25 11
	q3map_foliage models/pop/foliages/sch_weed_a.md3 1.10 256 0.25 10
	q3map_foliage models/pop/foliages/sch_weed_b.md3 1.10 256 0.25 9

  q3map_foliage models/warzone/foliage/plant01.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant02.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant03.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant04.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant05.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant06.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant07.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant08.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant09.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant10.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant11.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant12.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant13.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant14.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant15.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant16.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant17.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant18.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant19.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant20.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant21.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant22.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant23.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant24.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant25.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant26.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant27.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant28.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant29.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant30.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant31.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant32.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant33.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant34.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant35.md3 1.0 256 0.005 5
  q3map_foliage models/warzone/foliage/plant36.md3 1.0 256 0.005 5
	
	q3map_foliage models/map_objects/yavin/tree08_b.md3 1.0 256 0.01 2
	q3map_foliage models/map_objects/yavin/tree08_b.md3 0.7 256 0.01 2
	q3map_foliage models/map_objects/yavin/tree08_b.md3 1.4 256 0.01 2
}

textures/yavin/groundfoliagedense
{
	q3map_baseshader textures/yavin/foliagedense_base
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

textures/yavin/foliagelight_base
{
	q3map_material	GreenLeaves
	
	//surfaceparm pointlight
	surfaceparm trans
	surfaceparm	noimpact
	surfaceparm	nomarks
	
	distanceCull 480 1280 0.49
	sort seethrough
	
	cull disable

	q3map_foliage models/map_objects/yavin/grass_b.md3 0.7 256 0.1 12
	q3map_foliage models/map_objects/yavin/grass_tall_b.md3 0.1 256 0.25 11
	q3map_foliage models/pop/foliages/alp_weedy_c.md3 1.5 256 0.015 10
	q3map_foliage models/pop/foliages/sch_weed_a.md3 1.5 256 0.015 9
	q3map_foliage models/pop/foliages/sch_weed_b.md3 1.5 256 0.015 8
	q3map_foliage models/pop/foliages/pop_flower_a.md3 0.4 256 0.01 7
	q3map_foliage models/pop/foliages/pop_flower_b.md3 0.4 256 0.01 6
	q3map_foliage models/pop/foliages/pop_flower_c.md3 0.4 256 0.01 5
	
	q3map_foliage models/map_objects/yavin/tree06_b.md3 0.9 256 0.001 3
	q3map_foliage models/map_objects/yavin/tree08_b.md3 0.9 256 0.001 2
	q3map_foliage models/map_objects/yavin/tree09_b.md3 0.7 256 0.001 1
}

textures/yavin/groundfoliagelight
{
	q3map_baseshader textures/yavin/foliagelight_base
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
