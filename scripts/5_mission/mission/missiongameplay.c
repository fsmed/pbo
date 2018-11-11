// #include "Scripts/GUI/IngameHud.c"

class MissionGameplay extends MissionBase
{
	int								m_LifeState;
	bool							m_Initialized;
	
	protected UIManager				m_UIManager;
	
	Widget							m_HudRootWidget;
	ImageWidget 					m_MicrophoneIcon;
	
	private ref WidgetCache 		m_WidgetsCache;
	
	ref InventoryMenu				m_InventoryMenu;
	ref Chat 						m_Chat;
	ref ActionMenu					m_ActionMenu;
	ref IngameHud					m_Hud;
	ref HudDebug					m_HudDebug;
	ref LogoutMenu					m_Logout;
	ref DebugMonitor				m_DebugMonitor;
	
	ref Timer						m_ChatChannelHideTimer;
	ref WidgetFadeTimer				m_ChatChannelFadeTimer;
	
	Widget							m_ChatChannelArea;
	TextWidget						m_ChatChannelText;
	NoteMenu 						m_Note;
	
	protected ref Timer				m_ToggleHudTimer;
	protected const int 			HOLD_LIMIT_TIME	= 300; //ms
	protected int					m_ActionDownTime;
	protected int					m_ActionUpTime;
	protected bool 					m_InitOnce;
	protected bool 					m_ControlDisabled;
	
	void MissionGameplay()
	{
		DestroyAllMenus();
		m_Initialized				= false;
		m_HudRootWidget				= null;
		m_Chat						= new Chat;
		m_ActionMenu				= new ActionMenu;
		m_LifeState					= -1;
		m_Hud						= new IngameHud;
		m_ChatChannelFadeTimer		= new WidgetFadeTimer;
		m_ChatChannelHideTimer		= new Timer(CALL_CATEGORY_GUI);
		
		m_ToggleHudTimer			= new Timer(CALL_CATEGORY_GUI);
		
		g_Game.m_loadingScreenOn	= true;
		
		SyncEvents.RegisterEvents();
		g_Game.SetGameState( DayZGameState.IN_GAME );
		PlayerBase.Event_OnPlayerDeath.Insert( Pause );
	}
	
	void ~MissionGameplay()
	{
		DestroyInventory();
		PlayerBase.Event_OnPlayerDeath.Remove( Pause );
		PPEffects.ResetAll();
		//GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(this.UpdateDebugMonitor);
	#ifndef NO_GUI
		if (g_Game.GetUIManager() && g_Game.GetUIManager().ScreenFadeVisible())
		{
			GetGame().SetEVUser(0);
			g_Game.GetUIManager().ScreenFadeOut(0);
		}
	#endif

	}
	
	InventoryMenu GetInventory()
	{
		return m_InventoryMenu;
	}
	
	override void OnInit()
	{
		if ( m_Initialized )
		{
			return;
		}
			
		PPEffects.Init();
		
		m_UIManager = GetGame().GetUIManager();
			
		g_Game.m_loadingScreenOn	= true;
		m_Initialized				= true;

		// init hud ui
		if ( !m_HudRootWidget )
		{
			m_HudRootWidget			= GetGame().GetWorkspace().CreateWidgets("gui/layouts/day_z_hud.layout");
			
			m_HudRootWidget.Show(false);
			
			m_Chat.Init(m_HudRootWidget.FindAnyWidget("ChatFrameWidget"));
			
			m_ActionMenu.Init( m_HudRootWidget.FindAnyWidget("ActionsPanel"), TextWidget.Cast( m_HudRootWidget.FindAnyWidget("DefaultActionWidget") ) );
			
			m_Hud.Init( m_HudRootWidget.FindAnyWidget("HudPanel") );
			
			m_MicrophoneIcon		= ImageWidget.Cast( m_HudRootWidget.FindAnyWidget("mic") );

			// notification window
			m_NotificationWidget	= new NotificationMessage( m_HudRootWidget );
			
			// chat channel
			m_ChatChannelArea		= m_HudRootWidget.FindAnyWidget("ChatChannelPanel");
			m_ChatChannelText		= TextWidget.Cast( m_HudRootWidget.FindAnyWidget("ChatChannelText") );
		}
		
		// init hud ui
		if ( GetGame().IsDebug() )
		{
			m_HudDebug				= new HudDebug;
			
			if ( !m_HudDebug.IsInitialized() )
			{
				m_HudDebug.Init( GetGame().GetWorkspace().CreateWidgets("gui/layouts/debug/day_z_hud_debug.layout") );
				
				Debug.SetEnabledLogs(PluginConfigDebugProfile.GetInstance().GetLogsEnabled());
			}
		}

		//AIBehaviourHL.RegAIBehaviour("zombie2",AIBehaviourHLZombie2,AIBehaviourHLDataZombie2);
		//RegBehaviour("zombie2",AIBehaviourHLZombie2,AIBehaviourHLDataZombie2);
		
		m_WidgetsCache				= new WidgetCache;
		
		if( GetGame().IsMultiplayer() )
		{
			OnlineServices.m_MuteUpdateAsyncInvoker.Insert( SendMuteListToServer );
		}
	}
	
	UIManager GetUIManager()
	{
		return m_UIManager;
	}

	override void OnMissionStart()
	{
		//does not display HUD until player is fully loaded
		//m_HudRootWidget.Show(true);
		GetUIManager().ShowUICursor(false);
		g_Game.SetMissionState( DayZGame.MISSION_STATE_GAME );
	}
	
	void InitInventory()
	{
		DestroyAllMenus();
		if ( !m_InventoryMenu )
		{
			m_InventoryMenu = InventoryMenu.Cast( GetUIManager().EnterScriptedMenu(MENU_INVENTORY, NULL) );
			GetUIManager().HideScriptedMenu( m_InventoryMenu );
		}
	}

	WidgetCache GetWidgetCache()
	{
		return m_WidgetsCache;
	}
	
	void TickScheduler(float timeslice)
	{
		PlayerBase player = PlayerBase.Cast( GetGame().GetPlayer() );
				
		if( player )
			player.OnTick();
	}
	
	void SendMuteListToServer( map<string, bool> mute_list )
	{
		if( mute_list && mute_list.Count() > 0 )
		{
			if ( ScriptInputUserData.CanStoreInputUserData() )
			{
				ScriptInputUserData ctx = new ScriptInputUserData;
				ctx.Write( INPUT_UDT_USER_SYNC_PERMISSIONS );
				ref map<string, bool> mute_list2;
				if( mute_list.Count() > 4 )
				{
					mute_list2 = new map<string, bool>;
					for( int i = mute_list.Count() - 1; i >= 4; i-- )
					{
						mute_list2.Insert( mute_list.GetKey( i ), mute_list.GetElement( i ) );
						mute_list.RemoveElement( i );
					}
				}
				ctx.Write( mute_list );
				Print( mute_list );
				ctx.Send();
				SendMuteListToServer( mute_list2 );
			}
		}
	}
	
	override void OnMissionFinish()
	{
		PlayerBase player = PlayerBase.Cast( GetGame().GetPlayer() );
		
		if ( player )
		{
			int player_last_shaved_seconds = player.GetLastShavedSeconds();
			GetGame().SetProfileString( "lastShavedSeconds", player_last_shaved_seconds.ToString() );
		}
		
		Print("OnMissionFinish");
		GetUIManager().HideDialog();
		DestroyAllMenus();
		
		m_Chat.Destroy();
		delete m_HudRootWidget;
		
		#ifdef PLATFORM_CONSOLE
			OnlineServices.m_MuteUpdateAsyncInvoker.Remove( SendMuteListToServer );
		#endif

		if (m_DebugMonitor)
			m_DebugMonitor.Hide();
		g_Game.GetUIManager().ShowUICursor(false);
		g_Game.SetMissionState( DayZGame.MISSION_STATE_FINNISH );
	}
	
	override void OnUpdate(float timeslice)
	{
		Man player = GetGame().GetPlayer();
		PlayerBase playerPB = PlayerBase.Cast(player);
		TickScheduler(timeslice);
		UpdateDummyScheduler();//for external entities
		UIScriptedMenu menu = m_UIManager.GetMenu();
		InventoryMenu inventory = InventoryMenu.Cast( m_UIManager.FindMenu(MENU_INVENTORY) );
		MapMenu mapmenu = MapMenu.Cast( m_UIManager.FindMenu(MENU_MAP) );
		//m_InventoryMenu = inventory;
		InspectMenuNew inspect = InspectMenuNew.Cast( m_UIManager.FindMenu(MENU_INSPECT) );
		Input input = GetGame().GetInput();
		
		//TODO should be switchable
		if (playerPB && playerPB.enterNoteMenuRead)
		{
			playerPB.enterNoteMenuRead = false;
			Paper paper = Paper.Cast(playerPB.GetItemInHands());
			m_Note = NoteMenu.Cast( GetUIManager().EnterScriptedMenu(MENU_NOTE, menu) ); //NULL means no parent
			m_Note.InitRead(paper.m_AdvancedText);
		}
		
		if (playerPB && playerPB.enterNoteMenuWrite)
		{
			playerPB.enterNoteMenuWrite = false;
			m_Note = NoteMenu.Cast( GetUIManager().EnterScriptedMenu(MENU_NOTE, menu) ); //NULL means no parent
			m_Note.InitWrite(playerPB.m_paper,playerPB.m_writingImplement,playerPB.m_Handwriting);
		}

#ifdef PLATFORM_CONSOLE
		//Quick Reload Weapon
		if ( !menu && input.GetActionDown( UAQuickReload, false ) )
		{
			if ( !GetGame().IsInventoryOpen() && playerPB && !playerPB.GetActionManager().FindActionTarget().GetObject() )
			{
				EntityAI entity_hands = playerPB.GetHumanInventory().GetEntityInHands();
				
				if ( entity_hands && entity_hands.IsWeapon() )
				{
					playerPB.QuickReloadWeapon( entity_hands );
				}
			}
		}

		//Switch beween weapons in quickslots 
		if( !menu && input.GetActionDown( UAUIRadialMenuPick, false ) )
		{
			if ( !GetGame().IsInventoryOpen() )
			{
				EntityAI entity_in_hands = playerPB.GetHumanInventory().GetEntityInHands();
				EntityAI quickbar_entity;
				int quickbar_index = 0;
				
				if ( entity_in_hands )
				{			
					int quickbar_entity_hands_index = playerPB.FindQuickBarEntityIndex( entity_in_hands );
					
					if ( quickbar_entity_hands_index > -1 && quickbar_entity_hands_index < MAX_QUICKBAR_SLOTS_COUNT - 1 )	//(0->8)
					{
						quickbar_index = quickbar_entity_hands_index + 1;
					}
				}

				//find next weapon
				for ( int iter = 0; iter < MAX_QUICKBAR_SLOTS_COUNT; ++iter )
				{
					quickbar_entity = playerPB.GetQuickBarEntity( quickbar_index );
					
					if ( quickbar_entity && ( quickbar_entity.IsWeapon() || ( quickbar_entity.IsMeleeWeapon() && !quickbar_entity.IsMagazine() ) ) )
					{
						break;
					}
					
					quickbar_index += 1;
					if ( quickbar_index > MAX_QUICKBAR_SLOTS_COUNT - 1 )
					{
						quickbar_index = 0;	//reset
					}
				}
				
				//swap
				int slot_id;
				if ( quickbar_index > -1 )
				{
					slot_id = quickbar_index + 1;
					if ( slot_id == MAX_QUICKBAR_SLOTS_COUNT )
					{
						slot_id = 0;
					}
					
					playerPB.RadialQuickBarSingleUse( slot_id );
				}
			}
		}

		//Gestures
		if(input.GetActionDown(UAUIGesturesOpen, false))
		{
			//open gestures menu
			if ( !GetUIManager().IsMenuOpen( MENU_GESTURES ) )
			{
				GesturesMenu.OpenMenu();
			}
		}
		
		if(input.GetActionUp(UAUIGesturesOpen, false))
		{
			//close gestures menu
			if ( GetUIManager().IsMenuOpen( MENU_GESTURES ) )
			{
				GesturesMenu.CloseMenu();
			}
		}

		//Radial quickbar
		if(input.GetActionDown(UAUIQuickbarRadialOpen, false))
		{
			//open gestures menu
			if ( !GetUIManager().IsMenuOpen( MENU_RADIAL_QUICKBAR ) )
			{
				RadialQuickbarMenu.OpenMenu();
			}
		}
		
		if(input.GetActionUp(UAUIQuickbarRadialOpen, false))
		{
			//close gestures menu
			if ( GetUIManager().IsMenuOpen( MENU_RADIAL_QUICKBAR ) )
			{
				RadialQuickbarMenu.CloseMenu();
			}
		}
		
		//Radial Quickbar from inventory
		if( GetGame().GetInput().GetActionUp( UAUIQuickbarRadialInventoryOpen, false ) )
		{
			//close radial quickbar menu
			if ( GetGame().GetUIManager().IsMenuOpen( MENU_RADIAL_QUICKBAR ) )
			{
				RadialQuickbarMenu.CloseMenu();
				RadialQuickbarMenu.SetItemToAssign( NULL );
			}
		}		
#endif

		if (player && m_LifeState == EPlayerStates.ALIVE && !player.IsUnconscious() )
		{
			// enables HUD on spawn
			if (m_HudRootWidget)
			{
				m_HudRootWidget.Show(true);
			}
			
		#ifndef NO_GUI
			// fade out black screen
			
			if ( GetUIManager().ScreenFadeVisible() )
			{
				 GetUIManager().ScreenFadeOut(0.5);
			}
			
		#endif
		
			if( input.GetActionDown(UAGear, false ) )
			{
				if( !inventory )
				{
					ShowInventory();
				}
				else if( menu == inventory )
				{
					HideInventory();
				}
			}

			if (input.GetActionDown(UAChat, false))
			{
				ChatInputMenu chat = ChatInputMenu.Cast( m_UIManager.FindMenu(MENU_CHAT) );		
				if( menu == NULL )
				{
					ShowChat();
				}
			}
			
			if ( input.GetActionDown( UAUIQuickbarToggle, false) )
			{
				SetActionDownTime( GetGame().GetTime() );
				bool hud_state = m_Hud.GetHudState();
				m_ToggleHudTimer.Run( 0.3, m_Hud, "ToggleHud", new Param1<bool>( !hud_state ) );
			}
			
			if ( input.GetActionUp( UAUIQuickbarToggle, false) )
			{
				SetActionUpTime( GetGame().GetTime() );
				
				if ( GetHoldActionTime() < HOLD_LIMIT_TIME )
				{
					if ( menu == NULL )
					{
						if ( m_Hud.GetQuickBarState() )
						{
							m_Hud.HideQuickbar( false, true );
						}
						else
						{
							m_Hud.ShowQuickbar();
						}
					}
				}
				
				m_ToggleHudTimer.Stop();
			}
			
			if ( g_Game.GetInput().GetActionDown( UAZeroingUp, false) || g_Game.GetInput().GetActionDown( UAZeroingDown, false) || g_Game.GetInput().GetActionDown( UAToggleWeapons, false) )
			{

				m_Hud.ZeroingKeyPress();
			}
			
			if ( menu == NULL )
			{
				m_ActionMenu.Refresh();
				
				if (input.GetActionDown(UANextAction, false))
				{
					m_ActionMenu.NextAction();
				}
				
				if (input.GetActionDown(UAPrevAction, false))
				{
					m_ActionMenu.PrevAction();
				}
			}
			else
			{
				m_ActionMenu.Hide();
			}
			
			//hologram rotation
			if (menu == NULL && playerPB.IsPlacingLocal())
			{
				if (input.GetActionUp(UANextAction, false))
				{
					playerPB.GetHologramLocal().SubtractProjectionRotation(15);
				}
				
				if (input.GetActionUp(UAPrevAction, false))
				{
					playerPB.GetHologramLocal().AddProjectionRotation(15);
				}
			}
		}
		
		// life state check
		if (player)
		{
			int life_state = player.GetPlayerState();
			
			// life state changed
			if (m_LifeState != life_state)
			{
				m_LifeState = life_state;
				
				if (m_LifeState != EPlayerStates.ALIVE)
				{
					CloseAllMenus();
				}
			}
		}
		
		if (menu && !menu.UseKeyboard() && menu.UseMouse())
		{
			int i;
			for (i = 0; i < 5; i++)
			{
				input.DisableKey(i | INPUT_DEVICE_MOUSE);
				input.DisableKey(i | INPUT_ACTION_TYPE_DOWN_EVENT | INPUT_DEVICE_MOUSE);
				input.DisableKey(i | INPUT_ACTION_TYPE_DOUBLETAP | INPUT_DEVICE_MOUSE);
			}
			
			for (i = 0; i < 6; i++)
			{
				input.DisableKey(i | INPUT_DEVICE_MOUSE_AXIS);
			}
		}
		
		if (!m_UIManager.IsDialogVisible())
		{
			if ( menu )
			{

				if ( IsPaused() )
				{
					if( input.GetActionDown(UAUIBack, false) )
					{
						Continue();						
					}
					else if( input.GetActionDown(UAUIMenu, false) )
					{
						Continue();
					}
				}
				else if ( menu == inventory )
				{
					if(input.GetActionDown(UAUIBack, false))
					{
						if( ItemManager.GetInstance().GetSelectedItem() == NULL )
						{
							HideInventory();
						}
					}
				}
				else if( menu == inspect )
				{
					if(input.GetActionDown(UAGear, false))
					{
						if( ItemManager.GetInstance().GetSelectedItem() == NULL )
						{
							HideInventory();
						}
					}
					else if(input.GetActionDown(UAUIBack, false))
					{
						if( ItemManager.GetInstance().GetSelectedItem() == NULL )
						{
							HideInventory();
						}
					}
				}
				else if(input.GetActionDown(UAUIBack, false))
				{
					m_UIManager.Back();
					PlayerControlEnable();
				}
				else if(menu == mapmenu && !m_ControlDisabled)
				{
					PlayerControlDisable();
				}
				
			}
			else if (input.GetActionDown(UAUIMenu, false))
			{
				Pause();
				PlayerControlDisable();
			}
		}
		
		UpdateDebugMonitor();
		
		SEffectManager.Event_OnFrameUpdate(timeslice);
	}
	
	override void OnKeyPress(int key)
	{
		super.OnKeyPress(key);
		m_Hud.KeyPress(key);
		
		
		//temporary
		//Gestures [.]
#ifdef DEVELOPER
		/*if ( key == KeyCode.KC_PERIOD )
		{
			//open gestures menu
			if ( !GetUIManager().IsMenuOpen( MENU_GESTURES ) )
			{
				//TODO reconnect when appropriate
				GesturesMenu.OpenMenu();
			}
		}*/
#endif
		/*
		//temporary
		//Radial Quickbar [,]
		if ( key == KeyCode.KC_COMMA )
		{
			//open radial quickbar menu
			if ( !GetGame().GetUIManager().IsMenuOpen( MENU_RADIAL_QUICKBAR ) )
			{
				//TODO reconnect when appropriate
				RadialQuickbarMenu.OpenMenu();
			}
		}
		*/
		
#ifdef DEVELOPER
		// drop item prototype
		if ( key == KeyCode.KC_N )
		{
			PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
			if (player && player.GetItemInHands() && !GetUIManager().GetMenu())
			{
				ActionManagerClient manager = ActionManagerClient.Cast(player.GetActionManager());
				manager.ActionDropItemStart(player.GetItemInHands(),null);
			}
		}
		
		/*if ( key == KeyCode.KC_P )
		{
			if (!player)
				player = PlayerBase.Cast(GetGame().GetPlayer());
			player.SetHealth("Brain","Health",0);
			player.SetHealth("","Health",0);
		}*/
		
		/*if ( key == KeyCode.KC_P )
		{
			if (!player)
				player = PlayerBase.Cast(GetGame().GetPlayer());
			int slot_id = InventorySlots.GetSlotIdFromString("Legs"); 
 			EntityAI players_legs = player.GetInventory().FindPlaceholderForSlot( slot_id );
			Print("--attachment type = " + players_legs.GetType());
		}*/
		
		/*if ( key == KeyCode.KC_P )
		{
			if (!player)
				player = PlayerBase.Cast(GetGame().GetPlayer());
			if (player && player.m_EmoteManager && player.GetItemInHands())
			{
				player.DropItem(player.GetItemInHands());
			}
		}*/
		
		if ( key == KeyCode.KC_Q )
		{
			//SEffectManager.PlaySound("HandCuffs_A_SoundSet", GetGame().GetPlayer().GetPosition(), 0, 0, false);
		}
#endif
	}
		
			
	
	override void OnKeyRelease(int key)
	{
		super.OnKeyRelease(key);
		
		//temporary
		//Gestures [.]
#ifdef DEVELOPER
		/*if ( key == KeyCode.KC_PERIOD )
		{
			//close gestures menu
			if ( GetUIManager().IsMenuOpen( MENU_GESTURES ) )
			{
				//TODO reconnect when appropriate
				GesturesMenu.CloseMenu();
			}
		}*/
#endif
		/*
		//temporary
		//Radial Quickbar [,]
		if ( key == KeyCode.KC_COMMA )
		{
			//close radial quickbar menu
			if ( GetGame().GetUIManager().IsMenuOpen( MENU_RADIAL_QUICKBAR ) )
			{
				//TODO reconnect when appropriate
				RadialQuickbarMenu.CloseMenu();
			}
		}
		*/
	}
	
	override void OnEvent(EventType eventTypeId, Param params)
	{
		super.OnEvent(eventTypeId, params);
		InventoryMenu menu;
		Man player = GetGame().GetPlayer();
		
		switch(eventTypeId)
		{
		case ChatMessageEventTypeID:
			ChatMessageEventParams chat_params = ChatMessageEventParams.Cast( params );			
			if (m_LifeState == EPlayerStates.ALIVE)
			{
				m_Chat.Add(chat_params);
			}
			break;
			
		case ChatChannelEventTypeID:
			ChatChannelEventParams cc_params = ChatChannelEventParams.Cast( params );
			ChatInputMenu chatMenu = ChatInputMenu.Cast( GetUIManager().FindMenu(MENU_CHAT_INPUT) );
			if (chatMenu)
			{
				chatMenu.UpdateChannel();
			}
			else
			{
				m_ChatChannelText.SetText(ChatInputMenu.GetChannelName(cc_params.param1));
				m_ChatChannelFadeTimer.FadeIn(m_ChatChannelArea, 0.5, true);
				m_ChatChannelHideTimer.Run(2, m_ChatChannelFadeTimer, "FadeOut", new Param3<Widget, float, bool>(m_ChatChannelArea, 0.5, true));
			}
			break;
			
		case WindowsResizeEventTypeID:
			if( m_InventoryMenu != NULL )
			{
				m_InventoryMenu.Reset();
			}
			else if( WidgetCache.GetInstance() )
			{
				WidgetCache.GetInstance().Reset();
			}
			CloseAllMenus();
			DestroyAllMenus();
			
			break;
			
		case VONStateEventTypeID:
			VONStateEventParams vonStateParams = VONStateEventParams.Cast( params );
			if (vonStateParams.param1)
			{
				m_MicrophoneIcon.Show(true);
			}
			else
			{
				m_MicrophoneIcon.Show(false);
			}
			break;
		
		case SetFreeCameraEventTypeID:
			SetFreeCameraEventParams set_free_camera_event_params = SetFreeCameraEventParams.Cast( params );
			PluginDeveloper plugin_developer = PluginDeveloper.Cast( GetPlugin(PluginDeveloper) );
			plugin_developer.OnSetFreeCameraEvent( PlayerBase.Cast( player ), set_free_camera_event_params.param1 );
			break;
		}
	}
	
	override void OnItemUsed(InventoryItem item, Man owner)
	{
		if (item && GetUIManager().GetMenu() == NULL)
		{
			if (item.IsInherited(ItemBook))
			{
				BookMenu bookMenu = BookMenu.Cast( GetUIManager().EnterScriptedMenu(MENU_BOOK, NULL) );
				if (bookMenu)
				{
					bookMenu.ReadBook(item);
				}
			}
		}
	}
	
	override void PlayerControlEnable()
	{
		//Print("Enabling Controls");
#ifdef WIP_INPUTS
		GetUApi().ActivateGroup("infantry");
#endif	
		m_ControlDisabled = false;	
	}

	override void PlayerControlDisable()
	{
		//Print("Disabling Controls");
#ifdef WIP_INPUTS
			GetUApi().ActivateExclude("inventory");
			GetUApi().ActivateGroup("infantry");
#endif
		m_ControlDisabled = true;
	}

	bool IsControlDisabled()
	{
		return m_ControlDisabled;
	}
	
	void CloseAllMenus()
	{
		HideInventory();
		GetUIManager().CloseAll();
	}
	
	
	void DestroyAllMenus()
	{
		if( GetUIManager() )
		{
			GetUIManager().HideDialog();
			GetUIManager().CloseAll();
		}
		
		DestroyInventory();
		
		if( m_Chat )
			m_Chat.Clear();
	}
	
	void MoveHudForInventory( bool inv_open )
	{
		#ifdef PLATFORM_CONSOLE
		IngameHud hud = IngameHud.Cast( GetHud() );
		if( hud )
		{
		if( inv_open )
		{
			hud.GetHudPanelWidget().SetPos( 0, -0.09 );
		}
		else
		{
			hud.GetHudPanelWidget().SetPos( 0, 0 );
		}
		}
		#endif
	}
	
	override void ShowInventory()
	{
		bool init = false;
		UIScriptedMenu menu = GetUIManager().GetMenu();
		
		if (menu == NULL && GetGame().GetPlayer().GetHumanInventory().CanOpenInventory() && !GetGame().GetPlayer().IsInventorySoftLocked() && GetGame().GetPlayer().GetHumanInventory().IsInventoryUnlocked() )
		{
			if (m_InventoryMenu == NULL)
			{
				m_InventoryMenu = InventoryMenu.Cast( GetUIManager().EnterScriptedMenu(MENU_INVENTORY, NULL) );
			}
			else if ( GetUIManager().FindMenu(MENU_INVENTORY) == NULL )
			{
				GetUIManager().ShowScriptedMenu(m_InventoryMenu, NULL);
			}
			MoveHudForInventory( true );
			init = true;
		}
		
		if (menu && menu == m_InventoryMenu)
		{
			init = true;
		}
		
		if (m_InventoryMenu && init)
		{
			PlayerBase player = PlayerBase.Cast( GetGame().GetPlayer() );
			vector rayStart = GetGame().GetCurrentCameraPosition();
			vector rayEnd = rayStart + GetGame().GetCurrentCameraDirection() * 1.5;
			vector hitPos;
			vector hitNormal;
			int hitComponentIndex;
			ref set<Object> hitObjects = new set<Object>;
			DayZPhysics.RaycastRV(rayStart, rayEnd, hitPos, hitNormal, hitComponentIndex, hitObjects, NULL, player);
			
			Object target = NULL;
			if( hitObjects.Count() )
				target = hitObjects.Get(0);
				
			m_InventoryMenu.InitContainers(EntityAI.Cast(target));
			
			PlayerControlDisable();		
		}
	}
	
	override void HideInventory()
	{
		InventoryMenu inventory = InventoryMenu.Cast( GetUIManager().FindMenu(MENU_INVENTORY) );
		if (inventory)
		{
			GetUIManager().HideScriptedMenu(inventory);
			MoveHudForInventory( false );
			PlayerControlEnable();	
		}	
	}
	
	void DestroyInventory()
	{
		if (m_InventoryMenu)
		{
			m_InventoryMenu.Close();
			m_InventoryMenu = NULL;
		}
		if (m_InventoryMenu)
		{
			m_InventoryMenu.Close();
			m_InventoryMenu = NULL;
		}
	}
	
	override void ShowChat()
	{
		m_ChatChannelHideTimer.Stop();
		m_ChatChannelFadeTimer.Stop();
		m_ChatChannelArea.Show(false);
		m_UIManager.EnterScriptedMenu(MENU_CHAT_INPUT, NULL);
		
		PlayerControlDisable();		
	}

	override void HideChat()
	{
		PlayerControlEnable();
	}
	
	void ShowVehicleInfo()
	{
		if( GetHud() )
			GetHud().ShowVehicleInfo();
	}
	
	void HideVehicleInfo()
	{
		if( GetHud() )
			GetHud().HideVehicleInfo();
	}
	
	override Hud GetHud()
	{
		return m_Hud;
	}
	
	HudDebug GetHudDebug()
	{
		return m_HudDebug;
	}
	
	override void RefreshCrosshairVisibility()
	{
		GetHudDebug().RefreshCrosshairVisibility();
	}
	
	override void HideCrosshairVisibility()
	{
		GetHudDebug().HideCrosshairVisibility();
	}
	
	override bool IsPaused()
	{
		return GetGame().GetUIManager().IsMenuOpen(MENU_INGAME);
	}
	
	override void Pause()
	{
		if ( IsPaused() )
		{
			return;
		} 
		
		CloseAllMenus();
		
		// open ingame menu
		GetUIManager().EnterScriptedMenu( MENU_INGAME, GetGame().GetUIManager().GetMenu() );
	}
	
	override void Continue()
	{
		if ( !IsPaused() )
		{
			return;
		}

		PlayerControlEnable();
		GetUIManager().CloseMenu(MENU_INGAME);
	}
	
	override bool IsMissionGameplay()
	{
		return true;
	}
	
	override void AbortMission()
	{
		#ifdef BULDOZER
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).Call(g_Game.RequestExit, IDC_MAIN_QUIT);
		#else
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).Call(GetGame().AbortMission);
		#endif
	}
	
	override void CreateLogoutMenu(UIMenuPanel parent)
	{
		PlayerBase player = PlayerBase.Cast( GetGame().GetPlayer() );
		
		// do not show logout screen if player's dead
		if (!player || player.IsDamageDestroyed())
		{
			// exit the game immediately
			AbortMission();
			return;
		}
		
		if (parent)
		{
			m_Logout = LogoutMenu.Cast(parent.EnterScriptedMenu(MENU_LOGOUT));
			
			if (m_Logout)
			{
				m_Logout.SetLogoutTime();
			}
		}		
	}
	
	override void StartLogoutMenu(int time)
	{
		if (m_Logout)
		{
			if (time > 0)
			{
				// character will be deleted from server int "time" seconds
				m_Logout.SetTime(time);
				m_Logout.Show();
				
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(m_Logout.UpdateTime, 1000, true);
			}
			else
			{
				// no waiting time -> player is most likely dead
				m_Logout.Exit();
			}
		}
	}
	
	override void CreateDebugMonitor()
	{
		if (!m_DebugMonitor)
		{
			m_DebugMonitor = new DebugMonitor();
			m_DebugMonitor.Init();
		}
	}
	
	void UpdateDebugMonitor()
	{
		if (!m_DebugMonitor) return;
		
		PlayerBase player = PlayerBase.Cast( GetGame().GetPlayer() );
		if (player)
		{
			DebugMonitorValues values = player.GetDebugMonitorValues();
			if (values)
			{
				m_DebugMonitor.SetHealth(values.GetHealth());
				m_DebugMonitor.SetBlood(values.GetBlood());
				m_DebugMonitor.SetLastDamage(values.GetLastDamage());
				m_DebugMonitor.SetPosition(player.GetPosition());
			}
		}
	}
	
	void SetActionDownTime( int time_down )
	{
		m_ActionDownTime = time_down;
	}
	
	void SetActionUpTime( int time_up )
	{
		m_ActionUpTime = time_up;
	}
	
	int GetActionDownTime()
	{
		return m_ActionDownTime;
	}
	
	int GetActionUpTime()
	{
		return m_ActionUpTime;
	}
	
	float GetHoldActionTime()
	{
		float hold_action_time = GetActionUpTime() - GetActionDownTime();
		return hold_action_time;
	}
}
