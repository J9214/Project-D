#include "PDGameplayTags.h"

namespace PDGameplayTags
{
	/** Input Tags **/
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Equip_Slot1, "InputTag.Weapon.Equip.Slot1");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Equip_Slot2, "InputTag.Weapon.Equip.Slot2");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Equip_Slot3, "InputTag.Weapon.Equip.Slot3");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Equip_Slot4, "InputTag.Weapon.Equip.Slot4");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Unequip, "InputTag.Weapon.Unequip");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Weapon_Fire, "InputTag.Weapon.Fire");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Weapon_Reload, "InputTag.Weapon.Reload");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Weapon_ADS, "InputTag.Weapon.ADS");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Weapon_ChangeFireMode, "InputTag.Weapon.ChangeFireMode");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Throwable_Throw, "InputTag.Throwable.Throw");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Ability_SkillQ, "InputTag.Ability.SkillQ");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Ability_SkillE, "InputTag.Ability.SkillE");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Ability_Movement_Dash, "InputTag.Ability.Movement.Dash");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Ability_Movement_WallJump, "InputTag.Ability.Movement.WallJump");

	/** Player Tags **/
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Equip, "Player.Ability.Weapon.Equip");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Unequip, "Player.Ability.Weapon.Unequip");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Fire, "Player.Ability.Weapon.Fire");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Reload, "Player.Ability.Weapon.Reload");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_ADS, "Player.Ability.Weapon.ADS");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_ChangeFireMode, "Player.Ability.Weapon.ChangeFireMode");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Throwable_Throw, "Player.Ability.Throwable.Throw");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Movement, "Player.Ability.Movement");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Movement_Dash, "Player.Ability.Movement.Dash");
	UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Movement_WallJump, "Player.Ability.Movement.WallJump");

	/** Player State Tags **/
	UE_DEFINE_GAMEPLAY_TAG(Player_State_Firinig, "Player.State.Firing");
	UE_DEFINE_GAMEPLAY_TAG(Player_State_Equipping, "Player.State.Equipping");
	UE_DEFINE_GAMEPLAY_TAG(Player_State_Unequipping, "Player.State.Unequipping");
	UE_DEFINE_GAMEPLAY_TAG(Player_State_Reloading, "Player.State.Reloading");
	UE_DEFINE_GAMEPLAY_TAG(Player_State_Throwing, "Player.State.Throwing");
	
	/** Event Tags **/
	UE_DEFINE_GAMEPLAY_TAG(Event_Weapon_EquipRequest, "Event.Weapon.EquipRequest");
	UE_DEFINE_GAMEPLAY_TAG(Event_Weapon_UnequipRequest, "Event.Weapon.UnequipRequest");
	UE_DEFINE_GAMEPLAY_TAG(Event_Weapon_ReloadRequest, "Event.Weapon.ReloadRequest");
	UE_DEFINE_GAMEPLAY_TAG(Event_Throwable_ThrowRequest, "Event.Throwable.ThrowRequest");
	
	/** GameplayCue Tags **/
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Fire, "GameplayCue.Weapon.Fire");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Throwable_Explosion_Fragment, "GameplayCue.Throwable.Explosion.Fragment");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Throwable_Explosion_Flame, "GameplayCue.Throwable.Explosion.Flame");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Throwable_Explosion_Smoke, "GameplayCue.Throwable.Explosion.Smoke");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Skill_Shield, "GameplayCue.Skill.Shield");

	/** SetByCaller Tags **/
	UE_DEFINE_GAMEPLAY_TAG(Data_Weapon_Damage, "Data.Weapon.Damage");
	UE_DEFINE_GAMEPLAY_TAG(Data_Weapon_DestructDamage, "Data.Weapon.DestructDamage");
	UE_DEFINE_GAMEPLAY_TAG(Data_Weapon_FireInterval, "Data.Weapon.FireInterval");
	UE_DEFINE_GAMEPLAY_TAG(Data_Throwable_Fragment_Damage, "Data.Throwable.Fragment.Damage");
	UE_DEFINE_GAMEPLAY_TAG(Data_Throwable_Flame_Damage, "Data.Throwable.Flame.Damage");
	UE_DEFINE_GAMEPLAY_TAG(Data_ThrowableObject_ExplodeDamage, "Data.ThrowableObject.ExplodeDamage");
	UE_DEFINE_GAMEPLAY_TAG(Data_ThrowableObject_ContinuousDamage, "Data.ThrowableObject.ContinuousDamage");
	UE_DEFINE_GAMEPLAY_TAG(Data_ThrowableObject_DestructDamage, "Data.ThrowableObject.DestructDamage");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Weapon_Fire, "Cooldown.Weapon.Fire");

	/** Gimmick Tags **/
	PROJECTD_API UE_DEFINE_GAMEPLAY_TAG(Gimmick_Interactable_ZipLine, "Gimmick.Interactable.ZipLine");
}