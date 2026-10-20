-- Software-stepped 1D sweep: one MEASURE_STREAM buffer per step/getter.
-- Domain arrays are positional, in setter order, and contain native domains.
-- Sweep helpers; use require("sweep_math") to avoid Lua's built-in math module.
local function linspace(vmin, vmax, nSteps)
	assert(nSteps >= 1 and nSteps % 1 == 0, "step count must be a positive integer")
	local values = {}
	for i = 1, nSteps do
		local fraction = nSteps == 1 and 0 or (i - 1) / (nSteps - 1)
		values[i] = vmin + fraction * (vmax - vmin)
	end
	return values
end

local function build1DSweep(domains, nSteps)
	local fractions = linspace(0, 1, nSteps)
	local sweep = {}
	for i, fraction in ipairs(fractions) do
		local voltages = {}
		for k, domain in ipairs(domains) do
			voltages[k] = domain:get_min() + fraction * (domain:get_max() - domain:get_min())
		end
		sweep[i] = voltages
	end
	return sweep
end

local function applyVector(ctx, setters, voltages)
	assert(#setters == #voltages, "one voltage is required per setter")
	for i, setter in ipairs(setters) do
		local command = instrument_call_stack.new({
			instrument = setter:get_instrument_name(),
			group = setter:get_channel_group(),
			channel = setter:get_channel(),
			command = "SET_VOLTAGE",
		})
		ctx:call(command, voltages[i])
	end
end

local function readGetters(ctx, getters)
	for _, getter in ipairs(getters) do
		local command = instrument_call_stack.new({
			instrument = getter:get_instrument_name(),
			group = getter:get_channel_group(),
			channel = getter:get_channel(),
			command = "MEASURE_STREAM",
		})
		ctx:call(command)
	end
end

local function Measure1DBuffered(ctx, setVoltageDomains, getters, numSteps, setters)
	assert(#setVoltageDomains == #setters, "one domain is required per setter")
	local sweep = build1DSweep(setVoltageDomains, numSteps)
	for _, voltages in ipairs(sweep) do
		applyVector(ctx, setters, voltages)
		readGetters(ctx, getters)
	end
end

return { main = Measure1DBuffered }
