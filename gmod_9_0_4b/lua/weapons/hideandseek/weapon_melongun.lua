--##############################
--## Hide and Seek
--## fayte, gdhughes@gmail.com
--##############################

--## How powerfully the melon is shot.
SHOT_POWER = 8000;


--###########################################################################


MyIndex					= 0;
Owner 					= 0;
CurrentTime 				= 0;

function onInit()
	_EntPrecacheModel("models/props_junk/watermelon01.mdl");
	_SWEPSetSound(MyIndex, "single_shot", "weapon_xm1014.Single")
end
	
function onPrimaryAttack()
	if (_PlayerInfo(Owner, "alive") == false) then
		return
	end
	
	local melon = _EntCreate( "prop_physics" );
	local aim = _PlayerGetShootAng(Owner);

	aim.x = aim.x * SHOT_POWER
	aim.y = aim.y * SHOT_POWER
	aim.z = aim.z * SHOT_POWER

	_EntSetModel(melon, "models/props_junk/watermelon01.mdl");	
	_EntSetOwner(melon, Owner);
	_EntSetPos(melon, _PlayerGetShootPos(Owner));
	_EntSetAng(melon, _PlayerGetShootAng(Owner));

	_EntSetKeyValue(melon, "fademindist", -1);
	_EntSetKeyValue(melon, "fademaxdist", 0);
	_EntSetKeyValue(melon, "fadescale", 0);
	
	_EntSpawn(melon);
	_PhysApplyForce(melon, aim);

	_SWEPUseAmmo(MyIndex, 0, 1);
end

function getPrintName() return "Melon Gun"; end
function getClassName() return "melongun"; end
function getPrimaryAmmoType() return "buckshot"; end
function getSecondaryAmmoType() return "buckshot"; end
function getViewModel() return "models/weapons/v_shot_xm1014.mdl"; end
function getWorldModel() return "models/weapons/w_shot_xm1014.mdl"; end
function getAnimPrefix() return "shotgun"; end
function getWeaponSwapHands() return true; end
function getWeaponFOV() return 100; end
function getWeaponSlot() return 1; end
function getWeaponSlotPos() return 1; end
function getFiresUnderwater() return false; end
function getReloadsSingly() return false; end
function getDamage() return 0; end
function getPrimaryShotDelay() return 0.1; end
function getSecondaryShotDelay() return 0.1; end
function getPrimaryIsAutomatic() return false; end
function getSecondaryIsAutomatic() return false; end
function getMaxClipPrimary() return 2; end
function getMaxClipSecondary() return 0; end
function getDefClipPrimary() return 2; end
function getDefClipSecondary() return 0; end
function getPrimaryScriptOverride() return 1; end
function getSecondaryScriptOverride() return 3; end
function getBulletSpread() return vector3(0.01, 0.01, 0.01); end
function getViewKick() return vector3(0, 0.0, 0.0); end
function getViewKickRandom() return vector3(2.0, 1.0, 1.0); end
