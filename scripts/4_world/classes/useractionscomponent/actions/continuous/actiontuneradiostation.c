class ActionTuneRadioStationCB : ActionContinuousBaseCB
{
	private const float REPEAT_AFTER_SEC = 1.0;
	
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousRepeat(REPEAT_AFTER_SEC);
	}
}

class ActionTuneRadioStation: ActionContinuousBase
{
	void ActionTuneRadioStation()
	{
		m_CallbackClass = ActionTuneRadioStationCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_ITEM_TUNE;
		m_CommandUIDProne = DayZPlayerConstants.CMD_ACTIONFB_ITEM_TUNE;	
		m_MessageStartFail = "I've failed to change the radio station.";
		m_MessageStart = "I've changed the radio station.";
		m_MessageFail = "I've failed to change the radio station.";
		m_MessageCancel = "I have stopped the radio tunning.";
		m_SpecialtyWeight = UASoftSkillsWeight.ROUGH_LOW;
	}
	
	override void CreateConditionComponents()  
	{	
		m_ConditionTarget = new CCTNone;
		m_ConditionItem = new CCINonRuined;
	}
	
	override bool HasProneException()
	{
		return true;
	}

	override int GetType()
	{
		return AT_TUNE_RADIO_STATION;
	}
		
	override string GetText()
	{
		return "#tune_radio_station";
	}
	
	override bool HasTarget()
	{
		return false;
	}	

	override bool ActionCondition ( PlayerBase player, ActionTarget target, ItemBase item )
	{	
		Radio radio = Radio.Cast( item );
		
		if ( radio.CanOperate() ) 
		{
			return true;
		}
		
		return false;
	}

	override void OnFinishProgressServer( ActionData action_data )
	{	
		Radio radio = Radio.Cast( action_data.m_MainItem );
		radio.TuneNextStation();
		action_data.m_Player.GetSoftSkillManager().AddSpecialty( m_SpecialtyWeight );
	}
}