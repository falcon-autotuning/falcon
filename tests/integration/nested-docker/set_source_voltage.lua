-- Set a source voltage in V. ISS supplies a native InstrumentTarget setter.
local function Set_Source_Voltage(ctx, setter, setVoltage)
	local command = instrument_call_stack.new({
		instrument = setter:get_instrument_name(),
		group = setter:get_channel_group(),
		channel = setter:get_channel(),
		command = "SET_VOLTAGE",
	})
	return ctx:call(command, setVoltage)
end

return { main = Set_Source_Voltage }
