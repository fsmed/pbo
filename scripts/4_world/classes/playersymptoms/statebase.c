class SymptomBase
{
	int m_Priority;
	SoundOnVehicle m_SoundObject;
	bool m_PlayedSound;
	bool m_IsActivated;
	PlayerBase m_Player;
	float m_ServerUpdateInterval = 1;
	float m_ServerUpdateDelta;
	
	int m_ID;//ID for the type of Symptom
	int m_UID;//unique ID
	bool m_IsClientOnly;
	bool m_DestroyOnAnimFinish = true;
	bool m_DestroyRequested = false;
	int m_SymptomType = -1;
	bool m_IsPersistent = false;
	SymptomManager m_Manager;
	bool m_SyncToClient = false;
	float m_Duration;
	bool m_QuitAnimOnSymptomDestroy = true;

	SymptomCB m_AnimCallback;
	
	ref array<Param> m_PersistentParams = new array<Param>;
	
	void SymptomBase()
	{
	}
	
	void ~SymptomBase()
	{
		if( m_QuitAnimOnSymptomDestroy && m_AnimCallback ) 
		{
			m_AnimCallback.Cancel();
		}
		OnDestructed();
	}

	void Init(SymptomManager manager, PlayerBase player, int uid)
	{
		m_Manager = manager;
		m_Player = player;
		m_UID = uid;
		OnInit();
	}
	
	int GetUID()
	{
		return m_UID;
	}

	void OnOwnerKilled()
	{
		
		
	}
	
	bool IsClientOnly()
	{
		return m_IsClientOnly;
	}
	
	
	void SetDuration(float duration)
	{
		m_Duration = duration;
	}
	
	float GetDuration()
	{
		return m_Duration;
	}
	
	string GetName()
	{
		return this.ClassName();		
	}

	SymptomManager GetManager()
	{
		return m_Manager;
	}
	
	int GetType()
	{
		return m_ID;
	}

	bool IsSyncToClient()
	{
		return m_SyncToClient;		
	}

	void GetPersistentParams(array<Param> params)
	{
		for(int i = 0; i < m_PersistentParams.Count(); i++)
		{
			params.Insert(m_PersistentParams.Get(i));
		}
	}

	void MakeParamObjectPersistent(Param object)
	{
		if( !GetGame().IsServer() && !GetGame().IsMultiplayer() ) return;
		
		m_PersistentParams.Insert(object);
	}
	
	bool IsPersistent()
	{
		return m_IsPersistent;
	}
	
	bool IsPrimary()
	{
		if( m_SymptomType == SymptomTypes.PRIMARY)
		return true;
		else return false;
	}
	
	PlayerBase GetPlayer()
	{
		return m_Player;		
	}

	int GetPriority()
	{
		return m_Priority;
	}
	
	bool OnConstructed(SymptomManager manager)
	{
		
	}
	
	void OnDestructed()
	{
		if( IsActivated() ) Deactivate();
		if( GetManager() ) m_Manager.OnSymptomExit(this, m_UID);
	}

	void Activate()
	{
		m_IsActivated = true;
		if( GetGame() && GetGame().IsServer() )
		{
			OnGetActivatedServer(m_Player);
			if( GetGame().IsMultiplayer() )
			{
				SyncToClientActivated( GetType(), GetUID() );
				GetManager().SendServerDebugToClient();
			}
		}
		if( !GetGame().IsMultiplayer() || GetGame().IsClient() )
		{
			OnGetActivatedClient(m_Player);
		}
	}
	
	void Deactivate()
	{
		if( !GetGame() ) return;
		m_IsActivated = false;
		if( GetGame().IsServer() ) 
		{
			OnGetDeactivatedServer(m_Player);
			if( GetGame().IsMultiplayer() ) SyncToClientDeactivated( GetType(), GetUID() );
		}
		if( !GetGame().IsMultiplayer() || GetGame().IsClient() )
		{
			OnGetDeactivatedClient(m_Player);
		}
		
	}
	
	bool IsActivated()
	{
		return m_IsActivated;
	}
	
	
	
	void Update(float deltatime)
	{
		if( GetGame().IsServer() ) 
		{
			m_ServerUpdateDelta += deltatime;
			if(m_ServerUpdateDelta > m_ServerUpdateInterval )
			{
				OnUpdateServer(m_Player, m_ServerUpdateDelta);
				m_ServerUpdateDelta = 0;
			}
		}
		if( GetGame().IsClient() )
		{
			OnUpdateClient(m_Player, deltatime);
		}
		if( GetGame().IsServer() && !GetGame().IsMultiplayer() )
		{
			OnUpdateClient(m_Player, deltatime);
		}
		
		CheckDestroy();
	}
	
	void PlayAnimation(int animation, int stance_mask, float running_time = -1, bool destroy_on_finish = true)
	{
		DayZPlayerSyncJunctures.SendPlayerSymptomAnim(m_Player, animation, GetUID() , stance_mask, running_time );
		/*
		m_AnimCallback = GetPlayer().StartCommand_Action(animation, SymptomCB, DayZPlayerConstants.STANCEMASK_CROUCH);
		m_AnimCallback.Init(this, running_time);
		*/
	}
	/*
	void PlaySound(string sound, bool destroy_on_finish = true)
	{
		//m_SoundObject = GetGame().CreateSoundOnObject(GetPlayer(), sound, 0, false);
		m_SoundObject = GetPlayer().PlaySound( sound, 0);
		
		PrintString("sample length:" + m_SoundObject.GetSoundLength().ToString() );
		m_PlayedSound = true;
	}
	*/
	
	void PlaySound(EPlayerSoundEventID id, bool destroy_on_finish = true)
	{
		GetPlayer().RequestSoundEvent(id);
		m_PlayedSound = true;
	}

	void SyncToClientActivated( int SYMPTOM_id, int uid )
	{
		if( !GetPlayer() || !GetGame().IsServer() || !IsSyncToClient() ) return;
		
		CachedObjectsParams.PARAM2_INT_INT.param1 = SYMPTOM_id;
		CachedObjectsParams.PARAM2_INT_INT.param2 = uid;
		GetGame().RPCSingleParam(GetPlayer(), ERPCs.RPC_PLAYER_SYMPTOM_ON, CachedObjectsParams.PARAM2_INT_INT,true,GetPlayer().GetIdentity() );
	}
	
	void SyncToClientDeactivated( int SYMPTOM_id, int uid )
	{
		if( !GetPlayer() || !GetGame().IsServer() || !IsSyncToClient() ) return;
		CachedObjectsParams.PARAM2_INT_INT.param1 = SYMPTOM_id;
		CachedObjectsParams.PARAM2_INT_INT.param2 = uid;
		GetGame().RPCSingleParam(GetPlayer(), ERPCs.RPC_PLAYER_SYMPTOM_OFF, CachedObjectsParams.PARAM2_INT_INT,true,GetPlayer().GetIdentity() );
	}

	void CheckSoundFinished()
	{
		if (m_PlayedSound && !m_SoundObject)
		{
			RequestDestroy();	
		}
	}
	void CheckDestroy()
	{
		CheckSoundFinished();
		if (m_DestroyRequested) Destroy();
	}
	

	void RequestDestroy()
	{
		m_DestroyRequested = true;
		if(!IsActivated() ) Destroy();
	}

	void Destroy()
	{
		delete this;
	}
	
	//!gets called upon animation Symptom exit
	void OnAnimationFinish()
	{
		if( m_DestroyOnAnimFinish ) RequestDestroy();
	}
	//!this is just for the Symptom parameters set-up and is called even if the Symptom doesn't execute, don't put any gameplay code in here
	void OnInit(){}
	
	//!gets called every frame
	void OnUpdateServer(PlayerBase player, float deltatime){}
	bool CanActivate(){return true;}//server only
	//!gets called every frame
	void OnUpdateClient(PlayerBase player, float deltatime){}
	//!gets called once on an Symptom which is being activated
	void OnGetActivatedServer(PlayerBase player){}
	void OnGetActivatedClient(PlayerBase player){}
	//!only gets called once on an active Symptom that is being deactivated
	void OnGetDeactivatedServer(PlayerBase player){}
	void OnGetDeactivatedClient(PlayerBase player){}
}
