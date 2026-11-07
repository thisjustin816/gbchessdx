#ifndef GB_CHESS_RULES_STATUS_H
#define GB_CHESS_RULES_STATUS_H
#include "rules_core.h"
#define RC_PLAYING 0
#define RC_MATE 1
#define RC_STALEMATE 2
#define RC_DEAD 3
#define RC_DRAW_75 4
#define RC_DRAW_FIVEFOLD 5
#define RC_CLAIM_FIFTY 1
#define RC_CLAIM_THREEFOLD 2
typedef struct {uint8_t terminal,claimable;} RcJudgment;
/* repetition_count is supplied by the history layer, including this state;
 * this covers claims AFTER a move, not intended-move claim procedures.
 * Dead-position recognition deliberately covers only certain cases.
 */
RcJudgment rc_judge(const RcPosition *p,uint8_t repetition_count,RcMove *moves,uint16_t cap,RcPosition *scratch);
uint8_t rc_recognized_dead(const RcPosition *p);
#endif
