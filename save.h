const char *DefaultSave="spider.sav";

typedef struct _save {
	char sign1;
	char sign2;
	char deck;
	char suit;
	char recent;
	char slot1;
	char slot2;
	char slot3;
} SAVE,*PSAVE;

typedef struct _slot {
	char deal_remain;
	CARD deal_card[DEAL_SIZE*DEAL_NUM];
	char stack_size;
	char stack_goal;
	char suit_completed[SUIT_MAX];
	struct {
		char size;
		char down;
		char chain;
		CARD card[PILE_MAX];
	} pile_info[PILE_NUM];
} SLOT,*PSLOT;

void CreateNewSave(BOOL exist) {
	if(exist)SetFileAttributes(DefaultSave,FILE_ATTRIBUTE_NORMAL);
	FILE *f=fopen(DefaultSave,"wb");
	fputs("\x53\x50\x2\x4",f);
	int i;
	for(i=4;i<sizeof(SAVE)+sizeof(SLOT)*4;++i)fputc(0,f);
	SetFileAttributes(DefaultSave,FILE_ATTRIBUTE_HIDDEN);
}

void SaveOption(char deck,char suit) {
	FILE *f=fopen(DefaultSave,"rb+");
	fseek(f,2,SEEK_SET);
	fputc(deck,f);
	fputc(suit,f);
	fclose(f);
}

void SaveSlot(int slot_num,PCARD pool,PDEAL deal,PSTACK stack,PPILE piles) {
	int i,j;
	SLOT slot;
	slot.deal_remain=deal->remain;
	for(i=0;i<DEAL_SIZE*DEAL_NUM;++i)slot.deal_card[i]=pool[i];
	slot.stack_size=stack->size;
	slot.stack_goal=stack->goal;
	for(i=0;i<SUIT_MAX;++i)slot.suit_completed[i]=stack->suit[i];
	for(i=0;i<PILE_NUM;++i){
		slot.pile_info[i].size=piles[i].size;
		slot.pile_info[i].down=piles[i].down;
		slot.pile_info[i].chain=piles[i].chain;
		for(j=0;j<PILE_MAX;++j)slot.pile_info[i].card[j]=piles[i].card[j];
	}
	char *data=(char*)&slot;
	
	FILE *f=fopen(DefaultSave,"rb+");
	fseek(f,4+slot_num,SEEK_SET);
	fputc(1,f);
	fseek(f,sizeof(SAVE)+sizeof(SLOT)*slot_num,SEEK_SET);
	for(i=0;i<sizeof(SLOT);++i)fputc(*(data+i),f);
	fclose(f);
}

void LoadSlot(int slot_num,PCARD pool,PDEAL deal,PSTACK stack,PPILE piles,PHOLD hold,PTEXTURE texture) {
	int i,j;
	FILE *f=fopen(DefaultSave,"rb+");
	fseek(f,2,SEEK_SET);
	deck=(BYTE)fgetc(f);
	fseek(f,sizeof(SAVE)+sizeof(SLOT)*slot_num,SEEK_SET);
	char data[sizeof(SLOT)];
	for(i=0;i<sizeof(SLOT);++i)data[i]=fgetc(f);
	PSLOT slot=(PSLOT)data;
	
	deal->remain=slot->deal_remain;
	if(deal->remain){
		deal->rc.left=deal->rc.right-CARD_WIDTH-CARD_SPACE*(deal->remain-1);
		deal->rcHit.left=deal->rc.right-CARD_WIDTH;
	} else {
		deal->rc.left=deal->rc.right;
		deal->rcHit.left=deal->rc.right;
	}
	for(i=0;i<DEAL_SIZE*DEAL_NUM;++i)pool[i]=slot->deal_card[i];
	
	stack->size=slot->stack_size;
	if(stack->size)stack->rc.right=stack->rc.left+CARD_WIDTH+CARD_SPACE*(stack->size-1);
	else stack->rc.right=stack->rc.left;
	stack->goal=slot->stack_goal;
	for(i=0;i<SUIT_MAX;++i)stack->suit[i]=slot->suit_completed[i];
	stack->victory=FALSE;
	
	for(i=0;i<PILE_NUM;++i){
		piles[i].size=slot->pile_info[i].size;
		if(piles[i].size){
			piles[i].rc.bottom=piles[i].rc.top+CARD_HEIGHT+CARD_SPACE*(piles[i].size-1);
			piles[i].rcDock.bottom=piles[i].rc.bottom;
			piles[i].rcDock.top=piles[i].rc.bottom-CARD_HEIGHT;
		} else {
			piles[i].rc.bottom=piles[i].rc.top;
			piles[i].rcDock.top=piles[i].rc.top;
			piles[i].rcDock.bottom=piles[i].rc.top+CARD_HEIGHT;
		}
		piles[i].down=slot->pile_info[i].down;
		piles[i].chain=slot->pile_info[i].chain;
		for(j=0;j<PILE_MAX;++j)piles[i].card[j]=slot->pile_info[i].card[j];
	}
	
	UpdateTexture(texture,piles,deal,stack,hold);
}
