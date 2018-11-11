class ActionTurnOffWeaponFlashlight: ActionSingleUseBase
{
	ItemBase m_flashlight;
	
	void ActionTurnOffWeaponFlashlight()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_ITEM_OFF;//CMD_ACTIONMOD_INTERACTONCE
		
		m_MessageSuccess = "It's turned on.";
		m_MessageFail = "It's out of energy";
	}
	
	override void CreateConditionComponents()  
	{	
		m_ConditionItem = new CCINonRuined;
		m_ConditionTarget = new CCTNone;
	}

	override int GetType()
	{
		return AT_TURN_OFF_WEAPON_FLASHLIGHT;
	}

	override bool HasTarget()
	{
		return false;
	}

	override string GetText()
	{
		return "#switch_off";
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
		if ( item.IsInherited(Rifle_Base) )
		{
			//m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_LIGHTFLARE;
			m_flashlight = ItemBase.Cast(item.FindAttachmentBySlotName("weaponFlashlight"));
		}
		else if (item.IsInherited(Pistol_Base))
		{
			//m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_LITCHEMLIGHT;
			m_flashlight = ItemBase.Cast(item.FindAttachmentBySlotName("pistolFlashlight"));
		}
		else //is not valid item
			return false;
		
		if ( m_flashlight && m_flashlight.HasEnergyManager() && m_flashlight.GetCompEM().CanSwitchOff() ) //TODO review conditions for turning off
		{
			return true;
		}
		
		return false;
	}

	override void OnExecuteServer( ActionData action_data )
	{		
		if ( m_flashlight.HasEnergyManager() )
		{
			if ( m_flashlight.GetCompEM().IsSwitchedOn() )
			{
				m_flashlight.GetCompEM().SwitchOff();
			}
			Weapon_Base.Cast(action_data.m_MainItem).FlashlightOff(); //currently seems to be doing nothing
		}
	}
};
