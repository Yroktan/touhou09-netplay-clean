# Network protocol

UDP/WinSock2, protocol version 2, rules `0x20261112`. `NetManager` handles transport; `NetConfig` reads the endpoint from `.env` or process environment. Hostnames resolve to IPv4 when opening a connection. There is no built-in owner endpoint.

The server matches players by mode and difficulty. The legacy boss field is reserved and sent as zero. A private room uses the host's rules. START/READY/GO provide a loading barrier with a shared seed and start epoch.

Duels retain their existing asynchronous local-authority simulation: frame packets contain current and two previous samples, remote positions are interpolated with limited prediction, and attacks use reliable events. Boss patterns use separate deterministic RNG states. Reliable events use IDs, acknowledgements, retries, deduplication and a bounded ordering window.

Story co-op simulates the original campaign on both machines. Both human players occupy side 0; side 1 remains the original story CPU. The host's character selects the story route. Each human retains independent controls, health, shots and collision checks. The team loses a battle only when both humans are out. Native story dialogue, continues and stage transitions remain active. CPU health is doubled to 20 and boss projection health on side 0 is doubled, including phase thresholds.

Co-op inputs use ordered reliable TeamState events, identified by signature 0x53544f52, with six frames of input delay. Simulation waits for missing inputs, including during dialogue, pause and continue choices. It does not interpolate the companion or predict the story world. Both players need this client version; legacy boss-arena events are rejected. Existing relay framing and event forwarding are unchanged.

In co-op Esc toggles the shared pause and X while paused exits. Duels retain their existing per-player pause requests. Leaving or timing out ends the match; reconnect through a new room. Native replay recording of online matches is not supported.

Sessions use random tokens bound to the UDP source address. The relay rewrites capabilities and validates packet sizes, protocol values and game-event ranges. Packets are not encrypted. A room code gives access to a match, not SSH or server administration.

The server keeps sessions in memory, limits their count and expires inactive peers. Domain names do not conceal the destination IP from network participants.
