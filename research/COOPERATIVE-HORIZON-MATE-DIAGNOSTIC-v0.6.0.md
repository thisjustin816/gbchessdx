# Cooperative checkmate-horizon diagnostic v0.6.0 (experimental)

**Status:** verified, independently recoverable **standalone research add-on**, not a promoted main gbchessdx checkpoint, selected chess AI, or playable game. Main promoted research baseline remains v1.2.1 (ZIP SHA-256 `ea6e532e52dbe235d2ebda7ae5af2e7a8c654d9c467cd13e03c3fc2c26f7829f`); exact v1.2.1 bytes were unavailable for integration. `master` and pinned upstream MIT `engine.c` unchanged.

## Recoverable files

- Source/evidence ZIP: `GB-Chess-Search-EXPERIMENTAL-v0.6.0-source-and-evidence.zip`, 97,036 bytes, **53 entries**, **52 per-file manifest SHA-256 records**, archive SHA-256 `35a77064e066aa6d25218da5e7218ecb132cdd108de3953490b423d74eaa5396`.
- Standalone `GB-Chess-Search-EXPERIMENTAL-v0.6.0-STATUS-HANDOFF.md`, SHA-256 `697d30345f314ecb204573dd915f53080ac1ef2900b8933ad3fd05fc6897186a`, byte-identical to archived handoff.
- Optional nonplayable dual-mode diagnostic `GB-Chess-Search-EXPERIMENTAL-v0.6.0-diagnostic.gb`, 32,768 bytes, SHA-256 `1188a84a3dbed6ed81c1699dc29ae01e915c604dcb87ff37c4eb408e182d64f0`, MD5 `049141350c8089b70941a0218b9624ab`. **Never distribute this ROM without the matching source and handoff**; ROM/toolchain/generated objects are excluded from the source ZIP. These exact deliverables are linked in the GB Development ChatGPT conversation.

## Old-fails/new-passes and verification

Frozen v0.5 material-only horizon scored a forced mate as ordinary material at depth 1. New source uses a cooperative legal-reply cursor only when the opponent is checked at the final ply, and applies mate-distance scoring. Independently enumerated unique mates:
- Black FEN `4k3/8/8/2b5/3q2b1/3p4/8/4K3 b - - 0 1`: old picks `d3d2` (+1660), new picks `d4f2` (+29999).
- White FEN `7k/8/5KQ1/8/8/8/8/8 w - - 0 1`: old picks `f6g5` (+900), new picks `g6g7` (+29999).
- Depth 3 still prefers immediate `d4f2` over slower mating lines. Cancellation during nested probe restores both root and internal positions, preserving fallback and last complete iteration. Two deliberately broken C variants fail.

**Clean extraction verification:** all 52 manifest file hashes, forbidden-artifact audit and deterministic byte-identical repack passed. All **12 offline groups** passed, including historical rules/perft, 1,988 C/Python move differential positions, 22 independent Python minimax positions, old-fails/new-passes search/regression checks, two new source mutations, and new frozen evidence rejecting **seven** tamperings. Host GCC strict C99 and targeted UBSan pass.

Exact GBDK-2020 clean-source rebuild matched ROM SHA/MD5, dual-mode CGB flag $80, valid checksums, CGB double speed/DMG fallback, static WRAM end **$C395**. Same ROM passed **12 PyBoy 2.8.1 cold boots** (CGB/DMG x no input / two-frame A / two-frame B x two), with **byte-identical regenerated JSON**. Five cases: depth-two mate, opening B/watchdog cancellation, terminal no-moves, and both new depth-one mates. CGB/DMG depth-two mate 873 steps / **237/494 frames**, depth-one black mate **79/165 frames**, white mate **84/207 frames**. Max observed input-poll gaps **2/3 frames**, max step crossings **1/2 frames**.

An 8-step DMG batch initially passed correctness but observed a **four-frame poll gap**; preserved as rejected performance evidence (experimental ROM SHA `10462dc9c78747c560b6ce6bcd9613edae89c443692f95187651fdc3ea7a8826`). Final fixture uses 8 steps per batch on CGB and 3 on DMG, regaining three-frame observed DMG polling at the cost of extra elapsed frames.

## Limits and next research

The new checkmate probe **does not** detect stalemate at the last ply or implement complete FIDE draw/repetition claims. Search is still material-only, slow on the original GB and untested under real UI/audio/physical-hardware timing; host `sizeof(ChessSearch)` is 596 bytes and not a dynamic stack guarantee. No selected AI, Elo, save/link/UI, final story/art or playable modes. Next: obtain exact promoted v1.2.1 source before integration; independent mature FIDE oracle; stronger tactical evaluation; bounded yields under real audio/video input; physical DMG/GBC/Chromatic. Keep Trello **Exploring**.

Project card: https://trello.com/c/FYELcNLg
