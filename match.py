import chess
import chess.engine
import sys
import math
import argparse

# --- ARGUMENT PARSING ---
def parse_arguments():
    parser = argparse.ArgumentParser(description="Run a chess match between MyEngine and Stockfish.")
    
    parser.add_argument("--my-engine", type=str, default="./bin/uci",
                        help="Path to your compiled C engine")
    parser.add_argument("--stockfish-path", type=str, default="stockfish",
                        help="Path to stockfish executable")
    parser.add_argument("--games", type=int, default=100,
                        help="Number of games to play")
    parser.add_argument("--elo", type=int, default=1320,
                        help="Stockfish ELO rating")
    parser.add_argument("--time", type=float, default=0.1,
                        help="Time per move for Stockfish in seconds")
    parser.add_argument("--depth", type=int, default=3,
                        help="Fixed depth for your engine")

    return parser.parse_args()

# Load arguments
args = parse_arguments()

# --- HELPER FUNCTIONS ---
def get_elo_from_score(score_ratio, opponent_elo):
    epsilon = 1e-9
    if score_ratio <= epsilon: return "-Inf"
    if score_ratio >= 1 - epsilon: return "+Inf"
    diff = -400 * math.log10(1 / score_ratio - 1)
    return int(opponent_elo + diff)

def play_game(sf_elo):
    # Use paths from 'args'
    my_engine = chess.engine.SimpleEngine.popen_uci(args.my_engine)
    sf_engine = chess.engine.SimpleEngine.popen_uci(args.stockfish_path)

    if sf_elo < 1320:
        sf_engine.configure({"Skill Level": 0}) 
    else:
        sf_engine.configure({"UCI_LimitStrength": True, "UCI_Elo": sf_elo})

    board = chess.Board()
    
    while not board.is_game_over():
        if board.turn == chess.WHITE:
            # Use depth from 'args'
            result = my_engine.play(board, chess.engine.Limit(depth=args.depth))
        else:
            # Use time from 'args'
            result = sf_engine.play(board, chess.engine.Limit(time=args.time))

            if result.move.promotion and result.move.promotion != chess.QUEEN:
                my_engine.quit()
                sf_engine.quit()
                return None
            
        board.push(result.move)

    outcome = board.outcome()
    my_engine.quit()
    sf_engine.quit()
    
    if outcome.winner == chess.WHITE: return 1.0
    elif outcome.winner == chess.BLACK: return 0.0
    return 0.5

# --- MAIN LOOP ---
print(f"Starting Match: MyEngine (Depth {args.depth}) vs Stockfish (ELO {args.elo})")
print(f"Settings: {args.games} games, SF Time: {args.time}s")

results = []
played = 0

while played < args.games:
    print(f"Playing Game {played+1}...", end=" ", flush=True)
    result = play_game(args.elo)
    
    if result is None:
        print("Ignored (Under-promotion error).")
        continue
        
    played += 1
    results.append(result)
    
    # Statistics
    total_score = sum(results)
    mean_score = total_score / played
    variance = sum((r - mean_score) ** 2 for r in results) / played
    std_dev = math.sqrt(variance)
    standard_error = std_dev / math.sqrt(played)
    
    margin = 1.96 * standard_error
    lower_bound = mean_score - margin
    upper_bound = mean_score + margin
    
    current_elo = get_elo_from_score(mean_score, args.elo)
    elo_min = get_elo_from_score(lower_bound, args.elo)
    elo_max = get_elo_from_score(upper_bound, args.elo)
    
    print(f"Score: {total_score}/{played} ({mean_score:.1%}) | "
          f"Elo: {current_elo} [95% CI: {elo_min} ... {elo_max}]")

print(f"Final Score: {total_score}/{played}")
