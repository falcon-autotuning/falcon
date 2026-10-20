-- Read the meter's actual bin count; no numberOfSamples argument is needed.
local function Get_Number_Of_Samples(ctx, getter)
	local command = instrument_call_stack.new({
		instrument = getter:get_instrument_name(),
		group = getter:get_channel_group(),
		channel = getter:get_channel(),
		command = "GET_BINS",
	})
	return ctx:call(command)
end

return { main = Get_Number_Of_Samples }
