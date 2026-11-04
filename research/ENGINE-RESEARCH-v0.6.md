# Engine research v0.6.0: production difficulty behavior

Status: source/evidence research checkpoint. No game-engine changes, gameplay ROM, or new player UI. The default `master` branch remains unchanged.

Planning and test log: https://trello.com/c/FYELcNLg

## Recovered checkpoint

- Source and evidence archive: `GB-Chess-Research-v0.6.0-source-and-evidence.zip`
- SHA-256: `e5f65fcfca867fb7039b23e30362de2a1dd2f98ba6e05049073cd78a27f9ffec`
- 693,260 bytes, 383 entries; 382 file hashes recorded and validated in `MANIFEST.json`.
- Clean extraction passed seven offline verifier groups, including all older Chessmaster and gbchess research.
- Archive is available in the GB Development ChatGPT project, with `STATUS-HANDOFF.md` and pinned unmodified upstream MIT engine source.
- No `.gb`, `.gbc`, save, object, emulator state, or nested source ZIP is included.

## Measured production behavior

GBDK diagnostic fixture invokes actual `computer_move(level)`, not a direct fixed-depth search. It compiles a GB/GBC dual-mode ROM (header `$0143=$80`) and uses `cpu_fast()` only on GBC. Emulation: PyBoy 2.8.1 with `cgb=True`; speed register `KEY1` checked. RNG seeded with GBDK `initrand(1)`. Completed cases assert returned move legality, unchanged position hash, and identical cold-boot replay.

| Position | Setting | Calculated depth if searched | GBC double-speed frames | Move |
| --- | --- | ---: | ---: | --- |
| Knight fork, 4 pieces | Beginner | 1 / randomized | 11 | `g5e4` |
| Knight fork | Easy | 3 | 89 | `g5f3` |
| Knight fork | Medium | 4 | 251 | `g5f3` |
| Knight fork | Hard | 5 | 651 | `g5f3` |
| Unique mate, 6 pieces | Beginner | 1 / randomized | 20 | `d3d2` |
| Unique mate | Easy | 2 | 246 | `d4f2` |
| Unique mate | Medium | 3 | 989 | `d4f2` |
| Unique mate | Hard | 4 | more than 3,232 | No completed move within 3,300 ticks |
| Starting board | Beginner | 1 / randomized | 14 | `d2d4` |
| Starting board | Easy | 1 if searched | 2 | `e2e4` |

The Hard mate observation is a repeatable **watchdog timeout**, not a claim that the engine failed or would never finish. The starting-board Easy response is consistent with an opening-book move, but its source path has not been independently instrumented.

Easy GBC normal / GBC double / DMG fallback controls, respectively:
- Fork: 179 / 89 / 179 frames, all `g5f3`.
- Mate: 497 / 246 / 497 frames, all `d4f2`.

These are emulator search-window frame counts, not full game/UI response, physical hardware timing, or comparative Elo.

## Verification and recovery

The archived `modern-engine-research/verify_production_levels.py` validates 14 case/mode baselines with independent expected values, console/KEY1 states, position hashes for completed moves, cold-boot repeats and timeout handling. A deliberately modified move was rejected by the verifier. Previous six verification groups also pass from a clean extraction.

A fresh GBDK build from archived source compiled the five upstream engine modules and reproduced the Easy-fork CGB-normal ROM SHA-256 `a7ca181ed00cd39d6ac6e982181c94a634557e47e3f98fa95dd7d44a1b6cd6f4`, including its 179-frame result. The complete clean rebuild-and-repeat command hit the container execution limit and remains **unverified**, not passed. The existing independent emulator cold boots do pass and are preserved in JSON.

Build command for one full reproduction:

```sh
PYTHONPATH=/path/to/pyboy python3 modern-engine-research/build_and_check_production_levels.py \
  --gbdk /path/to/gbdk --only fork-l1 --repeat --output-dir /tmp/chess-prod
```

Current architecture direction: dual-mode GB/GBC with GBC double speed, full-board-first lesson UI. Avoid synchronous full-depth engine searches for routine hints; test a bounded analysis callback while retaining longer-thinking full-game difficulty options. Independent Stockfish/Maia and pedagogy research is separate and not yet incorporated. No physical console, SRAM or link-cable validation yet.

Current GB skills archive: `agents-main(4).zip`, SHA-256 `427b94f7a2af9f8e59d49a7d57110a9e909032f1c1ea64b6f9b5753e7f9e00de`. Hardware-general diagnostics should go to Bacon Bit Bench; this fixture is game-specific.
