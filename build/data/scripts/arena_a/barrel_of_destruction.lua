 -- Script for barrel of destruction. Author: Sebastian Łasisz

RIBBON = nil 
DMG_BULLET = 1
DMG_FIRE = 2
DMG_POISON = 3
DMG_PUNCH = 4
DMG_COLLISION = 5


explosion_delay = 0.25
explosion_time = 0
damage_mul = 1.0
explode = false

function onTakeDamage(dmg_type, dmg_value)
	if dmg_type == DMG_FIRE then
		damage_mul = 2.0
	end
	if dmg_type == DMG_FIRE or dmg_type == DMG_BULLET or dmg_type == DMG_PUNCH then
		explosion_time = GetTime()
		explode = true
	end
end

function onPostCreate()
	dmg_radius = GetFieldValue(this, "m_damageRadius")
	if dmg_radius == 4 then
		RIBBON = CreateObject(this, "ModelObject", "barrel_ribbon")
	end
	if dmg_radius == 2 then
		RIBBON = CreateObject(this, "ModelObject", "barrel_ribbon_2")
	end
	if dmg_radius == 8 then
		RIBBON = CreateObject(this, "ModelObject", "barrel_ribbon_8")
	end
	SetFieldValue(RIBBON:GetObject(), "Name", GetObjectName(this).."_rib")
	SetWorldPosition(RIBBON:GetObject(), GetWorldPosition(this))
end

function onExploded()
	dmg_radius = GetFieldValue(this, "m_damageRadius")
	dmg_type = GetFieldValue(this, "m_damageType")
	dmg_value = GetFieldValue(this, "m_damage") * damage_mul
	
	objects_in_radius = FindObjectsInRadius(GetWorldPosition(this), dmg_radius)
	
	for i,v in ipairs(objects_in_radius) do
		dmg_direction = (GetWorldPosition(v:GetObject()) - GetWorldPosition(this)):normalized()
		dmg_position = GetWorldPosition(v:GetObject())
		damaged_object = v
		if IsDerivedOrExactClass(v:GetObject(), "Character") then
			speed = GetFieldValue(v:GetObject(), "m_maxSpeed")
			SetFieldValue(v:GetObject(), "m_maxSpeed", speed / 5)
		end
		GiveDamage(this, damaged_object, dmg_type, dmg_value, dmg_direction, dmg_position)
	end
end

function onUpdate()
	SetWorldPosition(RIBBON:GetObject(), GetWorldPosition(this))
	if explode and (explosion_time + explosion_delay < GetTime()) then Explode(this) end
end

function onDestroy()
	dmg_radius = GetFieldValue(this, "m_damageRadius")
	objects_in_radius = FindObjectsInRadius(GetWorldPosition(this),dmg_radius)
	for i,v in ipairs(objects_in_radius) do
		if IsDerivedOrExactClass(v:GetObject(), "Character") then
			name = GetObjectName(this)
			name2 = GetObjectName(v:GetObject())
			bilboard = FindObject("bilb"..name)
			if not bilboard:IsValid() then
				bilboard = CreateObject(this, "ModelObject", "bilb_5")
				SetFieldValue(bilboard:GetObject(), "Name", "bilb"..name)			
				if IsDerivedOrExactClass(v:GetObject(), "Player") then
					SetWorldPosition(bilboard:GetObject(), (GetWorldPosition(v:GetObject())+mkVec3.new(0, 2.1 ,0)))		
				end
				if IsDerivedOrExactClass(v:GetObject(), "ActorAI") then
					SetWorldPosition(bilboard:GetObject(), (GetWorldPosition(v:GetObject())+mkVec3.new(0, 2.1 ,0)))		
				end
			end
		end
	end
	DestroyObject(RIBBON:GetObject())
end