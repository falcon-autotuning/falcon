# falconCore/physics/config

Provides device configuration loading and querying for quantum dot devices in Falcon.

## Features
- **Loader**: Load YAML device configuration files into structured `Config` objects.
- **Config**: Query channel gates (`GetChannelPlungerGates`, `GetChannelBarrierGates`, `GetChannelReservoirGates`, `GetChannelScreeningGates`, `GetChannelDotGates`, `GetChannelGates`, `GetChannelOhmics`), associated ohmics (`GetAssociatedOhmic`), groups (`SelectGroup`, `GetAllGnames`), dot counts (`GetDotNumber`), DC wiring impedances (`WiringDC`, `GetImpedance`), and voltage constraints (`VoltageConstraints`).
- **Group**: Group-level topologies, dot counts, and gate linear ordering (`Order`).
- **VoltageConstraints**: Safe voltage limits and constraint matrix representations.
- **Adjacency**: Adjacency graph representation of gates in the device layout.
