 -- Script for lava lake. Author: Sebastian Łasisz

DMG_POISON = 3

explosion_time = 0
increased_damage_time = 30

check_time = true
done = true;
explosion_time = 0
last_damage_times = {}
dmg_period = 2.0
dps = 10
barrel1 = nil
barrel2 = nil
barrel3 = nil

function onPostCreate()
	barrel1 = FindObject("second_left_barrel_base")
	barrel2 = FindObject("second_right_barrel_base")
	barrel3 = FindObject("first_mid_barrel_base")
end

function onTouched(touched_object)
	if IsDerivedOrExactClass(touched_object:GetObject(), "Character") then		
		name = GetObjectName(this)
		name2 = GetObjectName(touched_object:GetObject())
		bilboard = FindObject("bilb"..name..name2)
		if not bilboard:IsValid() then
			bilboard = CreateObject(this, "ModelObject", "bilb_3")
			name = GetObjectName(this)
			SetFieldValue(bilboard:GetObject(), "Name", "bilb"..name..name2)		
			if IsDerivedOrExactClass(touched_object:GetObject(), "Player") then
				SetWorldPosition(bilboard:GetObject(), (GetWorldPosition(touched_object:GetObject())+mkVec3.new(0,2.5,0)))
			end
			if IsDerivedOrExactClass(touched_object:GetObject(), "ActorAI") then
				SetWorldPosition(bilboard:GetObject(), (GetWorldPosition(touched_object:GetObject())+mkVec3.new(0,5.5,0)))
			end
			done = true
		else
			if IsDerivedOrExactClass(touched_object:GetObject(), "Player") then
				SetWorldPosition(bilboard:GetObject(), (GetWorldPosition(touched_object:GetObject())+mkVec3.new(0,2.5,0)))
			end
			if IsDerivedOrExactClass(touched_object:GetObject(), "ActorAI") then
				SetWorldPosition(bilboard:GetObject(), (GetWorldPosition(touched_object:GetObject())+mkVec3.new(0,5.5,0)))
			end	
		end		
	end
	
	last_dmg_time = last_damage_times[GetObjectName(touched_object:GetObject())]
	if last_dmg_time == nil or GetTime() - last_dmg_time > dmg_period then
		if (explosion_time + increased_damage_time) <  GetTime() then 
			dps = 10  
			check_time = true 
		end
		dmg_value = dps / dmg_period
		GiveDamage(this, touched_object, DMG_POISON, dmg_value, mkVec3.new(), mkVec3.new())
		last_damage_times[GetObjectName(touched_object:GetObject())] = GetTime()
	end
end

function onUpdate()
	objects_in_radius = FindObjectsInRadius(GetWorldPosition(this), 4)
	objects_in_radius2 = FindObjectsInRadius(GetWorldPosition(this), 8)
	delete = true
	obj = FindObject(GetObjectName(this))
	for i,v in ipairs(objects_in_radius2) do
		if string.sub(GetObjectName(v:GetObject()), 1, 8) == "bilblava" then
			bilb = FindObject(GetObjectName(v:GetObject()))
			obj = bilb
			for j,x in ipairs(objects_in_radius) do			
				if string.sub(GetObjectName(x:GetObject()), 1, 8) == "bilblava" then
					if (GetObjectName(x:GetObject())) == (GetObjectName(v:GetObject())) then
						delete = false
					end
				end
			end
		end
	end
	if obj:IsValid() and string.sub(GetObjectName(obj:GetObject()), 1, 8) == "bilblava"   and delete then
		delete2 = true
		for k,z in ipairs(objects_in_radius) do	
			if IsDerivedOrExactClass(z:GetObject(), "Character") then		
				delete2 = false
			end
		end
		if delete2 then
			DestroyObject(obj:GetObject())
		end
	end
	
	if(GetObjectName(this) == "lava_mid_lava_1") then
		barrel1 = FindObject("second_left_barrel_base")
		barrel2 = FindObject("second_right_barrel_base")
		barrel3 = FindObject("first_mid_barrel_base")
		if not barrel1:IsValid() and not barrel2:IsValid() and not barrel3:IsValid() then
			DestroyObject(this)
		end
	end
	if(GetObjectName(this) == "lava_first_left_lava_base" or GetObjectName(this) == "lava_first_right_lava_base") then
		if not barrel1:IsValid() or not barrel2:IsValid() or not barrel3:IsValid() then
			dps = 20
			if check_time then
				explosion_time = GetTime()
				check_time = false
			end
		end
	end
end