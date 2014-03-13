-- Script for coordinator. Author: Sebastian Łasisz

count_time = 0
ktrap = {}
ktrap_position = {}
damage = {}

get_dmg = true
done = true

dd_timer = 0
done_time_dd = false
dont_t = true

done1= true
done2=true
done3=true
done4=true

function onPostCreate()
	createKnockbackTrapsReferences()
	ktrap_position = { GetWorldPosition(ktrap[1]:GetObject()), GetWorldPosition(ktrap[2]:GetObject()), GetWorldPosition(ktrap[3]:GetObject()), GetWorldPosition(ktrap[4]:GetObject()) }
end

function onUpdate()
	createKnockbackTrapsReferences()
	createTraps()
	createObjects()
	dd_time()
	dd_controller()
	
	if (math.floor(GetTime() )%27 ==0) then
		done1 = true
		done2=true
		done3=true
		done4=true
	end
end

function createKnockbackTrapsReferences()
	ktrap = { FindObject("cvt_ktrap_1"), FindObject("cvt_ktrap_2"), FindObject("cvt_ktrap_3"), FindObject("cvt_ktrap_4") }
end

function createTraps()
	if not ktrap[1]:IsValid() and (math.floor(GetTime()) % 20 == 0) then
		ktrap[1] = CreateObject(this, "CylinderVolumeTrigger", "knockback_trap_r2")
		SetFieldValue(ktrap[1]:GetObject(), "Name", "cvt_ktrap_1")
		SetWorldPosition(ktrap[1]:GetObject(), ktrap_position[1])
	end
	if not ktrap[2]:IsValid() and (math.floor(GetTime()) % 20 == 0) then
		ktrap[2] = CreateObject(this, "CylinderVolumeTrigger", "knockback_trap_r2")
		SetFieldValue(ktrap[2]:GetObject(), "Name", "cvt_ktrap_2")
		SetWorldPosition(ktrap[2]:GetObject(), ktrap_position[2])
	end
	if not ktrap[3]:IsValid() and (math.floor(GetTime()) % 20 == 0) then
		ktrap[3] = CreateObject(this, "CylinderVolumeTrigger", "knockback_trap_r2")
		SetFieldValue(ktrap[3]:GetObject(), "Name", "cvt_ktrap_3")
		SetWorldPosition(ktrap[3]:GetObject(), ktrap_position[3])
	end
	if not ktrap[4]:IsValid() and (math.floor(GetTime()) % 20 == 0) then
		ktrap[4] = CreateObject(this, "CylinderVolumeTrigger", "knockback_trap_r2")
		SetFieldValue(ktrap[4]:GetObject(), "Name", "cvt_ktrap_1")
		SetWorldPosition(ktrap[4]:GetObject(), ktrap_position[4])
	end
	if not (FindObject("cvt_ctrap_1"):IsValid()) and (math.ceil(GetTime()) % 20 ==0) and done1 then
		trap = CreateObject(this, "CylinderVolumeTrigger", "claymore_trap_r2")
		SetFieldValue(trap:GetObject(), "Name", "cvt_ctrap_1")
		SetWorldPosition(trap:GetObject(), mkVec3.new(-11.0, 8.0, 11.0))
		done1 = false
	end
	if not (FindObject("cvt_ctrap_2"):IsValid()) and (math.ceil(GetTime()) % 20 ==0) and done2 then
		trap = CreateObject(this, "CylinderVolumeTrigger", "claymore_trap_r2")
		SetFieldValue(trap:GetObject(), "Name", "cvt_ctrap_2")
		SetWorldPosition(trap:GetObject(), mkVec3.new(-11.0, 8.0, -11.0))
		done2 = false
	end
	if not (FindObject("cvt_ctrap_3"):IsValid()) and (math.ceil(GetTime()) % 20 ==0) and done3 then
		trap = CreateObject(this, "CylinderVolumeTrigger", "claymore_trap_r2")
		SetFieldValue(trap:GetObject(), "Name", "cvt_ctrap_3")
		SetWorldPosition(trap:GetObject(), mkVec3.new(11.0, 8.0, 11.0))
		done3 = false
	end
	if not (FindObject("cvt_ctrap_4"):IsValid()) and (math.ceil(GetTime()) % 20 ==0) and done4 then
		trap = CreateObject(this, "CylinderVolumeTrigger", "claymore_trap_r2")
		SetFieldValue(trap:GetObject(), "Name", "cvt_ctrap_4")
		SetWorldPosition(trap:GetObject(), mkVec3.new(11.0, 8.0, -11.0))
		done4 = false
	end
 end

 function createObjects()
 	hp = FindObject("cvt_hp")
 	if not hp:IsValid() and (math.floor(GetTime()%62) == 0) then
 		hp = CreateObject(this, "CylinderVolumeTrigger", "healing_bandage")
		SetFieldValue(hp:GetObject(), "Name", "cvt_hp")
		SetWorldPosition(hp:GetObject(), mkVec3.new(0.0, 8.0, 12.0))
	end
 	dd = FindObject("cvt_dd")
 	if not dd:IsValid() and (math.floor(GetTime()%62)==0) then
 		dd = CreateObject(this, "CylinderVolumeTrigger", "double_damage")
		SetFieldValue(dd:GetObject(), "Name", "cvt_dd")
		SetWorldPosition(dd:GetObject(), mkVec3.new(0.0, 8.0, -12.0))
	end
	 dd1 = FindObject("cvt_dd1")
 	if not dd1:IsValid() and (math.floor(GetTime()%62)==0) then
 		dd1 = CreateObject(this, "CylinderVolumeTrigger", "double_damage")
		SetFieldValue(dd:GetObject(), "Name", "cvt_dd1")
		SetWorldPosition(dd:GetObject(), mkVec3.new(8.0, 10.0, -56.0))
	end
 end
 
 function dd_time()
	dd = FindObject("cvt_dd")
	if dd:IsValid() then 
		done_time_dd = false
	end
	if not dd:IsValid() and not done_time_dd then
		dd_timer = GetTime()
		done_time_dd = true
	end
 end
 
 function dd_controller()
	dd = FindObject("cvt_dd")
	bilboard = FindObject("bilbcvt_dd")
	if bilboard:IsValid() and done_time_dd then
		pos = GetWorldPosition(bilboard:GetObject())
		objects_in_radius = FindObjectsInRadius(pos, 8)
		for i,v in ipairs(objects_in_radius) do
			if IsDerivedOrExactClass(v:GetObject(), "Player") then
				pos_act = GetWorldPosition(v:GetObject())
				SetWorldPosition(bilboard:GetObject(), (pos_act + mkVec3.new(0, 2.1, 0.0)))
				if (dd_timer + 10 < GetTime()) and dont_t then
					dont_t = false
					SetFieldValue(v:GetObject(), "m_shootingDamage", GetFieldValue(v:GetObject(), "m_shootingDamage") / 2)	
					DestroyObject(bilboard:GetObject())						
				end
			end
		end
	else
		dont_t = true
	end
 end