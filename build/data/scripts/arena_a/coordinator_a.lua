 -- Script for coordinator. Author: Sebastian Łasisz

respawn_time = 30
explosion_time = {0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
barrels = {}
done_speed = false
barrels_name = {"first_forest_top_barrel", "second_forest_top_barrel", "third_forest_top_barrel", "first_forest_bot_barrel", "second_forest_bot_barrel", 
				"third_forest_bot_barrel", "first_left_barrel_base", "second_left_barrel_base", "second_right_barrel_base",
				"b_1_1","b_1_2", "b_1_3", "b_1_4", "b_1_5", "b_1_6", "b_2_1", "b_2_2", "b_2_3", "b_2_4",
				"bod_01", "bod_02","bod_03","bod_04","bod_05","bod_06","bod_07","bod_08",
				"beam_01", "beam_02", "beam_03", "beam_04", "beam_05"}
barrels_type = {"barrel","barrel","barrel","barrel","barrel", "barrel","barrel","barrel","barrel",
				"barrel","barrel","barrel","barrel","barrel","barrel","barrel","barrel","barrel","barrel",
				"barrel_of_destruction","barrel_of_destruction","barrel_of_destruction","barrel_of_destruction","barrel_of_destruction","barrel_of_destruction","barrel_of_destruction","barrel_of_destruction",
				"beam_of_destruction", "beam_of_destruction", "beam_of_destruction", "beam_of_destruction", "beam_of_destruction"}
barrels_position = {}
barrels_radius = {}
hp2_spawned = false
hp_spawned = false
size = 0
done = { }
done_time = {}
barrels_time =  {0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

dd_timer = 0
done_time_dd = false
dont_t = true

function onPostCreate()
	hp = FindObject("healing_bandage")
	hp2 = FindObject("healing_bandage_2")
	size = table.getn(barrels_name) + 1
	createReferances(size)
	lava = FindObject("lava_lake_1")
	createBarrelsPosition(size)
	createBarrelsRadius(size)
	setDone(size)
	setDoneTime(size)
end


function onUpdate()
	createReferances(size)
	createBarrels(size)
	createObjects()
	destroyBillboard1()
	destroyBillboard2()	
	dd_time()
	dd_controller()
end

function getBarrelTime()
	a = 1
	repeat
		if (not barrels[a]:IsValid()) and done_time[a] then
			barrels_time[a] = GetTime()
			done_time[a] = false
		end
		if barrels[a]:IsValid() then
			done_time[a] = true
		end
		a = a +1;
	until a == size
end

function destroyBillboard1()
	getBarrelTime()
	j  = 20
	repeat
		bilbb = FindObject("bilb"..barrels_name[j])
		if bilbb:IsValid() then	
			objects_in_radius = FindObjectsInRadius(GetWorldPosition(bilbb:GetObject()), 4)
			for i,v in ipairs(objects_in_radius) do
				if IsDerivedOrExactClass(v:GetObject(), "Character") then
					pos = GetWorldPosition(v:GetObject())
					SetWorldPosition(bilbb:GetObject(), (pos + mkVec3.new(0, 2.1, 0.0)))
					if bilbb:IsValid() and (not barrels[j]:IsValid()) and (barrels_time[j] + 5 < GetTime()) then
							speed1 = GetFieldValue(v:GetObject(), "m_maxSpeed")
							SetFieldValue(v:GetObject(), "m_maxSpeed", speed1 * 5)
							DestroyObject(bilbb:GetObject())			
					end
				end
			end
		end
		j = j +1
	until j == 28	
end

function destroyBillboard2()
	getBarrelTime()
	j = 28
	repeat
		bb = FindObject("bilb"..barrels_name[j])
		if bb:IsValid() then 
			objects_in_radius = FindObjectsInRadius(GetWorldPosition(bb:GetObject()), barrels_radius[j])
			for i,v in ipairs(objects_in_radius) do
				if IsDerivedOrExactClass(v:GetObject(), "Character") then
					pos_act = GetWorldPosition(v:GetObject())
					SetWorldPosition(bb:GetObject(), (pos_act + mkVec3.new(0, 2.1, 0)))
					if (not barrels[j]:IsValid()) and (barrels_time[j] + 5 < GetTime()) then
						DestroyObject(bb:GetObject())			
					end
				end
			end
		end
	j = j+1
	until j == size
end

function createBarrelsRadius(k)
	i = 1
	repeat
		barrels_radius[i] = GetFieldValue(FindObject(barrels_name[i]):GetObject(),"m_damageRadius")
		i = i + 1
	until i == k
end

function createBarrelsPosition(k)
	i = 1
	repeat
		barrels_position[i] = GetWorldPosition(FindObject(barrels_name[i]):GetObject())
		i = i + 1
	until i == k
end

function setDone(k)
	i = 1
	repeat
		done[i] = true
		i = i + 1
	until i == k
end

function setDoneTime(k)
	i = 1
	repeat
		done_time[i] = true
		i = i + 1
	until i == k
end

function createBarrels(k)
	i = 1
	repeat 
		createBarrel(i)
		i = i + 1
	until i == k
end

function createBarrel(i)
	if not barrels[i]:IsValid()  then 
		if done[i] then 
			explosion_time[i] = GetTime()
			done[i] = false
		end
		if (explosion_time[i]+respawn_time)<GetTime() and not done[i] then
			if(barrels_type[i] == "barrel") then
				if barrels_radius[i] == 4 then
					barrel = CreateObject(this, "ExplodingObject", "barrel_e_r4")
				end
				if barrels_radius[i] == 2 then
					barrel = CreateObject(this, "ExplodingObject", "barrel_e_r2")
				end
				if barrels_radius[i] == 8 then
					barrel = CreateObject(this, "ExplodingObject", "barrel_e_r8")
				end
				SetFieldValue(barrel:GetObject(), "Name", barrels_name[i])
				SetWorldPosition(barrel:GetObject(), barrels_position[i])
				barrels[i] = FindObject(barrels_name[i])
				done[i] = true
			end
			if(barrels_type[i] == "beam_of_destruction") then
				if barrels_radius[i] == 4 then
					barrel = CreateObject(this, "ExplodingObject", "barrel_b_r4")
				end
				if barrels_radius[i] == 2 then
					barrel = CreateObject(this, "ExplodingObject", "barrel_b_r2")
				end
				if barrels_radius[i] == 8 then
					barrel = CreateObject(this, "ExplodingObject", "barrel_b_r8")
				end
				SetFieldValue(barrel:GetObject(), "Name", barrels_name[i])
				SetWorldPosition(barrel:GetObject(), barrels_position[i])
				barrels[i] = FindObject(barrels_name[i])
				done[i] = true
			end
			if(barrels_type[i] == "barrel_of_destruction") then
				if barrels_radius[i] == 4 then
					barrel = CreateObject(this, "ExplodingObject", "barrel_s_r4")
				end
				if barrels_radius[i] == 2 then
					barrel = CreateObject(this, "ExplodingObject", "barrel_s_r2")
				end
				if barrels_radius[i] == 8 then
					barrel = CreateObject(this, "ExplodingObject", "barrel_s_r8")
				end
				SetFieldValue(barrel:GetObject(), "Name", barrels_name[i])
				SetWorldPosition(barrel:GetObject(), barrels_position[i])
				barrels[i] = FindObject(barrels_name[i])
				done[i] = true
			end
		end
	end	
end

function createObjects()
	if not barrels[1]:IsValid() and not barrels[2]:IsValid() and not barrels[3]:IsValid() and not hp2:IsValid() and not hp2_spawned then
		healing = CreateObject(this, "CylinderVolumeTrigger", "double_damage")
		SetFieldValue(healing:GetObject(), "Name", "cvt_dd")
		SetWorldPosition(healing:GetObject(), mkVec3.new(1.0, 0.0, -29.0))
		hp2 = FindObject("cvt_dd")
		hp2_spawned = done
	end
	if barrels[1]:IsValid() and barrels[2]:IsValid() and barrels[3]:IsValid() then hp2_spawned = false end
	
	if not barrels[4]:IsValid() and not barrels[5]:IsValid() and not barrels[6]:IsValid() and not hp:IsValid() and not hp_spawned then
		healing = CreateObject(this, "CylinderVolumeTrigger", "healing_bandage")	
		SetFieldValue(healing:GetObject(), "Name", "healing_bandage_1")
		SetWorldPosition(healing:GetObject(), mkVec3.new(33.0, 0.0, 6.0))
		hp = FindObject("healing_bandage_1")	
		hp_spawned = done
	end
	if barrels[4]:IsValid() and barrels[5]:IsValid() and barrels[6]:IsValid() then hp_spawned = false end

	if barrels[8]:IsValid() and barrels[9]:IsValid() then
		lava = FindObject("lava_mid_lava_1")
		if not lava:IsValid() then
			CreateObject(this,"CylinderVolumeTrigger", "lava_lake_1")
			lava = FindObject("lava_mid_lava_1")
		end
	end
end

function createReferances(k)
	i = 1
	repeat
		barrels[i] = FindObject(barrels_name[i])
		i = i+1
	until i == k
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
		objects_in_radius = FindObjectsInRadius(pos, 1000)
		for i,v in ipairs(objects_in_radius) do
			if IsDerivedOrExactClass(v:GetObject(), "Character") then
				pos_act = GetWorldPosition(v:GetObject())
				SetWorldPosition(bilboard:GetObject(), (pos_act + mkVec3.new(0.0, 2.1, 0.0)))
				if (dd_timer + 10 < GetTime()) and dont_t then
					dont_t = false
					SetFieldValue(v:GetObject(), "m_shootingDamage", GetFieldValue(v:GetObject(), "m_shootingDamage") / 2)	
					DestroyObject(bilboard:GetObject())						
				end
			end
		end
	else
		dont_t = true
		done_time_dd = false
	end
 end