models/players/n1bomber_hat/n1bomber_hat
{
	cull disable
	{
		map models/players/n1bomber_hat/n1_mst.tga
		rgbGen lightingDiffuse
	}
	{
		map models/players/n1bomber_hat/env_chrome.tga
		blendfunc add
		rgbGen lightingDiffuse
		tcGen environment 
	}
	{
		map models/players/n1bomber_hat/n1_mst.tga
		blendfunc blend
		rgbGen lightingDiffuse
	}
	{
		map models/players/n1bomber_hat/n1_mst_glow.tga
		blendfunc add
		rgbGen wave noise 0 1 0 1 
	}
}
