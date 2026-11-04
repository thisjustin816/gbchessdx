# GB Chess Academy — research v0.9.0

Research-only checkpoint. No game ROM, selected engine, completed boss matches, or changes to master.

## Verified recoverability
Archive: GB-Chess-Research-v0.9.0-source-and-evidence.zip
Size: 801,968 bytes. ZIP entries: 467. Per-file SHA-256 records: 466.
ZIP SHA-256: efcef799cd29745520774cbb8c242b2839074bb52619dc0c8397e76ad267f390
Standalone: GB-Chess-STATUS-HANDOFF-v0.9.0.md
Handoff SHA-256: 60e7619c743945c71ac10dccbfc2d074fbe02b586427a4e0a70fae0bcc116773
Clean extraction passed all 466 hashes and 14 offline verifier groups. Deterministic repackaging matched the ZIP hash. No ROMs, saves, toolchains, objects or nested ZIP in the source archive. Source and handoff downloads are available in the GB Development ChatGPT project.
Previous v0.8 source snapshot remains retained, SHA-256 5ae5b8d0b277deeb83dad7e261e638b0278fd8a995c58745af8306bd6690df00.

## Story Mode direction and prototype
User wants a narrative campaign: several lessons/practice challenges followed by a real chapter boss chess match. Campaign files include a proposed five-act curriculum, two prototype chapter state machines, safe retries and progression gates. Four transferred tactical problems were independently verified by the project's restricted sparse-position oracle (mirror or rotated-and-color-swapped variants of the previously verified fork and mate).
Bosses are NOT yet playable; they need an actual engine/game-state result. The host prototype's match-result flag is just a test seam. Exact story setting, identities and final chapter count need design approval. Existing board-first storyboard is preserved; final font, art and highlighting deferred.

## Input-latching experiment
A separate chess-specific diagnostic tests an A/B press lasting only two frames while the pinned MIT upstream engine executes synchronous computer_move(3). With a VBlank interrupt latch, both inputs were captured and later serviced by the engine callback. The callback-only control missed both at deliberately chosen pulse timings.
CGB double-speed pulse at emulator frame 110: latch A/B detected at relative frame 35 and serviced at 41 (6-frame delay). Original DMG pulse at emulator frame 130: detected at 46, serviced at 83 (37-frame delay). B requests cooperative cancellation; returned moves remain legal with unchanged position hash. The callback-only DMG variant can catch these presses at other phases, so missed input is timing dependent.
Sixteen deterministic JSON recordings passed two cold boots per combination, with negative mutation detection. Strict 32 KiB dual-mode build and ROM header/KEY1/WRAM checks passed. Clean rebuild reproduced ISR ROM SHA-256 46761f7825f114d907aefffb1cd60f2f321a0e9d9da25a54b05279ab752e7a5c; control ROM SHA-256 9d7621e5e28b51a326d80830904e123c44419d408476572c1b755e44357de1a5. Neither ROM distributed. Original engine.c Git blob 495890f379d449be5c051f8525034881574716ff unchanged.

IMPORTANT: This is input capture, NOT non-blocking or resumable chess search. Callback service gaps reached 65 simulated frames on DMG. These are PyBoy measurements, not physical hardware guarantees or the final game UI.

Next: agree on story and bosses, validate more lessons and real miniature chess matches, prototype genuinely resumable per-frame search with legal fallback, compare engines independently, test physical hardware, then decide engine and save/mapper.
Trello: https://trello.com/c/FYELcNLg
