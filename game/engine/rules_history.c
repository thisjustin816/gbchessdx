#include "rules_history.h"
#include "rh_keys.h"
#include <string.h>

/* Query phases */
#define Q_REPS 0u
#define Q_LEGAL 1u
#define Q_STATUS 2u
#define Q_DUPS 3u
#define Q_ANY_NEXT 4u
#define Q_ANY_MOVE 5u
#define Q_DONE 6u
/* Intended-move job phases */
#define M_APPLY 0u
#define M_EP 1u
#define M_FIFTY 2u
#define M_COUNT 3u
#define M_COUNTING 4u
#define M_DONE 5u
#define BOTH_CLAIMS (RC_CLAIM_FIFTY | RC_CLAIM_THREEFOLD)
/* m_need bit asking for the repetition count even once threefold is known */
#define NEED_COUNT 0x80u

#ifdef RH_TEST_CONSTANT_KEYS
#define PIECE_KEY(piece, sq88) ((void)(piece), (void)(sq88), 0u)
#define CASTLE_KEY(rights) ((void)(rights), 0u)
#define SIDE_KEY 0u
#else
#define PIECE_KEY(piece, sq88) \
    rh_piece_keys[(((piece) & RC_BLACK) ? 6u : 0u) + ((piece) & 7u) - 1u][(((sq88) >> 4) << 3) | ((sq88) & 7u)]
#define SIDE_KEY RH_SIDE_KEY
static uint16_t castle_key(uint8_t rights) {
    uint16_t k = 0;
    uint8_t i;
    for (i = 0; i < 4u; ++i)
        if (rights & (uint8_t)(1u << i)) k ^= rh_castle_keys[i];
    return k;
}
#define CASTLE_KEY(rights) castle_key(rights)
#endif

uint16_t rh_key(const RcPosition *p) {
    uint16_t k = p->side == RC_BLACK ? SIDE_KEY : 0u;
    uint8_t i, piece;
    for (i = 0; i < 128u; ++i) {
        if (i & 0x88u) { i = (uint8_t)(i + 7u); continue; }
        piece = p->board[i];
        if (piece) k ^= PIECE_KEY(piece, i);
    }
    return (uint16_t)(k ^ CASTLE_KEY(p->castle));
}

/* Mirrors rc_make_inplace: moved piece, capture, en passant victim,
 * promotion, castling rook, castling rights and side to move. */
uint16_t rh_key_after(const RcPosition *p, uint16_t key, RcMove m) {
    uint8_t piece = p->board[m.from], color = (uint8_t)(piece & RC_BLACK), captured = p->board[m.to];
    uint8_t placed = m.promotion ? (uint8_t)(color | m.promotion) : piece, rights = p->castle;
    key ^= PIECE_KEY(piece, m.from);
    if (captured) key ^= PIECE_KEY(captured, m.to);
    key ^= PIECE_KEY(placed, m.to);
    if (m.flags & RC_EP) {
        uint8_t victim = (uint8_t)(m.to + (color == RC_WHITE ? -16 : 16));
        key ^= PIECE_KEY((uint8_t)((color ^ RC_BLACK) | RC_PAWN), victim);
    }
    if (m.flags & RC_CASTLE) {
        uint8_t rook = (uint8_t)(color | RC_ROOK);
        uint8_t rook_from = (uint8_t)(m.to > m.from ? m.from + 3u : m.from - 4u);
        uint8_t rook_to = (uint8_t)(m.to > m.from ? m.from + 1u : m.from - 1u);
        key ^= PIECE_KEY(rook, rook_from);
        key ^= PIECE_KEY(rook, rook_to);
    }
    if ((piece & 7u) == RC_KING) rights &= (uint8_t)~(color == RC_WHITE ? (RC_WK | RC_WQ) : (RC_BK | RC_BQ));
    if (m.from == 0u || m.to == 0u) rights &= (uint8_t)~RC_WQ;
    if (m.from == 7u || m.to == 7u) rights &= (uint8_t)~RC_WK;
    if (m.from == 112u || m.to == 112u) rights &= (uint8_t)~RC_BQ;
    if (m.from == 119u || m.to == 119u) rights &= (uint8_t)~RC_BK;
    if (rights != p->castle) key ^= (uint16_t)(CASTLE_KEY(p->castle) ^ CASTLE_KEY(rights));
    return (uint16_t)(key ^ SIDE_KEY);
}

uint8_t rh_effective_ep(const RcPosition *p, RcPosition *scratch) {
    uint8_t ep = p->ep, color = p->side, i;
    int16_t victim, from;
    RcMove m;
    if (ep == RC_NONE || (ep & 0x88u) || p->board[ep]) return RC_NONE;
    victim = (int16_t)ep + (color == RC_WHITE ? -16 : 16);
    if (victim < 0 || victim > 127 || (victim & 0x88) ||
        p->board[victim] != (uint8_t)((color ^ RC_BLACK) | RC_PAWN)) return RC_NONE;
    for (i = 0; i < 2u; ++i) {
        from = (int16_t)(victim + (i ? 1 : -1));
        if (from < 0 || from > 127 || (from & 0x88) || p->board[from] != (uint8_t)(color | RC_PAWN)) continue;
        m.from = (uint8_t)from;
        m.to = ep;
        m.promotion = 0;
        m.flags = RC_EP;
        rc_apply(p, m, scratch);
        if (!rc_in_check(scratch, color)) return ep;
    }
    return RC_NONE;
}

static uint8_t slot_back(const RcHistory *h, uint8_t back) {
    /* back = 1 is the newest entry */
    int16_t i = (int16_t)h->head - (int16_t)back;
    if (i < 0) i = (int16_t)(i + (int16_t)RH_CAPACITY);
    return (uint8_t)i;
}

uint16_t rh_key_at(const RcHistory *h, uint8_t b, uint8_t *ep) {
    const RhEntry *e;
    if (b == h->stored) {
        *ep = h->base_ep;
        return h->base_key;
    }
    e = &h->ring[slot_back(h, (uint8_t)(b + 1u))];
    *ep = e->ep_eff;
    return e->key;
}

void rh_begin(RcHistory *h, const RcPosition *start, RhQuery *work) {
    h->current = *start;
    h->base_key = rh_key(start);
    h->base_ep = rh_effective_ep(start, &work->probe);
    h->head = 0;
    h->stored = 0;
    h->plies = 0;
}

void rh_push(RcHistory *h, RcMove move, RhQuery *work) {
    RhEntry *e = &h->ring[h->head];
    uint8_t ep;
    uint16_t key = rh_key_after(&h->current, rh_key_at(h, 0, &ep), move);
    if (h->stored == RH_CAPACITY) {
        /* The oldest entry goes; the position after it becomes the base. */
        h->base_key = e->key;
        h->base_ep = e->ep_eff;
    }
    e->move = move;
    rc_make_inplace(&h->current, move, &e->undo);
    e->key = key;
    e->ep_eff = rh_effective_ep(&h->current, &work->probe);
    h->head = (uint8_t)(h->head + 1u == RH_CAPACITY ? 0u : h->head + 1u);
    if (h->stored < RH_CAPACITY) ++h->stored;
    if (h->plies != 0xFFFFu) ++h->plies;
}

static uint8_t overwritten(const RcHistory *h) { return (uint8_t)(h->plies != h->stored); }

uint8_t rh_exact(const RcHistory *h) {
    return (uint8_t)(!overwritten(h) || h->current.halfmove <= h->stored);
}

uint8_t rh_takeback(RcHistory *h) {
    const RhEntry *e;
    if (!h->stored) return 0;
    e = &h->ring[slot_back(h, 1u)];
    /* After undoing, the window is e->undo.halfmove plies over stored-1 entries. */
    if (overwritten(h) && e->undo.halfmove > (uint16_t)(h->stored - 1u)) return 0;
    rc_unmake_inplace(&h->current, e->move, &e->undo);
    h->head = slot_back(h, 1u);
    --h->stored;
    if (h->plies) --h->plies;
    return 1;
}

void rh_counter_begin(RhCounter *c, const RcHistory *h, const RcPosition *target, uint16_t key, uint8_t ep,
                      uint8_t distance) {
    c->target = target;
    c->key = key;
    c->ep = ep;
    c->distance = distance;
    c->b = (distance & 1u) ? 1u : 2u;
    c->walked = 0;
    c->count = 1;
    c->verify = 0;
    c->walk = h->current;
}

uint8_t rh_counter_step(RhCounter *c, const RcHistory *h) {
    uint8_t b = c->b, ep;
    const RhEntry *e;
    if (b > h->stored || (uint16_t)c->distance + b > c->target->halfmove) return 1;
    if (!c->verify) {
        if (rh_key_at(h, b, &ep) == c->key && ep == c->ep) c->verify = 1;
        else c->b = (uint8_t)(b + 2u);
        return 0;
    }
    if (c->walked < b) {
        e = &h->ring[slot_back(h, (uint8_t)(c->walked + 1u))];
        rc_unmake_inplace(&c->walk, e->move, &e->undo);
        ++c->walked;
        return 0;
    }
    if (c->walk.side == c->target->side && c->walk.castle == c->target->castle &&
        !memcmp(c->walk.board, c->target->board, sizeof c->walk.board) && c->count < 255u)
        ++c->count;
    c->verify = 0;
    c->b = (uint8_t)(b + 2u);
    return 0;
}

uint8_t rh_bucket(uint16_t key, uint8_t ep) { return (uint8_t)(key ^ (key >> 8) ^ ep); }

/* Intended-move job: the position after q->move, its repetition count and
 * the claims it allows. need: RC_CLAIM_* bits still wanted, plus NEED_COUNT. */
static void move_begin(RhQuery *q, RcMove move, uint8_t need) {
    q->move = move;
    q->m_need = need;
    q->m_claims = 0;
    q->m_reps = 1;
    q->m_phase = M_APPLY;
}

/* Returns RH_RUNNING, RH_DONE or RH_ERROR. */
static uint8_t move_step(RhQuery *q, const RcHistory *h) {
    uint8_t r, ep;
    switch (q->m_phase) {
    case M_APPLY:
        q->m_key = rh_key_after(&h->current, rh_key_at(h, 0, &ep), q->move);
        /* Inside rh_claims_any a threefold needs the new position's bucket to
         * have been hit twice by the duplicate scan (the new position has a
         * nonzero clock, so its effective en passant square is RC_NONE). */
        if (!(q->m_need & (NEED_COUNT | RC_CLAIM_FIFTY))) {
            uint8_t bucket = rh_bucket(q->m_key, RC_NONE);
            if (!(q->dup_twice[bucket >> 3] & (uint8_t)(1u << (bucket & 7u)))) {
                q->m_phase = M_DONE;
                return RH_DONE;
            }
        }
        rc_apply(&h->current, q->move, &q->after);
        q->m_phase = M_EP;
        return RH_RUNNING;
    case M_EP:
        /* After a double push the clock is 0 and nothing can repeat, so the
         * en passant test is needed only for an unusual loaded position. */
        q->m_ep = (q->after.ep != RC_NONE && q->after.halfmove) ? rh_effective_ep(&q->after, &q->probe) : RC_NONE;
        if ((q->m_need & RC_CLAIM_FIFTY) && q->after.halfmove >= 100u) {
            rc_iter_begin(&q->it_after);
            q->m_phase = M_FIFTY;
        } else q->m_phase = M_COUNT;
        return RH_RUNNING;
    case M_FIFTY:
        /* The claim stands only if the opponent then has a legal move. */
        r = rc_iter_step(&q->after, &q->it_after, &q->probe, &q->spare);
        if (r == 3u) return RH_ERROR;
        if (r == 1u) q->m_claims |= RC_CLAIM_FIFTY;
        if (r) q->m_phase = M_COUNT;
        return RH_RUNNING;
    case M_COUNT:
        if (!(q->m_need & (RC_CLAIM_THREEFOLD | NEED_COUNT))) {
            q->m_phase = M_DONE;
            return RH_DONE;
        }
        rh_counter_begin(&q->counter, h, &q->after, q->m_key, q->m_ep, 1u);
        q->m_phase = M_COUNTING;
        return RH_RUNNING;
    case M_COUNTING:
        if (!rh_counter_step(&q->counter, h)) return RH_RUNNING;
        q->m_reps = q->counter.count;
        if (q->m_reps >= 3u) q->m_claims |= RC_CLAIM_THREEFOLD;
        q->m_phase = M_DONE;
        return RH_DONE;
    default:
        return RH_DONE;
    }
}

void rh_query_begin(RhQuery *q, const RcHistory *h, uint8_t want) {
    uint8_t ep;
    uint16_t key = rh_key_at(h, 0, &ep);
    q->want = want;
    q->steps = 0;
    q->error = 0;
    q->reps = 1;
    q->claims_now = 0;
    q->claims_any = 0;
    q->any_need = 0;
    q->has_legal = 0;
    q->judgment.terminal = RC_PLAYING;
    q->judgment.claimable = 0;
    rh_counter_begin(&q->counter, h, &h->current, key, ep, 0u);
    q->phase = Q_REPS;
}

static void judge(RhQuery *q, const RcHistory *h, uint8_t in_check, uint8_t dead) {
    const RcPosition *p = &h->current;
    RcJudgment *j = &q->judgment;
    j->terminal = RC_PLAYING;
    j->claimable = 0;
    if (!q->has_legal) j->terminal = in_check ? RC_MATE : RC_STALEMATE;
    else if (dead) j->terminal = RC_DEAD;
    else if (p->halfmove >= 150u) j->terminal = RC_DRAW_75;
    else if (q->reps >= 5u) j->terminal = RC_DRAW_FIVEFOLD;
    else {
        if (p->halfmove >= 100u) j->claimable |= RC_CLAIM_FIFTY;
        if (q->reps >= 3u) j->claimable |= RC_CLAIM_THREEFOLD;
    }
    q->claims_now = (uint8_t)(((p->halfmove >= 100u && q->has_legal) ? RC_CLAIM_FIFTY : 0u) |
                              (q->reps >= 3u ? RC_CLAIM_THREEFOLD : 0u));
}

/* Ends the duplicate scan: without a candidate, no intended move can give a
 * threefold claim. */
static uint8_t dups_done(RhQuery *q, uint8_t found) {
    if (!found) q->any_need &= (uint8_t)~RC_CLAIM_THREEFOLD;
    if (!q->any_need) {
        q->phase = Q_DONE;
        return RH_DONE;
    }
    rc_iter_begin(&q->it);
    q->phase = Q_ANY_NEXT;
    return RH_RUNNING;
}

uint8_t rh_query_step(RhQuery *q, const RcHistory *h) {
    uint8_t r, ep;
    if (q->error) return RH_ERROR;
    if (q->phase == Q_DONE) return RH_DONE;
    if (q->steps != 0xFFFFu) ++q->steps;
    switch (q->phase) {
    case Q_REPS:
        if (rh_counter_step(&q->counter, h)) {
            q->reps = q->counter.count;
            rc_iter_begin(&q->it);
            q->phase = Q_LEGAL;
        }
        return RH_RUNNING;
    case Q_LEGAL:
        r = rc_iter_step(&h->current, &q->it, &q->probe, &q->spare);
        if (r == 3u) { q->error = 1; return RH_ERROR; }
        if (r == 1u) q->has_legal = 1;
        if (r) q->phase = Q_STATUS;
        return RH_RUNNING;
    case Q_STATUS:
        /* One scan: the check test without legal moves, else the dead test. */
        if (q->has_legal) judge(q, h, 0, rc_recognized_dead(&h->current));
        else judge(q, h, rc_in_check(&h->current, h->current.side), 0);
        q->claims_any = q->claims_now;
        if (!(q->want & RH_WANT_ANY) || !q->has_legal) {
            q->phase = Q_DONE;
            return RH_DONE;
        }
        /* An intended move ends at most one ply later, so a 50-move claim
         * needs the clock at 99 already. */
        q->any_need = (uint8_t)(BOTH_CLAIMS & ~q->claims_any);
        if (h->current.halfmove < 99u) q->any_need &= (uint8_t)~RC_CLAIM_FIFTY;
        if (!(q->any_need & RC_CLAIM_THREEFOLD)) return dups_done(q, 0);
        memset(q->dup_once, 0, sizeof q->dup_once);
        memset(q->dup_twice, 0, sizeof q->dup_twice);
        q->dup_found = 0;
        q->scan_b = 1;
        q->phase = Q_DUPS;
        return RH_RUNNING;
    case Q_DUPS:
        /* A position after an intended move has the opponent to move and can
         * only match positions an odd number of plies back, inside the
         * window. A threefold claim needs one of them to have appeared twice
         * already. Two 256-bit bitmaps of (key, en passant) buckets record
         * buckets hit once and twice; equal positions share a bucket, so an
         * unmarked bucket rules a candidate out. */
        if (q->scan_b > h->stored || q->scan_b > h->current.halfmove) return dups_done(q, q->dup_found);
        {
            uint16_t key = rh_key_at(h, q->scan_b, &ep);
            uint8_t bucket = rh_bucket(key, ep);
            uint8_t mask = (uint8_t)(1u << (bucket & 7u));
            if (q->dup_once[bucket >> 3] & mask) {
                q->dup_twice[bucket >> 3] |= mask;
                q->dup_found = 1;
            }
            q->dup_once[bucket >> 3] |= mask;
        }
        q->scan_b = (uint8_t)(q->scan_b + 2u);
        return RH_RUNNING;
    case Q_ANY_NEXT:
        if ((q->claims_any & q->any_need) == q->any_need) { q->phase = Q_DONE; return RH_DONE; }
        r = rc_iter_step(&h->current, &q->it, &q->probe, &q->spare);
        if (r == 3u) { q->error = 1; return RH_ERROR; }
        if (r == 2u) { q->phase = Q_DONE; return RH_DONE; }
        if (r == 1u) {
            move_begin(q, q->spare, (uint8_t)(q->any_need & ~q->claims_any));
            q->phase = Q_ANY_MOVE;
        }
        return RH_RUNNING;
    case Q_ANY_MOVE:
        r = move_step(q, h);
        if (r == RH_ERROR) { q->error = 1; return RH_ERROR; }
        if (r == RH_DONE) {
            q->claims_any |= q->m_claims;
            q->phase = Q_ANY_NEXT;
        }
        return RH_RUNNING;
    default:
        return RH_DONE;
    }
}

static uint8_t run_query(const RcHistory *h, RhQuery *q, uint8_t want) {
    uint8_t r;
    rh_query_begin(q, h, want);
    while ((r = rh_query_step(q, h)) == RH_RUNNING) {}
    return r;
}

static uint8_t run_move(const RcHistory *h, RcMove move, RhQuery *q) {
    uint8_t r;
    move_begin(q, move, (uint8_t)(BOTH_CLAIMS | NEED_COUNT));
    while ((r = move_step(q, h)) == RH_RUNNING) {}
    return r;
}

uint8_t rh_repetitions(const RcHistory *h, RhQuery *q) {
    run_query(h, q, 0);
    return q->reps;
}

uint8_t rh_claims_now(const RcHistory *h, RhQuery *q) {
    run_query(h, q, 0);
    return q->claims_now;
}

void rh_judge(const RcHistory *h, RhQuery *q, RcJudgment *out) {
    if (run_query(h, q, 0) == RH_ERROR) {
        out->terminal = 255u;
        out->claimable = 0;
        return;
    }
    *out = q->judgment;
}

uint8_t rh_claims_any(const RcHistory *h, RhQuery *q) {
    return run_query(h, q, RH_WANT_ANY) == RH_ERROR ? 255u : q->claims_any;
}

uint8_t rh_repetitions_after(const RcHistory *h, RcMove move, RhQuery *q) {
    run_move(h, move, q);
    return q->m_reps;
}

uint8_t rh_claims_for_move(const RcHistory *h, RcMove move, RhQuery *q) {
    return run_move(h, move, q) == RH_ERROR ? 255u : q->m_claims;
}
