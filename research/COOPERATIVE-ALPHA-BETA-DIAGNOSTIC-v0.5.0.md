# Cooperative alpha-beta chess search diagnostic v0.5.0 (experimental)

**Status: independently verified standalone research checkpoint.** Not a promoted gbchessdx v1.2.2 source, playable game, selected AI or complete FIDE engine. Promoted main project research baseline remains v1.2.1 (ZIP SHA-256 `ea6e532e52dbe235d2ebda7ae5af2e7a8c654d9c467cd13e03c3fc2c26f7829f`), whose exact raw bytes were unavailable for integration. `master` and pinned MIT `engine.c` remain unchanged.

## Exact recovery

- Source/evidence: `GB-Chess-Search-EXPERIMENTAL-v0.5.0-source-and-evidence.zip`, 76,592 bytes, 43 ZIP entries, 42 SHA-256 file manifest entries. Archive SHA-256 `81e269f5702dfd667046b0e9190da7bdbb7804f261620df735a1f73a3ee42531`.
- Standalone `GB-Chess-Search-EXPERIMENTAL-v0.5.0-STATUS-HANDOFF.md` SHA-256 `ed9e2bd6dc82fb53135a946d9f0e891ce13485a2b3f8b9f6e386c52b8744ac6c`, byte-identical to archived handoff.
- Optional diagnostic `GB-Chess-Search-EXPERIMENTAL-v0.5.0-diagnostic.gb`, 32,768 bytes, SHA-256 `cfc97ed1e9f2ae72256dc462b927cda68175d51efec357708dfd901606f4d9d9`, MD5 `a521bddf3a9f51700207094fe762ff59`. Kept **outside** source archive; distribute only with matching source and handoff. All files are linked in the ChatGPT GB Development conversation.
- Clean source extraction verified all 42 file bytes/hashes; forbidden-artifact inventory clean; deterministic repack was byte-identical. Clean GBDK-2020 rebuild reproduced exact ROM SHA/MD5, CGB flag $80, ROM-only/no SRAM, valid checksums and static WRAM end $C2FE.
- All ten offline groups passed from the clean extraction: original eight inherited tests, independent Python minimax on 22 additional positions, v0.4.0-old-fails/v0.5.0-new-passes alpha-beta step check, new 12-record evidence verifier rejecting four mutations, existing rules/cursor/search mutation controls and sanitized host tests. C rules, Python oracle, frozen v0.4 evidence are unchanged. v0.4.0 source C/H preserved under `historical/`.

## Experimental change

Portable, nonrecursive explicit-state depth-1..3 minimax gained alpha/beta bounds per active level and legal-move cutoff; cancellation, original-board preservation and completed-depth publication remain intact. Same selected moves/scores on four old/new regression positions. Logical step counts:
- Unique six-piece mate depth 2: 3,082 -> **736**, move `d4f2`.
- Mate depth 3: 13,873 -> **5,565**, move `d4f2`.
- Starting board depth 2: 1,870 -> **1,224**, move `b1c3`.
- Sparse knight position depth 2: 968 -> **301**, move `g5h7`.

Dual-mode diagnostic polls input and then performs at most eight search cursor steps; it stops the batch after a step crosses VBlank. **12 independent source-matched PyBoy 2.8.1 cold boots pass** (CGB/DMG x no input/A/B x two); full regenerated JSON matches frozen source evidence byte-for-byte. Search result/terminal/board integrity and two-frame A/B pulses pass. GBC mate 736 steps / 201 measured frames; DMG mate 736 steps / 354 frames. Observed worst search-step frame crossing: 1 CGB/2 DMG; input polling gap: 2 CGB/3 DMG. These are small-sample emulator timings, NOT physical hardware, universal latency bounds, or complete rendering/audio workload.

## Limits / next frontier

Alpha-beta reduces wasted work but material-only evaluation is weak; even this six-piece mate takes ~3.4 simulated seconds CGB, ~5.9 seconds DMG. No selected engine, independently established Elo, complete FIDE adjudication, quiescence, full UI/audio load, SRAM/link, save, physical DMG/GBC/Chromatic or playable Campaign/Coaching/Puzzles/VS/2 Players. The approved board-first storyboard remains untouched. Next: reconcile recoverable main v1.2.1 source, independently differential-test full FIDE cases, profile worst-case cursor/attack and dynamic stack under combined UI/audio, and improve tactical strength while retaining safe cancellation.

Trello: https://trello.com/c/FYELcNLg (keep **Exploring**).
