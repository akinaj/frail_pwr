-- Example script for AI jumping if it sees another actor around

TIME_BTW_JUMPS = 1.0

-- Global variables in script are instantiated for each AI actor, so they can be seen as members of ActorAI class
-- Note that actors can't access other actors' global variables
life_start_time = -1.0
last_jump_time = -1.0

-- This function is called once per AI actor lifetime, when it is created
-- It should be used for initializing stuff, finding references to other objects, etc.
function onPostCreate()
    life_start_time = GetTime()
end

function shouldJump()
    spotted_actors = ActorAI.GetSpottedActors(this)
    return (#spotted_actors > 0)
end

-- This function is called once per simulation frame
function onUpdate()    
    if shouldJump() then
        -- We don't want our actor to move anywhere while jumping
        Character.SetSpeed(this, 0.0)
    
        time_since_last_jump = GetTime() - last_jump_time
        if time_since_last_jump > TIME_BTW_JUMPS then
            Character.Jump(this)
            last_jump_time = GetTime()
        end
    else
        Character.SetSpeed(this, 0.1)
        Character.SetDirection(this, mkVec3.new(0, 0, 1))
    end
end

-- This function is called once per rendering frame. Use it to draw additional info about this actor
function onDebugDraw()
    ActorAI.DrawSensesInfo(this)
end