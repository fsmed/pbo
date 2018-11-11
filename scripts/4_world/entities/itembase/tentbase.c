class TentBase extends ItemBase
{
	protected const bool PACKED 	= false;
	protected const bool PITCHED 	= true;
	
	protected bool m_State;
	protected bool m_IsEntrance;
	protected bool m_IsWindow;
	protected bool m_IsToggle;
	
	protected ref map< ref ToggleAnimations, bool> m_ToggleAnimations;
	protected ref array<string> m_ShowAnimationsWhenPitched;
	protected ref array<string> m_ShowAnimationsWhenPacked;
	protected Object m_ClutterCutter;
	
	void TentBase()
	{
		m_ToggleAnimations = new map<ref ToggleAnimations, bool>;
		m_ShowAnimationsWhenPitched = new array<string>;
		m_ShowAnimationsWhenPacked = new array<string>;
		RegisterNetSyncVariableBool("m_State");
		RegisterNetSyncVariableBool("m_IsSoundSynchRemote");
		RegisterNetSyncVariableBool("m_IsEntrance");
		RegisterNetSyncVariableBool("m_IsWindow");
		RegisterNetSyncVariableBool("m_IsToggle");
	}
	
	void ~TentBase()
	{
		if ( GetGame() )
		{
			DestroyClutterCutter();
		}
	}
	
	override void OnStoreSave( ParamsWriteContext ctx )
	{   
		super.OnStoreSave( ctx );
		
		ctx.Write( m_State );
	}
	
	override void OnStoreLoad( ParamsReadContext ctx )
	{
		super.OnStoreLoad( ctx );
		
		ctx.Read( m_State );
		
		if ( GetState() == PITCHED )
		{
			Pitch();
		}
		else
		{
			Pack();
		}
	}
	
	override void EEInit()
	{
		super.EEInit();
		
		if ( GetState() == PITCHED )
		{
			Pitch();
		}
		else
		{
			Pack();
		}
	}
	
	override void OnItemLocationChanged(EntityAI old_owner, EntityAI new_owner)
	{		
		super.OnItemLocationChanged(old_owner, new_owner);
		
		if ( new_owner || old_owner )
		{
			Pack();
		}
	}
	
	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();
			
		if ( GetState() == PITCHED )
		{	
			if ( IsManipulatedEntrance() && IsSoundSynchRemote() )
			{						
				if ( m_IsToggle )
				{
					SoundTentOpenPlay();
				}
				else
				{
					SoundTentClosePlay();
				}	
			}
			else if ( IsManipulatedWindow() && IsSoundSynchRemote() )
			{						
				if ( m_IsToggle )
				{
					SoundTentOpenWindowPlay();
				}
				else
				{
					SoundTentCloseWindowPlay();
				}
			}
			else
			{
				Pitch();
			}
		}
		else
		{
			Pack();
		}			
	}

	void HideAllAnimationsAndProxyPhysics()
	{		
		string cfg_path = "cfgVehicles " + GetType() + " AnimationSources";
		
		if ( GetGame().ConfigIsExisting( cfg_path ) )
		{
			int	selections = GetGame().ConfigGetChildrenCount( cfg_path );
			string proxy_selection_name;
			
			for ( int i = 0; i < selections; i++ )
			{
				string selection_name;
				GetGame().ConfigGetChildName( cfg_path, i, selection_name );
				SetAnimationPhase( selection_name, 1 );
				
				proxy_selection_name = selection_name;
				proxy_selection_name.ToLower();	
				RemoveProxyPhysics( proxy_selection_name );
			}
		}
	}
	
	override bool IsDeployable()
	{
		return true;
	}
	
	bool ConditionIntoInventory( EntityAI player )
	{
		return CanBeManipulated();
	}

	override bool CanPutIntoHands( EntityAI parent )
	{
		if( !super.CanPutIntoHands( parent ) )
		{		
			return false;
		}
				
		return CanBeManipulated();
	}

	override bool CanPutInCargo( EntityAI parent )
	{
		if( !super.CanPutInCargo( parent ) )
		{			
			return false;
		}
				
		return CanBeManipulated();
	}
	
	bool ConditionOutOfHands( EntityAI player )
	{
		return CanBeManipulated();
	}
	
	override void EEItemAttached(EntityAI item, string slot_name)
	{
		super.EEItemAttached(item, slot_name);
		
		if ( item.IsKindOf ( "CamoNet" ) ) 
		{
			SetAnimationPhase( "Camonet", 0 );
			
			if ( !IsKindOf ( "MediumTent" ) )
			{
				AddProxyPhysics( "camonet" );
			}		
		}
		
		if ( item.IsKindOf ( "XmasLights" ) ) 
		{
			SetAnimationPhase( "Xlights", 0 );
			SetAnimationPhase( "Xlights_glass_r", 0 );
			SetAnimationPhase( "Xlights_glass_g", 0 );
			SetAnimationPhase( "Xlights_glass_b", 0 );
			SetAnimationPhase( "Xlights_glass_y", 0 );
			
			XmasLights xlights = XmasLights.Cast( item );
			xlights.AttachToObject( this );
		}
	}

	override void EEItemDetached(EntityAI item, string slot_name)
	{
		super.EEItemDetached(item, slot_name);
				
		if ( item.IsKindOf ( "CamoNet" ) ) 
		{
			SetAnimationPhase( "Camonet", 1 );
			
			if ( !IsKindOf ( "MediumTent" ) )
			{
				RemoveProxyPhysics( "camonet" );
			}
		}
		
		if ( item.IsKindOf ( "XmasLights" ) ) 
		{
			SetAnimationPhase( "Xlights", 1 );
			SetAnimationPhase( "Xlights_glass_r", 1 );
			SetAnimationPhase( "Xlights_glass_g", 1 );
			SetAnimationPhase( "Xlights_glass_b", 1 );
			SetAnimationPhase( "Xlights_glass_y", 1 );
			
			XmasLights xlights = XmasLights.Cast( item );
			xlights.DetachFromObject( this );
		}
	}
	
	override void OnPlacementComplete( Man player )
	{
		Pitch( true );
	}
	
	int GetState()
	{
		return m_State;
	}

	bool CanBePacked()
	{
		if ( GetState() == PITCHED )
		{
			if ( GetInventory().GetCargo().GetItemCount() == 0 && GetInventory().AttachmentCount() == 0 )
			{
				return true;
			}
		}
		
		return false;
	}

	bool CanBeManipulated()
	{
		if ( GetState() == PACKED )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	
	bool CanAttach( ItemBase item )
	{
		if ( item.IsKindOf ( "CamoNet" ) && GetState() == PITCHED ) 
		{
			return true;
		}
		
		return false;
	}
	
	void Pack( bool update_navmesh = true )
	{	
		HideAllAnimationsAndProxyPhysics();
		
		m_State = PACKED;
		m_IsEntrance = PACKED;
		m_IsWindow = PACKED;
		m_IsToggle = PACKED;
		
		Refresh();
				
		GetInventory().LockInventory(HIDE_INV_FROM_SCRIPT);
		
		if ( update_navmesh ) 
		{
			RegenerateNavmesh();	
		}
						
		DestroyClutterCutter();
		
		SetViewIndex( PACKED );
		
		SetSynchDirty();
	}

	void Pitch( bool update_navmesh = true )
	{		
		HideAllAnimationsAndProxyPhysics();
		
		m_State = PITCHED;
		m_IsEntrance = PITCHED;
		m_IsWindow = PITCHED;
		m_IsToggle = PITCHED;
		
		Refresh();
		
		GetInventory().UnlockInventory(HIDE_INV_FROM_SCRIPT);
		
		if ( update_navmesh ) 
		{
			RegenerateNavmesh();
		}
		
		SetViewIndex( PITCHED );

		SetSynchDirty();
	}
	
	void UpdateVisuals()
	{
		string proxy_selection_name;
		string animation_name;
		
		if ( GetState() == PITCHED )
		{	
			for ( int i = 0; i < m_ShowAnimationsWhenPitched.Count(); i++ )
			{
				animation_name = m_ShowAnimationsWhenPitched.Get(i);
				
				SetAnimationPhase( animation_name, 0 );
			}
		}
		else
		{	
			for ( int j = 0; j < m_ShowAnimationsWhenPacked.Count(); j++ )
			{
				animation_name = m_ShowAnimationsWhenPacked.Get(j);
				
				SetAnimationPhase( animation_name, 0 );
			}
		}		
	}
	
	void UpdatePhysics()
	{
		string proxy_selection_name;
		string animation_name;
		
		if ( GetState() == PITCHED )
		{	
			for ( int i = 0; i < m_ShowAnimationsWhenPitched.Count(); i++ )
			{
				animation_name = m_ShowAnimationsWhenPitched.Get(i);
				
				proxy_selection_name = animation_name;
				proxy_selection_name.ToLower();
				AddProxyPhysics( proxy_selection_name );
			}
		}
		else
		{	
			for ( int j = 0; j < m_ShowAnimationsWhenPacked.Count(); j++ )
			{
				animation_name = m_ShowAnimationsWhenPacked.Get(j);
				
				proxy_selection_name = animation_name;
				proxy_selection_name.ToLower();
				AddProxyPhysics( proxy_selection_name );
			}
		}	
	}
	
	//refresh visual/physics state
	void Refresh()
	{
		GetGame().GetCallQueue( CALL_CATEGORY_GAMEPLAY ).Call( UpdateVisuals );
		GetGame().GetCallQueue( CALL_CATEGORY_GAMEPLAY ).CallLater( UpdatePhysics, 100, false );
	}
	
	bool CanToggleAnimations( string selection )
	{		
		for ( int i = 0; i < m_ToggleAnimations.Count(); i++ )
		{
			ToggleAnimations toggle = m_ToggleAnimations.GetKey(i);
			string toggle_off = toggle.GetToggleOff();
			toggle_off.ToLower();
			string toggle_on = toggle.GetToggleOn();
			toggle_on.ToLower();
			
			if ( toggle_off == selection || toggle_on == selection )
			{
				return true;
			}
		}
		
		return false;
	}

	void ResetToggle()
	{
		m_IsEntrance = false;
		m_IsWindow = false;
		m_IsToggle = false;
	}
	
	void ManipulateEntrance()
	{
		m_IsEntrance = true;
	}
	
	void ManipulateWindow()
	{
		m_IsWindow = true;
	}
	
	bool IsManipulatedEntrance()
	{
		return m_IsEntrance;
	}
	
	bool IsManipulatedWindow()
	{
		return m_IsWindow;
	}
	
	void ToggleAnimation( string selection )
	{	
		ResetToggle();
		
		for ( int i = 0; i < m_ToggleAnimations.Count(); i++ )
		{
			ToggleAnimations toggle = m_ToggleAnimations.GetKey(i);
			
			string toggle_off = toggle.GetToggleOff();
			toggle_off.ToLower();
			string toggle_on = toggle.GetToggleOn();
			toggle_on.ToLower();
						
			if ( toggle_off == selection || toggle_on == selection )
			{				
				if ( m_ToggleAnimations.GetElement(i) )
				{
					SetAnimationPhase( toggle.GetToggleOff(), 0 );
					AddProxyPhysics( toggle.GetToggleOff() );
					SetAnimationPhase( toggle.GetToggleOn(), 1 );
					RemoveProxyPhysics( toggle.GetToggleOn() );
					m_ToggleAnimations.Set( toggle, false );
					m_IsToggle = true;
										
					if ( selection.IndexOfFrom( 0, "entrance" ) )
					{
						ManipulateWindow();
					}
					
					if ( selection.IndexOfFrom( 0, "window" ) )
					{
						ManipulateEntrance();
					}
				}
				else
				{				
					SetAnimationPhase( toggle.GetToggleOff(), 1 );
					RemoveProxyPhysics( toggle.GetToggleOff() );
					SetAnimationPhase( toggle.GetToggleOn(), 0 );
					AddProxyPhysics( toggle.GetToggleOn() );
					m_ToggleAnimations.Set( toggle, true );
					m_IsToggle = false;
					
					if ( selection.IndexOfFrom( 0, "entrance" ) )
					{
						ManipulateWindow();
					}
					
					if ( selection.IndexOfFrom( 0, "window" ) )
					{
						ManipulateEntrance();
					}
				}
			}
		}
		
		SoundSynchRemote();
	}
	
	string GetSoundOpen()
	{
		return "";
	}
	
	string GetSoundClose()
	{
		return "";
	}
	
	string GetSoundOpenWindow()
	{
		return "";
	}
	
	string GetSoundCloseWindow()
	{
		return "";
	}
	
	void SoundTentOpenPlay()
	{
		EffectSound sound =	SEffectManager.PlaySound( GetSoundOpen(), GetPosition() );
		sound.SetSoundAutodestroy( true );
	}
	
	void SoundTentClosePlay()
	{
		EffectSound sound =	SEffectManager.PlaySound( GetSoundClose(), GetPosition() );
		sound.SetSoundAutodestroy( true );
	}
	
	void SoundTentOpenWindowPlay()
	{
		EffectSound sound =	SEffectManager.PlaySound( GetSoundOpenWindow(), GetPosition() );
		sound.SetSoundAutodestroy( true );
	}
	
	void SoundTentCloseWindowPlay()
	{
		EffectSound sound =	SEffectManager.PlaySound( GetSoundCloseWindow(), GetPosition() );
		sound.SetSoundAutodestroy( true );
	}
	
	void RegenerateNavmesh()
	{
		SetAffectPathgraph( true, false );
		
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(GetGame().UpdatePathgraphRegionByObject, 100, false, this);
	}
	
	void DestroyClutterCutter()
	{
		if ( GetGame().IsMultiplayer() || GetGame().IsServer() )
		{
			if ( m_ClutterCutter )
			{
				GetGame().ObjectDelete( m_ClutterCutter );
			}
		}
	}
};
