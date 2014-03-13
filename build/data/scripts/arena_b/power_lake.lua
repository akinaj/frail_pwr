 -- Script for power lake. Author: Sebastian Łasisz

last_healing_times = {}
heal_period = 2.0
healing_per_second = 10

function onTouched(touched_object)	
	if IsDerivedOrExactClass(touched_object:GetObject(), "Character") then		
		name = GetObjectName(this)
		name2 = GetObjectName(touched_object:GetObject())
		bilboard = FindObject("bilb"..name..name2)
		if not bilboard:IsValid() then
			bilboard = CreateObject(this, "ModelObject", "bilb_1")
			name = GetObjectName(this)
			SetFieldValue(bilboard:GetObject(), "Name", "bilb"..name..name2)					
			if IsDerivedOrExactClass(touched_object:GetObject(), "Player") then
				SetWorldPosition(bilboard:GetObject(), (GetWorldPosition(touched_object:GetObject())+mkVec3.new(0, 2.5 ,0)))		
			end
			if IsDerivedOrExactClass(touched_object:GetObject(), "ActorAI") then
				SetWorldPosition(bilboard:GetObject(), (GetWorldPosition(touched_object:GetObject())+mkVec3.new(0, 4.7 ,0)))	
			end
			done = true
		else
			if IsDerivedOrExactClass(touched_object:GetObject(), "Player") then
				SetWorldPosition(bilboard:GetObject(), (GetWorldPosition(touched_object:GetObject())+mkVec3.new(0, 2.5 ,0)))		
			end
			if IsDerivedOrExactClass(touched_object:GetObject(), "ActorAI") then
				SetWorldPosition(bilboard:GetObject(), (GetWorldPosition(touched_object:GetObject())+mkVec3.new(0, 4.7 ,0)))				
			end
		end		
	end
	
	last_heal_time = last_healing_times[GetObjectName(touched_object:GetObject())]
	if last_heal_time == nil or GetTime() - last_heal_time > heal_period then
		heal_value = healing_per_second / heal_period
		
		if IsDerivedOrExactClass(touched_object:GetObject(), "Character") then
			Character.AddHealth(touched_object:GetObject(), heal_value)
		end		
		last_healing_times[GetObjectName(touched_object:GetObject())] = GetTime()
	end
end

function onUpdate()
	objects_in_radius = FindObjectsInRadius(GetWorldPosition(this), 2)
	objects_in_radius2 = FindObjectsInRadius(GetWorldPosition(this), 4)
	delete = true
	obj = FindObject(GetObjectName(this))
	for i,v in ipairs(objects_in_radius2) do
		if string.sub(GetObjectName(v:GetObject()), 1, 13) == "bilbcvt_power" then
			bilb = FindObject(GetObjectName(v:GetObject()))
			obj = bilb
			for j,x in ipairs(objects_in_radius) do			
				if string.sub(GetObjectName(x:GetObject()), 1, 13) == "bilbcvt_power" then
					if (GetObjectName(x:GetObject())) == (GetObjectName(v:GetObject())) then
						delete = false
					end
				end
			end
		end
	end
		if obj:IsValid() and string.sub(GetObjectName(obj:GetObject()), 1, 13) == "bilbcvt_power"   and delete then
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
end