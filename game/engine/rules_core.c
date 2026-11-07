#include "rules_core.h"
#include <string.h>

#define RC_ON(x) ((x)>=0 && (x)<128 && !((x)&0x88))
#define RC_TYPE(x) ((x)&7)
#define RC_COLOR(x) ((x)&8)
static const int8_t n_dirs[8]={-33,-31,-18,-14,14,18,31,33};
static const int8_t k_dirs[8]={-17,-16,-15,-1,1,15,16,17};
static const int8_t b_dirs[4]={-17,-15,15,17};
static const int8_t r_dirs[4]={-16,-1,1,16};
#ifdef RC_STEP_PROBE
unsigned long rc_probe_attack_tests;
#define RC_PROBE_ATTACK() (++rc_probe_attack_tests)
#else
#define RC_PROBE_ATTACK() ((void)0)
#endif

void rc_init(RcPosition *p){
 uint8_t i;
 memset(p,0,sizeof(*p));
 for(i=0;i<8;i++){
  p->board[16+i]=RC_PAWN;
  p->board[96+i]=RC_BLACK|RC_PAWN;
 }
 p->board[0]=p->board[7]=RC_ROOK;
 p->board[1]=p->board[6]=RC_KNIGHT;
 p->board[2]=p->board[5]=RC_BISHOP;
 p->board[3]=RC_QUEEN;p->board[4]=RC_KING;
 p->board[112]=p->board[119]=RC_BLACK|RC_ROOK;
 p->board[113]=p->board[118]=RC_BLACK|RC_KNIGHT;
 p->board[114]=p->board[117]=RC_BLACK|RC_BISHOP;
 p->board[115]=RC_BLACK|RC_QUEEN;p->board[116]=RC_BLACK|RC_KING;
 p->castle=15;p->ep=RC_NONE;p->fullmove=1;
}
uint8_t rc_attacked(const RcPosition *p,uint8_t sq,uint8_t by){
 uint8_t i;int t;const uint8_t *b=p->board;
 RC_PROBE_ATTACK();
 for(i=0;i<2;i++){
  t=(int)sq+(by==RC_WHITE?(i?-15:-17):(i?15:17));
  if(RC_ON(t)&&b[t]==(uint8_t)(by|RC_PAWN))return 1;
 }
 for(i=0;i<8;i++){
  t=(int)sq-n_dirs[i];
  if(RC_ON(t)&&b[t]==(uint8_t)(by|RC_KNIGHT))return 1;
  t=(int)sq-k_dirs[i];
  if(RC_ON(t)&&b[t]==(uint8_t)(by|RC_KING))return 1;
 }
 for(i=0;i<4;i++){
  for(t=(int)sq+r_dirs[i];RC_ON(t);t+=r_dirs[i]){
   if(b[t]){if(b[t]==(uint8_t)(by|RC_ROOK)||b[t]==(uint8_t)(by|RC_QUEEN))return 1;break;}
  }
  for(t=(int)sq+b_dirs[i];RC_ON(t);t+=b_dirs[i]){
   if(b[t]){if(b[t]==(uint8_t)(by|RC_BISHOP)||b[t]==(uint8_t)(by|RC_QUEEN))return 1;break;}
  }
 }
 return 0;
}
uint8_t rc_in_check(const RcPosition *p,uint8_t color){
 uint8_t i;for(i=0;i<128;i++)if(!(i&0x88)&&p->board[i]==(uint8_t)(color|RC_KING))return rc_attacked(p,i,color^RC_BLACK);
 return 1; /* missing king is always invalid */
}
void rc_apply(const RcPosition *p,RcMove m,RcPosition *next){
 uint8_t piece=p->board[m.from],color=RC_COLOR(piece),cap=p->board[m.to],type=RC_TYPE(piece);
 *next=*p;
 next->board[m.from]=0;
 next->board[m.to]=m.promotion?(uint8_t)(color|m.promotion):piece;
 if(m.flags&RC_EP){
  uint8_t capture=(uint8_t)(m.to+(color==RC_WHITE?-16:16));
  next->board[capture]=0;
 }
 if(m.flags&RC_CASTLE){
  uint8_t rook_from=(uint8_t)(m.to>m.from?m.from+3:m.from-4);
  uint8_t rook_to=(uint8_t)(m.to>m.from?m.from+1:m.from-1);
  next->board[rook_to]=next->board[rook_from];next->board[rook_from]=0;
 }
 if(type==RC_KING)next->castle&=(uint8_t)~(color==RC_WHITE?(RC_WK|RC_WQ):(RC_BK|RC_BQ));
 if(m.from==0||m.to==0)next->castle&=(uint8_t)~RC_WQ;
 if(m.from==7||m.to==7)next->castle&=(uint8_t)~RC_WK;
 if(m.from==112||m.to==112)next->castle&=(uint8_t)~RC_BQ;
 if(m.from==119||m.to==119)next->castle&=(uint8_t)~RC_BK;
 next->ep=RC_NONE;
 if(type==RC_PAWN){
  if(m.to==(uint8_t)(m.from+32u)||m.from==(uint8_t)(m.to+32u))
   next->ep=(uint8_t)((m.from+m.to)/2u);
 }
 next->halfmove=(type==RC_PAWN||cap||((m.flags&RC_EP)!=0))?0:(uint16_t)(p->halfmove+1);
 if(color==RC_BLACK)next->fullmove=(uint16_t)(p->fullmove+1);
 next->side=color^RC_BLACK;
}
void rc_make_inplace(RcPosition *p,RcMove m,RcUndo *undo){
 uint8_t piece=p->board[m.from],color=RC_COLOR(piece),type=RC_TYPE(piece);
 undo->captured=p->board[m.to];undo->castle=p->castle;undo->ep=p->ep;
 undo->halfmove=p->halfmove;undo->fullmove=p->fullmove;
 p->board[m.from]=0;
 p->board[m.to]=m.promotion?(uint8_t)(color|m.promotion):piece;
 if(m.flags&RC_EP){
  uint8_t at=(uint8_t)(m.to+(color==RC_WHITE?-16:16));p->board[at]=0;
 }
 if(m.flags&RC_CASTLE){
  uint8_t rf=(uint8_t)(m.to>m.from?m.from+3:m.from-4);
  uint8_t rt=(uint8_t)(m.to>m.from?m.from+1:m.from-1);
  p->board[rt]=p->board[rf];p->board[rf]=0;
 }
 if(type==RC_KING)p->castle&=(uint8_t)~(color==RC_WHITE?(RC_WK|RC_WQ):(RC_BK|RC_BQ));
 if(m.from==0||m.to==0)p->castle&=(uint8_t)~RC_WQ;
 if(m.from==7||m.to==7)p->castle&=(uint8_t)~RC_WK;
 if(m.from==112||m.to==112)p->castle&=(uint8_t)~RC_BQ;
 if(m.from==119||m.to==119)p->castle&=(uint8_t)~RC_BK;
 p->ep=RC_NONE;
 if(type==RC_PAWN){
  if(m.to==(uint8_t)(m.from+32u)||m.from==(uint8_t)(m.to+32u))p->ep=(uint8_t)((m.from+m.to)/2u);
 }
 p->halfmove=(type==RC_PAWN||undo->captured||(m.flags&RC_EP))?0:(uint16_t)(p->halfmove+1u);
 if(color==RC_BLACK)p->fullmove++;
 p->side^=RC_BLACK;
}
void rc_unmake_inplace(RcPosition *p,RcMove m,const RcUndo *undo){
 uint8_t color=(uint8_t)(p->side^RC_BLACK);
 uint8_t piece=p->board[m.to];
 p->side=color;
 p->board[m.from]=m.promotion?(uint8_t)(color|RC_PAWN):piece;
 p->board[m.to]=undo->captured;
 if(m.flags&RC_EP){
  uint8_t cap=(uint8_t)(m.to+(color==RC_WHITE?-16:16));
  p->board[cap]=(uint8_t)((color^RC_BLACK)|RC_PAWN);
 }
 if(m.flags&RC_CASTLE){
  uint8_t rf=(uint8_t)(m.to>m.from?m.from+3:m.from-4);
  uint8_t rt=(uint8_t)(m.to>m.from?m.from+1:m.from-1);
  p->board[rf]=p->board[rt];p->board[rt]=0;
 }
 p->castle=undo->castle;p->ep=undo->ep;p->halfmove=undo->halfmove;p->fullmove=undo->fullmove;
}
static uint8_t add(RcMove *out,uint16_t cap,uint16_t *count,uint8_t from,uint8_t to,uint8_t promo,uint8_t flags){
 if(*count>=cap)return 0;
 out[*count].from=from;out[*count].to=to;out[*count].promotion=promo;out[*count].flags=flags;(*count)++;
 return 1;
}
static uint8_t add_pawn(RcMove *out,uint16_t cap,uint16_t *count,uint8_t from,uint8_t to,uint8_t flags){
 uint8_t rank=(uint8_t)(to>>4),i;
 if(rank==0||rank==7){
  static const uint8_t promotions[]={RC_QUEEN,RC_ROOK,RC_BISHOP,RC_KNIGHT};
  for(i=0;i<4;i++)if(!add(out,cap,count,from,to,promotions[i],flags))return 0;
 }else if(!add(out,cap,count,from,to,0,flags))return 0;
 return 1;
}
/* castling=0 leaves castling to the caller (the iterator stages it). */
static int16_t pseudo(const RcPosition *p,RcMove *out,uint16_t capacity,int16_t only_square,uint8_t castling){
 uint16_t count=0;int from,to,dir,n,i,j;uint8_t piece,type,color,target;
 const uint8_t *b=p->board;
 for(from=only_square>=0?only_square:0;from<(only_square>=0?only_square+1:128);from++){
  if(!RC_ON(from))continue;
  piece=b[from];if(!piece||RC_COLOR(piece)!=p->side)continue;
  type=RC_TYPE(piece);color=RC_COLOR(piece);
  if(type==RC_PAWN){
   dir=color==RC_WHITE?16:-16;to=from+dir;
   if(RC_ON(to)&&!b[to]){
    if(!add_pawn(out,capacity,&count,(uint8_t)from,(uint8_t)to,0))return -1;
    n=to+dir;
    if((from>>4)==(color==RC_WHITE?1:6)&&!b[n])
     if(!add(out,capacity,&count,(uint8_t)from,(uint8_t)n,0,0))return -1;
   }
   for(i=-1;i<=1;i+=2){
    to=from+dir+i;
    if(!RC_ON(to))continue;
    target=b[to];
    if(target&&RC_COLOR(target)!=color&&RC_TYPE(target)!=RC_KING){
     if(!add_pawn(out,capacity,&count,(uint8_t)from,(uint8_t)to,0))return -1;
    }else if(to==p->ep&&!target&&b[to-dir]==(uint8_t)((color^RC_BLACK)|RC_PAWN)){
     if(!add(out,capacity,&count,(uint8_t)from,(uint8_t)to,0,RC_EP))return -1;
    }
   }
  }else if(type==RC_KING){
   for(i=0;i<8;i++){
    to=from+k_dirs[i];
    if(!RC_ON(to))continue;
    target=b[to];
    if(!target||(RC_COLOR(target)!=color&&RC_TYPE(target)!=RC_KING))
     if(!add(out,capacity,&count,(uint8_t)from,(uint8_t)to,0,0))return -1;
   }
   if(castling&&((color==RC_WHITE&&from==4)||(color==RC_BLACK&&from==116))){
    const uint8_t home=(uint8_t)from,rook=(uint8_t)(color|RC_ROOK);
    if(!rc_attacked(p,home,color^RC_BLACK)){
     if((p->castle&(color==RC_WHITE?RC_WK:RC_BK))&&b[home+3]==rook&&!b[home+1]&&!b[home+2]&&
        !rc_attacked(p,home+1,color^RC_BLACK)&&!rc_attacked(p,home+2,color^RC_BLACK))
      if(!add(out,capacity,&count,home,(uint8_t)(home+2),0,RC_CASTLE))return -1;
     if((p->castle&(color==RC_WHITE?RC_WQ:RC_BQ))&&b[home-4]==rook&&!b[home-1]&&!b[home-2]&&!b[home-3]&&
        !rc_attacked(p,home-1,color^RC_BLACK)&&!rc_attacked(p,home-2,color^RC_BLACK))
      if(!add(out,capacity,&count,home,(uint8_t)(home-2),0,RC_CASTLE))return -1;
    }
   }
  }else{
   const int8_t *dirs;
   n=type==RC_KNIGHT?8:4;
   for(j=0;j<(type==RC_QUEEN?2:1);j++){
    dirs=type==RC_KNIGHT?n_dirs:type==RC_BISHOP?b_dirs:type==RC_ROOK?r_dirs:j==0?r_dirs:b_dirs;
    for(i=0;i<n;i++){
     for(to=from+dirs[i];RC_ON(to);to+=dirs[i]){
      target=b[to];
      if(target&&RC_COLOR(target)==color)break;
      if(!target||RC_TYPE(target)!=RC_KING)
       if(!add(out,capacity,&count,(uint8_t)from,(uint8_t)to,0,0))return -1;
      if(type==RC_KNIGHT||target)break;
     }
    }
   }
  }
 }
 return (int16_t)count;
}
int16_t rc_legal(const RcPosition *p,RcMove *out,uint16_t capacity,RcPosition *scratch){
 uint16_t i,good=0;int16_t n=pseudo(p,out,capacity,-1,1);
 if(n<0)return -1;
 for(i=0;i<(uint16_t)n;i++){
  rc_apply(p,out[i],scratch);
  if(!rc_in_check(scratch,p->side))out[good++]=out[i];
 }
 return (int16_t)good;
}
uint8_t rc_same_repetition_state(const RcPosition *a,const RcPosition *b,RcMove *moves,RcPosition *scratch){
 int16_t n;uint16_t i;
 if(a->side!=b->side||a->castle!=b->castle||memcmp(a->board,b->board,128))return 0;
 if(a->ep==b->ep)return 1;
 /* An EP square matters only if there is a currently legal EP capture. */
 n=rc_legal(a,moves,RC_MAX_MOVES,scratch);if(n<0)return 0;
 for(i=0;i<(uint16_t)n;i++)if(moves[i].flags&RC_EP)return 0;
 n=rc_legal(b,moves,RC_MAX_MOVES,scratch);if(n<0)return 0;
 for(i=0;i<(uint16_t)n;i++)if(moves[i].flags&RC_EP)return 0;
 return 1;
}

void rc_iter_begin(RcIterator *it){
 it->square=0;it->offset=0;it->count=0;it->stage=0;it->king=RC_NONE;it->kscan=0;it->ray=0;it->rays=0;it->filter=RC_ITER_ALL;
}
void rc_iter_begin_filter(RcIterator *it,uint8_t filter){
 rc_iter_begin(it);
 it->filter=filter;
}
/* One ray of a bishop, rook or queen on the square just visited, in
 * pseudo()'s direction order: a queen's rook rays, then its bishop rays. */
static uint8_t ray_step(const RcPosition *p,RcIterator *it){
 const uint8_t from=(uint8_t)(it->square-1u),piece=p->board[from],color=RC_COLOR(piece),type=RC_TYPE(piece);
 const int8_t dir=type==RC_BISHOP?b_dirs[it->ray]:type==RC_ROOK?r_dirs[it->ray]:it->ray<4u?r_dirs[it->ray]:b_dirs[it->ray-4u];
 uint16_t count=it->count;
 int to;
 uint8_t target;
 for(to=from+dir;RC_ON(to);to+=dir){
  target=p->board[to];
  if(target&&RC_COLOR(target)==color)break;
  if(!target||RC_TYPE(target)!=RC_KING)
   if(!add(it->candidates,RC_PIECE_MOVES,&count,from,(uint8_t)to,0,0))return 3;
  if(target)break;
 }
 it->count=(uint8_t)count;
 it->ray++;
 return 0;
}
/* Castling for a king on its home square, one attack test per call, in the
 * order pseudo() uses: home square, then king side (f, g), then queen side
 * (d, c). Castling candidates follow the king's ordinary moves. */
static uint8_t castle_stage(const RcPosition *p,RcIterator *it){
 const uint8_t home=(uint8_t)(it->square-1u),color=p->side,enemy=(uint8_t)(color^RC_BLACK);
 const uint8_t rook=(uint8_t)(color|RC_ROOK),*b=p->board;
 uint16_t count=it->count;
 switch(it->stage){
 case 1:
  it->stage=rc_attacked(p,home,enemy)?0:2;
  return 0;
 case 2:
  if((p->castle&(color==RC_WHITE?RC_WK:RC_BK))&&b[home+3]==rook&&!b[home+1]&&!b[home+2])
   it->stage=rc_attacked(p,(uint8_t)(home+1),enemy)?4:3;
  else it->stage=4;
  return 0;
 case 3:
  it->stage=4;
  if(!rc_attacked(p,(uint8_t)(home+2),enemy)){
   if(!add(it->candidates,RC_PIECE_MOVES,&count,home,(uint8_t)(home+2),0,RC_CASTLE))return 3;
   it->count=(uint8_t)count;
  }
  return 0;
 case 4:
  if((p->castle&(color==RC_WHITE?RC_WQ:RC_BQ))&&b[home-4]==rook&&!b[home-1]&&!b[home-2]&&!b[home-3])
   it->stage=rc_attacked(p,(uint8_t)(home-1),enemy)?0:5;
  else it->stage=0;
  return 0;
 case 5:
  it->stage=0;
  if(!rc_attacked(p,(uint8_t)(home-2),enemy)){
   if(!add(it->candidates,RC_PIECE_MOVES,&count,home,(uint8_t)(home-2),0,RC_CASTLE))return 3;
   it->count=(uint8_t)count;
  }
  return 0;
 default:
  it->stage=0;
  return 0;
 }
}
uint8_t rc_iter_step(const RcPosition *p,RcIterator *it,RcPosition *scratch,RcMove *out){
 int16_t n;
 if(it->stage)return castle_stage(p,it);
 if(it->ray<it->rays)return ray_step(p,it);
 if(it->offset>=it->count){
  uint8_t sq=it->square,type;
  if(sq>=128)return 2;
  it->offset=0;it->count=0;it->ray=0;it->rays=0;
  /* Squares without a piece of the side to move yield nothing: pass over
   * them to the end of this rank in one call. */
  while((sq&0x88)||p->board[sq]==0||RC_COLOR(p->board[sq])!=p->side){
   sq++;
   if(sq>=128||!(sq&15u)){it->square=sq;return 0;}
  }
  it->square=(uint8_t)(sq+1);
  type=RC_TYPE(p->board[sq]);
  if(type==RC_BISHOP||type==RC_ROOK||type==RC_QUEEN){
   it->rays=type==RC_QUEEN?8u:4u;
   return 0;
  }
  n=pseudo(p,it->candidates,RC_PIECE_MOVES,sq,0);
  if(n<0)return 3;
  it->count=(uint8_t)n;
  if(RC_TYPE(p->board[sq])==RC_KING&&sq==(p->side==RC_WHITE?4u:116u)&&
     (p->castle&(p->side==RC_WHITE?(RC_WK|RC_WQ):(RC_BK|RC_BQ))))it->stage=1;
  return 0;
 }
 /* Before the first legality check, find the mover's king one rank per
  * call; each check then tests the king's square after the move directly,
  * which is the test rc_in_check ends in. RC_NO_KING: none, all illegal. */
 /* A filtered iterator skips the other class of candidate before any
  * legality test. Tactical: a capture, en passant or a promotion. */
 if(it->filter){
  while(it->offset<it->count){
   const RcMove c=it->candidates[it->offset];
   const uint8_t tactical=(uint8_t)(p->board[c.to]||(c.flags&RC_EP)||c.promotion);
   if((it->filter==RC_ITER_TACTICAL)==(tactical!=0))break;
   it->offset++;
  }
  if(it->offset>=it->count)return 0;
 }
 if(it->king==RC_NONE){
  uint8_t sq=(uint8_t)(it->kscan<<4),end=(uint8_t)(sq+8u),king=(uint8_t)(p->side|RC_KING);
  for(;sq<end;sq++)if(p->board[sq]==king){it->king=sq;return 0;}
  if(++it->kscan==8u)it->king=RC_NO_KING;
  return 0;
 }
 {
  const RcMove m=it->candidates[it->offset];
  const uint8_t ksq=m.from==it->king?m.to:it->king;
  it->offset++;
  rc_apply(p,m,scratch);
  if(it->king!=RC_NO_KING&&!rc_attacked(scratch,ksq,(uint8_t)(p->side^RC_BLACK))){
   *out=m;
   return 1;
  }
  return 0;
 }
}
