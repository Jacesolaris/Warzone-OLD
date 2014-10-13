models/players/gl_dooku/dooku_cape
{
	{
		map models/players/gl_dooku/dooku_cape.tga
		blendfunc gl_one_minus_src_alpha gl_src_alpha
		rgbGen lightingDiffuse
		depthWrite
	}
}

models/players/gl_dooku/dooku_cape_blue
{
	{
		map models/players/gl_dooku/dooku_cape_blue.tga
		blendfunc gl_one_minus_src_alpha gl_src_alpha
		rgbGen lightingDiffuse
		depthWrite
	}
}

models/players/gl_dooku/dooku_red
{
	{
		map models/players/gl_dooku/dooku_cape_red.tga
		blendfunc gl_one_minus_src_alpha gl_src_alpha
		rgbGen lightingDiffuse
		depthWrite
	}
}

models/players/gl_dooku/dooku_legs
{
	{
		map models/players/gl_dooku/dooku_legs.tga
		rgbGen lightingDiffuse
	}
	{
		map models/players/gl_dooku/envmap1.tga
		blendfunc add
		rgbGen lightingDiffuse
		tcGen environment 
	}
	{
		map models/players/gl_dooku/dooku_legs.tga
		blendfunc blend
		rgbGen lightingDiffuse
	}
}

