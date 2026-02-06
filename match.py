import chess
import chess.engine
import sys

#import logging
#logging.basicConfig(level=logging.DEBUG) 

# CONFIGURATION
MY_ENGINE_PATH = "./bin/uci"     # Path to your compiled C engine
STOCKFISH_PATH = "stockfish"     # Path to stockfish executable (must be in PATH or specify full path)
GAMES_TO_PLAY = 1000
STOCKFISH_ELO = 1320
STOCKFISH_TIME = 0.1
DEPTH=1

def play_game(sf_elo):
    # Setup Engines
    my_engine = chess.engine.SimpleEngine.popen_uci(MY_ENGINE_PATH)
    
    # Stockfish allows setting ELO via UCI options
    sf_engine = chess.engine.SimpleEngine.popen_uci(STOCKFISH_PATH)

    # Use Skill Level for very weak play (Level 0 is roughly 800-1000 ELO)
    # If sf_elo is very low, use Skill Level 0.
    if sf_elo < 1320:
        # Map roughly: 0=weakest, 20=strongest
        # Skill Level 0 is often clumsy enough for testing early engines
        sf_engine.configure({"Skill Level": 0}) 
    else:
        sf_engine.configure({"UCI_LimitStrength": True, "UCI_Elo": sf_elo})

    board = chess.Board()
    
    # Game Loop
    while not board.is_game_over():
        # Whose turn?
        if board.turn == chess.WHITE:
            # My Engine (White)
            # We enforce a fixed depth limit for consistency
            result = my_engine.play(board, chess.engine.Limit(depth=DEPTH))
        else:
            # Stockfish (Black)
            # We give Stockfish a time limit
            result = sf_engine.play(board, chess.engine.Limit(time=STOCKFISH_TIME))

            # Check for Under-Promotion (Knight, Rook, Bishop)
            # If Stockfish tries to promote to a knight, abort.
            # The Game Boy engine will promote to a queen regardless.
            if result.move.promotion and result.move.promotion == chess.KNIGHT:
                print(f"Aborting game: Stockfish tried under-promotion to knight ({result.move})")
                my_engine.quit()
                sf_engine.quit()
                return None  # Return None to signal "ignore this game"
            
        board.push(result.move)
        
        # Optional: Print move
        # print(f"{board.fullmove_number}. {result.move}")

    # Result
    outcome = board.outcome()
    print(f"Game Over: {outcome.result()} ({outcome.termination.name})")
    
    my_engine.quit()
    sf_engine.quit()
    
    # Return 1 if White (MyEngine) won, 0 if Black (SF) won, 0.5 draw
    if outcome.winner == chess.WHITE: return 1.0
    elif outcome.winner == chess.BLACK: return 0.0
    return 0.5

# Main Tournament Loop
print(f"Starting Match: MyEngine (Depth {DEPTH}) vs Stockfish (ELO {STOCKFISH_ELO})")
score = 0
played = 0
while played < GAMES_TO_PLAY:
    print(f"Playing Game {played+1}...", end=" ", flush=True)
    result = play_game(STOCKFISH_ELO)
    if result is None:
        print("Ignored.")
        continue
    played += 1
    score += result
    print(f"Current Score: {score}/{played}")

print(f"Final Score: {score}/{played}")
