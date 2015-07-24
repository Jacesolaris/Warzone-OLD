models/players/n1_mst/n1_mst
{
	cull disable
	{
		map models/players/n1_mst/n1_mst.tga
		rgbGen lightingDiffuse
	}
	{
		map models/players/n1_mst/env_chrome.tga
		blendfunc add
		rgbGen lightingDiffuse
		tcGen environment 
	}
	{
		map models/players/n1_mst/n1_mst.tga
		blendfunc blend
		rgbGen lightingDiffuse
	}
	{
		map models/players/n1_mst/n1_mst_glow.tga
		blendfunc add
		rgbGen wave noise 0 1 0 1 
	}
}
