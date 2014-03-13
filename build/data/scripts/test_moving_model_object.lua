original_pos = nil

function onPostCreate()
	original_pos = GetWorldPosition(this)
end

function onUpdate()
	time_s = GetTimeMs(this) * 0.001
	att = 5.0
	new_pos = original_pos + att * mkVec3.new(math.cos(time_s), math.sin(time_s), 0.2 * math.cos(time_s))
	SetWorldPosition(this, new_pos)
end

function lolTest()
    LogError("lol hiehie udalo sie!")
end