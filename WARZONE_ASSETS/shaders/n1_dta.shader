models/players/n1_dta/n1_dta
{
	cull disable
	{
		map models/players/n1_dta/n1_dta.tga
		rgbGen lightingDiffuse
	}
	{
		map models/players/n1_dta/env_chrome.tga
		blendfunc add
		rgbGen lightingDiffuse
		tcGen environment 
	}
	{
		map models/players/n1_dta/n1_dta.tga
		blendfunc blend
		rgbGen lightingDiffuse
	}
	{
		map models/players/n1_dta/n1_dta_glow.tga
		blendfunc add
		rgbGen wave noise 0 1 0 1 
	}
}
