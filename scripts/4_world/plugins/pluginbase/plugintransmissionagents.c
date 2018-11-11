enum InjectTypes
{
    PLAYER_TO_ITEM, 
	ITEM_TO_PLAYER,
	PLAYER_AIR_PLAYER,
};

class PluginTransmissionAgents extends PluginBase
{
	static ref map<int, ref AgentBase> m_AgentList =  new map<int, ref AgentBase>;
	ref map<int, string> m_SimpleAgentList = new map<int, string>;
	bool m_IsConstructed = false;
	
	void PluginTransmissionAgents()
	{
		//add new agents here
		m_AgentList.Insert(eAgents.INFLUENZA, new InfluenzaAgent);
		m_AgentList.Insert(eAgents.CHOLERA, new CholeraAgent);
		m_AgentList.Insert(eAgents.SALMONELLA, new SalmonellaAgent);
		m_AgentList.Insert(eAgents.BRAIN, new BrainAgent);
	}
	

	void ConstructSimpleAgentList()
	{
		string agent_name;
		int agent_type;
		
		for(int i = 0; i < m_AgentList.Count();i++)
		{
			AgentBase agent = m_AgentList.GetElement(i);
			agent_name = agent.GetName();
			agent_type = agent.GetAgentType();
			m_SimpleAgentList.Insert(agent_type, agent_name);
		}
	}

	map<int, ref AgentBase> GetAgentList()
	{
		return	m_AgentList;
	}
	// this is a list which is easy to work with for displaying all agents and interacting with them, it doesn't serve any gameplay purpose
	map<int, string> GetSimpleAgentList()
	{
		if( !m_IsConstructed )
		{
			ConstructSimpleAgentList();
			m_IsConstructed	= true;
		}
		return m_SimpleAgentList;
	}
	
	string GetNameByID(int agent_id)
	{
		return m_AgentList.Get(agent_id).GetName();
	}
	
	void RemoveAllAgents(EntityAI target)
	{
		target.RemoveAllAgents();
	}
	
	static void RemoveAgent(EntityAI target, int agent_id )
	{
		target.RemoveAgent( agent_id );
	}
	
	protected float GetAgentTransferabilityIn( int agent_id )
	{
		if( !m_AgentList.Get(agent_id) ) return 0;
		return m_AgentList.Get(agent_id).GetTransferabilityIn();
	}
	
	protected float GetAgentTransferabilityOut( int agent_id )
	{
		if(!m_AgentList.Get(agent_id)) return 0;
		return m_AgentList.Get(agent_id).GetTransferabilityOut();
	}	
	
	protected float GetAgentTransferabilityAirOut( int agent_id )
	{
		if(!m_AgentList.Get(agent_id)) return 0;
		return m_AgentList.Get(agent_id).GetTransferabilityAirOut();
	}
	/*
	float GetAgentChance( int agent_id )
	{
		return m_AgentList.Get(agent_id).GetChanceOfInfection();
	}			
	*/
	float GetAgentInvasibility( int agent_id )
	{
		if( !m_AgentList.Get(agent_id) ) return 0;
		return m_AgentList.Get(agent_id).GetInvasibility();
	}

	static int GetAgentMaxCount( int agent_id )
	{
		if( !m_AgentList.Get(agent_id) ) return 0;
		return m_AgentList.Get(agent_id).GetMaxCount();
	}
	
	/*
	private float GetAgentInitialCount( int agent_id )
	{
		return m_AgentList.Get(agent_id).GetInitialCount();
	}
	*/
	
	float GetImmunityResistance( int agent_id )
	{
		if( !m_AgentList.Get(agent_id) ) return 0;
		return m_AgentList.Get(agent_id).GetImmunityResistance();
	}
	
	
	void TransmitAgents(EntityAI source, EntityAI target, int pathway, int dose_size = 1000)
	{
		//Debug.Log("Transmitting agents for source: " +source.ToString()+", target: " +target.ToString(),"Agents");
		int sourceAgents;
		int targetAgents;
		if(source) sourceAgents = source.GetAgents();
		if(target) targetAgents = target.GetAgents();
		
		switch (pathway)
		{
			case AGT_INV_OUT: //item leaving inventory
				break;
				 
			case AGT_INV_IN: //item entering inventory
				break;
				
			case AGT_UACTION_TOUCH: //player touched the item
				//InjectAgents( source, targetAgents ,GetProtectionLevel(DEF_BIOLOGICAL,InventorySlots.GLOVES, player) );
				break;

			case AGT_WATER_POND:
				//target.InsertAgent(eAgents.CHOLERA, dose_size);
				InjectAgentsWithPlayer( target, eAgents.CHOLERA , 0, dose_size, InjectTypes.ITEM_TO_PLAYER );
				break;
				
			case AGT_UACTION_CONSUME: //user action of a consumption, from item to player and player to item(should not be used in continuous actions)
				InjectAgentsWithPlayer( target, sourceAgents , 0, dose_size, InjectTypes.ITEM_TO_PLAYER );
				InjectAgentsWithPlayer( source, targetAgents , 0, dose_size, InjectTypes.PLAYER_TO_ITEM );
				break;
			
			case AGT_UACTION_TO_PLAYER: //user action of a consumption, only from item to player
				InjectAgentsWithPlayer( target, sourceAgents , 0, dose_size, InjectTypes.ITEM_TO_PLAYER );
				break;

			case AGT_UACTION_TO_ITEM: //to transfer from the player to the consumed item
				InjectAgentsWithPlayer( target, sourceAgents , 0, dose_size, InjectTypes.PLAYER_TO_ITEM );
				break;
				
			case AGT_TRANSFER_COPY: //transferring liquid
				InjectAgentsWithoutPlayer( target, sourceAgents );
				break;

			case AGT_AIRBOURNE:
				InjectAgentsWithPlayer( target, sourceAgents , GetProtectionLevel(DEF_BIOLOGICAL,InventorySlots.MASK, Man.Cast( target )), dose_size, InjectTypes.PLAYER_AIR_PLAYER );
		}
	}
	
	protected void InjectAgentsWithoutPlayer(EntityAI target, int agents)
	{
		if( target.IsItemBase() )
		{
			ItemBase ib_target = ItemBase.Cast( target );
			ib_target.TransferAgents(agents);
		}
	}
	
	//! will add agents to a given target
	protected void InjectAgentsWithPlayer(EntityAI target, int agents,float protection, int dose_size, int inject_type)//target,array_of_agents,protection_lvl
	{
		if(target && (agents != 0) && target.IsEntityAI() )
		{
			ref array<int> agents_aray = new array<int>;
			PluginTransmissionAgents.BuildAgentArray(agents, agents_aray);
			
			for(int i = 0; i < agents_aray.Count(); i++)
			{
				float count = CalculateAgentsToTransmit(agents_aray.Get(i), protection, dose_size, inject_type);
				target.InsertAgent(agents_aray.Get(i),count);
			}
		}
	}

	static void BuildAgentArray(int agents, array<int> agents_out)
	{
		int mask = 1;
		for(int i = 0; i < 31; i++)
		{
			if( mask & agents ) agents_out.Insert(mask);
			mask = mask * 2;
		}
	}

	static int MakeMaskFromArray(array<int> agents)
	{
		int mask = 0;
		for(int i = 0; i < agents.Count(); i++)
		{
			int type = 	agents.Get(i);
			mask = type | mask;
		}
		return mask;
	}
	
	
	protected float GetProtectionLevel(int type, int slot, Man player)
	{
		EntityAI attachment = player.GetInventory().FindAttachment(slot);
		if(!attachment) return 0;
		string subclass_path = "CfgVehicles " + attachment.GetType() + " DamageArmor ";
		string entryName;
		switch (type)
		{
			case DEF_BIOLOGICAL:
				entryName = "biological";
				break;
				
			default:
				entryName = "biological";
				break;
		}
		
		float value = GetGame().ConfigGetFloat (subclass_path + entryName);
		return value;
	}

	//------------------------------------------------------------------------------------------------------
	protected float CalculateAgentsToTransmit(int agent_id, float protection, int dose_size, int inject_type)
	{

		Debug.Log("protection: "+protection.ToString());
		float prot = 1 - protection; //reverse the value (in config, the higher the value, the higher the protection: 0 - 1) so that we can easily interpolate between 0 and 1 by multiplication
		Debug.Log("prot: "+prot.ToString(), "Agents");
		float transf;
		
		if( inject_type == InjectTypes.PLAYER_TO_ITEM )
		{
			transf = GetAgentTransferabilityOut(agent_id);
		}
		if( inject_type == InjectTypes.ITEM_TO_PLAYER )
		{
			transf = GetAgentTransferabilityIn(agent_id);
		}
		if( inject_type == InjectTypes.PLAYER_AIR_PLAYER )
		{
			transf = GetAgentTransferabilityAirOut(agent_id);
		}

		Debug.Log("transf: "+transf.ToString(), "Agents");
		//float result =  GetAgentInitialCount(agent_id) * prot * transf * dose_size;//final formula
		float result =  1 * prot * transf * dose_size;//final formula
		//result = Math.Ceil(result);
		Debug.Log("result: "+result.ToString(), "Agents");
		return result;
	}
	//------------------------------------------------------------------------------------------------------

};