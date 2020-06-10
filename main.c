#include<windows.h>
#include<windowsx.h>
#include<stdio.h>
#include<time.h>
#include"main.h"
LONG gheight;
#include"system.h"
#include"save.h"

LRESULT CALLBACK WndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	static PAINTSTRUCT ps;
	static HMENU hMenu;
	static PCARD pool;
	static PPILE piles;
	static PHOLD hold;
	static PDEAL deal;
	static PSTACK stack;
	static PTEXTURE texture;
	static BYTE deck=2,suit=4;
	switch(msg) {
		case WM_PAINT:
			BeginPaint(hwnd,&ps);
			if(stack->victory)BitBlt(ps.hdc,0,0,G_WIDTH,gheight,stack->hdcVictory,0,0,SRCCOPY);
			else {
				int i;
				BitBlt(ps.hdc,0,0,G_WIDTH,gheight,texture->bg,0,0,SRCCOPY);
				BitBlt(ps.hdc,deal->rc.left,deal->rc.top,deal->rc.right-deal->rc.left,CARD_HEIGHT,deal->hdc,0,0,SRCCOPY);
				BitBlt(ps.hdc,stack->rc.left,stack->rc.top,stack->rc.right-stack->rc.left,CARD_HEIGHT,stack->hdc,0,0,SRCCOPY);
				for(i=0;i<PILE_NUM;++i)BitBlt(ps.hdc,piles[i].rc.left,piles[i].rc.top,CARD_WIDTH,piles[i].rc.bottom-piles[i].rc.top,piles[i].hdc,0,0,SRCCOPY);
				if(hold->active)BitBlt(ps.hdc,hold->rc.left,hold->rc.top,CARD_WIDTH,hold->rc.bottom-hold->rc.top,hold->hdc,0,0,SRCCOPY);
			}
			EndPaint(hwnd,&ps);
			break;
		case WM_TIMER:
			if(!stack->victory){
				KillTimer(hwnd,wParam);
				break;
			}
			if(wParam==TIMER_LAUNCH){
				WORD card=Launch(stack);
				if(card==(WORD)-1)KillTimer(hwnd,TIMER_LAUNCH);
				else SetTimer(hwnd,TIMER_PHYSICS+card,10,NULL);
			} else {
				WORD card=wParam-TIMER_PHYSICS;
				if(card<CARD_MAX)if(!Physics(card,hwnd,stack,texture))KillTimer(hwnd,TIMER_PHYSICS+card);
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case ID_SAVE1:
				case ID_SAVE2:
				case ID_SAVE3:{
					int cmd=LOWORD(wParam-ID_SAVE);
					SaveSlot(cmd,pool,deal,stack,piles);
					EnableMenuItem(hMenu,ID_LOAD+cmd,MF_ENABLED);
					break;
				}
				case ID_LOADRECENT:
				case ID_LOAD1:
				case ID_LOAD2:
				case ID_LOAD3:{
					if(stack->victory){
						KillTimer(hwnd,TIMER_LAUNCH);
						KillTimer(hwnd,TIMER_PHYSICS);
						EnableMenuItem(GetSubMenu(hMenu,0),2,MF_BYPOSITION|MF_ENABLED);
					}
					LoadSlot(LOWORD(wParam)-ID_LOAD,pool,deal,stack,piles,hold,texture);
					/* RESIZE WINDOW */
					gheight=G_HEIGHT(deck);
					RECT rc={0,0,G_WIDTH,gheight},wrc;
					AdjustWindowRect(&rc,WS_VISIBLE|WS_OVERLAPPED|WS_CAPTION|WS_MINIMIZEBOX|WS_SYSMENU,TRUE);
					GetWindowRect(hwnd,&wrc);
					MoveWindow(hwnd,wrc.left,wrc.top,rc.right-rc.left,rc.bottom-rc.top,FALSE);
					InvalidateRect(hwnd,NULL,FALSE);
					deal->rcHit.bottom=deal->rc.bottom=stack->rc.bottom=gheight-G_PADDING;
					deal->rcHit.top=deal->rc.top=stack->rc.top=stack->rc.bottom-CARD_HEIGHT;
					break;
				}	
				case ID_2DECK:
				case ID_4DECK:{
					int i,cmd=LOWORD(wParam)-ID_0DECK;
					deck=cmd;
					for(i=2;i<=4;i+=2)CheckMenuItem(hMenu,ID_0DECK+i,i==cmd?MF_CHECKED:MF_UNCHECKED);
					break;
				}
				case ID_1SUIT:
				case ID_2SUIT:
				case ID_3SUIT:
				case ID_4SUIT:{
					int i,cmd=LOWORD(wParam)-ID_0SUIT;
					suit=cmd;
					for(i=1;i<=4;++i)CheckMenuItem(hMenu,ID_0SUIT+i,i==cmd?MF_CHECKED:MF_UNCHECKED);
					break;
				}
				case ID_START:{
					if(stack->victory){
						KillTimer(hwnd,TIMER_LAUNCH);
						KillTimer(hwnd,TIMER_PHYSICS);
						EnableMenuItem(GetSubMenu(hMenu,0),2,MF_BYPOSITION|MF_ENABLED);
					} else {
						SaveSlot(0,pool,deal,stack,piles);
						EnableMenuItem(hMenu,ID_LOADRECENT,MF_ENABLED);
					}
					InitGame(deck,suit,pool,piles,deal,stack,hold,texture);
					/* RESIZE WINDOW */
					gheight=G_HEIGHT(deck);
					RECT rc={0,0,G_WIDTH,gheight},wrc;
					AdjustWindowRect(&rc,WS_VISIBLE|WS_OVERLAPPED|WS_CAPTION|WS_MINIMIZEBOX|WS_SYSMENU,TRUE);
					GetWindowRect(hwnd,&wrc);
					MoveWindow(hwnd,wrc.left,wrc.top,rc.right-rc.left,rc.bottom-rc.top,FALSE);
					InvalidateRect(hwnd,NULL,FALSE);
					deal->rcHit.bottom=deal->rc.bottom=stack->rc.bottom=gheight-G_PADDING;
					deal->rcHit.top=deal->rc.top=stack->rc.top=stack->rc.bottom-CARD_HEIGHT;
					break;
				}	
				case ID_STANDARD:
				case ID_NIGHT:
				case ID_COSMOS:
				case ID_THINK:{
					int i,cmd=LOWORD(wParam)-ID_BACKGROUND;
					for(i=0;i<4;++i)CheckMenuItem(hMenu,ID_BACKGROUND+i,i==cmd?MF_CHECKED:MF_UNCHECKED);
					UpdateBackground(texture,GetModuleHandle(NULL),MAKEINTRESOURCE(BM_BACKGROUND+cmd));
					InvalidateRect(hwnd,NULL,FALSE);
					break;
				}
				case ID_BLUE:
				case ID_RED:
				case ID_GREEN:
				case ID_PURPLE:
				case ID_CYAN:
				case ID_YELLOW:
				case ID_SILVER:{
					int i,cmd=LOWORD(wParam)-ID_CARD;
					for(i=0;i<7;++i)CheckMenuItem(hMenu,ID_CARD+i,i==cmd?MF_CHECKED:MF_UNCHECKED);
					UpdateCard(texture,base[cmd],back[cmd]);
					UpdateTexture(texture,piles,deal,stack,hold);
					InvalidateRect(hwnd,NULL,TRUE);
					break;
				}
				case ID_AFTERIMAGE:
					hold->afterimage^=1;
					CheckMenuItem(hMenu,ID_AFTERIMAGE,hold->afterimage?MF_CHECKED:MF_UNCHECKED);
					break;
				case ID_EXIT:SendMessage(hwnd,WM_CLOSE,(WPARAM)NULL,(LPARAM)NULL);
			}
			break;
		case WM_MOUSEMOVE:{
			if(!hold->active)break;
			POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
			RECT rc=MoveHold(hold,pt);
			InvalidateRect(hwnd,&rc,FALSE);
			break;
		}
		case WM_LBUTTONDOWN:{
			if(stack->victory)break;
			POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
			long k=(pt.x-G_PADDING)/(CARD_WIDTH+CARD_SPACE),m=piles[k].rc.bottom-pt.y,n=m<CARD_HEIGHT?1:(m-CARD_HEIGHT+CARD_SPACE)/CARD_SPACE+1;
			if(PtInRect(&piles[k].rc,pt))HoldCard(hold,&piles[k],n,pt,texture);
			break;
		}
		case WM_LBUTTONUP:{
			if(stack->victory)break;
			POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
			if(hold->active){
				long k=(pt.x-G_PADDING+CARD_SPACE/2)/(CARD_WIDTH+CARD_SPACE);
				if(PtInRect(&piles[k].rcDock,pt))MoveCard(hold,&piles[k],stack,texture);
				else ReleaseCard(hold,texture);
			} else if(PtInRect(&deal->rcHit,pt))NewDeal(deal,pool,piles,stack,texture);
			if(stack->victory){
				EnableMenuItem(GetSubMenu(hMenu,0),2,MF_BYPOSITION|MF_GRAYED);
				Victory(hwnd,stack,texture);
			}
			InvalidateRect(hwnd,NULL,FALSE);
			break;
		}
		case WM_CREATE:{
			FILE *f=fopen(DefaultSave,"rb+");
			if(f==NULL)CreateNewSave(FALSE);
			else if(fgetc(f)!='S'||fgetc(f)!='P')CreateNewSave(TRUE);
			fclose(f);
			f=fopen(DefaultSave,"rb+");
			SAVE save;
			save.sign1=fgetc(f);
			save.sign2=fgetc(f);
			save.deck=fgetc(f);
			save.suit=fgetc(f);
			save.recent=fgetc(f);
			save.slot1=fgetc(f);
			save.slot2=fgetc(f);
			save.slot3=fgetc(f);
			fclose(f);
			
			int i;
			deck=save.deck;
			suit=save.suit;
			hMenu=GetMenu(hwnd);
			for(i=2;i<=4;i+=2)CheckMenuItem(hMenu,ID_0DECK+i,i==deck?MF_CHECKED:MF_UNCHECKED);
			for(i=1;i<=4;++i)CheckMenuItem(hMenu,ID_0SUIT+i,i==suit?MF_CHECKED:MF_UNCHECKED);
			if(save.recent)EnableMenuItem(hMenu,ID_LOADRECENT,MF_ENABLED);
			if(save.slot1)EnableMenuItem(hMenu,ID_LOAD1,MF_ENABLED);
			if(save.slot2)EnableMenuItem(hMenu,ID_LOAD2,MF_ENABLED);
			if(save.slot3)EnableMenuItem(hMenu,ID_LOAD3,MF_ENABLED);
			
			HDC hdc=GetDC(hwnd);
			pool=CreatePool();
			piles=CreatePiles(hdc);
			hold=CreateHold(hdc);
			deal=CreateDeal(hdc);
			stack=CreateStack(hdc);
			texture=CreateTexture(hdc,GetModuleHandle(NULL),MAKEINTRESOURCE(BM_BACKGROUND),MAKEINTRESOURCE(BM_CARD),MAKEINTRESOURCE(BM_BACK),MAKEINTRESOURCE(BM_MASK),base[0],back[0]);
			
			ReleaseDC(hwnd,hdc);
			if(save.recent)LoadSlot(0,pool,deal,stack,piles,hold,texture);
			else InitGame(deck,suit,pool,piles,deal,stack,hold,texture);
			
			/* RESIZE WINDOW */
			gheight=G_HEIGHT(deck);
			RECT rc={0,0,G_WIDTH,gheight},wrc;
			AdjustWindowRect(&rc,WS_VISIBLE|WS_OVERLAPPED|WS_CAPTION|WS_MINIMIZEBOX|WS_SYSMENU,TRUE);
			GetWindowRect(hwnd,&wrc);
			MoveWindow(hwnd,wrc.left,wrc.top,rc.right-rc.left,rc.bottom-rc.top,FALSE);
			deal->rcHit.bottom=deal->rc.bottom=stack->rc.bottom=gheight-G_PADDING;
			deal->rcHit.top=deal->rc.top=stack->rc.top=stack->rc.bottom-CARD_HEIGHT;
			break;
		}
		case WM_CLOSE:DestroyWindow(hwnd);break;
		case WM_DESTROY:
			SaveOption(stack->goal/FULLSUIT,suit);
			if(stack->victory==FALSE)SaveSlot(0,pool,deal,stack,piles);
			DestroyPool(pool);
			DestroyPiles(piles);
			DestroyHold(hold);
			DestroyDeal(deal);
			DestroyStack(stack);
			DestroyTexture(texture);
			PostQuitMessage(0);
			break;
		default:return DefWindowProc(hwnd,msg,wParam,lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hPrevInst,LPSTR lpCmdLine,int nCmdShow) {
	srand(time(NULL));
	
	WNDCLASSEX wc;
	HWND hwnd;
	MSG msg;

	memset(&wc,0,sizeof(wc));
	wc.cbSize		 = sizeof(WNDCLASSEX);
    wc.lpfnWndProc   = WndProc;
	wc.hInstance	 = hInst;
	wc.hCursor		 = LoadCursor(NULL,IDC_HAND);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);;
	wc.lpszClassName = "WindowClass";
	wc.lpszMenuName  = MAKEINTRESOURCE(ID_MAINMENU);
	wc.hIcon		 = LoadIcon(hInst,"A");
	wc.hIconSm		 = NULL;
	if(!RegisterClassEx(&wc)){
		MessageBox(NULL,"Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}
	
	RECT rc={0,0,G_WIDTH,G_HEIGHT_G};
	AdjustWindowRect(&rc,WS_VISIBLE|WS_OVERLAPPED|WS_CAPTION|WS_MINIMIZEBOX|WS_SYSMENU,TRUE);
	hwnd=CreateWindow("WindowClass","Ultra Spider Patience HD",WS_VISIBLE|WS_OVERLAPPED|WS_CAPTION|WS_MINIMIZEBOX|WS_SYSMENU,10,10,rc.right-rc.left,rc.bottom-rc.top,NULL,NULL,hInst,NULL);
	if(hwnd==NULL){
		MessageBox(NULL,"Window Creation Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	while(GetMessage(&msg,NULL,0,0)>0){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
