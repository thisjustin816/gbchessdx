#ifndef GB_CHESS_RULES_HISTORY_H
#define GB_CHESS_RULES_HISTORY_H
/* Game history on top of the main v1.3 rules core: exact repetition
 * counting, after-move and prospective (intended-move) draw claims, and
 * adjudication. Portable C99, no heap, no recursion.
 *
 * Window. Only plies since the last pawn move or capture can repeat a
 * position, and the 75-move rule ends the game at 150 such plies, so a ring
 * of RH_CAPACITY >= 151 entries keeps counting exact while a game is in
 * progress. Older entries are overwritten. Counting stays exact while the
 * halfmove clock fits in the stored entries (rh_exact); rh_takeback refuses
 * a takeback that would break that. A complete game record for saves or
 * review is a separate decision.
 *
 * Matching. Each entry keeps a 16-bit key and the effective en passant
 * square of the position after its move (the square only when a legal en
 * passant capture exists, as FIDE 9.2.3 requires). Keys skip positions that
 * cannot match; every key match is confirmed by walking back with exact
 * unmake and comparing the boards, so a key collision never changes a result.
 *
 * Cooperative queries. rh_query_begin and rh_query_step compute repetitions,
 * claims and adjudication in small steps so a caller can render and poll
 * input between them. One step does at most one of: a key comparison, one
 * unmake, one board comparison, one rules-core iterator step, one position
 * copy with its incremental key, one en passant test, one check test or
 * dead-position scan. The longest is the rules core's iterator step for a
 * king on its home square with castling rights. Intended moves are only
 * enumerated when a prospective claim is possible: a clock of at least 99,
 * or an earlier position (opponent to move) that already appeared twice.
 * Then a move is examined in full only if a 50-move claim is still wanted or
 * its key falls in a bucket that the earlier positions hit twice.
 * The history must not change while a query runs; abandoning a query needs
 * no cleanup.
 *
 * Positions before rh_begin are unknown: a start position loaded with a
 * nonzero halfmove clock counts as appearing once.
 */
#include "rules_status.h"

#ifndef RH_CAPACITY
#define RH_CAPACITY 160u
#endif
#if RH_CAPACITY < 151u || RH_CAPACITY > 250u
#error "RH_CAPACITY must cover the 150-ply 75-move window and leave uint8_t headroom"
#endif

typedef struct {
    RcMove move;
    RcUndo undo;
    uint16_t key;   /* rh_key of the position after the move */
    uint8_t ep_eff; /* rh_effective_ep of that position */
} RhEntry;

typedef struct {
    RcPosition current;
    RhEntry ring[RH_CAPACITY];
    uint16_t base_key; /* position before the oldest stored entry */
    uint8_t base_ep;
    uint8_t head;      /* next write slot */
    uint8_t stored;    /* entries available for walking back / takeback */
    uint16_t plies;    /* pushes minus takebacks since rh_begin, saturating */
} RcHistory;

/* Exact repetition counter for a target position c->distance plies after
 * h->current (0 = the current position). Keys skip candidates; each key
 * match is confirmed by walking back with exact unmake and comparing the
 * boards. The target and the history must not change while it runs. */
typedef struct {
    const RcPosition *target;
    uint16_t key;
    uint8_t ep, distance, b, walked, count, verify;
    RcPosition walk;
} RhCounter;
void rh_counter_begin(RhCounter *c, const RcHistory *h, const RcPosition *target, uint16_t key, uint8_t ep,
                      uint8_t distance);
/* One unit of work. Returns 1 when c->count (appearances, >= 1) is final. */
uint8_t rh_counter_step(RhCounter *c, const RcHistory *h);
/* Key and effective en passant square of the position b plies back from
 * h->current (b = stored is the base). */
uint16_t rh_key_at(const RcHistory *h, uint8_t b, uint8_t *ep);
/* 8-bit bucket of a (key, effective en passant) pair, for bitmap filters. */
uint8_t rh_bucket(uint16_t key, uint8_t ep);

#define RH_RUNNING 0u
#define RH_DONE 1u
#define RH_ERROR 2u

/* Query state and workspace. Results are valid after RH_DONE. The same
 * struct serves as workspace for rh_begin, rh_push and the synchronous
 * helpers; do not call those while a cooperative query is in progress. */
typedef struct {
    uint8_t reps;          /* appearances of the current position, >= 1 */
    uint8_t claims_now;    /* RC_CLAIM_* the player to move may claim now */
    uint8_t claims_any;    /* claims_now OR any intended move (RH_WANT_ANY) */
    RcJudgment judgment;   /* adjudication with this history */
    uint16_t steps;        /* steps taken by this query, saturating */
    /* private */
    uint8_t phase, want, has_legal, error, any_need;
    uint8_t m_phase, m_need, m_reps, m_claims, m_ep;
    uint8_t dup_found, scan_b, dup_once[32], dup_twice[32];
    uint16_t m_key;
    RhCounter counter;
    RcMove move, spare;
    RcPosition after, probe;
    RcIterator it, it_after;
} RhQuery;

#define RH_WANT_ANY 1u

/* Position key: pieces, side to move and castling rights. En passant is
 * compared separately through rh_effective_ep. */
uint16_t rh_key(const RcPosition *p);
/* rh_key of the position after the legal move, from the key before it. */
uint16_t rh_key_after(const RcPosition *before, uint16_t key, RcMove move);
/* The en passant square if the side to move has a legal en passant
 * capture, else RC_NONE. scratch is overwritten. */
uint8_t rh_effective_ep(const RcPosition *p, RcPosition *scratch);

void rh_begin(RcHistory *h, const RcPosition *start, RhQuery *work);
/* The move must be legal in h->current (the caller validates it with the
 * rules core). Never fails: when the ring is full the oldest entry goes. */
void rh_push(RcHistory *h, RcMove move, RhQuery *work);
/* Undo the latest stored ply. Returns 0, changing nothing, when no ply is
 * stored or when undoing it would leave a position whose repetition window
 * reaches entries the ring has already overwritten. */
uint8_t rh_takeback(RcHistory *h);
/* 1 when repetition counts and claims for h->current are exact relative to
 * rh_begin: no entry was overwritten, or the halfmove window fits in the
 * stored entries. The ring wraps only after RH_CAPACITY plies, so only a
 * game continued past the automatic 75-move draw can lose it. */
uint8_t rh_exact(const RcHistory *h);

/* Cooperative query. want: 0 or RH_WANT_ANY. Terminal priority is the
 * rules core's rc_judge order: mate, stalemate, recognized dead position,
 * 75-move, fivefold. A 50-move claim is offered only while the game
 * continues: never in a mate or stalemate, never for a move that mates or
 * stalemates. Returns RH_RUNNING, RH_DONE or RH_ERROR. */
void rh_query_begin(RhQuery *q, const RcHistory *h, uint8_t want);
uint8_t rh_query_step(RhQuery *q, const RcHistory *h);

/* Synchronous helpers: each runs the cooperative steps to completion. */
uint8_t rh_repetitions(const RcHistory *h, RhQuery *q);
/* Appearances of the position after the legal move, counting that new
 * occurrence. History is not modified. */
uint8_t rh_repetitions_after(const RcHistory *h, RcMove move, RhQuery *q);
/* RC_CLAIM_* bits now (9.2.1.2 / 9.3.2). */
uint8_t rh_claims_now(const RcHistory *h, RhQuery *q);
/* RC_CLAIM_* bits for writing this legal move as the intended move
 * (9.2.1.1 / 9.3.1). */
uint8_t rh_claims_for_move(const RcHistory *h, RcMove move, RhQuery *q);
/* rh_claims_now OR any legal intended move. 255 on generator overflow. */
uint8_t rh_claims_any(const RcHistory *h, RhQuery *q);
/* Judgment with this history; terminal 255 on generator overflow. */
void rh_judge(const RcHistory *h, RhQuery *q, RcJudgment *out);
#endif
