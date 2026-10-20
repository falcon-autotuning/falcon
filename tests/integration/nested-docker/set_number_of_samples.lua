-- Set the meter bin count. ISS supplies a native InstrumentTarget setter.
-- The hub sends this target as "setter"; older Teal called it "getter".
local function Set_Number_Of_Samples(ctx, setter, numberOfSamples)
	local count = math.tointeger(numberOfSamples)
	assert(count and count > 0, "numberOfSamples must be a positive integer")
	local command = instrument_call_stack.new({
		instrument = setter:get_instrument_name(),
		group = setter:get_channel_group(),
		channel = setter:get_channel(),
		command = "SET_BINS",
	})
	return ctx:call(command, count)
end

return { main = Set_Number_Of_Samples }
