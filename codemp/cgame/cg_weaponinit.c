//
// cg_weaponinit.c -- events and effects dealing with weapons
#include "cg_local.h"
#include "fx_local.h"

void CG_PrecacheScopes(void)
{
	int sc = 0;

	for (sc = 0; sc < SCOPE_MAX_SCOPES; sc++) {
		if (strncmp(scopeData[sc].scopeModel, "", strlen(scopeData[sc].scopeModel))) trap->R_RegisterModel(scopeData[sc].scopeModel);
		if (strncmp(scopeData[sc].scopeModelShader, "", strlen(scopeData[sc].scopeModelShader))) trap->R_RegisterShader(scopeData[sc].scopeModelShader);
		if (strncmp(scopeData[sc].gunMaskShader, "", strlen(scopeData[sc].gunMaskShader))) trap->R_RegisterShader(scopeData[sc].gunMaskShader);
		if (strncmp(scopeData[sc].insertShader, "", strlen(scopeData[sc].insertShader))) trap->R_RegisterShader(scopeData[sc].insertShader);
		if (strncmp(scopeData[sc].maskShader, "", strlen(scopeData[sc].maskShader))) trap->R_RegisterShader(scopeData[sc].maskShader);
		if (strncmp(scopeData[sc].lightShader, "", strlen(scopeData[sc].lightShader))) trap->R_RegisterShader(scopeData[sc].lightShader);
		if (strncmp(scopeData[sc].tickShader, "", strlen(scopeData[sc].tickShader))) trap->R_RegisterShader(scopeData[sc].tickShader);
		if (strncmp(scopeData[sc].chargeShader, "", strlen(scopeData[sc].chargeShader))) trap->R_RegisterShader(scopeData[sc].chargeShader);
		if (strncmp(scopeData[sc].zoomStartSound, "", strlen(scopeData[sc].zoomStartSound))) trap->S_RegisterSound(scopeData[sc].zoomStartSound);
		if (strncmp(scopeData[sc].zoomEndSound, "", strlen(scopeData[sc].zoomEndSound))) trap->S_RegisterSound(scopeData[sc].zoomEndSound);
	}
}

/*
=================
CG_RegisterWeapon

The server says this item is used on this level
=================
*/
void CG_RegisterWeapon( int weaponNum) {
	weaponInfo_t	*weaponInfo;
	gitem_t			*item, *ammo;
	char			path[MAX_QPATH];
	vec3_t			mins, maxs;
	int				i;

	if (weaponNum < WP_NONE) 
	{// Missing SP weapons...
		return;
	}

	if (!cgs.effects.rocketExplosionEffect) cgs.effects.rocketExplosionEffect = trap->FX_RegisterEffect("orginal_weapon_efx/rocket/explosion");

	weaponInfo = &cg_weapons[weaponNum];

	if ( weaponNum == 0 ) {
		return;
	}

	if ( weaponInfo->registered ) {
		return;
	}

	//cgs.effects.blasterWallImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/bryar/wall_impact");

	//if ( cgs.wDisable & (1<<weaponNum) )
	//	return;

	memset( weaponInfo, 0, sizeof( *weaponInfo ) );
	weaponInfo->registered = qtrue;

	for ( item = bg_itemlist + 1 ; item->classname ; item++ ) {
		if ( item->giType == IT_WEAPON && item->giTag == weaponNum ) {
			weaponInfo->item = item;
			break;
		}
	}
	if ( !item->classname ) {
		trap->Error( ERR_DROP, "Couldn't find weapon %i", weaponNum );
		return;
	}

	CG_RegisterItemVisuals( item - bg_itemlist );

	// UQ1: Set all names in the bg_weapons.c struct and copy it here... Don't set them below...
	if (weaponInfo->item)
		weaponInfo->item->classname = weaponData[weaponNum].classname;

	// load cmodel before model so filecache works
	weaponInfo->weaponModel = trap->R_RegisterModel( item->world_model[0] );
	// load in-view model also
	weaponInfo->viewModel = trap->R_RegisterModel(item->view_model);

	// calc midpoint for rotation
	trap->R_ModelBounds( weaponInfo->weaponModel, mins, maxs );
	for ( i = 0 ; i < 3 ; i++ ) {
		weaponInfo->weaponMidpoint[i] = mins[i] + 0.5 * ( maxs[i] - mins[i] );
	}

	weaponInfo->weaponIcon = trap->R_RegisterShader( item->icon );
	weaponInfo->ammoIcon = trap->R_RegisterShader( item->icon );

	for ( ammo = bg_itemlist + 1 ; ammo->classname ; ammo++ ) {
		if ( ammo->giType == IT_AMMO && ammo->giTag == weaponNum ) {
			break;
		}
	}
	if ( ammo->classname && ammo->world_model[0] ) {
		weaponInfo->ammoModel = trap->R_RegisterModel( ammo->world_model[0] );
	}

//	strcpy( path, item->view_model );
//	COM_StripExtension( path, path );
//	strcat( path, "_flash.md3" );
	weaponInfo->flashModel = 0;//trap->R_RegisterModel( path );

	if (weaponNum == WP_DISRUPTOR ||
		weaponNum == WP_FLECHETTE ||
		weaponNum == WP_REPEATER ||
		weaponNum == WP_ROCKET_LAUNCHER ||
		weaponNum == WP_CONCUSSION 	||	
		weaponNum == WP_Z6_BLASTER_CANON ||
		weaponNum == WP_E60_ROCKET_LAUNCHER ||
		weaponNum == WP_ARC_CASTER_IMPERIAL ||
		weaponNum == WP_PULSECANON ||
		weaponNum == WP_CW_ROCKET_LAUNCHER ||
		weaponNum == WP_BRYAR_RIFLE_SCOPE ||
		weaponNum == WP_DLT_19)

	{
		Q_strncpyz( path, item->view_model, sizeof(path) );
		COM_StripExtension( path, path, sizeof( path ) );
		Q_strcat( path, sizeof(path), "_barrel.md3" );
		weaponInfo->barrelModel = trap->R_RegisterModel( path );
	}
	else if (weaponNum == WP_STUN_BATON)
	{ //only weapon with more than 1 barrel..
		trap->R_RegisterModel("models/weapons2/stun_baton/baton_barrel.md3");
		trap->R_RegisterModel("models/weapons2/stun_baton/baton_barrel2.md3");
		trap->R_RegisterModel("models/weapons2/stun_baton/baton_barrel3.md3");
	}
	else
	{
		weaponInfo->barrelModel = 0;
	}

	if (weaponNum != WP_SABER)
	{
		Q_strncpyz( path, item->view_model, sizeof(path) );
		COM_StripExtension( path, path, sizeof( path ) );
		Q_strcat( path, sizeof(path), "_hand.md3" );
		weaponInfo->handsModel = trap->R_RegisterModel( path );
	}
	else
	{
		weaponInfo->handsModel = 0;
	}

	switch ( weaponNum ) {
	case WP_STUN_BATON:
	case WP_MELEE:

		trap->FX_RegisterEffect( "stunBaton/flesh_impact" );

		if (weaponNum == WP_STUN_BATON)
		{
			trap->S_RegisterSound( "sound/weapons/baton/idle.wav" );
			weaponInfo->flashSound[0] = trap->S_RegisterSound( "sound/weapons/baton/fire.mp3" );
			weaponInfo->altFlashSound[0] = trap->S_RegisterSound( "sound/weapons/baton/fire.mp3" );
		}
		else
		{
		
		}
		break;
	case WP_SABER:
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f );
		weaponInfo->firingSound = trap->S_RegisterSound( "sound/weapons/saber/saberhum1.wav" );
		weaponInfo->missileModel		= trap->R_RegisterModel( "models/weapons2/saber/saber_w.glm" );
		break;

	case WP_CONCUSSION:
		weaponInfo->selectSound			= trap->S_RegisterSound("sound/weapons/concussion/select.wav");
		weaponInfo->flashSound[0]		= trap->S_RegisterSound("sound/weapons/slugthrowers/w-90_1.wav");
		weaponInfo->flashSound[1]		= trap->S_RegisterSound("sound/weapons/slugthrowers/w-90_2.wav");
		weaponInfo->flashSound[2]		= trap->S_RegisterSound("sound/weapons/slugthrowers/w-90_3.wav");
		weaponInfo->flashSound[3]		= trap->S_RegisterSound("sound/weapons/slugthrowers/w-90_4.wav");
		weaponInfo->firingSound			= NULL_SOUND;
		weaponInfo->chargeSound			= NULL_SOUND;
		weaponInfo->muzzleEffect		= trap->FX_RegisterEffect( "slugthrowers/muzzleflash" );
		weaponInfo->missileModel		= NULL_HANDLE;
		weaponInfo->missileSound		= NULL_SOUND;
		weaponInfo->missileDlight		= 0;
		weaponInfo->missileHitSound		= NULL_SOUND;
		weaponInfo->missileTrailFunc	= FX_WeaponProjectileThink;
		weaponInfo->altFlashSound[0]	= trap->S_RegisterSound("sound/weapons/slugthrowers/w-90_1.wav");
		weaponInfo->altFlashSound[1]	= trap->S_RegisterSound("sound/weapons/slugthrowers/w-90_2.wav");
		weaponInfo->altFlashSound[2]	= trap->S_RegisterSound("sound/weapons/slugthrowers/w-90_3.wav");
		weaponInfo->altFlashSound[3]	= trap->S_RegisterSound("sound/weapons/slugthrowers/w-90_4.wav");
		weaponInfo->altFiringSound		= NULL_SOUND;
		weaponInfo->altChargeSound		= NULL_SOUND;
		weaponInfo->altMuzzleEffect		= trap->FX_RegisterEffect( "slugthrowers/muzzleflash" );
		weaponInfo->altMissileModel		= NULL_HANDLE;
		weaponInfo->altMissileSound		= NULL_SOUND;
		weaponInfo->altMissileDlight	= 0;
		weaponInfo->altMissileHitSound	= NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect( "explosives/shot_concussion" );
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect( "explosives/shot_concussion" );

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/concussion/explosion");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/concussion/explosion");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect( "orginal_weapon_efx/concussion/explosion" );
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect( "orginal_weapon_efx/concussion/explosion" );

		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("explosives/concussion3medium");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("explosives/concussion1medium");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("explosives/concussion3medium");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("explosives/concussion1medium");

		trap->R_RegisterShader("gfx_base/effects/blueLine");
		trap->R_RegisterShader("gfx_base/misc/whiteline2");
		break;

	case WP_BRYAR_PISTOL:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_pistol.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/bryar_pistol1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/bryar_pistol2.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/bryar_pistol3.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/bryar_pistol4.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_small");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle1.mp3");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle2.mp3");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle3.mp3");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle4.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_small");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;

		weaponInfo->Chargingfx = NULL_FX;
		weaponInfo->Altchargingfx = trap->FX_RegisterEffect("weapons/charge_bryar");

		weaponInfo->altMissileTrailFunc = FX_BryarAltProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_redorange_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_redorange_small");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/bryar/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/bryar/flesh_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/bryar/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/bryar/wall_impact");
		
		
		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		//cgs.media.redFrontFlash = trap->R_RegisterShader("gfx_base/effects/bryarFrontFlash");
		trap->FX_RegisterEffect("blasters/red_deflect");

		break;

	case WP_BRYAR_OLD:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_pistol.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/bryar_pistol1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/bryar_pistol2.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/bryar_pistol3.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/bryar_pistol4.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle1.mp3");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle2.mp3");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle3.mp3");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle4.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_BryarAltProjectileThink;

		weaponInfo->Chargingfx = NULL_FX;
		weaponInfo->Altchargingfx = trap->FX_RegisterEffect("weapons/charge_bryar");

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_redorange_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_redorange_small");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/bryar/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/bryar/flesh_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/bryar/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/bryar/wall_impact");
		
		
		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		trap->FX_RegisterEffect("blasters/red_deflect");
		cgs.media.redFrontFlash = trap->R_RegisterShader("gfx_base/effects/bryarFrontFlash");
		break;
	case WP_BLASTER:
	case WP_EMPLACED_GUN: //rww - just use the same as this for now..
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/blaster/select.wav");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/e11b_1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/e11b_2.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/e11b_3.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/e11b_4.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/e11b_1.mp3");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/e11b_2.mp3");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/e11b_3.mp3");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/e11b_4.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = NULL_SOUND;
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;//FX_WeaponProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_red_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_red_small");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		
		
		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");

		trap->FX_RegisterEffect("blasters/red_deflect");
		
		cgs.effects.blasterDroidImpactEffect = trap->FX_RegisterEffect("blaster/droid_impact");
		break;

	case WP_DISRUPTOR:
		weaponInfo->selectSound			= trap->S_RegisterSound("sound/weapons/select_sniper.mp3");
		weaponInfo->flashSound[0]		= trap->S_RegisterSound( "sound/weapons/Blasters/disruptor1.mp3");
		weaponInfo->flashSound[1]		= trap->S_RegisterSound("sound/weapons/Blasters/disruptor1.mp3");
		weaponInfo->flashSound[2]		= trap->S_RegisterSound("sound/weapons/Blasters/disruptor1.mp3");
		weaponInfo->flashSound[3]		= trap->S_RegisterSound("sound/weapons/Blasters/disruptor1.mp3");
		weaponInfo->firingSound			= NULL_SOUND;
		weaponInfo->chargeSound			= NULL_SOUND;
		weaponInfo->muzzleEffect		= trap->FX_RegisterEffect("orginal_weapon_efx/disruptor/muzzle_flash");
		weaponInfo->missileModel		= NULL_HANDLE;
		weaponInfo->missileSound		= NULL_SOUND;
		weaponInfo->missileDlight		= 0;
		weaponInfo->missileHitSound		= NULL_SOUND;
		weaponInfo->missileTrailFunc	= 0;
		weaponInfo->altFlashSound[0]	= trap->S_RegisterSound("sound/weapons/Blasters/disruptor1.mp3");
		weaponInfo->altFlashSound[1]	= trap->S_RegisterSound("sound/weapons/Blasters/disruptor1.mp3");
		weaponInfo->altFlashSound[2]	= trap->S_RegisterSound("sound/weapons/Blasters/disruptor1.mp3");
		weaponInfo->altFlashSound[3]	= trap->S_RegisterSound("sound/weapons/Blasters/disruptor1.mp3");
		weaponInfo->altFiringSound		= NULL_SOUND;
		weaponInfo->altChargeSound		= trap->S_RegisterSound("sound/weapons/disruptor/altcharge.mp3");
		weaponInfo->altMuzzleEffect		= trap->FX_RegisterEffect("orginal_weapon_efx/disruptor/muzzle_flash");
		weaponInfo->altMissileModel		= NULL_HANDLE;
		weaponInfo->altMissileSound		= NULL_SOUND;
		weaponInfo->altMissileDlight	= 0;
		weaponInfo->altMissileHitSound	= NULL_SOUND;
		weaponInfo->altMissileTrailFunc = 0;

		weaponInfo->Chargingfx = NULL_FX;
		weaponInfo->Altchargingfx = trap->FX_RegisterEffect("weapons/charge_bryar");

		cgs.effects.disruptorRingsEffect		= trap->FX_RegisterEffect( "orginal_weapon_efx/disruptor/rings" );
		cgs.effects.disruptorProjectileEffect	= trap->FX_RegisterEffect( "weapons/laser_red" );
		cgs.effects.disruptorWallImpactEffect	= trap->FX_RegisterEffect( "orginal_weapon_efx/disruptor/wall_impact" );
		cgs.effects.disruptorFleshImpactEffect	= trap->FX_RegisterEffect( "orginal_weapon_efx/disruptor/flesh_impact" );
		cgs.effects.disruptorEnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		cgs.effects.disruptorAltMissEffect		= trap->FX_RegisterEffect( "orginal_weapon_efx/disruptor/alt_miss" );
		cgs.effects.disruptorAltHitEffect		= trap->FX_RegisterEffect( "orginal_weapon_efx/disruptor/alt_hit" );




		trap->R_RegisterShader( "gfx_base/effects/redLine" );
		trap->R_RegisterShader( "gfx_base/misc/whiteline2" );
		trap->R_RegisterShader( "gfx_base/effects/smokeTrail" );


		trap->S_RegisterSound("sound/weapons/disruptor/zoomstart.wav");
		trap->S_RegisterSound("sound/weapons/disruptor/zoomend.wav");
		cgs.media.disruptorZoomLoop		= trap->S_RegisterSound( "sound/weapons/disruptor/zoomloop.wav" );
		break;

	case WP_BOWCASTER:
		weaponInfo->selectSound				= trap->S_RegisterSound("sound/weapons/select_sniper.mp3");
		weaponInfo->firingSound				= NULL_SOUND;
		weaponInfo->flashSound[0]			= trap->S_RegisterSound("sound/weapons/blasters/crossbow_small1.mp3");
		weaponInfo->flashSound[1]			= trap->S_RegisterSound("sound/weapons/blasters/crossbow_small2.mp3");
		weaponInfo->flashSound[2]			= trap->S_RegisterSound("sound/weapons/blasters/crossbow_small3.mp3");
		weaponInfo->flashSound[3]			= trap->S_RegisterSound("sound/weapons/blasters/crossbow_small4.mp3");

		weaponInfo->muzzleEffect			= trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->missileModel			= NULL_HANDLE;
		weaponInfo->missileSound			= NULL_SOUND;
		weaponInfo->missileDlight			= 0;
		weaponInfo->missileHitSound			= NULL_SOUND;
		weaponInfo->missileTrailFunc		= FX_WeaponProjectileThink;
		weaponInfo->powerupShotRenderfx		= NULL_FX;


		weaponInfo->altMuzzleEffect			= trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->altFlashSound[0]		= trap->S_RegisterSound("sound/weapons/blasters/crossbow_big1.mp3");
		weaponInfo->altFlashSound[1]		= trap->S_RegisterSound("sound/weapons/blasters/crossbow_big2.mp3");
		weaponInfo->altFlashSound[2]		= trap->S_RegisterSound("sound/weapons/blasters/crossbow_big3.mp3");
		weaponInfo->altFlashSound[3]		= trap->S_RegisterSound("sound/weapons/blasters/crossbow_big4.mp3");
		weaponInfo->chargeSound				= trap->S_RegisterSound("sound/weapons/charge_bowcaster.mp3");
		weaponInfo->altMissileModel			= NULL_HANDLE;
		weaponInfo->altMissileSound			= NULL_SOUND;
		weaponInfo->altMissileDlight		= 0;
		weaponInfo->altMissileHitSound		= NULL_SOUND;
		weaponInfo->altMissileTrailFunc		= FX_WeaponAltProjectileThink;

		weaponInfo->Chargingfx = trap->FX_RegisterEffect("weapons/charge_bryar");
		weaponInfo->Altchargingfx = NULL_FX;

		weaponInfo->missileRenderfx			= trap->FX_RegisterEffect( "blasters/shot_red_small" );
		weaponInfo->altMissileRenderfx		= trap->FX_RegisterEffect( "blasters/shot_red_small" );

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/bowcaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/bowcaster/flesh_impact");
		weaponInfo->missileWallImpactfx		= trap->FX_RegisterEffect("orginal_weapon_efx/bowcaster/wall_impact");
		weaponInfo->altMissileWallImpactfx  = trap->FX_RegisterEffect("orginal_weapon_efx/bowcaster/wall_impact");
		
		
		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		trap->FX_RegisterEffect("blasters/green_deflect");
		cgs.media.redFrontFlash = trap->R_RegisterShader("gfx_base/effects/bryarFrontFlash");
		break;

	case WP_REPEATER:
		weaponInfo->selectSound			= trap->S_RegisterSound("sound/weapons/select_repeater.mp3");
		weaponInfo->flashSound[0]		= trap->S_RegisterSound( "sound/weapons/imperial_repeater/fire.wav");
		weaponInfo->firingSound			= NULL_SOUND;
		weaponInfo->chargeSound			= NULL_SOUND;
		weaponInfo->muzzleEffect		= trap->FX_RegisterEffect( "blasters/muzzleflash2_Yellow_medium" );
		weaponInfo->missileModel		= NULL_HANDLE;
		weaponInfo->missileSound		= NULL_SOUND;
		weaponInfo->missileDlight		= 0;
		weaponInfo->missileHitSound		= NULL_SOUND;
		weaponInfo->missileTrailFunc	= FX_WeaponProjectileThink;
		weaponInfo->altFlashSound[0]	= trap->S_RegisterSound( "sound/weapons/imperial_repeater/alt_fire.wav");
		weaponInfo->altFiringSound		= NULL_SOUND;
		weaponInfo->altChargeSound		= NULL_SOUND;
		weaponInfo->altMuzzleEffect		= trap->FX_RegisterEffect( "blasters/muzzleflash2_Blue_medium" );
		weaponInfo->altMissileModel		= NULL_HANDLE;
		weaponInfo->altMissileSound		= NULL_SOUND;
		weaponInfo->altMissileDlight	= 0;
		weaponInfo->altMissileHitSound	= NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_RepeaterAltProjectileThink;

		weaponInfo->missileRenderfx		= trap->FX_RegisterEffect("orginal_weapon_efx/imperial_repeater/projectile");
		/*weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("orginal_weapon_efx/imperial_repeater/alt_projectile");*/
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_Blue_Ball_big");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/imperial_repeater/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/imperial_repeater/concussion"); // not sure about this one
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/imperial_repeater/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/imperial_repeater/concussion");
		
		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/yellow_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("explosives/concussion1medium");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_yellow_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("explosives/concussion1medium");
		trap->FX_RegisterEffect("blasters/yellow_deflect");
		break;

	case WP_DEMP2:
		weaponInfo->selectSound			= trap->S_RegisterSound("sound/weapons/demp2/select.wav");
		weaponInfo->flashSound[0]		= trap->S_RegisterSound("sound/weapons/demp2/fire.wav");
		weaponInfo->firingSound			= NULL_SOUND;
		weaponInfo->chargeSound			= NULL_SOUND;
		weaponInfo->muzzleEffect		= trap->FX_RegisterEffect("weapons/charge_arc");
		weaponInfo->missileModel		= NULL_HANDLE;
		weaponInfo->missileSound		= NULL_SOUND;
		weaponInfo->missileDlight		= 0;
		weaponInfo->missileHitSound		= NULL_SOUND;
		weaponInfo->missileTrailFunc	= FX_DEMP2_ProjectileThink;
		weaponInfo->altFlashSound[0]	= trap->S_RegisterSound("sound/weapons/demp2/altfire.wav");
		weaponInfo->altFiringSound		= NULL_SOUND;
		weaponInfo->altChargeSound		= trap->S_RegisterSound("sound/weapons/demp2/altCharge.wav");
		weaponInfo->altMuzzleEffect		= trap->FX_RegisterEffect("weapons/charge_arc");
		weaponInfo->altMissileModel		= NULL_HANDLE;
		weaponInfo->altMissileSound		= NULL_SOUND;
		weaponInfo->altMissileDlight	= 0;
		weaponInfo->altMissileHitSound	= NULL_SOUND;
		weaponInfo->altMissileTrailFunc = 0;

		weaponInfo->Chargingfx = NULL_FX;
		weaponInfo->Altchargingfx = trap->FX_RegisterEffect("weapons/charge_arc");


		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("Blasters/shot_electricity");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("Blasters/shot_electricity");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/demp2/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/demp2/flesh_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/demp2/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/demp2/altdetonate");
		
		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("Blasters/electric_impactmedium");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("Blasters/electric_impactbig");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("Blasters/electric_impactmedium");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("Blasters/electric_impactbig");

		/*cgs.media.demp2Shell = trap->R_RegisterModel( "models/items/sphere.md3" );
		cgs.media.demp2ShellShader = trap->R_RegisterShader( "gfx_base/effects/demp2shell" );
		cgs.media.DemplightningFlash = trap->R_RegisterShader("gfx_base/misc/lightningFlash");*/
		break;

	case WP_FLECHETTE:
		weaponInfo->selectSound			= trap->S_RegisterSound("sound/weapons/flechette/select.wav");
		weaponInfo->flashSound[0]		= trap->S_RegisterSound( "sound/weapons/flechette/fire.wav");
		weaponInfo->firingSound			= NULL_SOUND;
		weaponInfo->chargeSound			= NULL_SOUND;
		weaponInfo->muzzleEffect		= trap->FX_RegisterEffect( "slugthrowers/muzzleflash" );
		weaponInfo->missileModel		= trap->R_RegisterModel("models/ammo/flechette_missile.md3");
		weaponInfo->missileSound		= NULL_SOUND;
		weaponInfo->missileDlight		= 0;
		weaponInfo->missileHitSound		= NULL_SOUND;
		weaponInfo->missileTrailFunc	= FX_WeaponProjectileThink;
		weaponInfo->altFlashSound[0]	= trap->S_RegisterSound( "sound/weapons/flechette/alt_fire.wav");
		weaponInfo->altFiringSound		= NULL_SOUND;
		weaponInfo->altChargeSound		= NULL_SOUND;
		weaponInfo->altMuzzleEffect		= trap->FX_RegisterEffect( "slugthrowers/muzzleflash" );
		weaponInfo->altMissileModel		= trap->R_RegisterModel( "models/ammo/flechette_missile.md3" );
		weaponInfo->altMissileSound		= NULL_SOUND;
		weaponInfo->altMissileDlight	= 0;
		weaponInfo->altMissileHitSound	= NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("orginal_weapon_efx/flechette/shot");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("orginal_weapon_efx/flechette/alt_shot");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/flechette/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/flechette/flesh_impact"); // not sure about this one
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/flechette/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/flechette/wall_impact");
		
		
		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("effects/explosives/flechettemedium");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("effects/explosives/flechettemedium");
		break;

		//Add new guns here
	case WP_A280: // UQ1: Example. Should have it's own fx...
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_sniper.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/dlt-19_1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/dlt-19_1.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/dlt-19_1.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/dlt-19_1.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle1.mp3");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle2.mp3");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle3.mp3");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle4.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = NULL_SOUND;
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_red_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_red_small");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		
		
		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		trap->FX_RegisterEffect("blasters/red_deflect");
		trap->S_RegisterSound("sound/weapons/disruptor/zoomstart.wav");
		trap->S_RegisterSound("sound/weapons/disruptor/zoomend.wav");

		cgs.media.disruptorZoomLoop = trap->S_RegisterSound("sound/weapons/disruptor/zoomloop.wav");
		break;

	case WP_DC15:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_repeater.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/generic1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/generic2.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/generic3.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/generic4.mp3");
		weaponInfo->flashSound[4] = trap->S_RegisterSound("sound/weapons/blasters/generic5.mp3");
		weaponInfo->flashSound[5] = trap->S_RegisterSound("sound/weapons/blasters/generic6.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Blue_medium"); //Is working like it should, KEEP it like that
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/dc17m/alt_fire.wav");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = trap->S_RegisterSound("sound/weapons/SBDarm/cannon_charge.mp3");
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Blue_big");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		

		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_blue_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_Blue_Ball_big");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/imperial_repeater/concussion"); // not sure about this one
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/imperial_repeater/concussion");
		

		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/blue_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("explosives/concussion1medium");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_blue_medium_fire");//prime efx
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("explosives/concussion1medium");
		trap->FX_RegisterEffect("blasters/blue_deflect");
		break;

	case WP_WESTARM5:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_repeater.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/generic1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/generic2.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/generic3.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/generic4.mp3");
		weaponInfo->flashSound[4] = trap->S_RegisterSound("sound/weapons/blasters/generic5.mp3");
		weaponInfo->flashSound[5] = trap->S_RegisterSound("sound/weapons/blasters/generic6.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Blue_medium"); //Is working like it should, KEEP it like that
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/dc17m/alt_fire.wav");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = trap->S_RegisterSound("sound/weapons/SBDarm/cannon_charge.mp3");
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Blue_big");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;


		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_blue_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_Blue_Ball_big");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/imperial_repeater/concussion"); // not sure about this one
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/imperial_repeater/concussion");


		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/blue_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("explosives/concussion1medium");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_blue_medium_fire");//prime efx
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("explosives/concussion1medium");
		trap->FX_RegisterEffect("blasters/blue_deflect");
		break;

	case WP_T21:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_rifle.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/t-21_1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/t-21_2.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/t-21_3.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/t-21_4.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->missileModel = trap->R_RegisterModel("models/weapons3/golan_arms/projectileMain.md3");
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/walker1.mp3");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/walker2.mp3");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/walker3.mp3");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/walker4.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = NULL_SOUND;
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->altMissileModel = trap->R_RegisterModel("models/weapons3/golan_arms/projectile.md3");
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_Red_medium");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_Red_big");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		
		
		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		trap->FX_RegisterEffect("blasters/red_deflect");
		break;

	case WP_EE3:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_rifle.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/dlt-19_1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/dlt-19_1.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/dlt-19_1.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/dlt-19_1.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/genericweak1.mp3");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/genericweak2.mp3");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/genericweak3.mp3");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/genericweak4.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = NULL_SOUND;
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_redorange_medium");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_RedOrange_Flare_medium");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		
		
		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium");
		trap->FX_RegisterEffect("blasters/red_deflect");
		break;

	case WP_DC_15S_CLONE_PISTOL:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_pistol.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/demp2/fire.wav");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Blue_small");
		weaponInfo->missileModel = NULL_HANDLE;

		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_Clonepistol_ProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/demp2/altfire.wav");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = trap->S_RegisterSound("sound/weapons/demp2/altCharge.wav");
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Blue_small");
		weaponInfo->altMissileModel = NULL_HANDLE;

		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = 0;

		weaponInfo->Chargingfx = NULL_FX/*trap->FX_RegisterEffect("weapons/charge_samus")*/;
		weaponInfo->Altchargingfx = trap->FX_RegisterEffect("weapons/charge_samus");


		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_Blue_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_Blue_small");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altWallBounceEffectFX = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		
		
		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/blue_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/blue_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_blue_medium_fire");
		weaponInfo->EnhancedFX_altWallBouncefx = trap->FX_RegisterEffect("weapons/blaster_impact_blue_medium_fire");
		trap->FX_RegisterEffect("blasters/blue_deflect");
		cgs.media.demp2Shell = trap->R_RegisterModel("models/items/sphere.md3");
		cgs.media.demp2ShellShader = trap->R_RegisterShader("gfx/effects/demp2shell");
		cgs.media.lightningFlash = trap->R_RegisterShader("gfx/misc/lightningFlash");
		break;

	case WP_DLT_19:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_rifle.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/dlt-19_1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/dlt-19_1.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/dlt-19_1.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/dlt-19_1.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle1.mp3");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle2.mp3");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle3.mp3");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle4.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = trap->S_RegisterSound("sound/weapons/charge_bryar.mp3");
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_red_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_red_small");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		
		
		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		trap->FX_RegisterEffect("blasters/red_deflect");
		break;

	case WP_DC_15A_Rifle:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_rifle.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/generic1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/generic2.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/generic3.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/generic4.mp3");
		weaponInfo->flashSound[4] = trap->S_RegisterSound("sound/weapons/blasters/generic5.mp3");
		weaponInfo->flashSound[5] = trap->S_RegisterSound("sound/weapons/blasters/generic6.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Blue_medium");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;

		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/repeater/alt_fire.wav");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = trap->S_RegisterSound("sound/weapons/SBDarm/cannon_charge.mp3");
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Blue_Big");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_RepeaterAltProjectileThink;
		
		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_blue_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_Blue_Ball_big");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/concussion/explosion"); // not sure about this one
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/concussion/explosion");
		
		
		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/blue_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("explosives/concussion1medium");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_blue_medium_fire");//prime efx
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("explosives/concussion1medium");
		trap->FX_RegisterEffect("blasters/blue_deflect");
		break;

	case WP_WESTER_PISTOL:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_pistol.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/westar1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/westar2.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/westar3.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/westar4.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_small");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/westar1.mp3");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/westar2.mp3");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/westar3.mp3");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/westar4.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_small");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_BryarAltProjectileThink;

		weaponInfo->Chargingfx = NULL_FX/*trap->FX_RegisterEffect("weapons/charge_samus")*/;
		weaponInfo->Altchargingfx = /*NULL_FX*/ trap->FX_RegisterEffect("weapons/charge_bryar");

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_Red_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_Red_small");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/bryar/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/bryar/flesh_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/bryar/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/bryar/wall_impact");
		
		
		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		trap->FX_RegisterEffect("blasters/red_deflect");
		cgs.media.redFrontFlash = trap->R_RegisterShader("gfx/effects/bryarFrontFlash");
		break;

	case WP_ELG_3A:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_pistol.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/elg-3a_1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/elg-3a_2.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/elg-3a_3.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/elg-3a_4.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Green_small");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/greenblaster/fire2.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Green_small");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_BryarAltProjectileThink;

		weaponInfo->Chargingfx = NULL_FX/*trap->FX_RegisterEffect("weapons/charge_samus")*/;
		weaponInfo->Altchargingfx = /*NULL_FX*/ trap->FX_RegisterEffect("weapons/charge_bowcaster_cheap");

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_greenblue_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_greenblue_small");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		
		
		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/green_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/green_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_blue_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_blue_medium_fire");
		trap->FX_RegisterEffect("blasters/green_deflect");
		cgs.media.greenFrontFlash = trap->R_RegisterShader("gfx/effects/greenfrontflash");
		break;

	case WP_S5_PISTOL:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_pistol.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/spy_pistol/fire.wav");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;


		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/spy_pistol/alt_fire.wav");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_BryarAltProjectileThink;

		weaponInfo->Chargingfx = NULL_FX/*trap->FX_RegisterEffect("weapons/charge_samus")*/;
		weaponInfo->Altchargingfx = /*NULL_FX*/ trap->FX_RegisterEffect("weapons/charge_bryar");

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_redorange_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_redorange_small");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/bryar/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/bryar/flesh_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/bryar/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/bryar/wall_impact");
		
		
		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		trap->FX_RegisterEffect("blasters/red_deflect");
		cgs.media.redFrontFlash = trap->R_RegisterShader("gfx/effects/bryarFrontFlash");
		break;

	case WP_Z6_BLASTER_CANON:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_h_launcher.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/minigun/fire.wav");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash_Blue_medium");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/repeater/alt_fire.wav");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = trap->S_RegisterSound("sound/weapons/SBDarm/cannon_charge.mp3");
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash_Blue_medium");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;
		weaponInfo->spinSound = trap->S_RegisterSound("sound/weapons/z6/spinny.wav");
		weaponInfo->spindownSound = trap->S_RegisterSound("sound/weapons/z6/chaingun_spindown.wav");

		weaponInfo->Chargingfx = NULL_FX/*trap->FX_RegisterEffect("weapons/charge_samus")*/;
		weaponInfo->Altchargingfx = /*NULL_FX*/ trap->FX_RegisterEffect("blasters/shot_Blue_Ball_medium");

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_blue_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_Blue_Ball_big");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/concussion/explosion"); // not sure about this one
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/concussion/explosion");
		

		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/blue_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("explosives/concussion1medium");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_blue_medium_fire");//prime efx
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("explosives/concussion1medium");
		trap->FX_RegisterEffect("blasters/blue_deflect");
		//cgs.media.cannonChargeFlash = trap->R_RegisterShader("gfx_base/misc/lightningFlash");
		cgs.media.lightningFlash = trap->R_RegisterShader("gfx/misc/lightningFlash");
		break;

	case WP_WOOKIE_BOWCASTER:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_carbine.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_big1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_big2.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_big3.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_big4.mp3");
		weaponInfo->flashSound[4] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_small1.mp3");
		weaponInfo->flashSound[5] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_small2.mp3");
		weaponInfo->flashSound[6] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_small3.mp3");
		weaponInfo->flashSound[7] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_small4.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Green_medium");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_big1.mp3");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_big2.mp3");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_big3.mp3");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_big4.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = trap->S_RegisterSound("sound/weapons/bowcaster/altcharge.wav");
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Green_medium");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;

		weaponInfo->Chargingfx = NULL_FX/*trap->FX_RegisterEffect("weapons/charge_samus")*/;
		weaponInfo->Altchargingfx = /*NULL_FX*/ trap->FX_RegisterEffect("weapons/charge_bowcaster_cheap");

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_Green_Flare_medium");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_Green_Flare_medium");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/bowcaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/bowcaster/flesh_impact"); // not sure about this one
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/bowcaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/bowcaster/wall_impact");
		

		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/green_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/green_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_green_medium");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_green_medium");
		trap->FX_RegisterEffect("blasters/green_deflect");
		break;

	case WP_WOOKIES_PISTOL:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_pistol.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/bryar_pistol1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/bryar_pistol2.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/bryar_pistol3.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/bryar_pistol4.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_small");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/bryar_pistol1.mp3");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/bryar_pistol2.mp3");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/bryar_pistol3.mp3");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/bryar_pistol4.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_small");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_BryarAltProjectileThink;

		weaponInfo->Chargingfx = NULL_FX/*trap->FX_RegisterEffect("weapons/charge_samus")*/;
		weaponInfo->Altchargingfx = /*NULL_FX*/ trap->FX_RegisterEffect("weapons/charge_bryar");

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_redorange_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_redorange_small");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/bryar/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/bryar/flesh_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/bryar/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/bryar/wall_impact");
		
		
		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		trap->FX_RegisterEffect("blasters/red_deflect");
		cgs.media.redFrontFlash = trap->R_RegisterShader("gfx/effects/bryarFrontFlash");

		break;

	case WP_CLONE_BLASTER:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_carbine.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/generic1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/generic2.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/generic3.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/generic4.mp3");
		weaponInfo->flashSound[4] = trap->S_RegisterSound("sound/weapons/blasters/generic5.mp3");
		weaponInfo->flashSound[5] = trap->S_RegisterSound("sound/weapons/blasters/generic6.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Blue_medium");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/e11b_1.mp3");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/e11b_2.mp3");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/e11b_3.mp3");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/e11b_4.mp3");
		weaponInfo->altFlashSound[4] = trap->S_RegisterSound("sound/weapons/blasters/e11b_5.mp3");
		weaponInfo->altFlashSound[5] = trap->S_RegisterSound("sound/weapons/blasters/e11b_6.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = NULL_SOUND;
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Blue_medium");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;//FX_WeaponProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_Blue_medium");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_Blue_medium");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact"); // not sure about this one
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact"); // etc here.. then..
		

		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/blue_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/blue_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_blue_medium_fire");//prime efx
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_blue_medium_fire");
		trap->FX_RegisterEffect("blasters/blue_deflect");
		break;

	case WP_DC15_EXT:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_repeater.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/repeater/fire.wav");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Blue_medium");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/repeater/fire.wav");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = trap->S_RegisterSound("sound/weapons/SBDarm/cannon_charge.mp3");
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("slugthrowers/muzzleflash_big");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;

		weaponInfo->Chargingfx = NULL_FX/*trap->FX_RegisterEffect("weapons/charge_samus")*/;
		weaponInfo->Altchargingfx = /*NULL_FX*/ trap->FX_RegisterEffect("blasters/shot_Blue_Ball_small");

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_Blue_medium");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("explosives/shot_concussion");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/concussion/explosion"); // not sure about this one
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/concussion/explosion");
		

		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/blue_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("explosives/concussion3medium");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_blue_medium_fire");//prime efx
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("explosives/concussion3medium");//secder efx
		trap->FX_RegisterEffect("blasters/blue_deflect");
		break;

	case WP_E60_ROCKET_LAUNCHER:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_l_launcher.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher1.wav");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher2.wav");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher3.wav");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher4.wav");
		weaponInfo->flashSound[4] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher5.wav");
		weaponInfo->flashSound[5] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher6.wav");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("slugthrowers/muzzleflash_big");
		weaponInfo->missileModel = trap->R_RegisterModel("models/ammo/rocket1_proj.md3");
		weaponInfo->missileSound = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher_loop.wav");
		weaponInfo->missileDlight = 125;
		VectorSet(weaponInfo->missileDlightColor, 1.0, 1.0, 0.5);
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_RocketProjectileThink;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher1.wav");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher2.wav");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher3.wav");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher4.wav");
		weaponInfo->altFlashSound[4] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher5.wav");
		weaponInfo->altFlashSound[5] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher6.wav");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = NULL_SOUND;
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("slugthrowers/muzzleflash_big");
		weaponInfo->altMissileModel = trap->R_RegisterModel("models/ammo/rocket1_proj.md3");
		weaponInfo->altMissileSound = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher_loop.wav");
		weaponInfo->altMissileDlight = 125;
		VectorSet(weaponInfo->altMissileDlightColor, 1.0, 1.0, 0.5);
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_PulseRocketAltProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("explosives/shot_rpg");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("explosives/shot_rpg");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/rocket/explosion");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/rocket/explosion"); // not sure about this one
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/rocket/explosion");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/rocket/explosion");


		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("explosives/demomedium");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("explosives/demomedium");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("explosives/demomedium");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("explosives/demomedium");
		break;

	case WP_CW_ROCKET_LAUNCHER:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_l_launcher.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher1.wav");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher2.wav");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher3.wav");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher4.wav");
		weaponInfo->flashSound[4] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher5.wav");
		weaponInfo->flashSound[5] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher6.wav");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("slugthrowers/muzzleflash_big");
		weaponInfo->missileModel = trap->R_RegisterModel("models/ammo/rocket1_proj.md3");
		weaponInfo->missileSound = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher_loop.wav");
		weaponInfo->missileDlight = 125;
		VectorSet(weaponInfo->missileDlightColor, 1.0, 1.0, 0.5);
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_RocketProjectileThink;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher1.wav");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher2.wav");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher3.wav");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher4.wav");
		weaponInfo->altFlashSound[4] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher5.wav");
		weaponInfo->altFlashSound[5] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher6.wav");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = NULL_SOUND;
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("slugthrowers/muzzleflash_big");
		weaponInfo->altMissileModel = trap->R_RegisterModel("models/ammo/rocket1_proj.md3");
		weaponInfo->altMissileSound = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher_loop.wav");
		weaponInfo->altMissileDlight = 125;
		VectorSet(weaponInfo->altMissileDlightColor, 1.0, 1.0, 0.5);
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_PulseRocketAltProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("explosives/shot_rpg");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("explosives/shot_rpg");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/rocket/explosion");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/rocket/explosion"); // not sure about this one
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/rocket/explosion");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/rocket/explosion");


		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("explosives/demomedium");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("explosives/demomedium");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("explosives/demomedium");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("explosives/demomedium");
		break;

	case WP_TESTGUN:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/blaster/select.wav");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/Blasters/dl-44_1.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("Blasters/muzzleflash2_Purple_small");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/Blasters/dl-44_3.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = NULL_SOUND;
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("Blasters/muzzleflash2_Purple_small");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;//FX_WeaponProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_RedPurple_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_RedPurple_small");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		
		
		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");

		trap->FX_RegisterEffect("blasters/purple_deflect");

		cgs.effects.blasterDroidImpactEffect = trap->FX_RegisterEffect("blaster/droid_impact");
		break;

	case WP_FRAG_GRENADE:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_grenade.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/thermal/fire.wav");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = trap->S_RegisterSound("sound/weapons/grenade_cook.mp3");
		weaponInfo->muzzleEffect = NULL_FX;
		weaponInfo->missileModel = trap->R_RegisterModel("models/weapons3/fraggrenade/thermal_proj.md3");
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_ThermalProjectileThink;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/thermal/fire.wav");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = trap->S_RegisterSound("sound/weapons/grenade_cook.mp3");
		weaponInfo->altMuzzleEffect = NULL_FX;
		weaponInfo->altMissileModel = trap->R_RegisterModel("models/weapons3/fraggrenade/thermal_proj.md3");
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_PulseGrenadeProjectileThink;

		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("weapons/grenaderibbon_red");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("weapons/grenaderibbon_red");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/rocket/explosion");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/rocket/explosion"); // not sure about this one
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/rocket/explosion");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/rocket/explosion");
		
		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("explosives/fragmedium");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("explosives/fragmedium");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("explosives/fragmedium");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("explosives/fragmedium");


		cgs.media.grenadeBounce1 = trap->S_RegisterSound("sound/weapons/grenade_bounce1.mp3");
		cgs.media.grenadeBounce2 = trap->S_RegisterSound("sound/weapons/grenade_bounce2.mp3");
		trap->S_RegisterSound("sound/weapons/grenade_thermdetloop.mp3");
		break;

	case WP_FRAG_GRENADE_OLD:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_grenade.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/thermal/fire.wav");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = trap->S_RegisterSound("sound/weapons/grenade_cook.mp3");
		weaponInfo->muzzleEffect = NULL_FX;
		weaponInfo->missileModel = trap->R_RegisterModel("models/weapons3/oldfraggrenade/thermal_proj.md3");
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_ThermalProjectileThink;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/thermal/fire.wav");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = trap->S_RegisterSound("sound/weapons/grenade_cook.mp3");
		weaponInfo->altMuzzleEffect = NULL_FX;
		weaponInfo->altMissileModel = trap->R_RegisterModel("models/weapons3/oldfraggrenade/thermal_proj.md3");
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_PulseGrenadeProjectileThink;

		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("weapons/grenaderibbon_red");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("weapons/grenaderibbon_red");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/rocket/explosion");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/rocket/explosion"); // not sure about this one
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/rocket/explosion");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/rocket/explosion");
		
		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("explosives/fragmedium");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("explosives/fragmedium");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("explosives/fragmedium");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("explosives/fragmedium");


		cgs.media.grenadeBounce1 = trap->S_RegisterSound("sound/weapons/grenade_bounce1.mp3");
		cgs.media.grenadeBounce2 = trap->S_RegisterSound("sound/weapons/grenade_bounce2.mp3");
		trap->S_RegisterSound("sound/weapons/grenade_thermdetloop.mp3");
		break;

	case WP_DC_17_CLONE_PISTOL:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_pistol.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/demp2/fire.wav");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Blue_small");
		weaponInfo->missileModel = NULL_HANDLE;

		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_Clonepistol_ProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/demp2/altfire.wav");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = trap->S_RegisterSound("sound/weapons/demp2/altCharge.wav");
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Blue_small");
		weaponInfo->altMissileModel = NULL_HANDLE;
		
		weaponInfo->Chargingfx = NULL_FX;
		weaponInfo->Altchargingfx = trap->FX_RegisterEffect("weapons/charge_samus");

		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = 0;


		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_Blue_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_Blue_small");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altWallBounceEffectFX = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		
		
		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/blue_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/blue_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_blue_medium_fire");
		weaponInfo->EnhancedFX_altWallBouncefx = trap->FX_RegisterEffect("weapons/blaster_impact_blue_medium_fire");
		trap->FX_RegisterEffect("blasters/blue_deflect");
		//cgs.media.demp2Shell = trap->R_RegisterModel("models/items/sphere.md3");
		//cgs.media.demp2ShellShader = trap->R_RegisterShader("gfx/effects/demp2shell");
		/*cgs.media.lightningFlash = trap->R_RegisterShader("gfx/misc/lightningFlash");*/
		break;

	case WP_SPOTING_BLASTER:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_pistol.pm3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/e11b_1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/e11b_2.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/e11b_3.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/e11b_4.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/e11b_1.mp3");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/e11b_2.mp3");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/e11b_3.mp3");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/e11b_4.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = NULL_SOUND;
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;//FX_WeaponProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_red_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_red_small");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");


		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");

		trap->FX_RegisterEffect("blasters/red_deflect");

		cgs.effects.blasterDroidImpactEffect = trap->FX_RegisterEffect("blaster/droid_impact");
		break;

	case WP_A200_ACP_BATTLERIFLE:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_rifle.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/arraygun1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/arraygun2.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/arraygun3.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/arraygun4.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("Slugthrowers/muzzleflash_extrasmoke");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/arraygun1.mp3");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/arraygun2.mp3");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/arraygun3.mp3");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/arraygun4.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = NULL_SOUND;
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("Slugthrowers/muzzleflash_extrasmoke");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;//FX_WeaponProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("Slugthrowers/acp_shot_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("Slugthrowers/acp_shot_small");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");


		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("Slugthrowers/acp_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("Slugthrowers/acp_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("Slugthrowers/acp_impactsmall");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("Slugthrowers/acp_impactsmall");
		//"weapons / laser_red"
		trap->FX_RegisterEffect("blasters/red_deflect");

		cgs.effects.blasterDroidImpactEffect = trap->FX_RegisterEffect("blaster/droid_impact");
		break;

	case WP_A200_ACP_PISTOL:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_array.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/slugthrowers/acp_pistol1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/slugthrowers/acp_pistol1.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/slugthrowers/acp_pistol1.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/slugthrowers/acp_pistol1.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("slugthrowers/muzzleflash");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/slugthrowers/acp_pistol1.mp3");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/slugthrowers/acp_pistol1.mp3");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/slugthrowers/acp_pistol1.mp3");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/slugthrowers/acp_pistol1.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = NULL_SOUND;
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("slugthrowers/muzzleflash");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;//FX_WeaponProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("Slugthrowers/acp_shot_medium");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("Slugthrowers/acp_shot_medium");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");


		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("slugthrowers/acp_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("slugthrowers/acp_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/acp_impact_highcal");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/acp_impact_highcal");

		trap->FX_RegisterEffect("blasters/red_deflect");

		cgs.effects.blasterDroidImpactEffect = trap->FX_RegisterEffect("blaster/droid_impact");
		break;

	case WP_ACP_ARRAYGUN:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_array.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/pulseshotgun1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/pulseshotgun2.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/pulseshotgun3.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/pulseshotgun4.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Blue_big");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/pulseshotgun1.mp3");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/pulseshotgun2.mp3");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/pulseshotgun3.mp3");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/pulseshotgun4.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = NULL_SOUND;
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Blue_big");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;//FX_WeaponProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_GreenBlue_Ball_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_GreenBlue_Ball_small");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");


		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/green_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/green_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/aa-35c_green_impacttiny");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/aa-35c_green_impacttiny");
		//"weapons / laser_red"
		trap->FX_RegisterEffect("blasters/red_deflect");

		cgs.effects.blasterDroidImpactEffect = trap->FX_RegisterEffect("blaster/droid_impact");
		break;

	case WP_ACP_SNIPER_RIFLE:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_rifle.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/slugthrowers/acp_sniper1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/slugthrowers/acp_sniper2.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/slugthrowers/acp_sniper3.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/slugthrowers/acp_sniper4.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("slugthrowers/muzzleflash");
		weaponInfo->missileModel = NULL_FX;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/slugthrowers/acp_sniper1.mp3");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/slugthrowers/acp_sniper2.mp3");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/slugthrowers/acp_sniper3.mp3");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/slugthrowers/acp_sniper4.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = NULL_SOUND;
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("slugthrowers/muzzleflash");
		weaponInfo->altMissileModel = NULL_FX;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("Slugthrowers/acp_shot_big");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("Slugthrowers/acp_shot_big");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");


		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("slugthrowers/acp_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("slugthrowers/acp_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/acp_impact_highcal");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/acp_impact_highcal");
		trap->FX_RegisterEffect("blasters/red_deflect");
		break;

	case WP_ARC_CASTER_IMPERIAL:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/blaster/select.wav");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/arc_caster1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/arc_caster1.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/arc_caster1.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/arc_caster1.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = trap->S_RegisterSound("sound/weapons/charge_arccaster.wav");
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash_arccaster");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = 0;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/effects/spark1_improved.wav");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/effects/spark1_improved.wav");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/effects/spark1_improved.wav");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/effects/spark1_improved.wav");
		weaponInfo->altFiringSound	 = trap->S_RegisterSound("sound/effects/spark1_improved.wav");
		weaponInfo->altChargeSound	 =  trap->S_RegisterSound("sound/weapons/blasters/e11b_4.mp3");
		weaponInfo->altMuzzleEffect	 = trap->FX_RegisterEffect("blasters/muzzleflash_arccaster");
		weaponInfo->altMissileModel  = NULL_HANDLE;
		weaponInfo->altMissileSound  = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = 0;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_electricity");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_electricity");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");




		//weaponInfo->tracelineShader = trap->R_RegisterShader("gfx/blasters/electricity_deform");

		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/electric_impacttiny");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/electric_impacttiny");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("blasters/electric_impactmedium");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("blasters/electric_impactmedium");

		cgs.media.cannonChargeFlash = trap->FX_RegisterEffect("gfx/electricity/electricityburst");
		cgs.media.LightningtradeShader = trap->R_RegisterShader("gfx/blasters/electricity_deform");
		cgs.media.LightningtradeShader2 = trap->R_RegisterShader("gfx/blasters/BlasterBolt_Beam2_Blue");
		break;

	case WP_BOWCASTER_CLASSIC:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/bowcaster/select.wav");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_small1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_small2.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_small3.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_small4.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Green_medium");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_big1.mp3");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_big2.mp3");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_big3.mp3");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_big4.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = trap->S_RegisterSound("sound/weapons/bowcaster/altcharge.wav");
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Green_medium");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;

		weaponInfo->Chargingfx = NULL_FX;
		weaponInfo->Altchargingfx = trap->FX_RegisterEffect("weapons/charge_bowcaster_cheap");

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_GreenYellow_Flare_medium");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_GreenYellow_Flare_medium");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/bowcaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/bowcaster/flesh_impact"); // not sure about this one
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/bowcaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/bowcaster/wall_impact");


		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/green_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/green_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_green_medium");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_green_medium");
		trap->FX_RegisterEffect("blasters/green_deflect");
		break;

	case WP_WOOKIE_BOWCASTER_SCOPE:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/bowcaster/select.wav");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_big1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_big2.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_big3.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_big4.mp3");
		weaponInfo->flashSound[4] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_small1.mp3");
		weaponInfo->flashSound[5] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_small2.mp3");
		weaponInfo->flashSound[6] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_small3.mp3");
		weaponInfo->flashSound[7] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_small4.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Green_medium");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_big1.mp3");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_big2.mp3");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_big3.mp3");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/crossbow_big4.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = trap->S_RegisterSound("sound/weapons/bowcaster/altcharge.wav");
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Green_medium");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_Green_Flare_medium");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_Green_Flare_medium");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/bowcaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/bowcaster/flesh_impact"); // not sure about this one
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/bowcaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/bowcaster/wall_impact");


		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/green_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/green_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_green_medium");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_green_medium");
		trap->FX_RegisterEffect("blasters/green_deflect");
		break;

	case WP_BRYAR_CARBINE:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_carbine.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle2.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle3.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle4.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle1.mp3");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle2.mp3");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle3.mp3");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle4.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = NULL_SOUND;
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;//FX_WeaponProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_redorange_medium");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_redorange_medium");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");


		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");

		trap->FX_RegisterEffect("blasters/red_deflect");

		cgs.effects.blasterDroidImpactEffect = trap->FX_RegisterEffect("blaster/droid_impact");
		break;

	case WP_BRYAR_RIFLE:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_carbine.mp3");
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/blaster/select.wav");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle2.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle3.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle4.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle1.mp3");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle2.mp3");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle3.mp3");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle4.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = NULL_SOUND;
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_red_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_RedOrange_Flare_medium");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");


		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		trap->FX_RegisterEffect("blasters/red_deflect");
		break;

	case WP_BRYAR_RIFLE_SCOPE:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/sound/weapons/select_carbine.mp3");
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/blaster/select.wav");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle2.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle3.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle4.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle1.mp3");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle2.mp3");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle3.mp3");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/bryar_rifle4.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = trap->S_RegisterSound("sound/weapons/charge_bryar.mp3");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_red_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_RedOrange_Flare_medium");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");


		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		trap->FX_RegisterEffect("blasters/red_deflect");
		break;

	case WP_PULSECANON:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_pulse.wav");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/plasmacannon1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/plasmacannon2.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/plasmacannon3.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/plasmacannon4.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Blue_medium");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;

		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/repeater/alt_fire.wav");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = trap->S_RegisterSound("sound/weapons/SBDarm/cannon_charge.mp3");
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Blue_Big");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_RepeaterAltProjectileThink;

		weaponInfo->Chargingfx = NULL_FX;
		weaponInfo->Altchargingfx = trap->FX_RegisterEffect("blasters/shot_BluePurple_Ball_medium");

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_BluePurple_Ball_medium");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_BluePurple_Ball_big");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/concussion/explosion"); // not sure about this one
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/concussion/explosion");


		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("weapons/blaster_impact_blue_big");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("weapons/blaster_impact_blue_huge");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_blue_big_fire");//prime efx
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_blue_huge_fire");
		trap->FX_RegisterEffect("blasters/blue_deflect");
		break;

	case WP_PROTON_CARBINE_RIFLE:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_carbine.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/protoncarbine1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/protoncarbine2.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/protoncarbine3.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/protoncarbine4.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Purple_small");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/protoncarbine1.mp3");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/protoncarbine2.mp3");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/protoncarbine3.mp3");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/protoncarbine4.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = NULL_SOUND;
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Purple_medium");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;//FX_WeaponProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_RedPurple_Ball_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_RedPurple_Flare_medium");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");


		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/purple_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/purple_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_purple_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_purple_big_fire");

		trap->FX_RegisterEffect("blasters/red_deflect");

		cgs.effects.blasterDroidImpactEffect = trap->FX_RegisterEffect("blaster/droid_impact");
		break;

	case WP_DH_17_PISTOL:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_pistol.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/dl-44_1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/dl-44_1.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/dl-44_1.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/dl-44_1.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->missileModel = NULL_HANDLE;
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_WeaponProjectileThink;
		weaponInfo->powerupShotRenderfx = NULL_FX;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/blasters/dl-44_1.mp3");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/blasters/dl-44_1.mp3");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/blasters/dl-44_1.mp3");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/blasters/dl-44_1.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = NULL_SOUND;
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Red_medium");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;//FX_WeaponProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_redpurple_medium");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_redpurple_medium");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/flesh_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/blaster/wall_impact");


		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");

		trap->FX_RegisterEffect("blasters/red_deflect");

		cgs.effects.blasterDroidImpactEffect = trap->FX_RegisterEffect("blaster/droid_impact");
		break;

// old weapons don't add anything under here. Stoiss
	case WP_ROCKET_LAUNCHER:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/rocket/select.wav");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher1.wav");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher2.wav");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher3.wav");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher4.wav");
		weaponInfo->flashSound[4] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher5.wav");
		weaponInfo->flashSound[5] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher6.wav");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = NULL_SOUND;
		weaponInfo->muzzleEffect = trap->FX_RegisterEffect("slugthrowers/muzzleflash_big");
		weaponInfo->missileModel = trap->R_RegisterModel("models/ammo/rocket1_proj.md3");
		weaponInfo->missileSound = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher_loop.wav");
		weaponInfo->missileDlight = 125;
		VectorSet(weaponInfo->missileDlightColor, 1.0, 1.0, 0.5);
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_RocketProjectileThink;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher1.wav");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher2.wav");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher3.wav");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher4.wav");
		weaponInfo->altFlashSound[4] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher5.wav");
		weaponInfo->altFlashSound[5] = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher6.wav");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = NULL_SOUND;
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("slugthrowers/muzzleflash_big");
		weaponInfo->altMissileModel = trap->R_RegisterModel("models/ammo/rocket1_proj.md3");
		weaponInfo->altMissileSound = trap->S_RegisterSound("sound/weapons/slugthrowers/rocketlauncher_loop.wav");
		weaponInfo->altMissileDlight = 125;
		VectorSet(weaponInfo->altMissileDlightColor, 1.0, 1.0, 0.5);
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_PulseRocketAltProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("explosives/shot_rpg");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("explosives/shot_rpg");
		
		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/rocket/explosion");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("orginal_weapon_efx/rocket/explosion"); // not sure about this one
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/rocket/explosion");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("orginal_weapon_efx/rocket/explosion");
		

		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("explosives/demomedium");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("explosives/demomedium");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("explosives/demomedium");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("explosives/demomedium");
		break;

	case WP_THERMAL:
		weaponInfo->selectSound			= trap->S_RegisterSound("sound/weapons/thermal/select.wav");
		weaponInfo->flashSound[0]		= trap->S_RegisterSound( "sound/weapons/melee/swing1.mp3");
		weaponInfo->flashSound[1]		= trap->S_RegisterSound("sound/weapons/melee/swing2.mp3");
		weaponInfo->flashSound[2]		= trap->S_RegisterSound("sound/weapons/melee/swing3.mp3");
		weaponInfo->flashSound[3]		= trap->S_RegisterSound("sound/weapons/melee/swing4.mp3");
		weaponInfo->firingSound			= NULL_SOUND;
		weaponInfo->chargeSound			= trap->S_RegisterSound( "sound/weapons/thermal/charge.wav");
		weaponInfo->muzzleEffect		= NULL_FX;
		weaponInfo->missileModel		= trap->R_RegisterModel( "models/weapons/Grenade_Thermal/model_proj.md3" );
		weaponInfo->missileSound		= NULL_SOUND;
		weaponInfo->missileDlight		= 0;
		weaponInfo->missileHitSound		= NULL_SOUND;
		weaponInfo->missileTrailFunc	= 0;
		weaponInfo->altFlashSound[0]	= trap->S_RegisterSound( "sound/weapons/melee/swing1.mp3");
		weaponInfo->altFlashSound[1]	= trap->S_RegisterSound("sound/weapons/melee/swing2.mp3");
		weaponInfo->altFlashSound[2]	= trap->S_RegisterSound("sound/weapons/melee/swing3.mp3");
		weaponInfo->altFlashSound[3]	= trap->S_RegisterSound("sound/weapons/melee/swing4.mp3");
		weaponInfo->altFiringSound		= NULL_SOUND;
		weaponInfo->altChargeSound		= trap->S_RegisterSound( "sound/weapons/grenade_cook.wav");
		weaponInfo->altMuzzleEffect		= NULL_FX;
		weaponInfo->altMissileModel		= trap->R_RegisterModel( "models/weapons/Grenade_Thermal/model_proj.md3" );
		weaponInfo->altMissileSound		= NULL_SOUND;
		weaponInfo->altMissileDlight	= 0;
		weaponInfo->altMissileHitSound	= NULL_SOUND;
		weaponInfo->altMissileTrailFunc = 0;

		cgs.effects.thermalShockwaveEffect = trap->FX_RegisterEffect("orginal_weapon_efx/thermal/shockwave");
		cgs.effects.thermalExplosionAltEffect = trap->FX_RegisterEffect("orginal_weapon_efx/thermal/explosion");

		cgs.effects.thermalShockwaveEffectEffectEnhancedFX = trap->FX_RegisterEffect("explosives/shockwave");
		cgs.effects.thermalExplosionEffectEnhancedFX = trap->FX_RegisterEffect("explosives/fragmedium");
		cgs.effects.thermalExplosionAltEffectEnhancedFX = trap->FX_RegisterEffect("explosives/fragmedium");

		cgs.media.grenadeBounce1		= trap->S_RegisterSound( "sound/weapons/thermal/bounce1.wav" );
		cgs.media.grenadeBounce2		= trap->S_RegisterSound( "sound/weapons/thermal/bounce2.wav" );
		trap->S_RegisterSound( "sound/weapons/grenade_thermdetloop.wav" );
		trap->S_RegisterSound( "sound/weapons/thermal/warning.wav" );
		break;

	case WP_TRIP_MINE:
		weaponInfo->selectSound			= trap->S_RegisterSound("sound/weapons/detpack/select.wav");
		weaponInfo->flashSound[0]		= trap->S_RegisterSound( "sound/weapons/laser_trap/fire.wav");
		weaponInfo->firingSound			= NULL_SOUND;
		weaponInfo->chargeSound			= NULL_SOUND;
		weaponInfo->muzzleEffect		= NULL_FX;
		weaponInfo->missileModel		= trap->R_RegisterModel("models/Weapons/Mine_LaserTrip/projectile.md3");
		weaponInfo->missileSound		= NULL_SOUND;
		weaponInfo->missileDlight		= 0;
		weaponInfo->missileHitSound		= NULL_SOUND;
		weaponInfo->missileTrailFunc	= 0;
		weaponInfo->altFlashSound[0]	= trap->S_RegisterSound( "sound/weapons/laser_trap/fire.wav");
		weaponInfo->altFiringSound		= NULL_SOUND;
		weaponInfo->altChargeSound		= NULL_SOUND;
		weaponInfo->altMuzzleEffect		= NULL_FX;
		weaponInfo->altMissileModel		= trap->R_RegisterModel("models/Weapons/Mine_LaserTrip/projectile.md3");
		weaponInfo->altMissileSound		= NULL_SOUND;
		weaponInfo->altMissileDlight	= 0;
		weaponInfo->altMissileHitSound	= NULL_SOUND;
		weaponInfo->altMissileTrailFunc = 0;
		cgs.effects.tripmineLaserFX = trap->FX_RegisterEffect("weapons/laser_blue.efx");
		cgs.effects.tripmineGlowFX = trap->FX_RegisterEffect("tripMine/glowbit.efx");
		trap->FX_RegisterEffect( "explosives/demosmall" );
		// NOTENOTE temp stuff
		trap->S_RegisterSound( "sound/weapons/laser_trap/stick.wav" );
		trap->S_RegisterSound( "sound/weapons/laser_trap/warning.wav" );
		break;

	case WP_DET_PACK:
		weaponInfo->selectSound			= trap->S_RegisterSound("sound/weapons/detpack/select.wav");
		weaponInfo->flashSound[0]		= trap->S_RegisterSound( "sound/weapons/detpack/fire.wav");
		weaponInfo->firingSound			= NULL_SOUND;
		weaponInfo->chargeSound			= NULL_SOUND;
		weaponInfo->muzzleEffect		= NULL_FX;
		weaponInfo->missileModel		= trap->R_RegisterModel( "models/weapons2/detpack/det_pack.md3" );
		weaponInfo->missileSound		= NULL_SOUND;
		weaponInfo->missileDlight		= 0;
		weaponInfo->missileHitSound		= NULL_SOUND;
		weaponInfo->missileTrailFunc	= 0;
		weaponInfo->altFlashSound[0]	= trap->S_RegisterSound( "sound/weapons/detpack/fire.wav");
		weaponInfo->altFiringSound		= NULL_SOUND;
		weaponInfo->altChargeSound		= NULL_SOUND;
		weaponInfo->altMuzzleEffect		= NULL_FX;
		weaponInfo->altMissileModel		= trap->R_RegisterModel( "models/weapons2/detpack/det_pack.md3" );
		weaponInfo->altMissileSound		= NULL_SOUND;
		weaponInfo->altMissileDlight	= 0;
		weaponInfo->altMissileHitSound	= NULL_SOUND;
		weaponInfo->altMissileTrailFunc = 0;
		trap->R_RegisterModel( "models/weapons2/detpack/det_pack.md3" );
		trap->S_RegisterSound( "sound/weapons/detpack/stick.wav" );
		trap->S_RegisterSound( "sound/weapons/detpack/warning.wav" );
		trap->S_RegisterSound( "sound/weapons/explosions/explode5.wav" );
		break;

	case WP_TURRET:
		weaponInfo->flashSound[0]		= NULL_SOUND;
		weaponInfo->firingSound			= NULL_SOUND;
		weaponInfo->chargeSound			= NULL_SOUND;
		weaponInfo->muzzleEffect		= NULL_HANDLE;
		weaponInfo->missileModel		= NULL_HANDLE;
		weaponInfo->missileSound		= NULL_SOUND;
		weaponInfo->missileDlight		= 0;
		weaponInfo->missileHitSound		= NULL_SOUND;
		weaponInfo->missileTrailFunc	= FX_WeaponProjectileThink;
		

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blaster/shot");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blaster/shot");

		weaponInfo->fleshImpactEffect = trap->FX_RegisterEffect("blaster/flesh_impact");
		weaponInfo->altFleshImpactEffect = trap->FX_RegisterEffect("blaster/flesh_impact");
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("blaster/wall_impact");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("blaster/wall_impact");
		
		
		weaponInfo->EnhancedFX_fleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact = trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx = trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		trap->FX_RegisterEffect("blasters/red_deflect");
		break;
	 default:
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 1 );
		weaponInfo->flashSound[0] = trap->S_RegisterSound( "sound/weapons/rocket/rocklf1a.wav" );
		break;
	}
}
