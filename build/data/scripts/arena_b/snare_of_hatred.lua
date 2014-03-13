 -- Script for snares of hatred. Author: Sebastian Łasisz

DEBUG = false

SHOWN = true
HIDING = false
HIDDEN = false
SHOWING = false

DMG_POISON = 3
time = 0


done_speed = false 

name = ""

last_damage_times = {}
dmg_period = 2.0
dps = 5.0
speed = {}
jump = {}
bilboard = {}
done = {}
radius = GetFieldValue(this, "m_radius")

function onTouched(touched_object)	
	if not SHOWN then return end
	last_dmg_time = last_damage_times[GetObjectName(touched_object:GetObject())]
	if last_dmg_time == nil or GetTime() - last_dmg_time > dmg_period then
		dmg_value = dps / dmg_period
		GiveDamage(this, touched_object, DMG_POISON, dmg_value, mkVec3.new(), mkVec3.new())
		last_damage_times[GetObjectName(touched_object:GetObject())] = GetTime()
	end
end

function onUpdate()
	if not done_speed then
		objects_in_radius = FindObjectsInRadius(GetWorldPosition(this), 100)
		for i,v in ipairs(objects_in_radius) do
			if IsDerivedOrExactClass(v:GetObject(), "Character") then
				speed[i] = GetFieldValue(v:GetObject(), "m_maxSpeed")	
				jump[i] = GetFieldValue(v:GetObject(), "m_canJump")
				done[i] = false
			end
		end
		done_speed = true
	end	

	objects_in_radius = FindObjectsInRadius(GetWorldPosition(this), radius)
	for i,v in ipairs(objects_in_radius) do
		if IsDerivedOrExactClass(v:GetObject(), "Character") then
			if not (GetFieldValue(v:GetObject(), "m_maxSpeed") == 0.0) then
				speed[i] = GetFieldValue(v:GetObject(), "m_maxSpeed")
				jump[i] = GetFieldValue(v:GetObject(), "m_canJump")
			end
			if SHOWN then
				SetFieldValue(v:GetObject(), "m_maxSpeed", 0.0)
				SetFieldValue(v:GetObject(), "m_canJump", false)
				if not done[i] then
					name = GetObjectName(this)
					bilboard[i] = CreateObject(this, "ModelObject", "bilb_6")
					SetFieldValue(bilboard[i]:GetObject(), "Name", "bilb"..i..name)
					pos = GetWorldPosition(v:GetObject())
					SetWorldPosition(bilboard[i]:GetObject(), pos + mkVec3.new(0,2.5,0))
					done[i] = true
				end
			end
			if not SHOWN then
				SetFieldValue(v:GetObject(), "m_maxSpeed", speed[i])
				if jump[i] == true then
					SetFieldValue(v:GetObject(), "m_canJump", true)
					bilboard[i] = FindObject("bilb"..i..name)
					if bilboard[i]:IsValid() then
						done[i] = false
						DestroyObject(bilboard[i]:GetObject())
					end
				end
				
			end				
		end
	end

	time = time + GetTimeDelta()
	pos = GetWorldPosition(this)	

	if SHOWN then
		if time > 4.0 then 
			SHOWN = false
			HIDING = true
			stop_time = false
			time = 0.0
		end
	end
	if HIDING then
		if time <= 2.0 then
			SetWorldPosition(this, pos - mkVec3.new(0, 0.95 * GetTimeDelta(), 0))
		else
			HIDING = false
			HIDDEN = true
			stop_time = false
			time = 0.0
		end
	end
	if HIDDEN then
		if time > 4.0 then
			HIDDEN = false
			SHOWING = true
			stop_time = false
			time = 0.0
		end
	end
	if SHOWING then
		if time <= 2.0 then
			SetWorldPosition(this, pos + mkVec3.new(0, 0.95 * GetTimeDelta(), 0))
		else
			SHOWING = false
			SHOWN = true
			stop_time = false
			time = 0.0
		end
	end
end