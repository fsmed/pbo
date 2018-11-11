class V3S_Chassis extends CarScript
{
	override void Init()
	{
		super.Init();
		m_dmgContactCoef = 0.018;
		m_enginePtcPos = "0 1.346 2.205";
	}
	
	override int GetAnimInstance()
	{
		return VehicleAnimInstances.V3S;
	}

	override int GetSeatAnimationType( int posIdx )
	{
		switch( posIdx )
		{
		case 0:
			return DayZPlayerConstants.VEHICLESEAT_DRIVER;
		case 1:
			return DayZPlayerConstants.VEHICLESEAT_CODRIVER;
		}

		return 0;
	}

	override void EEHealthLevelChanged(int oldLevel, int newLevel, string zone)
	{
		Print( zone );
	}

	override bool CrewCanGetThrough( int posIdx )
	{
		CarDoor carDoor;
		switch( posIdx )
		{
			case 0:
				if ( GetCarDoorsState( "V3SDriverDoors" ) == CarDoorState.DOORS_CLOSED )
				{
					return false;
				}
			
				return true;
			break;
			
			case 1:
				if ( GetCarDoorsState( "V3SCoDriverDoors" ) == CarDoorState.DOORS_CLOSED )
				{
					return false;
				}
			
				return true;
			break;
		}

		return false;
	}
	
	override float OnSound( CarSoundCtrl ctrl, float oldValue )
	{
		switch ( ctrl )
		{
			case CarSoundCtrl.DOORS:
				float newValue = 0;

				//-----
				CarDoor carDoor;
			 	Class.CastTo( carDoor, FindAttachmentBySlotName("V3SDriverDoors") );
				if ( carDoor )
				{	
					if ( carDoor.GetAnimationPhase("DoorsSource") > 0.5)
					{
						newValue = newValue + 0.8;
					}
				}
				else
				{
					newValue = newValue + 0.8;
				}

				//-----
			 	Class.CastTo( carDoor, FindAttachmentBySlotName("V3SCoDriverDoors") );
				if ( carDoor )
				{	
					if ( carDoor.GetAnimationPhase("DoorsSource") > 0.5)
					{
						newValue = newValue + 0.8;
					}
				}
				else
				{
					newValue = newValue + 0.8;
				}

				if ( newValue > 1 )
					newValue = 1;
			
				return newValue;
			break;
		}

		return oldValue;
	}
	
	override int GetCarDoorsState( string slotType )
	{
		CarDoor carDoor;
		
		switch( slotType )
		{
			case "V3SDriverDoors":
				Class.CastTo( carDoor, FindAttachmentBySlotName( slotType ) );
				if ( carDoor )
				{
					if ( GetAnimationPhase("DoorsDriver") > 0.5 )
					{
						return CarDoorState.DOORS_OPEN;
					}
					else
					{
						return CarDoorState.DOORS_CLOSED;
					}
				}

				return CarDoorState.DOORS_MISSING;
			break;
			
			case "V3SCoDriverDoors":
				Class.CastTo( carDoor, FindAttachmentBySlotName( slotType ) );
				if ( carDoor )
				{
					if ( GetAnimationPhase("DoorsCoDriver") > 0.5 )
					{
						return CarDoorState.DOORS_OPEN;
					}
					else
					{
						return CarDoorState.DOORS_CLOSED;
					}
				}

				return CarDoorState.DOORS_MISSING;
			break;
		}
		
		return CarDoorState.DOORS_MISSING;
	}

	override string GetAnimSourceFromSelection( string selection )
	{
		switch( selection )
		{
		case "doors_driver":
			return "DoorsDriver";
		case "doors_codriver":
			return "DoorsCoDriver";
		case "doors_hood":
			return "DoorsHood";
		case "doors_trunk":
			return "DoorsTrunk";			
		case "wheelsideplate1":
			return "WheelSidePlate1";
		case "wheelsideplate2":
			return "WheelSidePlate2";
		}

		return "";
	}

	override bool IsVitalCarBattery()
	{
		return false;
	}

	override bool IsVitalSparkPlug()
	{
		return false;
	}

	override bool IsVitalGlowPlug()
	{
		return false;
	}

	override bool IsVitalEngineBelt()
	{
		return false;
	}
	
	override bool IsVitalRadiator()
	{
		return false;
	}
}