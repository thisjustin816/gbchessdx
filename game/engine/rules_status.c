#include "rules_status.h"
uint8_t rc_recognized_dead(const RcPosition *p){
 uint8_t count=0,all_bishops=1,colors=0,knights=0;
 uint8_t i,piece,type;
 for(i=0;i<128;i++){
  if(i&0x88)continue;
  piece=p->board[i];type=piece&7;
  if(!type||type==RC_KING)continue;
  if(type==RC_PAWN||type==RC_ROOK||type==RC_QUEEN)return 0;
  count++;
  if(type==RC_KNIGHT){knights++;all_bishops=0;}
  else if(type==RC_BISHOP){colors|=(uint8_t)(1u<<(((i&7)+(i>>4))&1));}
 }
 if(count<=1)return 1;
 return all_bishops && !knights && (colors==1||colors==2);
}
RcJudgment rc_judge(const RcPosition *p,uint8_t repetitions,RcMove *moves,uint16_t cap,RcPosition *scratch){
 RcJudgment j={RC_PLAYING,0};int16_t count=rc_legal(p,moves,cap,scratch);
 if(count<0){j.terminal=255;return j;}
 if(!count){j.terminal=rc_in_check(p,p->side)?RC_MATE:RC_STALEMATE;return j;}
 if(rc_recognized_dead(p)){j.terminal=RC_DEAD;return j;}
 if(p->halfmove>=150){j.terminal=RC_DRAW_75;return j;}
 if(repetitions>=5){j.terminal=RC_DRAW_FIVEFOLD;return j;}
 if(p->halfmove>=100)j.claimable|=RC_CLAIM_FIFTY;
 if(repetitions>=3)j.claimable|=RC_CLAIM_THREEFOLD;
 return j;
}
