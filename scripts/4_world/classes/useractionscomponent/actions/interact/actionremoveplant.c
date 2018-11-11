class ActionRemovePlant: ActionInteractBase
{
	void ActionRemovePlant()
	{
		m_MessageSuccess = "";
		m_MessageStartFail = "";
		m_MessageStart = "";
		m_MessageSuccess = "";
		m_MessageFail = "";
		m_MessageCancel = "";
		
		//m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_INTERACT;
		//m_FullBody = true;
	}

	override int GetType()
	{
		return AT_REMOVE_PLANT;
	}

	override string GetText()
	{
		return "#remove_plant";
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
		Object targetObject = target.GetObject();
		if ( targetObject != NULL && targetObject.IsInherited(PlantBase) )
		{
			PlantBase plant = PlantBase.Cast( targetObject );
			
			if ( plant.IsGrowing()  ||  plant.IsDry()  ||  !plant.HasCrops() )
			{
				return true;
			}
		}
		
		return false;
	}

	override void OnExecuteServer( ActionData action_data )
	{
		Object targetObject = action_data.m_Target.GetObject();
		if ( targetObject != NULL && targetObject.IsInherited(PlantBase) )
		{
			PlantBase plant = PlantBase.Cast( targetObject );
			m_MessageSuccess = plant.Remove( action_data.m_Player );
		}
	}
};