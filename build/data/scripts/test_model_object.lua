some_global_var = { a = 2 }

-- Consider setting object's field values through RTTI? Can be done rather easly

function onPostCreate()
   --logError("trolololo test" .. 3) 
   logError("This ma nazwe '" .. GetObjectName(this) .. "', obecny czas to " .. GetTimeMs(this) * 0.001 .. "s, ostatnia delta: " .. GetTimeDelta(this))
   
	a = mkVec3.new(1, 2, 3)
	b = mkVec3.new(6.1, 4.2, 3.3)
	
	LogError("a = " .. tostring(a) .. ", b = " .. tostring(b) .. ", a + b = " .. tostring(a + b) .. "dot = " .. mkVec3.dot(a, b) .. ", cross = " .. tostring(mkVec3.cross(a, b)) .. ",\ndist(a, b) = " .. mkVec3.dist(a, b) .. ", len(a) = " .. mkVec3.length(a) .. ", len(norm(a)) = " .. mkVec3.length(mkVec3.normalized(a)))
end

function onDestroy()
	--logError("This ma nazwe '" .. getObjectName(this) .. "', obecny czas to " .. getTimeMs(this) * 0.001 .. "s, ostatnia delta: " .. getTimeDelta(this))
end
