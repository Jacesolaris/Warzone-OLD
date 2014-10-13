models/players/vehicle_hazardtrooper/rifle_energy
{
    {
        map models/players/vehicle_hazardtrooper/power32
        tcMod scroll 0 6
    }
    {
        map models/players/vehicle_hazardtrooper/power38
        blendFunc GL_DST_COLOR GL_SRC_ALPHA
        tcMod scroll 8 1
    }
}

models/players/vehicle_hazardtrooper/rifle_energy_old
{
	qer_editorimage	models/players/vehicle_hazardtrooper/rifle_energy3
    {
        map models/players/vehicle_hazardtrooper/rifle_energy3
        tcMod scroll 5 0
        tcMod scale 5 1
    }
    {
        map models/players/vehicle_hazardtrooper/rifle_energy1
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 4
        tcMod transform 0.5 0 0 1 0 0
        tcMod turb 0 1 0 1
    }
    {
        map models/players/vehicle_hazardtrooper/rifle_energy2
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 -5
        tcMod stretch square 0.5 1 0 6
        tcMod turb 0 4 0 1
    }
}

models/players/vehicle_hazardtrooper/hazardtrooper_legs
{
    {
        map models/players/vehicle_hazardtrooper/hazardtrooper_legs
        rgbGen lightingDiffuse
    }
    {
        map models/players/vehicle_hazardtrooper/hazardtrooper_legs_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/players/vehicle_hazardtrooper/hazardtrooper_torso
{
    {
        map models/players/vehicle_hazardtrooper/hazardtrooper_torso
        rgbGen lightingDiffuse
    }
    {
        map models/players/vehicle_hazardtrooper/hazardtrooper_torso_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}

models/players/vehicle_hazardtrooper/hazardtrooper_arms
{
    {
        map models/players/vehicle_hazardtrooper/hazardtrooper_arms
        rgbGen lightingDiffuse
    }
    {
        map models/players/vehicle_hazardtrooper/hazardtrooper_arms_spec
        blendFunc GL_SRC_ALPHA GL_ONE
        detail
        alphaGen lightingSpecular
    }
}