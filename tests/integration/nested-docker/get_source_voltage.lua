-- Read a source voltage in V. ISS supplies a native InstrumentTarget getter.
local function Get_Source_Voltage(ctx, getter)
	local command = instrument_call_stack.new({
		instrument = getter:get_instrument_name(),
		group = getter:get_channel_group(),
		channel = getter:get_channel(),
		command = "GET_VOLTAGE",
	})
	-- ISS records the numeric ISA response for the hub's settings decoder.
	return ctx:call(command)
end

return { main = Get_Source_Voltage }
