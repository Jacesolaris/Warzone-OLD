//
// cg_weaponinit.c -- events and effects dealing with weapons
#include "cg_local.h"
#include "fx_local.h"

extern void CG_RegisterDefaultBlasterShaders(void);

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

static qboolean CG_IsGhoul2Model(const char *modelPath)
{
	size_t len = strlen(modelPath);
	if (len <= 4)
	{
		return qfalse;
	}

	if (Q_stricmp(modelPath + len - 4, ".glm") == 0)
	{
		return qtrue;
	}

	return qfalse;
}

static void CG_LoadViewWeapon(weaponInfo_t *weapon, const char *modelPath)
{
	char file[MAX_QPATH];
	char *slash;
	//int root;
	//int model;

	Q_strncpyz(file, modelPath, sizeof(file));
	slash = Q_strrchr(file, '/');

	Q_strncpyz(slash, "/model_default.skin", sizeof(file) - (slash - file));
	weapon->viewModelSkin = trap->R_RegisterSkin(file);

	trap->G2API_InitGhoul2Model(&weapon->g2ViewModel, modelPath, 0, weapon->viewModelSkin, 0, 0, 0);
	if (!trap->G2_HaveWeGhoul2Models(weapon->g2ViewModel))
	{
		return;
	}

	/*memset(weapon->viewModelAnims, 0, sizeof(weapon->viewModelAnims));
	trap->G2API_GetGLAName(weapon->g2ViewModel, 0, file);
	if (!file[0])
	{
		return;
	}

	slash = Q_strrchr(file, '/');

	Q_strncpyz(slash, "/animation.cfg", sizeof(file) - (slash - file));
	CG_LoadViewWeaponAnimations(weapon, file);*/
}

void CG_SetWeaponHandModel(weaponInfo_t    *weaponInfo, int weaponType)
{
	if (weaponType == WEAPONTYPE_PISTOL)
		weaponInfo->handsModel = trap->R_RegisterModel("models/weapons2/briar_pistol/briar_pistol_hand.md3");
	else if (weaponType == WEAPONTYPE_BLASTER || weaponType == WEAPONTYPE_NONE)
		weaponInfo->handsModel = trap->R_RegisterModel("models/weapons2/blaster_r/blaster_hand.md3");
	else if (weaponType == WEAPONTYPE_SNIPER)
		weaponInfo->handsModel = trap->R_RegisterModel("models/weapons2/disruptor/disruptor_hand.md3");
	else if (weaponType == WEAPONTYPE_ROCKET_LAUNCHER)
		weaponInfo->handsModel = trap->R_RegisterModel("models/weapons2/merr_sonn/merr_sonn_hand.md3");
	else if (weaponType == WEAPONTYPE_GRENADE)
		weaponInfo->handsModel = trap->R_RegisterModel("models/weapons2/thermal/thermal_hand.md3");
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
	char			extensionlessModel[MAX_QPATH];

	if (weaponNum < WP_NONE) 
	{// Missing SP weapons...
		return;
	}

	// Always register all the blaster bolt colors...
	CG_RegisterDefaultBlasterShaders();

	if (!cgs.effects.rocketExplosionEffect) cgs.effects.rocketExplosionEffect = trap->FX_RegisterEffect("explosives/demomedium");

	weaponInfo = &cg_weapons[weaponNum];

	if ( weaponNum == 0 ) {
		return;
	}

	if ( weaponInfo->registered ) {
		return;
	}

	memset( weaponInfo, 0, sizeof( *weaponInfo ) );
	weaponInfo->registered = qtrue;

#ifndef __FULL_VERSION_WEAPONS__
	// In non-full-version weapons mode, A280 all the guns...
	if (weaponNum != WP_MELEE
		&& weaponNum != WP_SABER
		&& weaponNum != WP_A280
		&& weaponNum != WP_THERMAL
		&& weaponNum != WP_FRAG_GRENADE
		&& weaponNum != WP_FRAG_GRENADE_OLD
		&& weaponNum != WP_TRIP_MINE
		&& weaponNum != WP_DET_PACK)
	{
		for (item = bg_itemlist + 1; item->classname; item++) {
			if (item->giType == IT_WEAPON && item->giTag == WP_A280) {
				weaponInfo->item = item;
				break;
			}
		}
	}
	else
	{
		for (item = bg_itemlist + 1; item->classname; item++) {
			if (item->giType == IT_WEAPON && item->giTag == weaponNum) {
				weaponInfo->item = item;
				break;
			}
		}
	}
#else //__FULL_VERSION_WEAPONS__
	for ( item = bg_itemlist + 1 ; item->classname ; item++ ) {
		if ( item->giType == IT_WEAPON && item->giTag == weaponNum ) {
			weaponInfo->item = item;
			break;
		}
	}
#endif //__FULL_VERSION_WEAPONS__
	if ( !item->classname ) {
		trap->Error( ERR_DROP, "Couldn't find weapon %i", weaponNum );
		return;
	}

	CG_RegisterItemVisuals( item - bg_itemlist );

	// UQ1: Set all names in the bg_weapons.c struct and copy it here... Don't set them below...
	if (weaponInfo->item)
		weaponInfo->item->classname = weaponData[weaponNum].classname;

	// Clear gunPosition...
	VectorClear(weaponInfo->gunPosition);

	weaponInfo->viewModel = NULL_HANDLE;
	if (weaponInfo->g2ViewModel)
	{
		trap->G2API_CleanGhoul2Models(&weaponInfo->g2ViewModel);
		weaponInfo->g2ViewModel = NULL;
	}

	//trap->Print("^3Loading viewModel %s.\n", item->view_model);
	if (CG_IsGhoul2Model(item->view_model))
	{
		//trap->Print("^2Loaded glm viewModel for %s.\n", item->view_model);
		CG_LoadViewWeapon(weaponInfo, item->view_model);
	}
	else
	{
		// load in-view model also
		weaponInfo->viewModel = trap->R_RegisterModel(item->view_model);
	}
	
	// load cmodel before model so filecache works
	weaponInfo->weaponModel = trap->R_RegisterModel(item->world_model[0]);


	memset (weaponInfo->barrelModels, NULL_HANDLE, sizeof (weaponInfo->barrelModels));
    COM_StripExtension (item->view_model, extensionlessModel, sizeof(extensionlessModel));
    
    for ( i = 0; i < 4; i++ )
    {
        const char *barrelModel;
        int len;
        qhandle_t barrel;

		if( i == 0 )
			barrelModel = va("%s_barrel.md3", extensionlessModel);
		else
			barrelModel = va("%s_barrel%i.md3", extensionlessModel, i+1);

		len = strlen( barrelModel );
        
        if ( (len + 1) > MAX_QPATH )
        {
			trap->Print(S_COLOR_YELLOW "Warning: barrel model path %s is too long (%d chars). Max length is 63.\n", barrelModel, len);
            break;
        }
        
        barrel = trap->R_RegisterModel (barrelModel);

        if ( barrel == NULL_HANDLE )
        {
            break;
        }
        
        weaponInfo->barrelModels[i] = barrel;
		weaponInfo->barrelCount++;
    }
	

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
	weaponInfo->flashModel = NULL;

	if (weaponNum == WP_DISRUPTOR ||
		weaponNum == WP_FLECHETTE ||
		weaponNum == WP_REPEATER ||
		weaponNum == WP_ROCKET_LAUNCHER ||
		weaponNum == WP_CONCUSSION 	||	
		weaponNum == WP_Z6_BLASTER_CANON ||
		weaponNum == WP_PULSECANON ||
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

#if 0
		if (!weaponInfo->handsModel)
		{
			if (WeaponIsGrenade(weaponNum))
			{
				//CG_SetWeaponHandModel(weaponInfo, WEAPONTYPE_GRENADE);
				weaponInfo->handsModel = NULL;
			}
			else if (WeaponIsRocketLauncher(weaponNum))
			{
				//CG_SetWeaponHandModel(weaponInfo, WEAPONTYPE_ROCKET_LAUNCHER);
				weaponInfo->handsModel = NULL;
			}
			else if (WeaponIsSniper(weaponNum))
			{
				CG_SetWeaponHandModel(weaponInfo, WEAPONTYPE_SNIPER);
			}
			else if (WeaponIsPistol(weaponNum))
			{
				//CG_SetWeaponHandModel(weaponInfo, WEAPONTYPE_PISTOL);
				weaponInfo->handsModel = NULL;
			}
			else
			{
				CG_SetWeaponHandModel(weaponInfo, WEAPONTYPE_BLASTER);
			}
		}
#endif
	}
	else
	{
		weaponInfo->handsModel = NULL;
	}

	/*
	// Debugging...
	weaponInfo->bolt3DShader = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun, using this color shader...
	weaponInfo->bolt3DShaderAlt = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun's alt fire, using this color shader...
	weaponInfo->bolt3DLength = 1.0; // If not set, 1.0 is the default length.
	weaponInfo->bolt3DLengthAlt = 1.25; // If not set, 1.0 is the default length.
	weaponInfo->bolt3DWidth = 1.0; // If not set, 1.0 is the default length.
	weaponInfo->bolt3DWidthAlt = 1.25; // If not set, 1.0 is the default length.
	*/

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
		weaponInfo->bolt3DShader = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun, using this color shader...
		weaponInfo->bolt3DShaderAlt = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun's alt fire, using this color shader...
		weaponInfo->bolt3DLength = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DLengthAlt = 1.25; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidth = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidthAlt = 1.25; // If not set, 1.0 is the default length.

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

										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 11.0, 8.0, -5.0);
		weaponInfo->missileRenderfx = trap->FX_RegisterEffect( "explosives/shot_concussion" );
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect( "explosives/shot_concussion" );

		//needs this to player the efx faster to speed up fps on slow pcs
		weaponInfo->fleshImpactEffect						= trap->FX_RegisterEffect("explosives/concussion3medium");
		weaponInfo->altFleshImpactEffect					= trap->FX_RegisterEffect("explosives/concussion1medium");
		weaponInfo->missileWallImpactfx						= trap->FX_RegisterEffect("explosives/concussion3medium");
		weaponInfo->altMissileWallImpactfx					= trap->FX_RegisterEffect("explosives/concussion1medium");
		//for fast computers to run normal high efx
		weaponInfo->EnhancedFX_fleshImpact					= trap->FX_RegisterEffect("explosives/concussion3medium");
		weaponInfo->EnhancedFX_altfleshImpact				= trap->FX_RegisterEffect("explosives/concussion1medium");
		weaponInfo->EnhancedFX_missileWallImpactfx			= trap->FX_RegisterEffect("explosives/concussion3medium");
		weaponInfo->EnhancedFX_altmissileWallImpactfx		= trap->FX_RegisterEffect("explosives/concussion1medium");

		trap->R_RegisterShader("gfx/effects/blueLine");
		trap->R_RegisterShader("gfx/misc/whiteline2");
		break;

	case WP_BRYAR_PISTOL:
		weaponInfo->bolt3DShader = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun, using this color shader...
		weaponInfo->bolt3DShaderAlt = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun's alt fire, using this color shader...
		weaponInfo->bolt3DLength = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DLengthAlt = 1.25; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidth = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidthAlt = 1.25; // If not set, 1.0 is the default length.

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
										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 20, -4, -3);
		weaponInfo->Chargingfx = NULL_FX;
		weaponInfo->Altchargingfx = trap->FX_RegisterEffect("weapons/charge_bryar");

		weaponInfo->altMissileTrailFunc = FX_BryarAltProjectileThink;

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_redorange_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_redorange_small");

		weaponInfo->fleshImpactEffect						= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->altFleshImpactEffect					= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->missileWallImpactfx						= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->altMissileWallImpactfx					= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		
		
		weaponInfo->EnhancedFX_fleshImpact					= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact				= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx			= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx		= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		trap->FX_RegisterEffect("blasters/red_deflect");

		break;

	case WP_BRYAR_OLD:
		weaponInfo->bolt3DShader = cgs.media.orangeBlasterShot; // Setting this enables 3D bolts for this gun, using this color shader...
		weaponInfo->bolt3DShaderAlt = cgs.media.orangeBlasterShot; // Setting this enables 3D bolts for this gun's alt fire, using this color shader...
		weaponInfo->bolt3DLength = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DLengthAlt = 1.25; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidth = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidthAlt = 1.25; // If not set, 1.0 is the default length.

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
										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 18, - 3, - 2);

		weaponInfo->Chargingfx = NULL_FX;
		weaponInfo->Altchargingfx = trap->FX_RegisterEffect("weapons/charge_bryar");

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_redorange_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_redorange_small");

		weaponInfo->fleshImpactEffect					= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->altFleshImpactEffect				= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->missileWallImpactfx					= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->altMissileWallImpactfx				= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		
		
		weaponInfo->EnhancedFX_fleshImpact				= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact			= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx		= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx	= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		trap->FX_RegisterEffect("blasters/red_deflect");
		break;
	case WP_BLASTER:
	//case WP_EMPLACED_GUN:
		weaponInfo->bolt3DShader = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun, using this color shader...
		weaponInfo->bolt3DShaderAlt = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun's alt fire, using this color shader...
		weaponInfo->bolt3DLength = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DLengthAlt = 1.25; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidth = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidthAlt = 1.25; // If not set, 1.0 is the default length.

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
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;
										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 18, - 5, - 3);

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_red_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_red_small");

		weaponInfo->fleshImpactEffect						= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->altFleshImpactEffect					= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->missileWallImpactfx						= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->altMissileWallImpactfx					= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		
		
		weaponInfo->EnhancedFX_fleshImpact					= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact				= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx			= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx		= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");

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
		weaponInfo->muzzleEffect		= trap->FX_RegisterEffect("blasters/muzzleflash2_Red_small");
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
		weaponInfo->altMuzzleEffect		= trap->FX_RegisterEffect("blasters/muzzleflash2_Red_small");
		weaponInfo->altMissileModel		= NULL_HANDLE;
		weaponInfo->altMissileSound		= NULL_SOUND;
		weaponInfo->altMissileDlight	= 0;
		weaponInfo->altMissileHitSound	= NULL_SOUND;
		weaponInfo->altMissileTrailFunc = 0;
										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 10.0, -3.5, -0.0);

		weaponInfo->Chargingfx = trap->FX_RegisterEffect("weapons/charge_bryar");
		weaponInfo->Altchargingfx = NULL_FX;

		cgs.effects.disruptorWallImpactEffect 						= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		cgs.effects.disruptorFleshImpactEffect						= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		cgs.effects.disruptorEnhancedFX_missileWallImpactfx 		= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		cgs.effects.disruptorAltMissEffect							= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		cgs.effects.disruptorAltHitEffect							= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");

		trap->R_RegisterShader( "gfx/effects/redLine" );
		trap->R_RegisterShader( "gfx/misc/whiteline2" );
		trap->R_RegisterShader( "gfx/effects/smokeTrail" );

		trap->S_RegisterSound("sound/weapons/disruptor/zoomstart.wav");
		trap->S_RegisterSound("sound/weapons/disruptor/zoomend.wav");
		cgs.media.disruptorZoomLoop		= trap->S_RegisterSound( "sound/weapons/disruptor/zoomloop.wav" );
		break;

	case WP_BOWCASTER:
		weaponInfo->bolt3DShader = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun, using this color shader...
		weaponInfo->bolt3DShaderAlt = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun's alt fire, using this color shader...
		weaponInfo->bolt3DLength = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DLengthAlt = 1.25; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidth = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidthAlt = 1.25; // If not set, 1.0 is the default length.

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
										//	 X,	  ,Y   ,Z	
		VectorSet(weaponInfo->gunPosition,	 14, - 5, - 3);

		weaponInfo->Chargingfx = trap->FX_RegisterEffect("weapons/charge_bryar");
		weaponInfo->Altchargingfx = NULL_FX;

		weaponInfo->missileRenderfx			= trap->FX_RegisterEffect( "blasters/shot_red_small" );
		weaponInfo->altMissileRenderfx		= trap->FX_RegisterEffect( "blasters/shot_red_small" );

		weaponInfo->fleshImpactEffect					= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->altFleshImpactEffect				= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->missileWallImpactfx					= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->altMissileWallImpactfx				= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		
		
		weaponInfo->EnhancedFX_fleshImpact				= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact			= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx		= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx	= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		trap->FX_RegisterEffect("blasters/green_deflect");
		break;

	case WP_REPEATER:
		weaponInfo->bolt3DShader = cgs.media.yellowBlasterShot; // Setting this enables 3D bolts for this gun, using this color shader...
		weaponInfo->bolt3DShaderAlt = cgs.media.yellowBlasterShot; // Setting this enables 3D bolts for this gun's alt fire, using this color shader...
		weaponInfo->bolt3DLength = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DLengthAlt = 1.25; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidth = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidthAlt = 1.25; // If not set, 1.0 is the default length.

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
										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 11.0, 8.0, -5.0);

		weaponInfo->missileRenderfx		= trap->FX_RegisterEffect("blasters/shot_Yellow_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_Blue_Ball_big");

		weaponInfo->fleshImpactEffect					= trap->FX_RegisterEffect("blasters/yellow_flesh_impact");
		weaponInfo->altFleshImpactEffect				= trap->FX_RegisterEffect("explosives/concussion1medium");
		weaponInfo->missileWallImpactfx					= trap->FX_RegisterEffect("weapons/blaster_impact_yellow_medium_fire");
		weaponInfo->altMissileWallImpactfx				= trap->FX_RegisterEffect("explosives/concussion1medium");
		
		weaponInfo->EnhancedFX_fleshImpact				= trap->FX_RegisterEffect("blasters/yellow_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact			= trap->FX_RegisterEffect("explosives/concussion1medium");
		weaponInfo->EnhancedFX_missileWallImpactfx		= trap->FX_RegisterEffect("weapons/blaster_impact_yellow_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx	= trap->FX_RegisterEffect("explosives/concussion1medium");
		trap->FX_RegisterEffect("blasters/yellow_deflect");
		break;

	case WP_DEMP2:
		//weaponInfo->bolt3DShader = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun, using this color shader...
		//weaponInfo->bolt3DShaderAlt = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun's alt fire, using this color shader...
		//weaponInfo->bolt3DLength = 1.0; // If not set, 1.0 is the default length.
		//weaponInfo->bolt3DLengthAlt = 1.25; // If not set, 1.0 is the default length.
		//weaponInfo->bolt3DWidth = 1.0; // If not set, 1.0 is the default length.
		//weaponInfo->bolt3DWidthAlt = 1.25; // If not set, 1.0 is the default length.

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
										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 12.0, -3.5, -2.5);
		weaponInfo->Chargingfx = NULL_FX;
		weaponInfo->Altchargingfx = trap->FX_RegisterEffect("weapons/charge_arc");


		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("Blasters/shot_electricity");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("Blasters/shot_electricity");

		weaponInfo->fleshImpactEffect						= trap->FX_RegisterEffect("Blasters/electric_impactmedium");
		weaponInfo->altFleshImpactEffect					= trap->FX_RegisterEffect("Blasters/electric_impactbig");
		weaponInfo->missileWallImpactfx						= trap->FX_RegisterEffect("Blasters/electric_impactmedium");
		weaponInfo->altMissileWallImpactfx					= trap->FX_RegisterEffect("Blasters/electric_impactbig");
		
		weaponInfo->EnhancedFX_fleshImpact					= trap->FX_RegisterEffect("Blasters/electric_impactmedium");
		weaponInfo->EnhancedFX_altfleshImpact				= trap->FX_RegisterEffect("Blasters/electric_impactbig");
		weaponInfo->EnhancedFX_missileWallImpactfx			= trap->FX_RegisterEffect("Blasters/electric_impactmedium");
		weaponInfo->EnhancedFX_altmissileWallImpactfx		= trap->FX_RegisterEffect("Blasters/electric_impactbig");
		break;

	case WP_FLECHETTE:
		//weaponInfo->bolt3DShader = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun, using this color shader...
		//weaponInfo->bolt3DShaderAlt = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun's alt fire, using this color shader...
		//weaponInfo->bolt3DLength = 1.0; // If not set, 1.0 is the default length.
		//weaponInfo->bolt3DLengthAlt = 1.25; // If not set, 1.0 is the default length.
		//weaponInfo->bolt3DWidth = 1.0; // If not set, 1.0 is the default length.
		//weaponInfo->bolt3DWidthAlt = 1.25; // If not set, 1.0 is the default length.

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
										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 12.0, -3.5, -1.0);



		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("Slugthrowers/flechette_shot");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("Slugthrowers/flechette_alt_shot");

		weaponInfo->fleshImpactEffect						= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->altFleshImpactEffect					= trap->FX_RegisterEffect("effects/explosives/flechettesmall");
		weaponInfo->missileWallImpactfx						= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->altMissileWallImpactfx					= trap->FX_RegisterEffect("effects/explosives/flechettesmall");
		
		
		weaponInfo->EnhancedFX_fleshImpact					= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact				= trap->FX_RegisterEffect("effects/explosives/flechettesmall");
		weaponInfo->EnhancedFX_missileWallImpactfx			= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altmissileWallImpactfx		= trap->FX_RegisterEffect("effects/explosives/flechettesmall");
		break;

		//Add new guns here
	case WP_A280:
		weaponInfo->bolt3DShader = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun, using this color shader...
		weaponInfo->bolt3DShaderAlt = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun's alt fire, using this color shader...
		weaponInfo->bolt3DLength = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DLengthAlt = 1.25; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidth = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidthAlt = 1.25; // If not set, 1.0 is the default length.

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
										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 12, -4, -4);

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_red_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_red_small");

		weaponInfo->fleshImpactEffect							= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->altFleshImpactEffect						= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->missileWallImpactfx							= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->altMissileWallImpactfx						= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		
		
		weaponInfo->EnhancedFX_fleshImpact						= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact					= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx				= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx			= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		trap->FX_RegisterEffect("blasters/red_deflect");
		trap->S_RegisterSound("sound/weapons/disruptor/zoomstart.wav");
		trap->S_RegisterSound("sound/weapons/disruptor/zoomend.wav");

		cgs.media.disruptorZoomLoop = trap->S_RegisterSound("sound/weapons/disruptor/zoomloop.wav");
		break;

	case WP_T21:
		weaponInfo->bolt3DShader = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun, using this color shader...
		weaponInfo->bolt3DShaderAlt = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun's alt fire, using this color shader...
		weaponInfo->bolt3DLength = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DLengthAlt = 1.25; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidth = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidthAlt = 1.25; // If not set, 1.0 is the default length.

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
										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 10, - 3, - 4);

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_Red_medium");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_Red_big");

		weaponInfo->fleshImpactEffect					= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->altFleshImpactEffect				= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->missileWallImpactfx					= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->altMissileWallImpactfx				= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		
		
		weaponInfo->EnhancedFX_fleshImpact				= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact			= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx		= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx	= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		trap->FX_RegisterEffect("blasters/red_deflect");
		break;

	case WP_EE3:
		weaponInfo->bolt3DShader = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun, using this color shader...
		weaponInfo->bolt3DShaderAlt = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun's alt fire, using this color shader...
		weaponInfo->bolt3DLength = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DLengthAlt = 1.25; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidth = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidthAlt = 1.25; // If not set, 1.0 is the default length.

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
										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 14, -3, -3);

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_redorange_medium");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_RedOrange_Flare_medium");

		weaponInfo->fleshImpactEffect					= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->altFleshImpactEffect				= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->missileWallImpactfx					= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium");
		weaponInfo->altMissileWallImpactfx				= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium");
		
		
		weaponInfo->EnhancedFX_fleshImpact				= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact			= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx		= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium");
		weaponInfo->EnhancedFX_altmissileWallImpactfx	= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium");
		trap->FX_RegisterEffect("blasters/red_deflect");
		break;

	case WP_DLT_19:
		weaponInfo->bolt3DShader = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun, using this color shader...
		weaponInfo->bolt3DShaderAlt = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun's alt fire, using this color shader...
		weaponInfo->bolt3DLength = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DLengthAlt = 1.25; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidth = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidthAlt = 1.25; // If not set, 1.0 is the default length.

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
									   //  X,  ,Y  ,Z	
		VectorSet(weaponInfo->gunPosition, 12, -4, -4);

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_red_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_red_small");

		weaponInfo->fleshImpactEffect					= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->altFleshImpactEffect				= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->missileWallImpactfx					= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->altMissileWallImpactfx				= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		
		
		weaponInfo->EnhancedFX_fleshImpact				= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact			= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx		= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx	= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		trap->FX_RegisterEffect("blasters/red_deflect");
		break;

	case WP_DC_15A_RIFLE:
		weaponInfo->bolt3DShader = cgs.media.blueBlasterShot; // Setting this enables 3D bolts for this gun, using this color shader...
		weaponInfo->bolt3DShaderAlt = cgs.media.blueBlasterShot; // Setting this enables 3D bolts for this gun's alt fire, using this color shader...
		weaponInfo->bolt3DLength = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DLengthAlt = 1.25; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidth = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidthAlt = 1.25; // If not set, 1.0 is the default length.

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
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash_blue_big");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_RepeaterAltProjectileThink;
										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 12, -4, -4);
		
		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_blue_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_Blue_Ball_big");

		weaponInfo->fleshImpactEffect						= trap->FX_RegisterEffect("blasters/blue_flesh_impact");
		weaponInfo->altFleshImpactEffect					= trap->FX_RegisterEffect("explosives/concussion1medium");
		weaponInfo->missileWallImpactfx						= trap->FX_RegisterEffect("weapons/blaster_impact_blue_medium_fire");
		weaponInfo->altMissileWallImpactfx					= trap->FX_RegisterEffect("explosives/concussion1medium");
		
		
		weaponInfo->EnhancedFX_fleshImpact					= trap->FX_RegisterEffect("blasters/blue_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact				= trap->FX_RegisterEffect("explosives/concussion1medium");
		weaponInfo->EnhancedFX_missileWallImpactfx			= trap->FX_RegisterEffect("weapons/blaster_impact_blue_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx		= trap->FX_RegisterEffect("explosives/concussion1medium");
		trap->FX_RegisterEffect("blasters/blue_deflect");
		break;

	case WP_Z6_BLASTER_CANON:
		weaponInfo->bolt3DShader = cgs.media.blueBlasterShot; // Setting this enables 3D bolts for this gun, using this color shader...
		weaponInfo->bolt3DShaderAlt = cgs.media.blueBlasterShot; // Setting this enables 3D bolts for this gun's alt fire, using this color shader...
		weaponInfo->bolt3DLength = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DLengthAlt = 1.25; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidth = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidthAlt = 1.25; // If not set, 1.0 is the default length.

		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/select_h_launcher.mp3");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/minigun/fire.wav");
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
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash2_Blue_medium");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;
		weaponInfo->spinSound = trap->S_RegisterSound("sound/weapons/z6/spinny.wav");
		weaponInfo->spindownSound = trap->S_RegisterSound("sound/weapons/z6/chaingun_spindown.wav");
										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 11.0, 8.0, -5.0);
		weaponInfo->Chargingfx = NULL_FX;
		weaponInfo->Altchargingfx = trap->FX_RegisterEffect("blasters/shot_Blue_Ball_medium");

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_blue_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_Blue_Ball_big");

		weaponInfo->fleshImpactEffect					= trap->FX_RegisterEffect("blasters/blue_flesh_impact");
		weaponInfo->altFleshImpactEffect				= trap->FX_RegisterEffect("explosives/concussion1medium");
		weaponInfo->missileWallImpactfx					= trap->FX_RegisterEffect("weapons/blaster_impact_blue_medium_fire");
		weaponInfo->altMissileWallImpactfx				= trap->FX_RegisterEffect("explosives/concussion1medium");
		

		weaponInfo->EnhancedFX_fleshImpact				= trap->FX_RegisterEffect("blasters/blue_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact			= trap->FX_RegisterEffect("explosives/concussion1medium");
		weaponInfo->EnhancedFX_missileWallImpactfx		= trap->FX_RegisterEffect("weapons/blaster_impact_blue_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx	= trap->FX_RegisterEffect("explosives/concussion1medium");
		trap->FX_RegisterEffect("blasters/blue_deflect");
		cgs.media.lightningFlash = trap->R_RegisterShader("gfx/misc/lightningFlash");
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
										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 8.0, 10.0, -5.5);
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("weapons/grenaderibbon_red");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("weapons/grenaderibbon_red");

		weaponInfo->fleshImpactEffect					= trap->FX_RegisterEffect("explosives/fragmedium");
		weaponInfo->altFleshImpactEffect				= trap->FX_RegisterEffect("explosives/fragmedium");
		weaponInfo->missileWallImpactfx					= trap->FX_RegisterEffect("explosives/fragmedium");
		weaponInfo->altMissileWallImpactfx				= trap->FX_RegisterEffect("explosives/fragmedium");
		
		weaponInfo->EnhancedFX_fleshImpact				= trap->FX_RegisterEffect("explosives/fragmedium");
		weaponInfo->EnhancedFX_altfleshImpact			= trap->FX_RegisterEffect("explosives/fragmedium");
		weaponInfo->EnhancedFX_missileWallImpactfx		= trap->FX_RegisterEffect("explosives/fragmedium");
		weaponInfo->EnhancedFX_altmissileWallImpactfx	= trap->FX_RegisterEffect("explosives/fragmedium");


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
										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 8.0, 10.0, -5.5);
		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("weapons/grenaderibbon_red");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("weapons/grenaderibbon_red");

		weaponInfo->fleshImpactEffect					= trap->FX_RegisterEffect("explosives/fragmedium");
		weaponInfo->altFleshImpactEffect				= trap->FX_RegisterEffect("explosives/fragmedium");
		weaponInfo->missileWallImpactfx					= trap->FX_RegisterEffect("explosives/fragmedium");
		weaponInfo->altMissileWallImpactfx				= trap->FX_RegisterEffect("explosives/fragmedium");
		
		weaponInfo->EnhancedFX_fleshImpact				= trap->FX_RegisterEffect("explosives/fragmedium");
		weaponInfo->EnhancedFX_altfleshImpact			= trap->FX_RegisterEffect("explosives/fragmedium");
		weaponInfo->EnhancedFX_missileWallImpactfx		= trap->FX_RegisterEffect("explosives/fragmedium");
		weaponInfo->EnhancedFX_altmissileWallImpactfx	= trap->FX_RegisterEffect("explosives/fragmedium");


		cgs.media.grenadeBounce1 = trap->S_RegisterSound("sound/weapons/grenade_bounce1.mp3");
		cgs.media.grenadeBounce2 = trap->S_RegisterSound("sound/weapons/grenade_bounce2.mp3");
		trap->S_RegisterSound("sound/weapons/grenade_thermdetloop.mp3");
		break;

	case WP_BRYAR_CARBINE:
		weaponInfo->bolt3DShader = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun, using this color shader...
		weaponInfo->bolt3DShaderAlt = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun's alt fire, using this color shader...
		weaponInfo->bolt3DLength = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DLengthAlt = 1.25; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidth = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidthAlt = 1.25; // If not set, 1.0 is the default length.

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
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;
										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 10.0, - 3.0, - 5.4);

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_redorange_medium");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_redorange_medium");

		weaponInfo->fleshImpactEffect					= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->altFleshImpactEffect				= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->missileWallImpactfx					= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->altMissileWallImpactfx				= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");


		weaponInfo->EnhancedFX_fleshImpact				= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact			= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx		= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx	= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");

		trap->FX_RegisterEffect("blasters/red_deflect");

		cgs.effects.blasterDroidImpactEffect = trap->FX_RegisterEffect("blaster/droid_impact");
		break;

	case WP_BRYAR_RIFLE:
		weaponInfo->bolt3DShader = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun, using this color shader...
		weaponInfo->bolt3DShaderAlt = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun's alt fire, using this color shader...
		weaponInfo->bolt3DLength = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DLengthAlt = 1.25; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidth = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidthAlt = 1.25; // If not set, 1.0 is the default length.

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
										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 10.0, - 3.0, - 5.4);

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_red_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_RedOrange_Flare_medium");

		weaponInfo->fleshImpactEffect					= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->altFleshImpactEffect				= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->missileWallImpactfx					= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->altMissileWallImpactfx				= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");


		weaponInfo->EnhancedFX_fleshImpact				= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact			= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx		= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx	= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		trap->FX_RegisterEffect("blasters/red_deflect");
		break;

	case WP_BRYAR_RIFLE_SCOPE:
		weaponInfo->bolt3DShader = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun, using this color shader...
		weaponInfo->bolt3DShaderAlt = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun's alt fire, using this color shader...
		weaponInfo->bolt3DLength = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DLengthAlt = 1.25; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidth = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidthAlt = 1.25; // If not set, 1.0 is the default length.

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
										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 7.0, - 4.0, - 4.0);

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_red_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_RedOrange_Flare_medium");

		weaponInfo->fleshImpactEffect					= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->altFleshImpactEffect				= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->missileWallImpactfx					= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->altMissileWallImpactfx				= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");


		weaponInfo->EnhancedFX_fleshImpact				= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact			= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx		= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx	= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		trap->FX_RegisterEffect("blasters/red_deflect");
		break;

	case WP_PULSECANON:
		weaponInfo->bolt3DShader = cgs.media.BlasterBolt_Cap_BluePurple; // Setting this enables 3D bolts for this gun, using this color shader...
		weaponInfo->bolt3DShaderAlt = cgs.media.BlasterBolt_Cap_BluePurple; // Setting this enables 3D bolts for this gun's alt fire, using this color shader...
		weaponInfo->bolt3DLength = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DLengthAlt = 1.25; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidth = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidthAlt = 1.25; // If not set, 1.0 is the default length.

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
		weaponInfo->altMuzzleEffect = trap->FX_RegisterEffect("blasters/muzzleflash_Blue_Big");
		weaponInfo->altMissileModel = NULL_HANDLE;
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_RepeaterAltProjectileThink;
										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 15, -4, - 6);

		weaponInfo->Chargingfx = NULL_FX;
		weaponInfo->Altchargingfx = trap->FX_RegisterEffect("blasters/shot_BluePurple_Ball_medium");

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_BluePurple_Ball_medium");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_BluePurple_Ball_big");

		weaponInfo->fleshImpactEffect					= trap->FX_RegisterEffect("weapons/blaster_impact_blue_big");
		weaponInfo->altFleshImpactEffect				= trap->FX_RegisterEffect("weapons/blaster_impact_blue_huge");
		weaponInfo->missileWallImpactfx					= trap->FX_RegisterEffect("weapons/blaster_impact_blue_big_fire");
		weaponInfo->altMissileWallImpactfx				= trap->FX_RegisterEffect("weapons/blaster_impact_blue_huge_fire");


		weaponInfo->EnhancedFX_fleshImpact				= trap->FX_RegisterEffect("weapons/blaster_impact_blue_big");
		weaponInfo->EnhancedFX_altfleshImpact			= trap->FX_RegisterEffect("weapons/blaster_impact_blue_huge");
		weaponInfo->EnhancedFX_missileWallImpactfx		= trap->FX_RegisterEffect("weapons/blaster_impact_blue_big_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx	= trap->FX_RegisterEffect("weapons/blaster_impact_blue_huge_fire");
		trap->FX_RegisterEffect("blasters/blue_deflect");
		break;

	case WP_PROTON_CARBINE_RIFLE:
		weaponInfo->bolt3DShader = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun, using this color shader...
		weaponInfo->bolt3DShaderAlt = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun's alt fire, using this color shader...
		weaponInfo->bolt3DLength = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DLengthAlt = 1.25; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidth = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidthAlt = 1.25; // If not set, 1.0 is the default length.

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
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;
										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 11.0, - 3.0, - 6.2);

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_RedPurple_Ball_small");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_RedPurple_Flare_medium");

		weaponInfo->fleshImpactEffect						= trap->FX_RegisterEffect("blasters/purple_flesh_impact");
		weaponInfo->altFleshImpactEffect					= trap->FX_RegisterEffect("blasters/purple_flesh_impact");
		weaponInfo->missileWallImpactfx						= trap->FX_RegisterEffect("weapons/blaster_impact_purple_medium_fire");
		weaponInfo->altMissileWallImpactfx					= trap->FX_RegisterEffect("weapons/blaster_impact_purple_big_fire");


		weaponInfo->EnhancedFX_fleshImpact					= trap->FX_RegisterEffect("blasters/purple_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact				= trap->FX_RegisterEffect("blasters/purple_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx			= trap->FX_RegisterEffect("weapons/blaster_impact_purple_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx		= trap->FX_RegisterEffect("weapons/blaster_impact_purple_big_fire");

		trap->FX_RegisterEffect("blasters/red_deflect");

		cgs.effects.blasterDroidImpactEffect = trap->FX_RegisterEffect("blaster/droid_impact");
		break;

	case WP_DH_17_PISTOL:
		weaponInfo->bolt3DShader = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun, using this color shader...
		weaponInfo->bolt3DShaderAlt = cgs.media.redBlasterShot; // Setting this enables 3D bolts for this gun's alt fire, using this color shader...
		weaponInfo->bolt3DLength = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DLengthAlt = 1.25; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidth = 1.0; // If not set, 1.0 is the default length.
		weaponInfo->bolt3DWidthAlt = 1.25; // If not set, 1.0 is the default length.

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
		weaponInfo->altMissileTrailFunc = FX_WeaponAltProjectileThink;
										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 10.0, - 2.8, - 5.6);

		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("blasters/shot_redpurple_medium");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("blasters/shot_redpurple_medium");

		weaponInfo->fleshImpactEffect						= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->altFleshImpactEffect					= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->missileWallImpactfx						= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->altMissileWallImpactfx					= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");


		weaponInfo->EnhancedFX_fleshImpact					= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact				= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx			= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx		= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");

		trap->FX_RegisterEffect("blasters/red_deflect");

		cgs.effects.blasterDroidImpactEffect = trap->FX_RegisterEffect("blaster/droid_impact");
		break;

	case WP_CYROBAN_GRENADE:
		weaponInfo->selectSound = trap->S_RegisterSound("sound/weapons/thermal/select.wav");
		weaponInfo->flashSound[0] = trap->S_RegisterSound("sound/weapons/melee/swing1.mp3");
		weaponInfo->flashSound[1] = trap->S_RegisterSound("sound/weapons/melee/swing2.mp3");
		weaponInfo->flashSound[2] = trap->S_RegisterSound("sound/weapons/melee/swing3.mp3");
		weaponInfo->flashSound[3] = trap->S_RegisterSound("sound/weapons/melee/swing4.mp3");
		weaponInfo->firingSound = NULL_SOUND;
		weaponInfo->chargeSound = trap->S_RegisterSound("sound/weapons/thermal/charge.wav");
		weaponInfo->muzzleEffect = NULL_FX;
		weaponInfo->missileModel = trap->R_RegisterModel("models/weapons/Grenade_CryoBan/model_proj.md3");
		weaponInfo->missileSound = NULL_SOUND;
		weaponInfo->missileDlight = 0;
		weaponInfo->missileHitSound = NULL_SOUND;
		weaponInfo->missileTrailFunc = FX_ThermalProjectileThink;;
		weaponInfo->altFlashSound[0] = trap->S_RegisterSound("sound/weapons/melee/swing1.mp3");
		weaponInfo->altFlashSound[1] = trap->S_RegisterSound("sound/weapons/melee/swing2.mp3");
		weaponInfo->altFlashSound[2] = trap->S_RegisterSound("sound/weapons/melee/swing3.mp3");
		weaponInfo->altFlashSound[3] = trap->S_RegisterSound("sound/weapons/melee/swing4.mp3");
		weaponInfo->altFiringSound = NULL_SOUND;
		weaponInfo->altChargeSound = trap->S_RegisterSound("sound/weapons/grenade_cook.wav");
		weaponInfo->altMuzzleEffect = NULL_FX;
		weaponInfo->altMissileModel = trap->R_RegisterModel("models/weapons/Grenade_CryoBan/model_proj.md3");
		weaponInfo->altMissileSound = NULL_SOUND;
		weaponInfo->altMissileDlight = 0;
		weaponInfo->altMissileHitSound = NULL_SOUND;
		weaponInfo->altMissileTrailFunc = FX_PulseGrenadeProjectileThink;
										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 8.0, - 2.0, - 3.0);

		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("weapons/grenaderibbon_red");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("weapons/grenaderibbon_red");

		weaponInfo->fleshImpactEffect					= trap->FX_RegisterEffect("explosives/cryobanmedium");
		weaponInfo->altFleshImpactEffect				= trap->FX_RegisterEffect("explosives/cryobanmedium");
		weaponInfo->missileWallImpactfx					= trap->FX_RegisterEffect("explosives/cryobanmedium");
		weaponInfo->altMissileWallImpactfx				= trap->FX_RegisterEffect("explosives/cryobanmedium");

		weaponInfo->EnhancedFX_fleshImpact				= trap->FX_RegisterEffect("explosives/cryobanmedium");
		weaponInfo->EnhancedFX_altfleshImpact			= trap->FX_RegisterEffect("explosives/cryobanmedium");
		weaponInfo->EnhancedFX_missileWallImpactfx		= trap->FX_RegisterEffect("explosives/cryobanmedium");
		weaponInfo->EnhancedFX_altmissileWallImpactfx	= trap->FX_RegisterEffect("explosives/cryobanmedium");


		cgs.media.grenadeBounce1 = trap->S_RegisterSound("sound/weapons/grenade_bounce1.mp3");
		cgs.media.grenadeBounce2 = trap->S_RegisterSound("sound/weapons/grenade_bounce2.mp3");
		trap->S_RegisterSound("sound/weapons/grenade_thermdetloop.mp3");
		trap->S_RegisterSound("sound/weapons/thermal/warning.wav");
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
										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 8.0, -2.0, -3.0);
		weaponInfo->missileRenderfx = trap->FX_RegisterEffect("explosives/shot_rpg");
		weaponInfo->altMissileRenderfx = trap->FX_RegisterEffect("explosives/shot_rpg");
		
		weaponInfo->fleshImpactEffect					= trap->FX_RegisterEffect("explosives/demomedium");
		weaponInfo->altFleshImpactEffect				= trap->FX_RegisterEffect("explosives/demomedium");
		weaponInfo->missileWallImpactfx					= trap->FX_RegisterEffect("explosives/demomedium");
		weaponInfo->altMissileWallImpactfx				= trap->FX_RegisterEffect("explosives/demomedium");
		

		weaponInfo->EnhancedFX_fleshImpact				= trap->FX_RegisterEffect("explosives/demomedium");
		weaponInfo->EnhancedFX_altfleshImpact			= trap->FX_RegisterEffect("explosives/demomedium");
		weaponInfo->EnhancedFX_missileWallImpactfx		= trap->FX_RegisterEffect("explosives/demomedium");
		weaponInfo->EnhancedFX_altmissileWallImpactfx	= trap->FX_RegisterEffect("explosives/demomedium");
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
		weaponInfo->missileTrailFunc = FX_ThermalProjectileThink;;
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
		weaponInfo->altMissileTrailFunc = FX_PulseGrenadeProjectileThink;
										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 8.0, - 2.0, - 3.0);

		weaponInfo->missileWallImpactfx = trap->FX_RegisterEffect("weapons/grenaderibbon_red");
		weaponInfo->altMissileWallImpactfx = trap->FX_RegisterEffect("weapons/grenaderibbon_red");

		weaponInfo->fleshImpactEffect					= trap->FX_RegisterEffect("explosives/baradium_class-e");
		weaponInfo->altFleshImpactEffect				= trap->FX_RegisterEffect("explosives/shockwave");
		weaponInfo->missileWallImpactfx					= trap->FX_RegisterEffect("explosives/baradium_class-e");
		weaponInfo->altMissileWallImpactfx				= trap->FX_RegisterEffect("explosives/shockwave");

		weaponInfo->EnhancedFX_fleshImpact				= trap->FX_RegisterEffect("explosives/baradium_class-e");
		weaponInfo->EnhancedFX_altfleshImpact			= trap->FX_RegisterEffect("explosives/shockwave");
		weaponInfo->EnhancedFX_missileWallImpactfx		= trap->FX_RegisterEffect("explosives/baradium_class-e");
		weaponInfo->EnhancedFX_altmissileWallImpactfx	= trap->FX_RegisterEffect("explosives/shockwave");


		cgs.media.grenadeBounce1 = trap->S_RegisterSound("sound/weapons/grenade_bounce1.mp3");
		cgs.media.grenadeBounce2 = trap->S_RegisterSound("sound/weapons/grenade_bounce2.mp3");
		trap->S_RegisterSound("sound/weapons/grenade_thermdetloop.mp3");
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
										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 8.0, - 2.0, - 3.0);

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
										//  X,	 ,Y	   ,Z	
		VectorSet(weaponInfo->gunPosition, 10.0, -2.0, -3.4);
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

		weaponInfo->altFleshImpactEffect				= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->missileWallImpactfx					= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->altMissileWallImpactfx				= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		
		
		weaponInfo->EnhancedFX_fleshImpact				= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_altfleshImpact			= trap->FX_RegisterEffect("blasters/red_flesh_impact");
		weaponInfo->EnhancedFX_missileWallImpactfx		= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		weaponInfo->EnhancedFX_altmissileWallImpactfx	= trap->FX_RegisterEffect("weapons/blaster_impact_red_medium_fire");
		trap->FX_RegisterEffect("blasters/red_deflect");
		break;
	 default:
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 1 );
		weaponInfo->flashSound[0] = trap->S_RegisterSound( "sound/weapons/rocket/rocklf1a.wav" );
		break;
	}
}
