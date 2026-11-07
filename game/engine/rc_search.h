#ifndef GB_CHESS_RC_SEARCH_H
#define GB_CHESS_RC_SEARCH_H
/* Cooperative, material-only research search on the rules core: the v0.8.0
 * standalone search algorithm moved onto RcPosition, with optional game
 * history. It selects no game AI.
 *
 * The caller owns the root position and steps the search from its event
 * loop. No recursion or heap, and no step scans the whole board: one step
 * makes at most one rules-core iterator step, or one node operation (a make
 * or unmake with its incremental key and material, or a check test), or one
 * rank of the root's material, or one step of a repetition count. Only a completed depth is
 * published; until then the choice is the first legal root move.
 *
 * Scores are from the root side's view. RS_EVAL_MATERIAL counts material
 * (P 100, N 320, B 330, R 500, Q 900), the v0.8.0 evaluation; RS_EVAL_PST
 * adds the piece-square tables in rs_pst.h (generated from tests/pst.py).
 * A mate at ply d scores 30000 - d for the winner. Mate and stalemate
 * always take precedence. Draw scoring depends on the policy:
 *
 * RS_DRAW_AUTOMATIC_75 (the v0.8.0 behavior): a position reached in the tree
 *   at 150 halfmoves or more scores 0. History is ignored.
 * RS_DRAW_CLAIMABLE: a position reached in the tree scores 0 when either
 *   player could claim a draw there: 100 halfmoves or more, or a third
 *   appearance counting the game history (rh_ history ring) and the path.
 *   FIDE gives these claims both to the player who makes the move (9.2.1.1,
 *   9.3.1) and to the player then to move, so the side that is worse off
 *   can always take the draw. This includes the automatic 75-move and
 *   fivefold draws.
 *
 * Moves are searched captures, en passant and promotions first, then quiet
 * moves, and at the root the previous iteration's best move first. Order
 * changes how much is pruned, never the score; among equal moves the first
 * in this order is chosen.
 *
 * The root itself is never adjudicated: call rh_judge (or rh_query) first
 * and do not search a finished game. With history, the root must be
 * history->current and the history must not change during the search. A
 * repetition inside a tree of depth <= 3 needs at least four plies, so it
 * can only involve history positions, never the path or the root.
 */
#include "rules_history.h"

#define RS_MAX_DEPTH 3u
#define RS_RUNNING 0u
#define RS_COMPLETE 1u
#define RS_NO_MOVES 2u
#define RS_CANCELLED 3u
#define RS_ERROR 4u

#define RS_DRAW_AUTOMATIC_75 0u
#define RS_DRAW_CLAIMABLE 1u
#define RS_EVAL_MATERIAL 0u
#define RS_EVAL_PST 1u

typedef struct {
    RcPosition board;
    RcIterator its[RS_MAX_DEPTH];
    RcMove path[RS_MAX_DEPTH];
    RcUndo undo[RS_MAX_DEPTH];
    RcMove fallback, best, iteration_best;
    int16_t value[RS_MAX_DEPTH], alpha[RS_MAX_DEPTH], beta[RS_MAX_DEPTH];
    int16_t best_score, iteration_score;
    uint32_t steps;
    uint8_t seen[RS_MAX_DEPTH], cutoff[RS_MAX_DEPTH];
    uint8_t root_side, goal_depth, iteration_depth, level;
    uint8_t completed_depth, status, cancelled, has_fallback;
    uint8_t has_best, iteration_has_best;
    uint8_t pending;     /* tree or probe iterator result for the next step: 1 move, 2 exhausted, 3 error */
    RcMove pending_move;
    /* horizon: the leaf move is made; decide, probe replies, or count */
    RcIterator leaf_it;
    RcUndo leaf_undo;
    RcMove leaf_move;
    uint8_t leaf_pending, leaf_active, leaf_check;
    int16_t eval[RS_MAX_DEPTH + 1u]; /* root side's evaluation at each ply */
    uint8_t kings[RS_MAX_DEPTH + 1u][2]; /* king squares at each ply, [white, black] */
    RcPosition scratch;
    /* draw scoring */
    const RcHistory *history;
    uint8_t policy, eval_mode, preparing, prep_rank, prep_b;
    /* order: per level 0 = previous best (root), 1 = tactical, 2 = quiet */
    uint8_t pass[RS_MAX_DEPTH], has_pv;
    RcMove pv;
    uint16_t key[RS_MAX_DEPTH + 1u]; /* rh_key of the position at each ply */
    uint8_t drawish[RS_MAX_DEPTH + 1u]; /* that position is a draw once it has a legal move */
    uint8_t counting, count_ply;
    RhCounter counter;
    /* Buckets of history positions seen twice, by ply parity: a position in
     * the tree can only be a third appearance if its bucket is set. */
    uint8_t once[2][32], twice[2][32];
} RcSearch;

/* Returns 0 for an invalid depth (1..3), an unknown policy or evaluation,
 * or a history whose current position is not root, leaving out untouched.
 * history may be NULL. out must not alias root or history. For a time
 * limit, call rs_request_cancel when it expires: the choice is then the
 * best move of the deepest completed depth. */
uint8_t rs_begin(RcSearch *out, const RcPosition *root, uint8_t depth, const RcHistory *history, uint8_t policy,
                 uint8_t eval_mode);
uint8_t rs_step(RcSearch *s);
/* Cooperative cancel: the search unwinds its board, keeps any completed
 * depth, and otherwise finds a legal fallback first. */
void rs_request_cancel(RcSearch *s);
/* 1 with a legal root move when one exists. */
uint8_t rs_choice(const RcSearch *s, RcMove *out);
#endif
