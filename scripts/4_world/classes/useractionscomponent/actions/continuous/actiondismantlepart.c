class ActionDismantlePartCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(UATimeSpent.DEFAULT_DECONSTRUCT);
	}
};

class ActionDismantlePart: ActionContinuousBase
{
	void ActionDismantlePart()
	{
		m_CallbackClass = ActionDismantlePartCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_DISASSEMBLE;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT;	
		//TODO: when anims ready, uncoment crouch and erect	
		//m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH | DayZPlayerConstants.STANCEMASK_ERECT;		
		
		m_SpecialtyWeight = UASoftSkillsWeight.ROUGH_HIGH;
	}
	
	override void CreateConditionComponents()  
	{	
		m_ConditionItem = new CCINonRuined;
		m_ConditionTarget = new CCTNonRuined(UAMaxDistances.BASEBUILDING);
	}

	override int GetType()
	{
		return AT_DISMANTLE_PART;
	}
		
	override string GetText()
	{
		PlayerBase player = PlayerBase.Cast( GetGame().GetPlayer() );
		if ( player )
		{
			ConstructionActionData construction_action_data = player.GetConstructionActionData();
			ConstructionPart constrution_part = construction_action_data.GetTargetPart();
			
			if ( constrution_part )
			{
				return "#dismantle" + " " + constrution_part.GetName();
			}
		}
		
		return "";
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{	
		Object targetObject = target.GetObject();
		
		if ( targetObject && targetObject.CanUseConstruction() )
		{
			string part_name = targetObject.GetActionComponentName( target.GetComponentIndex() );
			
			BaseBuildingBase base_building = BaseBuildingBase.Cast( targetObject );
			Construction construction = base_building.GetConstruction();		
			ConstructionPart construction_part = construction.GetConstructionPartToDismantle( part_name );
			
			if ( construction_part && !construction_part.IsBase() && base_building.IsFacingBack( player ) )
			{
				//if part is base but more attachments are present
				if ( base_building.HasAttachmentsBesidesBase() )
				{
					return false;
				}
				
				ConstructionActionData construction_action_data = player.GetConstructionActionData();
				construction_action_data.SetTargetPart( construction_part );
				
				return true;
			}
		}
		
		return false;
	}
	
	override void OnFinishProgressServer( ActionData action_data )
	{	
		BaseBuildingBase base_building = BaseBuildingBase.Cast( action_data.m_Target.GetObject() );
		Construction construction = base_building.GetConstruction();
		ConstructionActionData construction_action_data = action_data.m_Player.GetConstructionActionData();
		ConstructionPart construction_part = construction_action_data.GetTargetPart();
		
		if ( construction.CanDismantlePart( construction_part.GetPartName() ) )
		{
			//build
			construction.DismantlePart( construction_part.GetPartName(), true );
			
			//add damage to tool
			action_data.m_MainItem.DecreaseHealth ( "", "", 2, true );
		}
		else
		{
			SendMessageToClient( action_data.m_Player, base_building.MESSAGE_CANNOT_BE_DECONSTRUCTED );
		}

		action_data.m_Player.GetSoftSkillManager().AddSpecialty( m_SpecialtyWeight );
	}
}