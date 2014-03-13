DEBUG = false

DMG_BULLET = 1
DMG_FIRE = 2
DMG_POISON = 3
DMG_PUNCH = 4
DMG_COLLISION = 5

damage_mul = 1.0

function onTakeDamage(dmg_type, dmg_value)
	-- Jesli beczka wybucha w wyniku wybuchniecia innej, pobliskiej beczki, to ta beczka zadaje 2x wiecej damage'u
	if dmg_type == DMG_FIRE then
		damage_mul = 2.0
	end
	
	if dmg_type == DMG_FIRE or dmg_type == DMG_BULLET then
		Explode(this)
	end
end

-- function GiveDamage(from, to, dmg_type, dmg_value, dmg_direction, dmg_position)

function onExploded()
	dmg_radius = GetFieldValue(this, "m_damageRadius")
	dmg_type = GetFieldValue(this, "m_damageType")
	dmg_value = GetFieldValue(this, "m_damage") * damage_mul
	
	if DEBUG then LogError("exploding in radius " .. tostring(dmg_radius) .. "!") end
	
	objects_in_radius = FindObjectsInRadius(GetWorldPosition(this), dmg_radius)
	
	if DEBUG then msg = "Found " .. tostring(#objects_in_radius) .. " objects to damage:"	end
	
	for i,v in ipairs(objects_in_radius) do
		if DEBUG then msg = msg .. "\n - " .. GetObjectName(v:GetObject()) end
		
		dmg_direction = (GetWorldPosition(v:GetObject()) - GetWorldPosition(this)):normalized()
		dmg_position = GetWorldPosition(v:GetObject())
		
		damaged_object = v
		GiveDamage(this, damaged_object, dmg_type, dmg_value, dmg_direction, dmg_position)
	end
	
	if DEBUG then LogError(msg) end
end