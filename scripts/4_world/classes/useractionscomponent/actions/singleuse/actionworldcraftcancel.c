class ActionWorldCraftCancel: ActionSingleUseBase
{
	void ActionWorldCraftCancel()
	{
		/*
		m_MessageStartFail = "Start failed.";
		m_MessageStart = "Player started placing object.";
		m_MessageSuccess = "Player placed the object."
		m_MessageFail = "Player failed to place the object.";
		m_MessageCancel = "You canceled action.";
		*/
	}
	
	override void CreateConditionComponents()  
	{		
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTNone;
	}

	override int GetType()
	{
		return AT_WORLD_CRAFT_CANCEL;
	}

	override string GetText()
	{
		return "#cancel";
	}
	
	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{	
		return player.GetCraftingManager().IsInventoryCraft();
	}
	
	override void Start( ActionData action_data ) //Setup on start of action
	{
		super.Start( action_data );
		//action_data.m_Player.GetCraftingManager().CancelInventoryCraft();
	}
	
	override bool IsLocal()
	{
		return true;
	}
};