class ActionSwitchLights: ActionInteractBase
{
	void ActionSwitchLights()
	{
		m_MessageSuccess    = "";
		m_CommandUID        = DayZPlayerConstants.CMD_ACTIONMOD_HEADLIGHT;
		m_StanceMask        = DayZPlayerConstants.STANCEMASK_CROUCH | DayZPlayerConstants.STANCEMASK_ERECT;
		m_HUDCursorIcon     = CursorIcons.LootCorpse;
	}

	override void CreateConditionComponents()  
	{
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTNone;
	}	
	
	override int GetType()
	{
		return AT_VEH_SWITCH_LIGHTS;
	}

	override string GetText()
	{
		return "#switch_lights";
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{	
		HumanCommandVehicle vehCommand = player.GetCommand_Vehicle();

		if ( vehCommand )
		{
			Transport trans = vehCommand.GetTransport();
			if ( trans )
			{
				CarScript car;
				if ( Class.CastTo(car, trans) )
				{
					if ( car.CrewMemberIndex( player ) == DayZPlayerConstants.VEHICLESEAT_DRIVER )
					{
						if ( !car.IsLightsOn() )
						{
							EntityAI neededItem = null;

							if ( car.IsVitalCarBattery() ) neededItem = car.FindAttachmentBySlotName("CarBattery");
							if ( car.IsVitalTruckBattery() ) neededItem = car.FindAttachmentBySlotName("TruckBattery");

							if ( !neededItem || (neededItem && neededItem.IsRuined()) )
								return false;

							return true;
						}
						else
						{
							return true;
						}
					}
				}
			}
		}
		return false;
	}

	override void OnExecuteServer( ActionData action_data )
	{
		HumanCommandVehicle vehCommand = action_data.m_Player.GetCommand_Vehicle();
		if ( vehCommand )
		{
			Transport trans = vehCommand.GetTransport();
			if ( trans )
			{
				CarScript car;
				if ( Class.CastTo(car, trans) )
				{
					car.SwitchLights();
				}
			}
		}
	}

	override bool CanBeUsedInVehicle()
	{
		return true;
	}
};