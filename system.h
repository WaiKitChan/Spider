/* CARD */
typedef struct _card {
	BYTE suit;
	BYTE rank;
} CARD,*PCARD;

/* POOL */
PCARD CreatePool() {
	return(PCARD)malloc(sizeof(CARD)*CARD_MAX);
}
void DestroyPool(PCARD pool) {
	free(pool);
}
void InitPool(PCARD pool,BYTE deck,BYTE suit) {
	int i,j,k,n=0;
	for(i=0;i<deck;++i){
		for(j=0;j<FULLRANK;++j){
			for(k=0;k<FULLSUIT;++k){
				pool[n++]=(CARD){k%suit,j};
			}
		}
	}
	
	CARD mem;
	for(i=deck*FULLDECK-1;i>0;--i){
		j=rand()%i;
		mem=pool[i];
		pool[i]=pool[j];
		pool[j]=mem;
	}
}

/* PILE */
typedef struct _pile {
	BYTE size;
	BYTE down;
	BYTE chain;
	CARD card[PILE_MAX];
	RECT rc;
	RECT rcDock;
	HDC hdc;
	HGDIOBJ hOld;
} PILE,*PPILE;
PPILE CreatePiles(HDC hdc) {
	int i;
	PPILE pPile=(PPILE)malloc(sizeof(PILE)*PILE_NUM);
	HBITMAP hbm;
	for(i=0;i<PILE_NUM;++i){
		pPile[i].hdc=CreateCompatibleDC(hdc);
		hbm=CreateCompatibleBitmap(hdc,CARD_WIDTH,CARD_HEIGHT+CARD_SPACE*(PILE_MAX-1));
		pPile[i].hOld=SelectObject(pPile[i].hdc,hbm);
		pPile[i].rc.left=G_PADDING+(CARD_WIDTH+CARD_SPACE)*i;
		pPile[i].rc.right=pPile[i].rc.left+CARD_WIDTH;
		pPile[i].rc.top=G_PADDING;
		pPile[i].rc.bottom=G_PADDING;
		pPile[i].rcDock=pPile[i].rc;
		pPile[i].rcDock.left-=CARD_SPACE/2;
		pPile[i].rcDock.right+=CARD_SPACE/2;
	}
	return pPile;
}
void DestroyPiles(PPILE pPile) {
	int i;
	for(i=0;i<PILE_NUM;++i){
		DeleteObject(SelectObject(pPile[i].hdc,pPile[i].hOld));
		DeleteDC(pPile[i].hdc);
	}
	free(pPile);
}
BOOL CountChain(PPILE pile) {
	int i;
	CARD mem;
	i=pile->size-1;
	mem=pile->card[i];
	BYTE suit=mem.suit,rank=mem.rank;
	
	pile->chain=0;
	do{
		++pile->chain;
		mem=pile->card[--i];
	}while(mem.suit==suit&&mem.rank==++rank&&i>=pile->down);
	return pile->chain==FULLRANK;
}

/* DEAL */
typedef struct _deal {
	BYTE remain;
	RECT rc;
	RECT rcHit;
	HDC hdc;
	HGDIOBJ hOld;
} DEAL,*PDEAL;
PDEAL CreateDeal(HDC hdc) {
	PDEAL deal=(PDEAL)malloc(sizeof(DEAL));
	deal->rc.right=G_WIDTH-G_PADDING;
	deal->rc.left=deal->rc.right-CARD_WIDTH;
	deal->rc.bottom=G_HEIGHT(deck)-G_PADDING;
	deal->rc.top=deal->rc.bottom-CARD_HEIGHT;
	deal->rcHit=deal->rc;
	deal->hdc=CreateCompatibleDC(hdc);
	HBITMAP hbm=CreateCompatibleBitmap(hdc,CARD_WIDTH+CARD_SPACE*(DEAL_NUM-1),CARD_HEIGHT);
	deal->hOld=SelectObject(deal->hdc,hbm);
	return deal;
}
void DestroyDeal(PDEAL deal) {
	DeleteObject(SelectObject(deal->hdc,deal->hOld));
	DeleteDC(deal->hdc);
	free(deal);
}

/* STACK */
typedef struct _stack {
	BYTE size;
	BYTE goal;
	BYTE suit[SUIT_MAX];
	RECT rc;
	HDC hdc;
	HGDIOBJ hOld;
	BOOL victory;
	POINT pt[CARD_MAX];
	POINT ptv[CARD_MAX];
	HDC hdcVictory;
	HGDIOBJ hOldV;
} STACK,*PSTACK;
PSTACK CreateStack(HDC hdc) {
	PSTACK stack=(PSTACK)malloc(sizeof(STACK));
	stack->rc.left=G_PADDING;
	stack->rc.right=G_PADDING;
	stack->rc.bottom=G_HEIGHT(deck)-G_PADDING;
	stack->rc.top=stack->rc.bottom-CARD_HEIGHT;
	stack->hdc=CreateCompatibleDC(hdc);
	HBITMAP hbm=CreateCompatibleBitmap(hdc,CARD_WIDTH+CARD_SPACE*(SUIT_MAX-1),CARD_HEIGHT);
	stack->hOld=SelectObject(stack->hdc,hbm);
	
	stack->hdcVictory=CreateCompatibleDC(hdc);
	HBITMAP hbmV=CreateCompatibleBitmap(hdc,G_WIDTH,G_HEIGHT(deck));
	stack->hOldV=SelectObject(stack->hdcVictory,hbmV);
	return stack;
}
void DestroyStack(PSTACK stack) {
	DeleteObject(SelectObject(stack->hdc,stack->hOld));
	DeleteDC(stack->hdc);
	DeleteObject(SelectObject(stack->hdcVictory,stack->hOldV));
	DeleteDC(stack->hdcVictory);
	free(stack);
}

/* HOLD */
typedef struct _hold {
	BOOL active;
	BYTE size;
	PPILE owner;
	CARD card[FULLRANK-1];
	POINT pt;
	RECT rc;
	BOOL afterimage;
	HDC hdc;
	HGDIOBJ hOld;
} HOLD,*PHOLD;
PHOLD CreateHold(HDC hdc) {
	PHOLD hold=(PHOLD)malloc(sizeof(HOLD));
	hold->active=FALSE;
	hold->afterimage=FALSE;
	hold->hdc=CreateCompatibleDC(hdc);
	HBITMAP hbm=CreateCompatibleBitmap(hdc,CARD_WIDTH,CARD_HEIGHT+CARD_SPACE*(FULLRANK-2));
	hold->hOld=SelectObject(hold->hdc,hbm);
	return hold;
}
void DestroyHold(PHOLD hold) {
	DeleteObject(SelectObject(hold->hdc,hold->hOld));
	DeleteDC(hold->hdc);
	free(hold);
}
RECT MoveHold(PHOLD hold,POINT pt) {
	long dx=pt.x-hold->pt.x,dy=pt.y-hold->pt.y;
	RECT rc=hold->rc;
	hold->rc.left+=dx;
	hold->rc.right+=dx;
	hold->rc.top+=dy;
	hold->rc.bottom+=dy;
	hold->pt=pt;
	if(hold->afterimage)return hold->rc;
	rc.left+=dx>0?0:dx;
	rc.top+=dy>0?0:dy;
	rc.right+=dx>0?dx:0;
	rc.bottom+=dy>0?dy:0;
	return rc;
}

/* TEXTURE */
typedef struct _texture {
	HDC bg;
	HDC card;
	HDC back;
	HDC mcard;
	HDC mback;
	HDC mask;
	HGDIOBJ hOldbg;
	HGDIOBJ hOldcard;
	HGDIOBJ hOldback;
	HGDIOBJ hOldmcard;
	HGDIOBJ hOldmback;
	HGDIOBJ hOldmask;
} TEXTURE,*PTEXTURE;
void UpdateBackground(PTEXTURE texture,HINSTANCE hInst,LPCSTR lpbg) {
	HBITMAP hbmbg=LoadBitmap(hInst,lpbg);
	HBRUSH hbr=CreatePatternBrush(hbmbg);
	DeleteObject(hbmbg);
	RECT rc={0,0,G_WIDTH,G_HEIGHT(deck)};
	FillRect(texture->bg,&rc,hbr);
	DeleteObject(hbr);
}
void UpdateCard(PTEXTURE texture,COLORREF base,COLORREF back) {
	HBRUSH hbr=CreateSolidBrush(base);
	RECT rc={0,0,CARD_WIDTH*FULLRANK,CARD_HEIGHT*FULLSUIT};
	FillRect(texture->card,&rc,hbr);
	rc=(RECT){0,0,CARD_WIDTH,CARD_HEIGHT};
	FillRect(texture->back,&rc,hbr);
	DeleteObject(hbr);
	
	int i,j;
	for(i=0;i<FULLRANK;++i)for(j=0;j<FULLSUIT;++j)BitBlt(texture->card,CARD_WIDTH*i,CARD_HEIGHT*j,CARD_WIDTH,CARD_HEIGHT,texture->mask,0,0,SRCAND);
	BitBlt(texture->card,0,0,CARD_WIDTH*FULLRANK,CARD_HEIGHT*FULLSUIT,texture->mcard,0,0,SRCPAINT);
	
	BitBlt(texture->back,0,0,CARD_WIDTH,CARD_HEIGHT,texture->mask,0,0,SRCAND);
	hbr=CreateSolidBrush(back);
	rc=(RECT){2,2,CARD_WIDTH-2,CARD_HEIGHT-2};
	FillRect(texture->back,&rc,hbr);
	DeleteObject(hbr);
	BitBlt(texture->back,0,0,CARD_WIDTH,CARD_HEIGHT,texture->mback,0,0,SRCPAINT);
}
PTEXTURE CreateTexture(HDC hdc,HINSTANCE hInst,LPCSTR lpbg,LPCSTR lpcard,LPCSTR lpback,LPCSTR lpmask,COLORREF base,COLORREF back) {
	PTEXTURE texture=(PTEXTURE)malloc(sizeof(TEXTURE));
	texture->bg=CreateCompatibleDC(hdc);
	texture->card=CreateCompatibleDC(hdc);
	texture->back=CreateCompatibleDC(hdc);
	texture->mcard=CreateCompatibleDC(hdc);
	texture->mback=CreateCompatibleDC(hdc);
	texture->mask=CreateCompatibleDC(hdc);
	HBITMAP hbmbg,hbmcard,hbmback,hbmmask,hbm;
	hbmbg=LoadBitmap(hInst,lpbg);
	hbmcard=LoadBitmap(hInst,lpcard);
	hbmback=LoadBitmap(hInst,lpback);
	hbmmask=LoadBitmap(hInst,lpmask);
	hbm=CreateCompatibleBitmap(hdc,G_WIDTH,G_HEIGHT(4));
	texture->hOldbg=SelectObject(texture->bg,hbm);
	hbm=CreateCompatibleBitmap(hdc,CARD_WIDTH*FULLRANK,CARD_HEIGHT*FULLSUIT);
	texture->hOldcard=SelectObject(texture->card,hbm);
	hbm=CreateCompatibleBitmap(hdc,CARD_WIDTH,CARD_HEIGHT);
	texture->hOldback=SelectObject(texture->back,hbm);
	texture->hOldmcard=SelectObject(texture->mcard,hbmcard);
	texture->hOldmback=SelectObject(texture->mback,hbmback);
	texture->hOldmask=SelectObject(texture->mask,hbmmask);
	
	RECT rc={0,0,G_WIDTH,G_HEIGHT(4)};
	HBRUSH hbr=CreatePatternBrush(hbmbg);
	DeleteObject(hbmbg);
	FillRect(texture->bg,&rc,hbr);
	DeleteObject(hbr);
	
	UpdateCard(texture,base,back);
	
	return texture;
}
void DestroyTexture(PTEXTURE texture) {
	DeleteObject(SelectObject(texture->bg,texture->hOldbg));
	DeleteObject(SelectObject(texture->card,texture->hOldcard));
	DeleteObject(SelectObject(texture->back,texture->hOldback));
	DeleteObject(SelectObject(texture->mcard,texture->hOldmcard));
	DeleteObject(SelectObject(texture->mback,texture->hOldmback));
	DeleteObject(SelectObject(texture->mask,texture->hOldmask));
	DeleteDC(texture->bg);
	DeleteDC(texture->card);
	DeleteDC(texture->back);
	DeleteDC(texture->mcard);
	DeleteDC(texture->mback);
	DeleteDC(texture->mask);
	free(texture);
}
const COLORREF base[7]={COLOR_LBLUE,COLOR_LRED,COLOR_LGREEN,COLOR_LPURPLE,COLOR_LCYAN,COLOR_LYELLOW,COLOR_LSILVER};
const COLORREF back[7]={COLOR_BLUE,COLOR_RED,COLOR_GREEN,COLOR_PURPLE,COLOR_CYAN,COLOR_YELLOW,COLOR_SILVER};

/* INTERFACE FUNCTIONS */
void HoldCard(PHOLD hold,PPILE from,int num,POINT pt,PTEXTURE texture) {
	if(num>from->chain)return;
	int i,offset=from->size-num;
	hold->active=TRUE;
	hold->owner=from;
	hold->size=num;
	hold->rc.left=from->rc.left;
	hold->rc.right=from->rc.right;
	hold->rc.bottom=from->rc.bottom;
	hold->rc.top=hold->rc.bottom-CARD_HEIGHT-CARD_SPACE*(num-1);
	hold->pt=pt;
	for(i=0;i<num;++i){
		hold->card[i]=from->card[offset+i];
		BitBlt(hold->hdc,0,CARD_SPACE*i,CARD_WIDTH,CARD_HEIGHT,from->hdc,0,CARD_SPACE*(offset+i),SRCCOPY);
	}
	
	if(from->size==num)from->rc.bottom=from->rc.top;
	else {
		offset=from->size-num-1;
		from->rc.bottom-=CARD_SPACE*num;
		CARD mem=from->card[offset];
		if(offset<from->down)BitBlt(from->hdc,0,CARD_SPACE*offset,CARD_WIDTH,CARD_HEIGHT,texture->back,0,0,SRCCOPY);
		else BitBlt(from->hdc,0,CARD_SPACE*offset,CARD_WIDTH,CARD_HEIGHT,texture->card,mem.rank*CARD_WIDTH,mem.suit*CARD_HEIGHT,SRCCOPY);
	}
}
void ReleaseCard(PHOLD hold,PTEXTURE texture) {
	if(!hold->active)return;
	hold->active=FALSE;
	
	PPILE from=hold->owner;
	if(from->size==hold->size)from->rc.bottom=from->rc.top+CARD_SPACE*(hold->size-1)+CARD_HEIGHT;
	else {
		int i,offset=from->size-hold->size;
		CARD mem;
		from->rc.bottom+=CARD_SPACE*hold->size;
		for(i=0;i<hold->size;++i){
			mem=from->card[offset+i];
			BitBlt(from->hdc,0,CARD_SPACE*(offset+i),CARD_WIDTH,CARD_HEIGHT,texture->card,mem.rank*CARD_WIDTH,mem.suit*CARD_HEIGHT,SRCCOPY);
		}
	}
}
void SuitComplete(PPILE pile,PSTACK stack,PTEXTURE texture) {
	BYTE suit=pile->card[pile->size-1].suit;
	stack->suit[stack->size]=suit;
	if(stack->size)stack->rc.right+=CARD_SPACE;
	else stack->rc.right+=CARD_WIDTH;
	BitBlt(stack->hdc,CARD_SPACE*stack->size++,0,CARD_WIDTH,CARD_HEIGHT,texture->card,0,CARD_HEIGHT*suit,SRCCOPY);
	if(stack->size==stack->goal)stack->victory=TRUE;
	
	pile->size-=FULLRANK;
	pile->rcDock.top-=CARD_SPACE*FULLRANK;
	pile->rcDock.bottom-=CARD_SPACE*FULLRANK;
	if(pile->size){
		if(pile->size==pile->down)--pile->down;
		CountChain(pile);
		pile->rc.bottom-=CARD_SPACE*FULLRANK;
	} else {
		pile->chain=0;
		pile->rc.bottom=G_PADDING;
	}
	CARD mem=pile->card[pile->size-1];
	BitBlt(pile->hdc,0,CARD_SPACE*(pile->size-1),CARD_WIDTH,CARD_HEIGHT,texture->card,mem.rank*CARD_WIDTH,mem.suit*CARD_HEIGHT,SRCCOPY);
}
void NewDeal(PDEAL deal,PCARD pool,PPILE pPile,PSTACK stack,PTEXTURE texture) {
	int i;
	for(i=0;i<PILE_NUM;++i)if(!pPile[i].size||pPile[i].size==PILE_MAX)return;
	int offset=DEAL_SIZE*--deal->remain;
	for(i=0;i<DEAL_SIZE;++i){
		pPile[i].card[pPile[i].size]=pool[offset+i];
		BitBlt(pPile[i].hdc,0,CARD_SPACE*pPile[i].size++,CARD_WIDTH,CARD_HEIGHT,texture->card,pool[offset+i].rank*CARD_WIDTH,pool[offset+i].suit*CARD_HEIGHT,SRCCOPY);
		pPile[i].rc.bottom+=CARD_SPACE;
		pPile[i].rcDock.top+=CARD_SPACE;
		pPile[i].rcDock.bottom+=CARD_SPACE;
		if(CountChain(&pPile[i]))SuitComplete(&pPile[i],stack,texture);
	}
	
	if(deal->remain){
		deal->rc.left+=CARD_SPACE;
		BitBlt(deal->hdc,CARD_SPACE*(deal->remain-1),0,CARD_WIDTH,CARD_HEIGHT,texture->back,0,0,SRCCOPY);
	} else {
		deal->rc.left=deal->rc.right;
		deal->rcHit.left=deal->rcHit.right;
	}
}
void MoveCard(PHOLD hold,PPILE to,PSTACK stack,PTEXTURE texture) {
	int hsize=hold->size,tsize=to->size;
	long space=CARD_SPACE*hsize;
	if(tsize+hsize>PILE_MAX||hold->owner==to){
		ReleaseCard(hold,texture);
		return;
	} else if(tsize){
		if(to->card[tsize-1].rank!=hold->card[0].rank+1){
			ReleaseCard(hold,texture);
			return;
		}
	}
	hold->active=FALSE;
	
	to->rcDock.top+=space;
	to->rcDock.bottom+=space;
	if(!tsize){
		to->chain=hsize;
		to->rc.bottom+=CARD_HEIGHT+CARD_SPACE*(hsize-1);
	} else {
		if(to->card[tsize-1].suit==hold->card[0].suit)to->chain+=hsize;
		else to->chain=hsize;
		to->rc.bottom+=space;
	}
	to->size+=hsize;
	
	int i;
	CARD mem;
	for(i=hsize;i>0;--i)to->card[to->size-i]=hold->card[hsize-i];
	for(i=0;i<hsize;++i){
		mem=to->card[tsize+i];
		BitBlt(to->hdc,0,CARD_SPACE*(tsize+i),CARD_WIDTH,CARD_HEIGHT,texture->card,mem.rank*CARD_WIDTH,mem.suit*CARD_HEIGHT,SRCCOPY);
	}
	
	if(to->chain==FULLRANK)SuitComplete(to,stack,texture);
	
	PPILE from=hold->owner;
	from->size-=hsize;
	from->rcDock.top-=space;
	from->rcDock.bottom-=space;
	if(!from->size)from->chain=0;
	else {
		if(from->size==from->down){
			mem=from->card[--from->down];
			BitBlt(from->hdc,0,CARD_SPACE*from->down,CARD_WIDTH,CARD_HEIGHT,texture->card,mem.rank*CARD_WIDTH,mem.suit*CARD_HEIGHT,SRCCOPY);
		}
		CountChain(from);
	}
}
void UpdateTexture(PTEXTURE texture,PPILE pPile,PDEAL deal,PSTACK stack,PHOLD hold) {
	if(hold->active){
		ReleaseCard(hold,texture);
		hold->active=FALSE;
	}
	int i,j;
	for(i=0;i<PILE_NUM;++i)for(j=0;j<pPile[i].size;++j){
		if(j<pPile[i].down)BitBlt(pPile[i].hdc,0,CARD_SPACE*j,CARD_WIDTH,CARD_HEIGHT,texture->back,0,0,SRCCOPY);
		else BitBlt(pPile[i].hdc,0,CARD_SPACE*j,CARD_WIDTH,CARD_HEIGHT,texture->card,pPile[i].card[j].rank*CARD_WIDTH,pPile[i].card[j].suit*CARD_HEIGHT,SRCCOPY);
	}
	for(i=0;i<deal->remain;++i)BitBlt(deal->hdc,CARD_SPACE*i,0,CARD_WIDTH,CARD_HEIGHT,texture->back,0,0,SRCCOPY);
	for(i=0;i<stack->size;++i)BitBlt(stack->hdc,CARD_SPACE*i,0,CARD_WIDTH,CARD_HEIGHT,texture->card,0,stack->suit[i]*CARD_HEIGHT,SRCCOPY);
}

/* LET THE PARTY BEGIN */
void InitGame(BYTE deck,BYTE suit,PCARD pool,PPILE piles,PDEAL deal,PSTACK stack,PHOLD hold,PTEXTURE texture) {
	InitPool(pool,deck,suit);
	
	int deal_num=DEAL_PER2DECK*deck/2,card_num=FULLDECK*deck;
	int i,j=0,k=deal_num*DEAL_SIZE;
	for(i=0;i<PILE_NUM;++i)piles[i].size=0;
	i=0;
	while(k<card_num){
		if(j==PILE_NUM){
			++i;
			j=0;
		}
		++piles[j].size;
		BitBlt(piles[j].hdc,0,CARD_SPACE*i,CARD_WIDTH,CARD_HEIGHT,texture->back,0,0,SRCCOPY);
		piles[j++].card[i]=pool[k++];
	}
	CARD mem;
	for(i=0;i<PILE_NUM;++i){
		piles[i].down=piles[i].size-1;
		piles[i].chain=1;
		mem=piles[i].card[piles[i].down];
		BitBlt(piles[i].hdc,0,CARD_SPACE*piles[i].down,CARD_WIDTH,CARD_HEIGHT,texture->card,mem.rank*CARD_WIDTH,mem.suit*CARD_HEIGHT,SRCCOPY);
		piles[i].rc.bottom=piles[i].rc.top+CARD_HEIGHT+CARD_SPACE*piles[i].down;
		piles[i].rcDock.top=piles[i].rc.top+CARD_SPACE*piles[i].down;
		piles[i].rcDock.bottom=piles[i].rcDock.top+CARD_HEIGHT;
	}
	
	deal->remain=deal_num;
	deal->rc.left=deal->rc.right-CARD_WIDTH-CARD_SPACE*(deal_num-1);
	deal->rcHit.left=deal->rc.right-CARD_WIDTH;
	for(i=0;i<deal_num;++i)BitBlt(deal->hdc,CARD_SPACE*i,0,CARD_WIDTH,CARD_HEIGHT,texture->back,0,0,SRCCOPY);
	
	stack->size=0;
	stack->goal=FULLSUIT*deck;
	stack->victory=FALSE;
	stack->rc.right=stack->rc.left;
	
	hold->active=FALSE;
}

void Victory(HWND hwnd,PSTACK stack,PTEXTURE texture) {
	BitBlt(stack->hdcVictory,0,0,G_WIDTH,G_HEIGHT(deck),texture->bg,0,0,SRCCOPY);
	BitBlt(stack->hdcVictory,stack->rc.left,stack->rc.top,stack->rc.right-stack->rc.left,CARD_HEIGHT,stack->hdc,0,0,SRCCOPY);
	int i,suit_num=stack->goal;
	stack->goal*=FULLRANK;
	stack->size=stack->goal;
	BYTE mem[suit_num];
	for(i=0;i<suit_num;++i)mem[i]=stack->suit[i];
	for(i=0;i<suit_num;++i)stack->suit[i]=mem[suit_num-1-i];
	SetTimer(hwnd,TIMER_LAUNCH,300,NULL);
}
WORD Launch(PSTACK stack) {
	WORD card=stack->goal-stack->size;
	if(stack->size--==0)return (WORD)-1;
	stack->pt[card].x=stack->rc.left+CARD_SPACE*(stack->size/FULLRANK)+CARD_WIDTH/2;
	stack->pt[card].y=stack->rc.bottom-CARD_HEIGHT/2;
	stack->ptv[card].x=VXMIN+rand()%VXRNG;
	stack->ptv[card].y=VYMIN+rand()%VYRNG;
	if(!stack->ptv[card].x)stack->ptv[card].x=-8;
	while(stack->ptv[card].x>-4&&stack->ptv[card].x<8)stack->ptv[card].x*=2;
	while(stack->ptv[card].x*stack->ptv[card].x+stack->ptv[card].y*stack->ptv[card].y<KEMIN)stack->ptv[card].x=stack->ptv[card].x*3/2;
	if(stack->ptv[card].x*stack->ptv[card].x+stack->ptv[card].y*stack->ptv[card].y>KEMAX)stack->ptv[card].x=stack->ptv[card].x*2/3;
	return card;
}
BOOL Physics(WORD card,HWND hwnd,PSTACK stack,PTEXTURE texture) {
	RECT rc={stack->pt[card].x-CARD_WIDTH/2,stack->pt[card].y-CARD_HEIGHT/2,stack->pt[card].x+CARD_WIDTH/2,stack->pt[card].y+CARD_HEIGHT/2};
	BitBlt(stack->hdcVictory,rc.left,rc.top,CARD_WIDTH,CARD_HEIGHT,texture->card,(card%FULLRANK)*CARD_WIDTH,stack->suit[card/FULLRANK]*CARD_HEIGHT,SRCCOPY);
	InvalidateRect(hwnd,&rc,FALSE);
	stack->pt[card].x+=stack->ptv[card].x;
	stack->pt[card].y+=stack->ptv[card].y;
	if(stack->pt[card].y>G_HEIGHT(deck)-CARD_HEIGHT/2){
		stack->pt[card].y=G_HEIGHT(deck)-CARD_HEIGHT/2;
		stack->ptv[card].y=-(stack->ptv[card].y*5/6);
	}
	stack->ptv[card].y+=GRAVITY;
	if(stack->pt[card].x<-CARD_WIDTH||stack->pt[card].x>G_WIDTH+CARD_WIDTH)return FALSE;
	else return TRUE;
}
