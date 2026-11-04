# GB Chess Academy — research v1.1.0: crowded-board sliced search

Research-only checkpoint; not a playable game, selected engine, or final coach/VS campaign implementation. master remains unchanged. Trello: https://trello.com/c/FYELcNLg

## Source-and-evidence recovery
- GB-Chess-Research-v1.1.0-source-and-evidence.zip — 869,028 bytes, 525 entries, 524 SHA-256 file records; SHA-256 08967ad6ac63e90d30bf16a0b31f17e3362963d7c743f27c9c733a60b35b7c25.
- GB-Chess-STATUS-HANDOFF-v1.1.0.md SHA-256 4177a64a80c2c81573689334f5573a873fc31ac81c5ca6f5153f26a12609677d; byte-identical to the handoff in ZIP.
- Clean extraction: all 524 hashes and all 17 verifier groups passed. Independent restricted opening replay validated 27-piece board and both castles; mutation checks reject changed-but-legal move and forged fallback.
- Uncached warning-clean GBDK rebuild from recovered source reproduced test ROM SHA-256 b003b6a7ee2a9c4c59d91e0e80128ae55a4c8d21f27509d1751239d885aa0f9e with two identical DMG cold boots. Repackaging reproduced the identical ZIP SHA-256.
- No ROMs, saves, compiled objects, emulator states, vendor tools or nested archives are included in source ZIP. Downloads supplied within GB Development ChatGPT project conversation. Previous promoted v1.0.0 ZIP retained with SHA-256 106f03303de86811e4057b939896cefa097de7bf4a1c35b3ee5e5fb48291cc43.
- Upstream MIT engine.c remains unchanged, Git blob 495890f379d449be5c051f8525034881574716ff. Commercial Game Boy Chessmaster ROM excluded.

## New two-console stress evidence
- Standard starting position, 32 pieces, 20 generated roots: full shallow 2-ply search chose legal b1c3 on CGB double-speed in 461 frames and DMG normal in 481 frames; 461 work units each.
- Developed 27-piece position, FEN r2qr1k1/ppp2ppp/2n1b3/3p4/2BPn3/BQP2N2/P4PPP/R3R1K1 w - - 1 13, 42 generated roots and up to 45 replies: full search chose legal c4d3 in 1,954 CGB frames / 2,000 DMG frames (1,954 work units). This is approximately 33 simulated seconds and unsuitable as a routine short hint.
- A 180-frame developed-position watchdog selected legal a1d1 on both consoles; an in-search two-frame B tap selected legal immediate fallback a1b1 in 24 CGB / 23 DMG frames. Different budgets produce different moves; no optimality claim.
- Eight case/mode/button configurations × two byte-identical PyBoy cold boots = 16 JSONs, verified with fixed ROM hashes and console/header/KEY1/position invariance. Maximum observed sampling gap was 1 CGB or 2 DMG frames; maximum individual work-unit overrun observed 0 CGB or 1 DMG frame. These are sampled experimental bounds, not universal guarantees or physical hardware results.
- Source linked 7-position host tests compile -Wall -Wextra -Werror; undefined behavior sanitizer passed. Strict diagnostic ROMs remain 32 KiB dual-mode with $0143=$80, automatic CGB double-speed and DMG fallback.
- Static WRAM end 0xC567, nominal space to E000 0x1A99 bytes; dynamic stack high-water not measured. Replaying 24 moves to create the developed diagnostic board costs 20 CGB/41 DMG frames before search, which is setup overhead rather than gameplay per-move cost. The legal preflight took one frame for these cases.

## Full-game design preserved
Five top-level modes: Campaign (story lessons and real boss chess matches), Coaching (ongoing hints, move evaluation and undo), Puzzles, VS Computer (chosen level) and 2 Players (Pass & Play or Link Cable). These are requirements rather than playable implementations. Source file analysis/GAME-MODES-REQUIREMENTS-v1.1.md preserves the design distinctions. Approved board-first lesson layout unchanged; final pixel art/fonts/highlights, save mapper, engine and narrative setting remain deferred.

## Next frontier
Profile worst-case cycles, per-step LY bounds and true dynamic stack high-water on more diverse game positions; validate all FIDE rules and independent engine move quality; prototype an actually responsive stronger engine and authentic coaching; test physical DMG/GBC/Chromatic and link/save behavior. Keep generalized hardware probes in Bacon Bit Bench and chess-specific experiments in gbchessdx research.
