# Network protocol

UDP/WinSock2, protocol version 2, rules `0x20261112`. `NetManager` handles transport; `NetConfig` reads the endpoint from `.env` or process environment. Hostnames resolve to IPv4 when opening a connection. There is no built-in owner endpoint.

The server matches players by mode, boss and difficulty. A private room uses the host's rules. START/READY/GO provide a loading barrier with a shared seed and start epoch. Normal gameplay does not wait for the other client's next frame.

Frame packets contain current and two previous samples. Reliable events use IDs, acknowledgements, retries, deduplication and a bounded ordering window. The client interpolates the remote player and predicts a limited number of ticks. This is asynchronous local-authority simulation, not full-world rollback. Boss patterns use separate deterministic RNG states.

In co-op the host confirms shared boss damage, resources, lives and team outcome. Each field uses the original boss scripts. Pauses are shared requests; each player clears their own request. Leaving or timing out ends the match; reconnect through a new room. Native replay recording of online matches is not supported.

Sessions use random tokens bound to the UDP source address. The relay rewrites capabilities and validates packet sizes, protocol values and game-event ranges. Packets are not encrypted. A room code gives access to a match, not SSH or server administration.

The server keeps sessions in memory, limits their count and expires inactive peers. Domain names do not conceal the destination IP from network participants.
