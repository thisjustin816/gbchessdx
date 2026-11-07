#include "rc_search.h"
#include "rs_pst.h"
#include <string.h>

static const int16_t piece_value[8] = {0, 100, 320, 330, 500, 900, 0, 0};

/* A piece's signed contribution to the evaluation from the root side's
 * view: its material value plus, with RS_EVAL_PST, its table value (Black
 * reads the table mirrored). */
static int16_t contrib(const RcSearch *s, uint8_t piece, uint8_t sq88) {
    uint8_t type = (uint8_t)(piece & 7u), sq64;
    int16_t v = piece_value[type];
    if (s->eval_mode == RS_EVAL_PST) {
        sq64 = (uint8_t)(((sq88 >> 4) << 3) | (sq88 & 7u));
        if (piece & RC_BLACK) sq64 ^= 56u;
        v = (int16_t)(v + rs_pst[type][sq64]);
    }
    return (uint8_t)(piece & RC_BLACK) == s->root_side ? v : (int16_t)-v;
}

/* Evaluation change when move is made on s->board: the mover leaves its
 * square, the placed piece (a promotion's new piece) arrives, a capture or
 * en passant victim leaves, and a castling rook moves. */
static int16_t move_delta(const RcSearch *s, RcMove m) {
    const uint8_t *b = s->board.board;
    uint8_t piece = b[m.from], color = (uint8_t)(piece & RC_BLACK);
    uint8_t placed = m.promotion ? (uint8_t)(color | m.promotion) : piece;
    int16_t d = (int16_t)(contrib(s, placed, m.to) - contrib(s, piece, m.from));
    if (m.flags & RC_EP) {
        uint8_t victim = (uint8_t)(m.to + (color == RC_WHITE ? -16 : 16));
        d = (int16_t)(d - contrib(s, (uint8_t)((color ^ RC_BLACK) | RC_PAWN), victim));
    } else if (b[m.to]) d = (int16_t)(d - contrib(s, b[m.to], m.to));
    if (m.flags & RC_CASTLE) {
        uint8_t rook = (uint8_t)(color | RC_ROOK);
        uint8_t rook_from = (uint8_t)(m.to > m.from ? m.from + 3u : m.from - 4u);
        uint8_t rook_to = (uint8_t)(m.to > m.from ? m.from + 1u : m.from - 1u);
        d = (int16_t)(d + contrib(s, rook, rook_to) - contrib(s, rook, rook_from));
    }
    return d;
}

static uint8_t same_move(RcMove a, RcMove b) {
    return (uint8_t)(a.from == b.from && a.to == b.to && a.promotion == b.promotion);
}

/* A mate at ply d (distance from the root) scores 30000 - d for the winner,
 * so a quicker mate is preferred over a later one. */
static int16_t checkmate_score(const RcSearch *s, uint8_t to_move, uint8_t ply) {
    int16_t magnitude = (int16_t)(30000 - ply);
    return to_move == s->root_side ? (int16_t)-magnitude : magnitude;
}

/* Move order at every node: tactical moves (captures, en passant,
 * promotions) first, then quiet moves; at the root, the best move of the
 * previous iteration before both. Order changes pruning, never the value. */
static void node_order_begin(RcSearch *s, uint8_t level) {
    s->pass[level] = (uint8_t)((level == 0u && s->has_pv) ? 0u : 1u);
    rc_iter_begin_filter(&s->its[level], RC_ITER_TACTICAL);
}

static void iteration_start(RcSearch *s, uint8_t depth) {
    uint8_t i;
    s->has_pv = s->has_best;
    s->pv = s->best;
    s->iteration_depth = depth;
    s->leaf_active = 0;
    s->leaf_pending = 0;
    s->counting = 0;
    s->pending = 0;
    s->iteration_has_best = 0;
    s->iteration_score = -32000;
    s->level = 0;
    for (i = 0; i < RS_MAX_DEPTH; ++i) {
        node_order_begin(s, i);
        s->seen[i] = 0;
        s->value[i] = (i & 1u) ? 32000 : -32000;
        s->alpha[i] = -32000;
        s->beta[i] = 32000;
        s->cutoff[i] = 0;
    }
}

static uint8_t uses_history(const RcSearch *s) {
    return (uint8_t)(s->policy == RS_DRAW_CLAIMABLE && s->history);
}

uint8_t rs_begin(RcSearch *out, const RcPosition *root, uint8_t depth, const RcHistory *history, uint8_t policy,
                 uint8_t eval_mode) {
    uint8_t ep;
    if (!out || !root || !depth || depth > RS_MAX_DEPTH) return 0;
    if (policy != RS_DRAW_AUTOMATIC_75 && policy != RS_DRAW_CLAIMABLE) return 0;
    if (eval_mode != RS_EVAL_MATERIAL && eval_mode != RS_EVAL_PST) return 0;
    if (history && memcmp(&history->current, root, sizeof *root)) return 0;
    memset(out, 0, sizeof *out);
    out->board = *root;
    out->root_side = root->side;
    out->goal_depth = depth;
    out->status = RS_RUNNING;
    out->history = history;
    out->policy = policy;
    out->eval_mode = eval_mode;
    if (uses_history(out)) out->key[0] = rh_key_at(history, 0, &ep);
    out->kings[0][0] = out->kings[0][1] = RC_NO_KING;
    out->preparing = 1;
    out->prep_b = 1;
    return 1;
}

void rs_request_cancel(RcSearch *s) {
    if (s && s->status == RS_RUNNING) s->cancelled = 1;
}

uint8_t rs_choice(const RcSearch *s, RcMove *out) {
    if (!s || !out || !s->has_fallback || s->status == RS_ERROR || s->status == RS_NO_MOVES) return 0;
    *out = s->has_best ? s->best : s->fallback;
    return 1;
}

/* Backpropagate a subtree's score to the parent (which still owns its
 * move). Even levels maximize the root side's score. */
static void accept_score(RcSearch *s, uint8_t level, int16_t score, RcMove move) {
    uint8_t better = !s->seen[level] || ((level & 1u) ? score < s->value[level] : score > s->value[level]);
    s->seen[level] = 1;
    if (better) {
        s->value[level] = score;
        if (!level) {
            s->iteration_best = move;
            s->iteration_score = score;
            s->iteration_has_best = 1;
        }
    }
    /* Fail-soft bounds; a cutoff is safe only after a legal child. */
    if (level & 1u) {
        if (s->value[level] < s->beta[level]) s->beta[level] = s->value[level];
    } else {
        if (s->value[level] > s->alpha[level]) s->alpha[level] = s->value[level];
    }
    if (s->alpha[level] >= s->beta[level]) s->cutoff[level] = 1;
}

/* Preparation, one unit per step: the root's evaluation one rank at a time,
 * then (with history) one history position at a time into the buckets seen
 * once and twice, by parity. A position in the tree at ply d can only
 * repeat history positions b plies back from the root with d + b even and
 * d + b within its halfmove window, which is at most the root's clock
 * plus d. */
static void prep_step(RcSearch *s) {
    uint8_t b = s->prep_b, ep, bucket, mask, parity, i, piece;
    uint16_t key;
    if (s->prep_rank < 8u) {
        for (i = (uint8_t)(s->prep_rank << 4); i < (uint8_t)((s->prep_rank << 4) + 8u); ++i) {
            piece = s->board.board[i];
            if (!piece) continue;
            if ((piece & 7u) == RC_KING) s->kings[0][piece >> 3] = i;
            s->eval[0] = (int16_t)(s->eval[0] + contrib(s, piece, i));
        }
        ++s->prep_rank;
        return;
    }
    if (!uses_history(s) || s->cancelled || b > s->history->stored || b > s->board.halfmove) {
        s->preparing = 0;
        iteration_start(s, 1);
        return;
    }
    key = rh_key_at(s->history, b, &ep);
    bucket = rh_bucket(key, ep);
    mask = (uint8_t)(1u << (bucket & 7u));
    parity = (uint8_t)(b & 1u);
    if (s->once[parity][bucket >> 3] & mask) s->twice[parity][bucket >> 3] |= mask;
    s->once[parity][bucket >> 3] |= mask;
    s->prep_b = (uint8_t)(b + 1u);
}

/* The position at ply has just been made. Decide whether it is a draw once
 * it has a legal move, starting an exact repetition count when its bucket
 * shows it may be a third appearance. A clock of at least 1 rules out a
 * double push, so its effective en passant square is RC_NONE. */
static void node_entered(RcSearch *s, uint8_t ply) {
    uint16_t hm = s->board.halfmove;
    uint8_t bucket;
    if (s->policy == RS_DRAW_AUTOMATIC_75) {
        s->drawish[ply] = (uint8_t)(hm >= 150u);
        return;
    }
    s->drawish[ply] = (uint8_t)(hm >= 100u);
    if (s->drawish[ply] || !s->history || hm < 4u) return;
    bucket = rh_bucket(s->key[ply], RC_NONE);
    if (s->twice[ply & 1u][bucket >> 3] & (uint8_t)(1u << (bucket & 7u))) {
        rh_counter_begin(&s->counter, s->history, &s->board, s->key[ply], RC_NONE, ply);
        s->counting = 1;
        s->count_ply = ply;
    }
}

/* Whether the side to move at ply is in check, from its tracked king. */
static uint8_t in_check_at(const RcSearch *s, uint8_t ply) {
    uint8_t side = s->board.side, king = s->kings[ply][side >> 3];
    return king == RC_NO_KING ? 1u : rc_attacked(&s->board, king, (uint8_t)(side ^ RC_BLACK));
}

static void make_node(RcSearch *s, uint8_t ply, RcMove move, RcUndo *undo) {
    uint8_t mover = s->board.board[move.from];
    s->kings[ply][0] = s->kings[ply - 1u][0];
    s->kings[ply][1] = s->kings[ply - 1u][1];
    if ((mover & 7u) == RC_KING) s->kings[ply][mover >> 3] = move.to;
    if (uses_history(s)) s->key[ply] = rh_key_after(&s->board, s->key[ply - 1u], move);
    s->eval[ply] = (int16_t)(s->eval[ply - 1u] + move_delta(s, move));
    rc_make_inplace(&s->board, move, undo);
    node_entered(s, ply);
}

/* The horizon position is on the board. A drawn position not in check
 * scores 0 (stalemate would too); otherwise probe for a legal reply, since
 * mate and stalemate take precedence over everything. */
static void leaf_decide(RcSearch *s) {
    s->leaf_pending = 0;
    s->leaf_check = in_check_at(s, s->iteration_depth);
    if (s->drawish[s->iteration_depth] && !s->leaf_check) {
        rc_unmake_inplace(&s->board, s->leaf_move, &s->leaf_undo);
        accept_score(s, s->level, 0, s->leaf_move);
    } else {
        rc_iter_begin(&s->leaf_it);
        s->leaf_active = 1;
    }
}

uint8_t rs_step(RcSearch *s) {
    uint8_t level, action, parent;
    RcMove move;
    if (!s || s->status != RS_RUNNING) return s ? s->status : RS_ERROR;
    ++s->steps;
    if (s->preparing) {
        prep_step(s);
        return s->status;
    }
    if (s->cancelled && s->has_fallback) {
        s->counting = 0;
        s->pending = 0;
        if (s->leaf_active || s->leaf_pending) {
            rc_unmake_inplace(&s->board, s->leaf_move, &s->leaf_undo);
            s->leaf_active = 0;
            s->leaf_pending = 0;
        } else if (s->level) {
            parent = (uint8_t)(s->level - 1u);
            rc_unmake_inplace(&s->board, s->path[parent], &s->undo[parent]);
            s->level = parent;
        } else s->status = RS_CANCELLED;
        return s->status;
    }
    if (s->counting) {
        if (!rh_counter_step(&s->counter, s->history)) return s->status;
        s->counting = 0;
        if (s->counter.count >= 3u) s->drawish[s->count_ply] = 1;
        return s->status;
    }
    if (s->leaf_pending) {
        leaf_decide(s);
        return s->status;
    }
    level = s->level;
    if (s->leaf_active) {
        int16_t score;
        uint8_t ply = s->iteration_depth;
        /* As in the tree: one step probes, the next scores the result. */
        if (!s->pending) {
            s->pending = rc_iter_step(&s->board, &s->leaf_it, &s->scratch, &move);
            return s->status;
        }
        action = s->pending;
        s->pending = 0;
        if (action == 3u) {
            rc_unmake_inplace(&s->board, s->leaf_move, &s->leaf_undo);
            s->leaf_active = 0;
            s->status = RS_ERROR;
            return s->status;
        }
        if (action == 2u) score = s->leaf_check ? checkmate_score(s, s->board.side, ply) : 0;
        else score = s->drawish[ply] ? 0 : s->eval[ply];
        rc_unmake_inplace(&s->board, s->leaf_move, &s->leaf_undo);
        s->leaf_active = 0;
        accept_score(s, level, score, s->leaf_move);
        return s->status;
    }
    /* One step runs the tree iterator; the next acts on what it found. */
    if (!s->pending) {
        if (s->cutoff[level]) action = 2u;
        else if (s->pass[level] == 0u) {
            s->pending_move = s->pv;
            s->pass[level] = 1u;
            action = 1u;
        } else {
            action = rc_iter_step(&s->board, &s->its[level], &s->scratch, &s->pending_move);
            if (action == 2u && s->pass[level] == 1u) {
                s->pass[level] = 2u;
                rc_iter_begin_filter(&s->its[level], RC_ITER_QUIET);
                return s->status;
            }
            /* The previous best move was searched first at the root. */
            if (action == 1u && level == 0u && s->has_pv && same_move(s->pending_move, s->pv)) return s->status;
        }
        if (action == 3u) { s->status = RS_ERROR; return s->status; }
        if (action) s->pending = action;
        return s->status;
    }
    action = s->pending;
    move = s->pending_move;
    s->pending = 0;
    if (action == 1u) {
        if (!level && !s->has_fallback) {
            s->fallback = move;
            s->has_fallback = 1;
        }
        /* A drawn non-root node scores 0 as soon as one legal move proves it
         * is not mate or stalemate. */
        if (level && s->drawish[level]) {
            parent = (uint8_t)(level - 1u);
            rc_unmake_inplace(&s->board, s->path[parent], &s->undo[parent]);
            s->level = parent;
            accept_score(s, parent, 0, s->path[parent]);
            return s->status;
        }
        if (level + 1u == s->iteration_depth) {
            s->leaf_move = move;
            s->leaf_pending = 1;
            make_node(s, (uint8_t)(level + 1u), move, &s->leaf_undo);
        } else {
            uint8_t child = (uint8_t)(level + 1u);
            s->path[level] = move;
            make_node(s, child, move, &s->undo[level]);
            s->level = child;
            s->seen[child] = 0;
            s->value[child] = (child & 1u) ? 32000 : -32000;
            s->alpha[child] = s->alpha[level];
            s->beta[child] = s->beta[level];
            s->cutoff[child] = 0;
            node_order_begin(s, child);
        }
        return s->status;
    }
    /* This node's moves are exhausted: mate or stalemate if it had none. */
    if (level) {
        int16_t score;
        parent = (uint8_t)(level - 1u);
        if (!s->seen[level])
            score = in_check_at(s, level) ? checkmate_score(s, s->board.side, level) : 0;
        else score = s->value[level];
        rc_unmake_inplace(&s->board, s->path[parent], &s->undo[parent]);
        s->level = parent;
        accept_score(s, parent, score, s->path[parent]);
        return s->status;
    }
    if (!s->seen[0]) {
        s->status = RS_NO_MOVES;
        return s->status;
    }
    /* Only a completed iteration is published. */
    s->best = s->iteration_best;
    s->best_score = s->iteration_score;
    s->has_best = s->iteration_has_best;
    s->completed_depth = s->iteration_depth;
    if (s->iteration_depth == s->goal_depth) s->status = RS_COMPLETE;
    else iteration_start(s, (uint8_t)(s->iteration_depth + 1u));
    return s->status;
}
