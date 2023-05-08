#include <windows.h> //--- 윈도우 헤더 파일
#include <tchar.h>
#include <random>
#include <cmath>
#define PI					3.141592
//윈도우 크기 관련입니다.
#define WIDTH				1080
#define LENGTH				900

//타이틀바 관련
#define TitleBarHeight		330			//메인 메뉴 화면에서 타이틀 바의 크기입니다.
#define SubTitleBarHeight	70			//메인 메뉴 화면에서 시작 / 설명 / 종료 등 서브 타이틀 바의 크기입니다.

//플레이어 관련
#define PlayerSpeed			7			//플레이어 속도입니다. 다만 8이상일 경우 크기에 의해 블럭 사이에 들어가지지 않는 경우가 있어 적당한 값으로의 조절 부탁드립니다.
#define PlayerLife			3			//시작 플레이어 생명력입니다. 0이 되면 죽으므로 적당한 값으로 조절 부탁드립니다.
#define ComboTime			1500		//약 1500ms동안 콤보 시간이 유지됩니다.
#define ChargeTime			1200		//플레이어가 (스페이스바)차지를 하는데 약 1200ms의 시간이 필요합니다.
//총알 관련		
#define MaxBulletNum		150			//최대로 소환 가능한 탄알 수 입니다.
#define BulletSpeed			17			//총알의 속도입니다. 적당한 값으로 조절해주세요.		
#define BulletReloadTime	1500		//총알 재장전 시간입니다. 약 1500ms라는 뜻입니다.
#define PlusBulletRate		30			//몬스터가 떨어뜨리는 추가탄환의 확률입니다. 0~100의 값으로 조정해주시면 됩니다. 100이상이면 100% 나옵니다.
//적군 관련
#define MaxMonsterNum		100			//몬스터 최대 수
#define MonsterSpeed		3			//몬스터 속도
#define MonsterSpawnTime	3000		//3000ms가 제일 적당한 리스폰 시간인것 같음
#define MonsterDieScore		150			//몬스터 점수
//이펙트 관련
#define EffectTime			3000		//이펙트 지속 시간 (3000ms)현재
#define	MaxEffectNum		100			//이펙트 최대 수
#define ShakeScreenTime		400			//화면 흔들리는 시간 (400ms)
//게임 관련
#define MaxScore			30000		//승리조건 달성 점수
#define PlusScorePrintTime	400			//추가되는 점수 출력 시간
//타이머 관련
#define Timer_Title_Update	100
#define Title_Update_Speed	30
#define Timer_Game_Update	101
#define Game_Update_Speed	16

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"Inversus - 2019180031이윤석";
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
HWND hWnd;
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	MSG Message;
	WNDCLASSEX WndClass;
	g_hInst = hInstance;
	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&WndClass);
	hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 70, 30, WIDTH, LENGTH, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

class Bomb {
public:
	int x, y;
	int radius;

	Bomb operator=(Bomb other) {
		x = other.x;
		y = other.y;
		radius = other.radius;
		return *this;
	}
	void InitBomb() {
		x = 0;
		y = 0;
		radius = 0;
	}
};
class Effect {
public:
	int time = EffectTime;		//#define 으로 값 조정, 3000ms/2000ms가 제일 적당해보임
	Bomb bomb[5];
	static int effectNum;

	Effect operator=(Effect other) {
		time = other.time;
		for (int i = 0; i < 5; ++i) {
			bomb[i] = other.bomb[i];
		}
		return *this;
	}
	void InitEffect() {
		time = 2000;
		for (int i = 0; i < 5; ++i) {
			bomb[i].InitBomb();
		}
	}
};
int Effect::effectNum = 0;
class Board {
public:
	COLORREF RGB;
	RECT rect;
	int plusBullet = 0;
	static int plusBulletAngle;
};
int Board::plusBulletAngle = 0;

class Object {
public:
	RECT rect;
};

enum BulletState { LEFT, RIGHT, UP, DOWN };
class Bullet {
public:
	int hp = 1;
	RECT rect = { 0,0,0,0 };
	int state = 0;
	int shotTime = 0;
	static int bulletCount;
	Bullet operator=(Bullet other) {
		hp = other.hp;
		rect = other.rect;
		state = other.state;
		shotTime = other.shotTime;
		return *this;
	}
	void InitBullet() {
		hp = 0;
		rect = { 0,0,0,0 };
		state = 0;
		shotTime = 0;
	}
};
int Bullet::bulletCount = 0;

class Player : public Object {
public:
	int life;
	int hp = 1;
	int reload;
	int nowBullet;
	int bulletAngle;
	int state;
	int reSpawn;
	int comboCount;
	int comboTime;
	int charge;
};

enum ObjectState { DIE = 0, SPAWN, LIVE };
class Monster : public Object {
public:
	int hp = 1;
	int state;
	static int nowMonster;
	static int reSpawn;

	Monster operator=(Monster other) {
		hp = other.hp;
		rect = other.rect;
		state = other.state;
		return *this;
	}
	void InitMonster() {
		hp = 0;
		rect = { 0,0,0,0 };
		state = DIE;
	}
};
int Monster::nowMonster = 0;
int Monster::reSpawn = 0;
class PlusScore {
public:
	int plusScore;
	int printTime;
};


BOOL pressLBUTTON = false;
BOOL pressUP = false;
BOOL pressDOWN = false;
BOOL pressLEFT = false;
BOOL pressRIGHT = false;
BOOL CheckPressOnly = false;
BOOL pressESC = false;
BOOL pressSPACE;
BOOL pressW = false;
BOOL pressA = false;
BOOL pressS = false;
BOOL pressD = false;

std::random_device rd;
std::uniform_int_distribution<int> uid(0, 2000);
HDC hdc, mainMemdc, plusMemdc;
RECT clientRect;
HBRUSH hBrush, oldBrush, oldBrush2;
HFONT titleFont, subTitleFont, oldFont;
HFONT hFont;
HPEN hPen, oldPen;
HBITMAP mainBit, boardBit;
HBITMAP oldBit1, oldBit2;
BOOL start;
BOOL pause;
RECT pauseRect;
RECT pauseStart;
RECT pauseMenu;
BOOL end;
BOOL godMode = false;
int mx, my;
int boardWidth = 0;
int boardHeight = 0;
int boardX;
int boardY;
int bulletRadius;
int shakeScreen;
double comboPlusScore = 1;
PlusScore plusScore;

BOOL selectStart = false;
BOOL selectHelp = false;
BOOL selectExit = false;
BOOL setTimerTitle = false;
BOOL setTimerGame = false;

Board board[21][15];


Player player;
int score;
Bullet bullet[MaxBulletNum];
Monster monster[MaxMonsterNum];
int monsterRespawnTime = 0;
Effect effect[100];

void InitWindow();
void End();
void CALLBACK DrawTitle();
void DrawBoard();
void DrawGame();

void PlayerSpawn();
void PlayerMove();

void shotBullet();
void BulletToSomething();
void BulletToMonster();
void BulletMove();
void DieBullet(int);

void SpawnMonster();
void MonsterMove();
void BombMonsterCheck(RECT, int);
void CheckMonsterHp();
void MonsterToPlayer();
void MonsterChangeBoard();
void BoardBlackToWhite(RECT);
void DieMonster(int);

void CreateEffect(RECT);
void DieEffect(int);

void CheckPlusBullet();

void CALLBACK GameUpdate();

void SettingTimer();
void InitGame();
void SetEasyGame();
void SetNormalGame();
void SetHardGame();



LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SettingTimer();
	if (clientRect.right == 0) GetClientRect(hWnd, &clientRect);
	if (godMode) {
		player.hp = 100;
	}

	PAINTSTRUCT ps;
	//--- 메시지 처리하기
	switch (uMsg) {
	case WM_CREATE:
		InitWindow();
		SetTimer(hWnd, Timer_Title_Update, Title_Update_Speed, (TIMERPROC)DrawTitle);
		break;
	case WM_CHAR:
		switch (wParam) {
		case 'Q':
		case 'q':
			End();
			break;
		}
		break;
	case WM_KEYDOWN:
		//if ((pressW && pressA) || (pressW && pressS) || (pressW && pressD) || (pressA && pressS) || (pressA && pressD) || (pressS && pressD)) break;
		switch (wParam) {
		case VK_ESCAPE:
			pressESC = true;
			break;
		case VK_SPACE:
			pressSPACE = true;
			break;
		case VK_F1:
			if (start) ++player.nowBullet;
			break;
		case VK_F2:
			if (start) if (player.life < 10) ++player.life;
			break;
		case VK_F3:
			if (godMode) {
				player.hp = 1;
				godMode = false;
			}
			else {
				player.hp = 100;
				godMode = true;
			}
			break;
		case VK_F9:
			if (start) --player.life;
			break;
		case 'W':
		case 'w':
			pressW = true;
			break;
		case 'A':
		case 'a':
			pressA = true;
			break;
		case 'S':
		case 's':
			pressS = true;
			break;
		case 'D':
		case 'd':
			pressD = true;
			break;
		case VK_UP:
			if (CheckPressOnly)break;
			pressUP = true;
			CheckPressOnly = true;
			break;
		case VK_DOWN:
			if (CheckPressOnly)break;
			pressDOWN = true;
			CheckPressOnly = true;
			break;
		case VK_LEFT:
			if (CheckPressOnly)break;
			pressLEFT = true;
			CheckPressOnly = true;
			break;
		case VK_RIGHT:
			if (CheckPressOnly)break;
			pressRIGHT = true;
			CheckPressOnly = true;
			break;
		}
		break;
	case WM_KEYUP:
		switch (wParam) {
		case VK_ESCAPE:
			pressESC = false;
			break;
		case VK_SPACE:
			if (start) {
				pressSPACE = false;
				player.charge = 0;
			}
			break;
		case 'W':
		case 'w':
			pressW = false;
			break;
		case 'A':
		case 'a':
			pressA = false;
			break;
		case 'S':
		case 's':
			pressS = false;
			break;
		case 'D':
		case 'd':
			pressD = false;
			break;
		case VK_UP:
			pressUP = false;
			CheckPressOnly = false;
			break;
		case VK_DOWN:
			pressDOWN = false;
			CheckPressOnly = false;
			break;
		case VK_LEFT:
			pressLEFT = false;
			CheckPressOnly = false;
			break;
		case VK_RIGHT:
			pressRIGHT = false;
			CheckPressOnly = false;
			break;
		}
		break;
	case WM_LBUTTONDOWN:
		//게임 진행중엔 마우스를 쓸 일이 없으니 처리를 하지 않게 진행하였습니다.
		pressLBUTTON = true;
		break;
	case WM_MOUSEMOVE:
		mx = LOWORD(lParam), my = HIWORD(lParam);
		break;
	case WM_LBUTTONUP:
		pressLBUTTON = false;
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		oldBit1 = (HBITMAP)SelectObject(mainMemdc, mainBit);
		BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom,
			mainMemdc, 0, 0, SRCCOPY);
		SelectObject(mainMemdc, oldBit1);

		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		End();
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
void InitWindow() {
	GetClientRect(hWnd, &clientRect);
	KillTimer(hWnd, Timer_Title_Update);
	KillTimer(hWnd, Timer_Game_Update);
	hdc = GetDC(hWnd);
	if (mainBit == NULL) mainBit = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
	if (mainMemdc == NULL) mainMemdc = CreateCompatibleDC(hdc);
	if (plusMemdc == NULL) plusMemdc = CreateCompatibleDC(mainMemdc);
	boardWidth = clientRect.right - 100, boardHeight = clientRect.bottom - 200;
	if (boardBit == NULL) boardBit = CreateCompatibleBitmap(hdc, boardWidth, boardHeight);
	if (titleFont == NULL)	titleFont = CreateFont(300, 70, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("titleFont"));
	if (subTitleFont == NULL) subTitleFont = CreateFont(50, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("titleFont"));

	start = false;
	end = false;
	pauseRect = { clientRect.right - 5 - 55,5,clientRect.right - 5,clientRect.top + 5 + 55 };
	int mediumX = 50 + boardWidth / 2;
	pauseStart = { (int)(50 + (double)boardWidth / 5 + 15), 150 + boardHeight / 4 + 30, mediumX - 15, 150 + boardHeight / 4 * 3 - 30 };
	pauseMenu = { mediumX + 15, 150 + boardHeight / 4 + 30, (int)(50 + (double)boardWidth / 5 * 4 - 15), 150 + boardHeight / 4 * 3 - 30 };

	selectStart = false;
	selectHelp = false;
	selectExit = false;
	setTimerTitle = true;
}
void End() {
	DeleteObject(mainBit);
	DeleteDC(mainMemdc);
	DeleteDC(plusMemdc);
	DeleteObject(boardBit);
	DeleteObject(titleFont);
	DeleteObject(subTitleFont);
	PostQuitMessage(0);
}
void CALLBACK DrawTitle() {
	if (pressESC) {
		selectHelp = false;
	}
	oldBit1 = (HBITMAP)SelectObject(mainMemdc, mainBit);
	Rectangle(mainMemdc, -5, -5, clientRect.right + 5, clientRect.bottom + 5);

	oldBrush = (HBRUSH)SelectObject(mainMemdc, (HBRUSH)GetStockObject(BLACK_BRUSH));

	SetBkMode(mainMemdc, TRANSPARENT);
	SetTextColor(mainMemdc, RGB(255, 255, 255));
	RECT titleRect = { -1, clientRect.bottom / 2 - TitleBarHeight + 50, clientRect.right + 1, clientRect.bottom / 2 + 50 };
	{	//타이틀바 관련
		oldFont = (HFONT)SelectObject(mainMemdc, titleFont);
		Rectangle(mainMemdc, titleRect.left, titleRect.top, titleRect.right, titleRect.bottom);

		DrawText(mainMemdc, TEXT("INVERSUS"), lstrlen(TEXT("INVERSUS")), &titleRect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
		SelectObject(mainMemdc, oldFont);
	}

	oldFont = (HFONT)SelectObject(mainMemdc, subTitleFont);
	RECT subTitleRect;
	{	//서브 타이틀바 관련	NEW GAME
		subTitleRect = { -5,clientRect.bottom / 2 + SubTitleBarHeight * 2, clientRect.right + 5, clientRect.bottom / 2 + SubTitleBarHeight * 3 };
		BOOL select = PtInRect(&subTitleRect, { mx, my });
		if (select) {
			hBrush = CreateSolidBrush(RGB(30, 30, 30));
			oldBrush2 = (HBRUSH)SelectObject(mainMemdc, hBrush);
			if (pressLBUTTON) {
				if (selectStart) selectStart = false;
				else selectStart = true;
				selectHelp = false;
				pressLBUTTON = false;
			}
		}
		Rectangle(mainMemdc, subTitleRect.left, subTitleRect.top, subTitleRect.right, subTitleRect.bottom);
		if (select) {
			SelectObject(mainMemdc, oldBrush2);
			DeleteObject(hBrush);
		}
		DrawText(mainMemdc, TEXT("시작"), lstrlen(TEXT("시작")), &subTitleRect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	}

	{	//서브 타이틀바 관련	HELP
		subTitleRect = { -5, clientRect.bottom / 2 + SubTitleBarHeight * 3 + 15, clientRect.right + 5, clientRect.bottom / 2 + SubTitleBarHeight * 4 + 15 };
		BOOL select = PtInRect(&subTitleRect, { mx, my });
		if (select) {
			hBrush = CreateSolidBrush(RGB(30, 30, 30));
			oldBrush2 = (HBRUSH)SelectObject(mainMemdc, hBrush);
			if (pressLBUTTON) {
				if (selectHelp) selectHelp = false;
				else selectHelp = true;
				selectStart = false;
				pressLBUTTON = false;
			}
		}
		Rectangle(mainMemdc, subTitleRect.left, subTitleRect.top, subTitleRect.right, subTitleRect.bottom);
		if (select) {
			SelectObject(mainMemdc, oldBrush2);
			DeleteObject(hBrush);
		}
		DrawText(mainMemdc, TEXT("설명"), lstrlen(TEXT("설명")), &subTitleRect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	}

	{	//서브 타이틀바 관련 EXIT
		subTitleRect = { -5, clientRect.bottom / 2 + SubTitleBarHeight * 4 + 30, clientRect.right + 5,clientRect.bottom / 2 + SubTitleBarHeight * 5 + 30 };
		BOOL select = PtInRect(&subTitleRect, { mx, my });
		if (select) {
			hBrush = CreateSolidBrush(RGB(30, 30, 30));
			oldBrush2 = (HBRUSH)SelectObject(mainMemdc, hBrush);
			if (pressLBUTTON) selectExit = true;
		}
		Rectangle(mainMemdc, subTitleRect.left, subTitleRect.top, subTitleRect.right, subTitleRect.bottom);
		if (select) {
			SelectObject(mainMemdc, oldBrush2);
			DeleteObject(hBrush);
		}
		DrawText(mainMemdc, TEXT("종료"), lstrlen(TEXT("종료")), &subTitleRect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	}

	if (selectStart) {

		selectHelp = false;
		Rectangle(mainMemdc, titleRect.left, titleRect.top, titleRect.right, titleRect.bottom);
		RECT easy, normal, hard, cancel;
		easy = { titleRect.left,titleRect.top,titleRect.right / 2,titleRect.top + (titleRect.bottom - titleRect.top) / 2 };
		normal = { titleRect.right / 2 ,titleRect.top,titleRect.right,titleRect.top + (titleRect.bottom - titleRect.top) / 2 };
		hard = { titleRect.left,titleRect.top + (titleRect.bottom - titleRect.top) / 2 ,titleRect.right / 2,titleRect.bottom };
		cancel = { titleRect.right / 2 ,titleRect.top + (titleRect.bottom - titleRect.top) / 2 ,titleRect.right,titleRect.bottom };
		oldPen = (HPEN)SelectObject(mainMemdc, (HPEN)GetStockObject(WHITE_PEN));
		{
			BOOL select = PtInRect(&easy, { mx,my });
			if (select) {
				hBrush = CreateSolidBrush(RGB(30, 30, 30));
				oldBrush2 = (HBRUSH)SelectObject(mainMemdc, hBrush);
				if (pressLBUTTON) {
					SetEasyGame();
					start = true;
					setTimerGame = true;
				}
			}
			Rectangle(mainMemdc, easy.left, easy.top, easy.right, easy.bottom);
			DrawText(mainMemdc, TEXT("Easy"), lstrlen(TEXT("Easy")), &easy, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
			if (select) {
				SelectObject(mainMemdc, oldBrush2);
				DeleteObject(hBrush);
			}
		}
		{
			BOOL select = PtInRect(&normal, { mx,my });
			if (select) {
				hBrush = CreateSolidBrush(RGB(30, 30, 30));
				oldBrush2 = (HBRUSH)SelectObject(mainMemdc, hBrush);
				if (pressLBUTTON) {
					SetNormalGame();
					start = true;
					setTimerGame = true;
				}
			}
			Rectangle(mainMemdc, normal.left, normal.top, normal.right, normal.bottom);
			DrawText(mainMemdc, TEXT("Normal"), lstrlen(TEXT("Normal")), &normal, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
			if (select) {
				SelectObject(mainMemdc, oldBrush2);
				DeleteObject(hBrush);
			}
		}
		{
			BOOL select = PtInRect(&hard, { mx,my });
			if (select) {
				hBrush = CreateSolidBrush(RGB(30, 30, 30));
				oldBrush2 = (HBRUSH)SelectObject(mainMemdc, hBrush);
				if (pressLBUTTON) {
					SetHardGame();
					start = true;
					setTimerGame = true;
				}
			}
			Rectangle(mainMemdc, hard.left, hard.top, hard.right, hard.bottom);
			DrawText(mainMemdc, TEXT("Hard"), lstrlen(TEXT("Hard")), &hard, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
			if (select) {
				SelectObject(mainMemdc, oldBrush2);
				DeleteObject(hBrush);
			}
		}
		{
			BOOL select = PtInRect(&cancel, { mx,my });
			if (select) {
				hBrush = CreateSolidBrush(RGB(30, 30, 30));
				oldBrush2 = (HBRUSH)SelectObject(mainMemdc, hBrush);
				if (pressLBUTTON) {
					selectStart = false;
				}
			}
			Rectangle(mainMemdc, cancel.left, cancel.top, cancel.right, cancel.bottom);
			DrawText(mainMemdc, TEXT("Menu"), lstrlen(TEXT("Menu")), &cancel, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
			if (select) {
				SelectObject(mainMemdc, oldBrush2);
				DeleteObject(hBrush);
			}
		}
		SelectObject(mainMemdc, oldPen);
	}

	//만약 선택한게 있다면
	if (selectHelp) {
		LPCWSTR helpStr = TEXT("설명문을 끄시려면 ESC를 눌러주세요.\n")
			TEXT("W A S D - 상 하 좌 우 이동          ESC - 게임 중지\n ")
			TEXT("방향키 - 해당 방향으로 총알 발사\n")
			TEXT("스페이스바 - 차징 (차징 완료시 총알 3개 발사)\n")
			TEXT("F1 - 탄알 추가 / F2 - 생명 추가 \nF3 - 무적 모드 / F9 - 생명 감소\n");
		Rectangle(mainMemdc, titleRect.left, titleRect.top, titleRect.right, titleRect.bottom);
		DrawText(mainMemdc, helpStr, -1, &titleRect, DT_LEFT | DT_WORDBREAK);
	}
	if (selectExit) {
		End();
	}

	SelectObject(mainMemdc, oldFont);
	SelectObject(mainMemdc, oldBrush);
	SelectObject(mainMemdc, oldBit1);
	InvalidateRect(hWnd, NULL, false);
}
void DrawBoard() {
	oldBit1 = (HBITMAP)SelectObject(mainMemdc, boardBit);
	hBrush = CreateSolidBrush(RGB(30, 30, 30));
	oldBrush = (HBRUSH)SelectObject(mainMemdc, hBrush);
	Rectangle(mainMemdc, 0, 0, boardWidth, boardHeight);
	SelectObject(mainMemdc, oldBrush);
	DeleteObject(hBrush);
	{//네모칸 그리기 확인용
		for (int i = 0; i < boardX; ++i) {
			for (int j = 0; j < boardY; ++j) {
				hBrush = CreateSolidBrush(board[i][j].RGB);
				oldBrush = (HBRUSH)SelectObject(mainMemdc, hBrush);
				Rectangle(mainMemdc, board[i][j].rect.left, board[i][j].rect.top, board[i][j].rect.right, board[i][j].rect.bottom);
				SelectObject(mainMemdc, oldBrush);
				DeleteObject(hBrush);
			}
		}
	}
	{//보드 선 긋기
		hPen = CreatePen(PS_SOLID, 1, RGB(50, 50, 50));
		oldPen = (HPEN)SelectObject(mainMemdc, hPen);
		for (int i = 0; i < boardX; ++i) {
			MoveToEx(mainMemdc, board[i][0].rect.right, 0, NULL);
			LineTo(mainMemdc, board[i][0].rect.right, boardHeight);
		}
		for (int j = 0; j < boardY; ++j) {
			MoveToEx(mainMemdc, 0, board[0][j].rect.bottom, NULL);
			LineTo(mainMemdc, boardWidth, board[0][j].rect.bottom);
		}
		SelectObject(mainMemdc, oldPen);
		DeleteObject(hPen);
	}

	double x = (double)boardWidth / boardX;
	double y = (double)boardHeight / boardY;
	{	//보드 추가 탄환 그리기

		for (int i = 0; i < boardX; ++i) {
			for (int j = 0; j < boardY; ++j) {
				if (board[i][j].plusBullet > 0) {
					int middleX = board[i][j].rect.left + x / 2;
					int middleY = board[i][j].rect.top + y / 2;
					int radius = (double)x / 5 * 1;
					int plusBulletRadius = (double)radius / 5 * 3.3;
					for (int k = 0; k < board[i][j].plusBullet; ++k) {
						int boardBulletX = middleX + radius * sin((board[0][0].plusBulletAngle - 90 * k) * PI / 180);
						int boardBulletY = middleY + radius * cos((board[0][0].plusBulletAngle - 90 * k) * PI / 180);
						Ellipse(mainMemdc, boardBulletX - plusBulletRadius, boardBulletY - plusBulletRadius, boardBulletX + plusBulletRadius, boardBulletY + plusBulletRadius);
					}
				}
			}
		}
	}

	{	//몬스터 있는곳 주변 3x3 타일 선긋기
		for (int i = 0; i < monster[0].nowMonster; ++i) {
			if (monster[i].state == SPAWN) continue;
			double monsterX = monster[i].rect.left + (double)(monster[i].rect.right - monster[i].rect.left) / 2;
			double monsterY = monster[i].rect.top + (double)(monster[i].rect.bottom - monster[i].rect.top) / 2;
			int X = monsterX / x;
			int Y = monsterY / y;

			{//보드 그리는부분
				hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
				oldPen = (HPEN)SelectObject(mainMemdc, hPen);
				if (X >= boardX || Y >= boardY) {
					if (X >= boardX) X = boardX - 1;
					if (Y >= boardY) Y = boardY - 1;
				}
				RECT hatchRect = { board[X][Y].rect.left - x,board[X][Y].rect.top - y,board[X][Y].rect.right + x,board[X][Y].rect.bottom + y };		//빗금영역이라는 뜻
				MoveToEx(mainMemdc, hatchRect.left, hatchRect.top, NULL);
				LineTo(mainMemdc, hatchRect.left, hatchRect.bottom);
				LineTo(mainMemdc, hatchRect.right, hatchRect.bottom);
				LineTo(mainMemdc, hatchRect.right, hatchRect.top);
				LineTo(mainMemdc, hatchRect.left, hatchRect.top);
				//윗쪽 라인 그리기
				double hatchRectWidth = hatchRect.right - hatchRect.left;
				double hatchRectHeight = hatchRect.bottom - hatchRect.top;

				for (int i = 0; i < 7; ++i) {
					MoveToEx(mainMemdc, hatchRect.left + (hatchRectWidth) / 7 * (i + 1), hatchRect.top, NULL);
					LineTo(mainMemdc, hatchRect.left, hatchRect.top + (hatchRectHeight) / 7 * (i + 1));
				}
				//아랫쪽 라인 그리기
				for (int i = 0; i < 5; ++i) {
					MoveToEx(mainMemdc, hatchRect.left + (hatchRectWidth) / 6 * (i + 1), hatchRect.bottom, NULL);
					LineTo(mainMemdc, hatchRect.right, hatchRect.top + (hatchRectHeight) / 6 * (i + 1));
				}
				SelectObject(mainMemdc, oldPen); DeleteObject(hPen);
			}

		}

	}

	{	//플레이어 그리기
		if (player.charge >= ChargeTime) {
			hBrush = CreateSolidBrush(RGB(150, 150, 150));
			oldBrush = (HBRUSH)SelectObject(mainMemdc, hBrush);
		}
		else {
			hBrush = CreateSolidBrush(RGB(200, 200, 200));
			oldBrush = (HBRUSH)SelectObject(mainMemdc, hBrush);
		}

		if (player.state == SPAWN) {
			for (int i = 0; i < 4; ++i) {
				RECT spawnRect = { player.rect.left + i,player.rect.top + i,player.rect.right - i,player.rect.bottom - i };
				FrameRect(mainMemdc, &spawnRect, hBrush);
			}
		}
		else if (player.state == LIVE) {
			RoundRect(mainMemdc, player.rect.left, player.rect.top, player.rect.right, player.rect.bottom, 5, 5);
		}

		SelectObject(mainMemdc, oldBrush);
		DeleteObject(hBrush);
	}

	{	//플레이어 총알 그리기
		if (player.state == LIVE) {
			int middleX = player.rect.left + (double)(player.rect.right - player.rect.left) / 2, middleY = player.rect.top + (double)(player.rect.bottom - player.rect.top) / 2;
			int radius = (double)(middleX - player.rect.left) / 5 * 3;
			//Ellipse(mainMemdc, middleX -radius , middleY - radius, middleX + radius, middleY + radius);
			bulletRadius = (double)radius / 5 * 2.5;
			for (int i = 0; i < player.nowBullet; ++i) {
				if (i >= 12) {
					hBrush = CreateSolidBrush(RGB(0, 128, 0));
					oldBrush = (HBRUSH)SelectObject(mainMemdc, hBrush);
				}
				else if (i >= 6) {
					hBrush = CreateSolidBrush(RGB(255, 215, 0));
					oldBrush = (HBRUSH)SelectObject(mainMemdc, hBrush);
				}
				int x = middleX + radius * sin((player.bulletAngle - 60 * i) * PI / 180);
				int y = middleY + radius * cos((player.bulletAngle - 60 * i) * PI / 180);
				Ellipse(mainMemdc, x - bulletRadius, y - bulletRadius, x + bulletRadius, y + bulletRadius);
				if (i >= 6) {
					SelectObject(mainMemdc, oldBrush);
					DeleteObject(hBrush);
				}
			}
			//생기고 있는 총알 그리기
			if (player.nowBullet < 6) {
				int reloadLevel = player.reload / (BulletReloadTime / Game_Update_Speed / 5);
				if (reloadLevel == 0)
					hBrush = CreateSolidBrush(RGB(30, 30, 30));
				else if (reloadLevel == 1)
					hBrush = CreateSolidBrush(RGB(60, 60, 60));
				else if (reloadLevel == 2)
					hBrush = CreateSolidBrush(RGB(90, 90, 90));
				else if (reloadLevel == 3)
					hBrush = CreateSolidBrush(RGB(150, 150, 150));
				else if (reloadLevel == 4)
					hBrush = CreateSolidBrush(RGB(200, 200, 200));

				oldBrush = (HBRUSH)SelectObject(mainMemdc, hBrush);

				int x = middleX + radius * sin((player.bulletAngle - 60 * player.nowBullet) * PI / 180);
				int y = middleY + radius * cos((player.bulletAngle - 60 * player.nowBullet) * PI / 180);
				Ellipse(mainMemdc, x - bulletRadius, y - bulletRadius, x + bulletRadius, y + bulletRadius);

				SelectObject(mainMemdc, oldBrush);
				DeleteObject(hBrush);

			}
		}
	}
	{	//날아가는 총알 그리기
		for (int i = 0; i < bullet[0].bulletCount; ++i) {
			hBrush = CreateSolidBrush(RGB(0, 0, 0));
			oldBrush = (HBRUSH)SelectObject(mainMemdc, hBrush);
			hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
			oldPen = (HPEN)SelectObject(mainMemdc, hPen);
			if (bullet[i].state == UP)
				Rectangle(mainMemdc, bullet[i].rect.left, bullet[i].rect.top, bullet[i].rect.right, bullet[i].rect.bottom + bullet[i].shotTime);
			else if (bullet[i].state == DOWN)
				Rectangle(mainMemdc, bullet[i].rect.left, bullet[i].rect.top - bullet[i].shotTime, bullet[i].rect.right, bullet[i].rect.bottom);
			else if (bullet[i].state == LEFT)
				Rectangle(mainMemdc, bullet[i].rect.left, bullet[i].rect.top, bullet[i].rect.right + bullet[i].shotTime, bullet[i].rect.bottom);
			else if (bullet[i].state == RIGHT)
				Rectangle(mainMemdc, bullet[i].rect.left - bullet[i].shotTime, bullet[i].rect.top, bullet[i].rect.right, bullet[i].rect.bottom);
			SelectObject(mainMemdc, oldPen); DeleteObject(hPen);
			SelectObject(mainMemdc, oldBrush); DeleteObject(hBrush);

			if (bullet[i].shotTime >= 30) {
				hBrush = CreateSolidBrush(RGB(30, 30, 30));
				oldBrush = (HBRUSH)SelectObject(mainMemdc, hBrush);
				hPen = CreatePen(PS_SOLID, 1, RGB(30, 30, 30));
				oldPen = (HPEN)SelectObject(mainMemdc, hPen);
				if (bullet[i].state == UP)
					Rectangle(mainMemdc, bullet[i].rect.left, bullet[i].rect.top + bullet[i].shotTime / 5 * 1, bullet[i].rect.right, bullet[i].rect.bottom + bullet[i].shotTime);
				else if (bullet[i].state == DOWN)
					Rectangle(mainMemdc, bullet[i].rect.left, bullet[i].rect.top - bullet[i].shotTime, bullet[i].rect.right, bullet[i].rect.bottom - bullet[i].shotTime / 5 * 1);
				else if (bullet[i].state == LEFT)
					Rectangle(mainMemdc, bullet[i].rect.left + bullet[i].shotTime / 5 * 1, bullet[i].rect.top, bullet[i].rect.right + bullet[i].shotTime, bullet[i].rect.bottom);
				else if (bullet[i].state == RIGHT)
					Rectangle(mainMemdc, bullet[i].rect.left - bullet[i].shotTime, bullet[i].rect.top, bullet[i].rect.right - bullet[i].shotTime / 5 * 1, bullet[i].rect.bottom);
				SelectObject(mainMemdc, oldPen); DeleteObject(hPen);
				SelectObject(mainMemdc, oldBrush); DeleteObject(hBrush);
			}
			if (bullet[i].shotTime >= 60) {
				hBrush = CreateSolidBrush(RGB(60, 60, 60));
				oldBrush = (HBRUSH)SelectObject(mainMemdc, hBrush);
				hPen = CreatePen(PS_SOLID, 1, RGB(60, 60, 60));
				oldPen = (HPEN)SelectObject(mainMemdc, hPen);
				if (bullet[i].state == UP)
					Rectangle(mainMemdc, bullet[i].rect.left, bullet[i].rect.top + bullet[i].shotTime / 5 * 2, bullet[i].rect.right, bullet[i].rect.bottom + bullet[i].shotTime);
				else if (bullet[i].state == DOWN)
					Rectangle(mainMemdc, bullet[i].rect.left, bullet[i].rect.top - bullet[i].shotTime, bullet[i].rect.right, bullet[i].rect.bottom - bullet[i].shotTime / 5 * 2);
				else if (bullet[i].state == LEFT)
					Rectangle(mainMemdc, bullet[i].rect.left + bullet[i].shotTime / 5 * 2, bullet[i].rect.top, bullet[i].rect.right + bullet[i].shotTime, bullet[i].rect.bottom);
				else if (bullet[i].state == RIGHT)
					Rectangle(mainMemdc, bullet[i].rect.left - bullet[i].shotTime, bullet[i].rect.top, bullet[i].rect.right - bullet[i].shotTime / 5 * 2, bullet[i].rect.bottom);
				SelectObject(mainMemdc, oldPen); DeleteObject(hPen);
				SelectObject(mainMemdc, oldBrush); DeleteObject(hBrush);
			}
			if (bullet[i].shotTime >= 90) {
				hBrush = CreateSolidBrush(RGB(90, 90, 90));
				oldBrush = (HBRUSH)SelectObject(mainMemdc, hBrush);
				hPen = CreatePen(PS_SOLID, 1, RGB(90, 90, 90));
				oldPen = (HPEN)SelectObject(mainMemdc, hPen);
				if (bullet[i].state == UP)
					Rectangle(mainMemdc, bullet[i].rect.left, bullet[i].rect.top + bullet[i].shotTime / 5 * 3, bullet[i].rect.right, bullet[i].rect.bottom + bullet[i].shotTime);
				else if (bullet[i].state == DOWN)
					Rectangle(mainMemdc, bullet[i].rect.left, bullet[i].rect.top - bullet[i].shotTime, bullet[i].rect.right, bullet[i].rect.bottom - bullet[i].shotTime / 5 * 3);
				else if (bullet[i].state == LEFT)
					Rectangle(mainMemdc, bullet[i].rect.left + bullet[i].shotTime / 5 * 3, bullet[i].rect.top, bullet[i].rect.right + bullet[i].shotTime, bullet[i].rect.bottom);
				else if (bullet[i].state == RIGHT)
					Rectangle(mainMemdc, bullet[i].rect.left - bullet[i].shotTime, bullet[i].rect.top, bullet[i].rect.right - bullet[i].shotTime / 5 * 3, bullet[i].rect.bottom);
				SelectObject(mainMemdc, oldPen); DeleteObject(hPen);
				SelectObject(mainMemdc, oldBrush); DeleteObject(hBrush);
			}
			if (bullet[i].shotTime >= 120) {
				hBrush = CreateSolidBrush(RGB(120, 120, 120));
				oldBrush = (HBRUSH)SelectObject(mainMemdc, hBrush);
				hPen = CreatePen(PS_SOLID, 1, RGB(120, 120, 120));
				oldPen = (HPEN)SelectObject(mainMemdc, hPen);
				if (bullet[i].state == UP)
					Rectangle(mainMemdc, bullet[i].rect.left, bullet[i].rect.top + bullet[i].shotTime / 5 * 4, bullet[i].rect.right, bullet[i].rect.bottom + bullet[i].shotTime);
				else if (bullet[i].state == DOWN)
					Rectangle(mainMemdc, bullet[i].rect.left, bullet[i].rect.top - bullet[i].shotTime, bullet[i].rect.right, bullet[i].rect.bottom - bullet[i].shotTime / 5 * 4);
				else if (bullet[i].state == LEFT)
					Rectangle(mainMemdc, bullet[i].rect.left + bullet[i].shotTime / 5 * 4, bullet[i].rect.top, bullet[i].rect.right + bullet[i].shotTime, bullet[i].rect.bottom);
				else if (bullet[i].state == RIGHT)
					Rectangle(mainMemdc, bullet[i].rect.left - bullet[i].shotTime, bullet[i].rect.top, bullet[i].rect.right - bullet[i].shotTime / 5 * 4, bullet[i].rect.bottom);
				SelectObject(mainMemdc, oldPen); DeleteObject(hPen);
				SelectObject(mainMemdc, oldBrush); DeleteObject(hBrush);
			}
		}
	}

	{	//플레이어 차지 게이지 그리기
		if (player.charge > 0 && player.state == LIVE) {
			RECT chargeRect = { player.rect.left - 4,player.rect.bottom,player.rect.right + 4,player.rect.bottom + 10 };
			Rectangle(mainMemdc, chargeRect.left, chargeRect.top, chargeRect.right, chargeRect.bottom);
			hBrush = CreateSolidBrush(RGB(255, 0, 0));
			oldBrush = (HBRUSH)SelectObject(mainMemdc, hBrush);
			int percent = (double)player.charge / ChargeTime * (chargeRect.right - chargeRect.left);
			Rectangle(mainMemdc, chargeRect.left, chargeRect.top, chargeRect.left + percent, chargeRect.bottom);
			SelectObject(mainMemdc, oldBrush);
			DeleteObject(hBrush);

		}
	}

	{	//적군 그리기
		for (int i = 0; i < monster[0].nowMonster; ++i) {
			hBrush = CreateSolidBrush(RGB(184, 223, 248));
			oldBrush = (HBRUSH)SelectObject(mainMemdc, hBrush);
			if (monster[i].state == SPAWN) {
				for (int j = 0; j < 6; ++j) {
					RECT monsterRect = { monster[i].rect.left + j,monster[i].rect.top + j,monster[i].rect.right - j,monster[i].rect.bottom - j };
					FrameRect(mainMemdc, &monsterRect, hBrush);
				}
			}
			else if (monster[i].state == LIVE) {
				//주변 빗금 칠하는거랑 그리기
				Rectangle(mainMemdc, monster[i].rect.left, monster[i].rect.top, monster[i].rect.right, monster[i].rect.bottom);
			}
			SelectObject(mainMemdc, oldBrush);
			DeleteObject(hBrush);
		}
	}
	{	//죽는 이펙트 그리기
		hBrush = CreateSolidBrush(RGB(184, 223, 248));
		oldBrush = (HBRUSH)SelectObject(mainMemdc, hBrush);
		for (int i = 0; i < effect[0].effectNum; ++i) {
			for (int j = 0; j < 5; ++j) {
				Ellipse(mainMemdc, effect[i].bomb[j].x - effect[i].bomb[j].radius, effect[i].bomb[j].y - effect[i].bomb[j].radius, effect[i].bomb[j].x + effect[i].bomb[j].radius, effect[i].bomb[j].y + effect[i].bomb[j].radius);
			}
		}
		SelectObject(mainMemdc, oldBrush);
		DeleteObject(hBrush);
	}
	SelectObject(mainMemdc, oldBit1);
}
void DrawGame() {
	oldBit1 = (HBITMAP)SelectObject(mainMemdc, mainBit);
	//전체 그리기
	Rectangle(mainMemdc, -5, -5, clientRect.right, clientRect.bottom);

	//윗쪽 점수 및 LIFE 바
	Rectangle(mainMemdc, -1, -1, clientRect.right, 100);
	//게임 전체 판
	hBrush = CreateSolidBrush(RGB(30, 30, 30));
	oldBrush = (HBRUSH)SelectObject(mainMemdc, hBrush);
	Rectangle(mainMemdc, -1, 100, clientRect.right, clientRect.bottom);
	SelectObject(mainMemdc, oldBrush);
	DeleteObject(hBrush);

	//보드 판
	oldBit2 = (HBITMAP)SelectObject(plusMemdc, boardBit);
	if (shakeScreen > 0) {
		BitBlt(mainMemdc, 50 + uid(rd) % 10 - 10, 150 + uid(rd) % 10 - 10, boardWidth, boardHeight,
			plusMemdc, 0, 0, SRCCOPY);
		shakeScreen -= Game_Update_Speed;
	}
	else {
		BitBlt(mainMemdc, 50, 150, boardWidth, boardHeight,
			plusMemdc, 0, 0, SRCCOPY);
	}

	{	//남은 라이프 출력
		SetTextColor(mainMemdc, RGB(0, 0, 0));
		RECT lifePrintRect = { clientRect.right - 200,0,clientRect.right - 10,75 };
		//Rectangle(mainMemdc,lifePrintRect.left, lifePrintRect.top, lifePrintRect.right, lifePrintRect.bottom);
		TCHAR life[30];
		hFont = CreateFont(50, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("subtitleFont"));
		oldFont = (HFONT)SelectObject(mainMemdc, hFont);
		wsprintf(life, L"생명: %d", player.life);
		DrawText(mainMemdc, life, lstrlen(life), &lifePrintRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
		SelectObject(mainMemdc, oldFont);
		DeleteObject(hFont);
	}
	SelectObject(plusMemdc, oldBit2);

	{	//몹 나오는 주기 
		Rectangle(mainMemdc, clientRect.right - 200, 75, clientRect.right - 10, 95);
		hBrush = CreateSolidBrush(RGB(255, 212, 0));
		oldBrush = (HBRUSH)SelectObject(mainMemdc, hBrush);
		double percent = (double)monster[0].reSpawn / (monsterRespawnTime / Game_Update_Speed) * 190;
		Rectangle(mainMemdc, clientRect.right - 200, 75, clientRect.right - 200 + percent, 95);
		SelectObject(mainMemdc, oldBrush);
		DeleteObject(hBrush);
	}

	{	//점수 출력
		TCHAR scoreStr[30];
		wsprintf(scoreStr, L"%7d / %-7d", score, MaxScore);
		hFont = CreateFont(60, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("subtitleFont"));
		oldFont = (HFONT)SelectObject(mainMemdc, hFont);
		TextOut(mainMemdc, 0, 10, scoreStr, lstrlen(scoreStr));
		SelectObject(mainMemdc, oldFont);
		DeleteObject(hFont);
	}

	{	//추가되는 점수 출력
		if (plusScore.printTime > 0) {
			TCHAR scoreStr[30];
			if (player.comboCount >= 5)
				swprintf_s(scoreStr, L"+%d (콤보 x%1.1f)", plusScore.plusScore, comboPlusScore);
			else wsprintf(scoreStr, L"+%d", plusScore.plusScore);
			hFont = CreateFont(30, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("subtitleFont"));
			oldFont = (HFONT)SelectObject(mainMemdc, hFont);
			TextOut(mainMemdc, 400, 25, scoreStr, lstrlen(scoreStr));
			SelectObject(mainMemdc, oldFont);
			DeleteObject(hFont);
		}
	}

	{	//콤보 중일 경우 시간 출력 및 콤보 출력
		if (player.comboTime > 0) {
			Rectangle(mainMemdc, clientRect.right - 400, 75, clientRect.right - 210, 95);
			hBrush = CreateSolidBrush(RGB(0, 212, 0));
			oldBrush = (HBRUSH)SelectObject(mainMemdc, hBrush);

			double percent = (double)player.comboTime / (ComboTime) * 190;
			Rectangle(mainMemdc, clientRect.right - 400, 75, clientRect.right - 400 + percent, 95);

			SelectObject(mainMemdc, oldBrush);
			DeleteObject(hBrush);

			TCHAR comboStr[10];
			wsprintf(comboStr, L"%d COMBO", player.comboCount);
			hFont = CreateFont(30, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("subtitleFont"));
			oldFont = (HFONT)SelectObject(mainMemdc, hFont);
			RECT comboRect = { clientRect.right - 400,0,clientRect.right - 210,75 };
			DrawText(mainMemdc, comboStr, lstrlen(comboStr), &comboRect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
			SelectObject(mainMemdc, oldFont);
			DeleteObject(hFont);
		}
	}

	{	//무적버프 출력
		if (godMode) {
			RECT godModeRect = { clientRect.left,clientRect.bottom - 30,clientRect.left + 70,clientRect.bottom };
			Rectangle(mainMemdc, godModeRect.left, godModeRect.top, godModeRect.right, godModeRect.bottom);
			DrawText(mainMemdc, TEXT("무적 모드"), lstrlen(TEXT("무적 모드")), &godModeRect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
		}
	}


	{	//중단 버튼 그리기
		oldBrush = (HBRUSH)SelectObject(mainMemdc, (HBRUSH)GetStockObject(BLACK_BRUSH));
		RoundRect(mainMemdc, pauseRect.left, pauseRect.top, pauseRect.right, pauseRect.bottom, 10, 10);
		SelectObject(mainMemdc, oldBrush);
		double pauseRectWidth = pauseRect.right - pauseRect.left;
		double pauseRectHeight = pauseRect.bottom - pauseRect.top;
		Rectangle(mainMemdc, pauseRect.left + pauseRectWidth / 7 * 2, pauseRect.top + pauseRectHeight / 5, pauseRect.left + pauseRectWidth / 7 * 3, pauseRect.top + pauseRectHeight / 5 * 4);
		Rectangle(mainMemdc, pauseRect.left + pauseRectWidth / 7 * 4, pauseRect.top + pauseRectHeight / 5, pauseRect.left + pauseRectWidth / 7 * 5, pauseRect.top + pauseRectHeight / 5 * 4);
		//만약 게임 중단시 메뉴바 출력
		if (pause) {
			Rectangle(mainMemdc, 50 + (double)boardWidth / 5, 150 + boardHeight / 4, 50 + (double)boardWidth / 5 * 4, 150 + boardHeight / 4 * 3);

			oldBrush = (HBRUSH)SelectObject(mainMemdc, (HBRUSH)GetStockObject(BLACK_BRUSH));
			Rectangle(mainMemdc, pauseStart.left, pauseStart.top, pauseStart.right, pauseStart.bottom);
			Rectangle(mainMemdc, pauseMenu.left, pauseMenu.top, pauseMenu.right, pauseMenu.bottom);
			SelectObject(mainMemdc, oldBrush);

			hPen = CreatePen(PS_SOLID, 10, RGB(255, 255, 255));
			oldPen = (HPEN)SelectObject(mainMemdc, hPen);
			double startWidth = pauseStart.right - pauseStart.left;
			double startHeight = pauseStart.bottom - pauseStart.top;
			MoveToEx(mainMemdc, pauseStart.left + startWidth / 5, pauseStart.top + startHeight / 5, NULL);
			LineTo(mainMemdc, pauseStart.left + startWidth / 5, pauseStart.top + startHeight / 5 * 4);
			LineTo(mainMemdc, pauseStart.left + startWidth / 5 * 4, pauseStart.top + startHeight / 5 * 2.5);
			LineTo(mainMemdc, pauseStart.left + startWidth / 5, pauseStart.top + startHeight / 5);
			SelectObject(mainMemdc, oldPen);
			DeleteObject(hPen);

			hFont = CreateFont(60, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("subtitleFont"));
			oldFont = (HFONT)SelectObject(mainMemdc, hFont);
			SetTextColor(mainMemdc, RGB(255, 255, 255));
			DrawText(mainMemdc, TEXT("MENU"), lstrlen(TEXT("MENU")), &pauseMenu, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
			SetTextColor(mainMemdc, RGB(0, 0, 0));
			SelectObject(mainMemdc, oldFont);
			DeleteObject(hFont);
		}
	}

	{	//만약 게임 끝났을 경우 안내문 출력
		if (end) {
			hFont = CreateFont(60, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("subtitleFont"));
			oldFont = (HFONT)SelectObject(mainMemdc, hFont);
			Rectangle(mainMemdc, 50, 150 + boardHeight / 4, 50 + boardWidth, 150 + boardHeight / 4 * 3);
			TCHAR endStr[30];
			if (score == MaxScore) {	//목표 점수 채워서 클리어 했을 경우
				wsprintf(endStr, L"게임 클리어! 축하드립니다.");
			}
			else if (player.life <= 0) {	//라이프 다 사용했을 경우
				wsprintf(endStr, L"게임 오버! 다시 도전해주세요.");
			}
			RECT endStrRect = { 50,150 + boardHeight / 2 - 100,50 + boardWidth,150 + boardHeight / 2 };
			DrawText(mainMemdc, endStr, -1, &endStrRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			endStrRect.top += 100;
			endStrRect.bottom += 100;
			TCHAR explainStr[30] = L"ESC를 눌러 메인 화면으로 돌아갈 수 있습니다.";
			DrawText(mainMemdc, explainStr, -1, &endStrRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			SelectObject(mainMemdc, oldFont);
			DeleteObject(hFont);

			hBrush = CreateSolidBrush(RGB(150, 150, 150));
			oldBrush = (HBRUSH)SelectObject(mainMemdc, hBrush);
			Rectangle(mainMemdc, 50, 150 + boardHeight / 4 + 15, 50 + boardWidth, 150 + boardHeight / 4 + 30);
			Rectangle(mainMemdc, 50, 150 + boardHeight / 4 * 3 - 30, 50 + boardWidth, 150 + boardHeight / 4 * 3 - 15);
			SelectObject(mainMemdc, oldBrush);
			DeleteObject(hBrush);
		}
	}
	SelectObject(mainMemdc, oldBit1);
}

void PlayerSpawn() {
	board[boardX / 2][boardY / 2].RGB = RGB(255, 255, 255);
	board[boardX / 2 + 1][boardY / 2].RGB = RGB(255, 255, 255);
	board[boardX / 2 - 1][boardY / 2].RGB = RGB(255, 255, 255);
	board[boardX / 2][boardY / 2 + 1].RGB = RGB(255, 255, 255);
	board[boardX / 2][boardY / 2 - 1].RGB = RGB(255, 255, 255);
	board[boardX / 2 - 1][boardY / 2 - 1].RGB = RGB(255, 255, 255);
	board[boardX / 2 - 1][boardY / 2 + 1].RGB = RGB(255, 255, 255);
	board[boardX / 2 + 1][boardY / 2 - 1].RGB = RGB(255, 255, 255);
	board[boardX / 2 + 1][boardY / 2 + 1].RGB = RGB(255, 255, 255);
	board[boardX / 2 - 1][boardY / 2 - 2].RGB = RGB(255, 255, 255);
	board[boardX / 2][boardY / 2 - 2].RGB = RGB(255, 255, 255);
	board[boardX / 2 + 1][boardY / 2 - 2].RGB = RGB(255, 255, 255);
	board[boardX / 2 - 1][boardY / 2 + 2].RGB = RGB(255, 255, 255);
	board[boardX / 2][boardY / 2 + 2].RGB = RGB(255, 255, 255);
	board[boardX / 2 + 1][boardY / 2 + 2].RGB = RGB(255, 255, 255);
	board[boardX / 2 - 2][boardY / 2 + 1].RGB = RGB(255, 255, 255);
	board[boardX / 2 - 2][boardY / 2].RGB = RGB(255, 255, 255);
	board[boardX / 2 - 2][boardY / 2 - 1].RGB = RGB(255, 255, 255);
	board[boardX / 2 + 2][boardY / 2 + 1].RGB = RGB(255, 255, 255);
	board[boardX / 2 + 2][boardY / 2].RGB = RGB(255, 255, 255);
	board[boardX / 2 + 2][boardY / 2 - 1].RGB = RGB(255, 255, 255);

	int blockWidth = board[0][0].rect.right - board[0][0].rect.left;
	int blockHeight = board[0][0].rect.bottom - board[0][0].rect.top;
	RECT spawnRect = { board[boardX / 2][boardY / 2].rect.left - blockWidth * 3,board[boardX / 2][boardY / 2].rect.top - blockWidth * 3,board[boardX / 2][boardY / 2].rect.right + blockWidth * 3,board[boardX / 2][boardY / 2].rect.bottom + blockWidth * 3 };
	BombMonsterCheck(spawnRect, 100);
	CheckMonsterHp();

	player.rect = { board[boardX / 2][boardY / 2].rect.left + 4,board[boardX / 2][boardY / 2].rect.top + 4,board[boardX / 2][boardY / 2].rect.right - 4,board[boardX / 2][boardY / 2].rect.bottom - 4 };
	player.nowBullet = 6;
	player.hp = 1;
	player.state = LIVE;
	monsterRespawnTime = MonsterSpawnTime;
}
void PlayerMove() {
	if (player.state == DIE) {
		player.reSpawn -= Game_Update_Speed;
		if (player.reSpawn < 0) {
			player.rect = { board[boardX / 2][boardY / 2].rect.left - 10,board[boardX / 2][boardY / 2].rect.top - 10,board[boardX / 2][boardY / 2].rect.right + 10,board[boardX / 2][boardY / 2].rect.bottom + 10 };
			player.state = SPAWN;
			player.reSpawn = 0;
		}
		return;
	}
	if (player.state == SPAWN) {
		player.rect.left += 1;
		player.rect.top += 1;
		player.rect.right -= 1;
		player.rect.bottom -= 1;
		if (player.rect.right - player.rect.left < (board[0][0].rect.right - board[0][0].rect.left) - 7) {
			player.rect = { board[boardX / 2][boardY / 2].rect.left + 4,board[boardX / 2][boardY / 2].rect.top + 4,board[boardX / 2][boardY / 2].rect.right - 4,board[boardX / 2][boardY / 2].rect.bottom - 4 };
			player.nowBullet = 6;
			player.hp = 1;
			player.state = LIVE;
			PlayerSpawn();
		}
		return;
	}
	double x = (double)boardWidth / boardX;
	double y = (double)boardHeight / boardY;
	RECT rect;
	if (pressW) {
		rect = { player.rect.left,player.rect.top - PlayerSpeed,player.rect.right,player.rect.bottom - PlayerSpeed };
		if (rect.top < 0) {
			rect.bottom = rect.bottom - rect.top + 1;
			rect.top = 1;
			player.rect = rect;
		}
		int leftX = rect.left / x;
		int rightX = rect.right / x;
		int topY = rect.top / y;
		if (leftX<0 || leftX>boardX || rightX<0 || rightX>boardX || topY<0 || topY>boardY) return;
		if (board[leftX][topY].RGB != RGB(255, 255, 255) || board[rightX][topY].RGB != RGB(255, 255, 255)) {
			if (board[leftX][topY].RGB != RGB(255, 255, 255)) {
				rect.bottom = board[leftX][topY].rect.bottom + (rect.bottom - rect.top) + 1;
				rect.top = board[leftX][topY].rect.bottom + 1;
			}
			else if (board[rightX][topY].RGB != RGB(255, 255, 255)) {
				rect.bottom = board[rightX][topY].rect.bottom + (rect.bottom - rect.top) + 1;
				rect.top = board[rightX][topY].rect.bottom + 1;
			}
		}
		player.rect = rect;
	}
	if (pressA) {
		rect = { player.rect.left - PlayerSpeed,player.rect.top,player.rect.right - PlayerSpeed,player.rect.bottom };
		if (rect.left < 0) {
			rect.right = rect.right - rect.left + 1;
			rect.left = 1;
			player.rect = rect;
		}
		int leftX = rect.left / x;
		int topY = rect.top / y;
		int bottomY = rect.bottom / y;
		if (leftX<0 || leftX>boardX || topY<0 || topY>boardY || bottomY<0 || bottomY>boardY) return;
		if (board[leftX][topY].RGB != RGB(255, 255, 255) || board[leftX][bottomY].RGB != RGB(255, 255, 255)) {
			if (board[leftX][topY].RGB != RGB(255, 255, 255)) {
				rect.right = board[leftX][topY].rect.right + (rect.right - rect.left) + 1;
				rect.left = board[leftX][topY].rect.right + 1;
			}
			else if (board[leftX][bottomY].RGB != RGB(255, 255, 255)) {
				rect.right = board[leftX][bottomY].rect.right + (rect.right - rect.left) + 1;
				rect.left = board[leftX][bottomY].rect.right + 1;
			}
		}
		player.rect = rect;
	}
	if (pressS) {
		rect = { player.rect.left,player.rect.top + PlayerSpeed,player.rect.right,player.rect.bottom + PlayerSpeed };
		if (rect.bottom >= boardHeight) {
			rect.top = boardHeight - (rect.bottom - rect.top) - 1;
			rect.bottom = boardHeight - 1;
			player.rect = rect;
		}
		int leftX = rect.left / x;
		int rightX = rect.right / x;
		int bottomY = rect.bottom / y;
		if (leftX<0 || leftX>boardX || rightX<0 || rightX>boardX || bottomY<0 || bottomY>boardY) return;
		if (board[leftX][bottomY].RGB != RGB(255, 255, 255) || board[rightX][bottomY].RGB != RGB(255, 255, 255)) {
			if (board[leftX][bottomY].RGB != RGB(255, 255, 255)) {
				rect.top = board[leftX][bottomY].rect.top - (rect.bottom - rect.top) - 1;
				rect.bottom = board[leftX][bottomY].rect.top - 1;
			}
			else if (board[rightX][bottomY].RGB != RGB(255, 255, 255)) {
				rect.top = board[rightX][bottomY].rect.top - (rect.bottom - rect.top) - 1;
				rect.bottom = board[rightX][bottomY].rect.top - 1;
			}
		}
		player.rect = rect;
	}
	if (pressD) {
		rect = { player.rect.left + PlayerSpeed,player.rect.top,player.rect.right + PlayerSpeed,player.rect.bottom };
		if (rect.right >= boardWidth) {
			rect.left = boardWidth - (rect.right - rect.left) - 1;
			rect.right = boardWidth - 1;
			player.rect = rect;
		}
		int rightX = rect.right / x;
		int topY = rect.top / y;
		int bottomY = rect.bottom / y;
		if (rightX<0 || rightX>boardX || topY<0 || topY>boardY || bottomY<0 || bottomY>boardY) return;
		if (board[rightX][topY].RGB != RGB(255, 255, 255) || board[rightX][bottomY].RGB != RGB(255, 255, 255)) {
			if (board[rightX][topY].RGB != RGB(255, 255, 255)) {
				rect.left = board[rightX][topY].rect.left - (rect.right - rect.left) - 1;
				rect.right = board[rightX][topY].rect.left - 1;
			}
			else if (board[rightX][bottomY].RGB != RGB(255, 255, 255)) {
				rect.left = board[rightX][bottomY].rect.left - (rect.right - rect.left) - 1;
				rect.right = board[rightX][bottomY].rect.left - 1;
			}
		}
		player.rect = rect;
	}
}

void shotBullet() {
	if (player.nowBullet == 0) return;

	--player.nowBullet;
	int middleX = player.rect.left + (double)(player.rect.right - player.rect.left) / 2, middleY = player.rect.top + (double)(player.rect.bottom - player.rect.top) / 2;
	int radius = (int)((double)(middleX - player.rect.left) / 5 * 3);
	bulletRadius = (int)((double)radius / 5 * 2.5);

	{	//새 탄환 추가
		int now = bullet[0].bulletCount;
		bullet[now].hp = 1;
		bullet[now].rect = { middleX - bulletRadius,middleY - bulletRadius, middleX + bulletRadius,middleY + bulletRadius };
		bullet[now].shotTime = 0;
		if (pressLEFT) {
			bullet[now].state = LEFT;
		}
		else if (pressRIGHT) {
			bullet[now].state = RIGHT;
		}
		else if (pressUP) {
			bullet[now].state = UP;
		}
		else if (pressDOWN) {
			bullet[now].state = DOWN;
		}
		++bullet[0].bulletCount;
	}

	{	//만약 차지중이었다면 2개 더 추가하기
		if (player.charge >= ChargeTime) {
			int now = bullet[0].bulletCount;
			bullet[now].hp = 1;
			bullet[now].shotTime = 0;
			if (pressLEFT) {
				bullet[now].state = LEFT;
				bullet[now].rect = { middleX - bulletRadius,middleY - bulletRadius - bulletRadius * 5, middleX + bulletRadius,middleY + bulletRadius - bulletRadius * 5 };
			}
			else if (pressRIGHT) {
				bullet[now].state = RIGHT;
				bullet[now].rect = { middleX - bulletRadius,middleY - bulletRadius - bulletRadius * 5, middleX + bulletRadius,middleY + bulletRadius - bulletRadius * 5 };
			}
			else if (pressUP) {
				bullet[now].state = UP;
				bullet[now].rect = { middleX - bulletRadius - bulletRadius * 5,middleY - bulletRadius, middleX + bulletRadius - bulletRadius * 5,middleY + bulletRadius };
			}
			else if (pressDOWN) {
				bullet[now].state = DOWN;
				bullet[now].rect = { middleX - bulletRadius - bulletRadius * 5,middleY - bulletRadius, middleX + bulletRadius - bulletRadius * 5,middleY + bulletRadius };
			}
			++bullet[0].bulletCount;


			now = bullet[0].bulletCount;
			bullet[now].hp = 1;
			bullet[now].shotTime = 0;
			if (pressLEFT) {
				bullet[now].state = LEFT;
				bullet[now].rect = { middleX - bulletRadius,middleY - bulletRadius + bulletRadius * 5, middleX + bulletRadius,middleY + bulletRadius + bulletRadius * 5 };
			}
			else if (pressRIGHT) {
				bullet[now].state = RIGHT;
				bullet[now].rect = { middleX - bulletRadius,middleY - bulletRadius + bulletRadius * 5, middleX + bulletRadius,middleY + bulletRadius + bulletRadius * 5 };
			}
			else if (pressUP) {
				bullet[now].state = UP;
				bullet[now].rect = { middleX - bulletRadius + bulletRadius * 5,middleY - bulletRadius, middleX + bulletRadius + bulletRadius * 5,middleY + bulletRadius };
			}
			else if (pressDOWN) {
				bullet[now].state = DOWN;
				bullet[now].rect = { middleX - bulletRadius + bulletRadius * 5,middleY - bulletRadius, middleX + bulletRadius + bulletRadius * 5,middleY + bulletRadius };
			}
			++bullet[0].bulletCount;
		}

	}
	{	//차지 관련 설정
		player.charge = 0;
		pressSPACE = false;
	}
	pressUP = false;
	pressDOWN = false;
	pressLEFT = false;
	pressRIGHT = false;
}
void BulletToSomething() {
	if (bullet[0].bulletCount == 0) return;
	double x = (double)boardWidth / boardX;
	double y = (double)boardHeight / boardY;
	for (int i = 0; i < bullet[0].bulletCount; ++i) {
		if (bullet[i].state == LEFT) {
			if (bullet[i].rect.left < 0) {		//bullet[i].rect.right< 0
				if (bullet[i].rect.right + bullet[i].shotTime < 0) {
					DieBullet(i);
					--i;
				}
				continue;
			}
		}
		else if (bullet[i].state == RIGHT) {
			if (bullet[i].rect.right > boardWidth) {//bullet[i].rect.left> boardWidth
				if (bullet[i].rect.left - bullet[i].shotTime > boardWidth) {
					DieBullet(i);
					--i;
				}
				continue;
			}
		}
		else if (bullet[i].state == UP) {
			if (bullet[i].rect.top < 0) {//bullet[i].rect.bottom <0
				if (bullet[i].rect.bottom + bullet[i].shotTime < 0) {
					DieBullet(i);
					--i;
				}
				continue;
			}
		}
		else if (bullet[i].state == DOWN) {
			if (bullet[i].rect.bottom > boardHeight) {//bullet[i].rect.top >boardHeight
				if (bullet[i].rect.top - bullet[i].shotTime > boardHeight) {
					DieBullet(i);
					--i;
				}
				continue;
			}
		}
		int leftX = bullet[i].rect.left / x;
		int topY = bullet[i].rect.top / y;
		int rightX = bullet[i].rect.right / x;
		int bottomY = bullet[i].rect.bottom / y;

		if (board[leftX][topY].RGB == RGB(0, 0, 0)) {
			board[leftX][topY].RGB = RGB(255, 255, 255);
		}
		if (board[leftX][topY].RGB == RGB(255, 0, 0)) {
			DieBullet(i);
			--i;
			continue;
		}

		if (board[leftX][bottomY].RGB == RGB(0, 0, 0)) {
			board[leftX][bottomY].RGB = RGB(255, 255, 255);
		}
		if (board[leftX][bottomY].RGB == RGB(255, 0, 0)) {
			DieBullet(i);
			--i;
			continue;
		}

		if (board[rightX][topY].RGB == RGB(0, 0, 0)) {
			board[rightX][topY].RGB = RGB(255, 255, 255);
		}
		if (board[rightX][topY].RGB == RGB(255, 0, 0)) {
			DieBullet(i);
			--i;
			continue;
		}

		if (board[rightX][bottomY].RGB == RGB(0, 0, 0)) {
			board[rightX][bottomY].RGB = RGB(255, 255, 255);
		}
		if (board[rightX][bottomY].RGB == RGB(255, 0, 0)) {
			DieBullet(i);
			--i;
			continue;
		}

	}
}
void BulletToMonster() {
	for (int j = 0; j < monster[0].nowMonster; ++j) {
		if (monster[j].state != LIVE) continue;
		for (int i = 0; i < bullet[0].bulletCount; ++i) {
			RECT nullRect;
			if (IntersectRect(&nullRect, &bullet[i].rect, &monster[j].rect)) {
				DieBullet(i);
				BombMonsterCheck(monster[j].rect, j);
				--i;
			}
		}
	}

}
void BulletMove() {
	for (int i = 0; i < bullet[0].bulletCount; ++i) {
		if (bullet[i].state == LEFT) {
			bullet[i].rect.left -= BulletSpeed;
			bullet[i].rect.right -= BulletSpeed;
		}
		else if (bullet[i].state == RIGHT) {
			bullet[i].rect.left += BulletSpeed;
			bullet[i].rect.right += BulletSpeed;
		}
		else if (bullet[i].state == UP) {
			bullet[i].rect.top -= BulletSpeed;
			bullet[i].rect.bottom -= BulletSpeed;
		}
		else if (bullet[i].state == DOWN) {
			bullet[i].rect.top += BulletSpeed;
			bullet[i].rect.bottom += BulletSpeed;
		}
		bullet[i].shotTime += 8;
		if (bullet[i].shotTime > 150) bullet[i].shotTime = 150;
	}
}
void CheckPlusBullet() {
	double x = (double)boardWidth / boardX;
	double y = (double)boardHeight / boardY;
	double playerX = player.rect.left + (double)(player.rect.right - player.rect.left) / 2;
	double playerY = player.rect.top + (double)(player.rect.bottom - player.rect.top) / 2;
	int X = playerX / x;
	int Y = playerY / y;

	if (board[X][Y].plusBullet > 0) {
		player.nowBullet += board[X][Y].plusBullet;
		board[X][Y].plusBullet = 0;
	}
}
void DieBullet(int dieNum) {
	bullet[dieNum] = bullet[--bullet[0].bulletCount];
	bullet[bullet[0].bulletCount].InitBullet();
}

void SpawnMonster() {
	if (monster[0].nowMonster >= MaxMonsterNum) return;
	int spawnNum = rand() % 3 + 1;
	if (monster[0].nowMonster + spawnNum > MaxMonsterNum) {
		--monster[0].reSpawn;
	}
	else {
		double x = (double)boardWidth / boardX;
		double y = (double)boardHeight / boardY;
		for (int i = 0; i < spawnNum; ++i) {
			int now = monster[0].nowMonster;
			monster[now].hp = 1;
			int monsterX = uid(rd) % boardX, monsterY = uid(rd) % boardY;
			monster[now].rect = { board[monsterX][monsterY].rect.left - 30,board[monsterX][monsterY].rect.top - 30,board[monsterX][monsterY].rect.right + 30,board[monsterX][monsterY].rect.bottom + 30 };
			//int monsterX = uid(rd) % boardWidth, monsterY = uid(rd) % boardHeight;

			monster[now].state = SPAWN;
			++monster[0].nowMonster;
		}
		monsterRespawnTime -= 150;
		if (monsterRespawnTime < MonsterSpawnTime / 3) monsterRespawnTime = MonsterSpawnTime / 3;
		monster[0].reSpawn = 0;
	}

}
void MonsterMove() {
	int playerX = 0, playerY = 0;
	playerX = player.rect.left + (double)(player.rect.right - player.rect.left) / 2;
	playerY = player.rect.top + (double)(player.rect.bottom - player.rect.top) / 2;
	for (int i = 0; i < monster[0].nowMonster; ++i) {
		if (monster[i].state == SPAWN) {
			monster[i].rect.left += 1;
			monster[i].rect.top += 1;
			monster[i].rect.right -= 1;
			monster[i].rect.bottom -= 1;
			if (monster[i].rect.right - monster[i].rect.left < board[0][0].rect.right - board[0][0].rect.left - 4) {
				monster[i].state = LIVE;
			}
		}
		else if (monster[i].state == LIVE) {
			
			for (int j = 0; j < MonsterSpeed; ++j) {
				int monsterX = monster[i].rect.left + (double)(monster[i].rect.right - monster[i].rect.left) / 2;
				int monsterY = monster[i].rect.top + (double)(monster[i].rect.bottom - monster[i].rect.top) / 2;
				if (monsterX < playerX) {
					monster[i].rect.left += 1;
					monster[i].rect.right += 1;
					if (monster[i].rect.right >= boardWidth) {
						monster[i].rect.left -= 1;
						monster[i].rect.right -= 1;
					}
				}
				else if (monsterX > playerX) {
					monster[i].rect.left -= 1;
					monster[i].rect.right -= 1;
					if (monster[i].rect.left <= 0) {
						monster[i].rect.left += 1;
						monster[i].rect.right += 1;
					}
				}
				if (monsterY < playerY) {
					monster[i].rect.top += 1;
					monster[i].rect.bottom += 1;
					if (monster[i].rect.bottom >= boardHeight) {
						monster[i].rect.top -= 1;
						monster[i].rect.bottom -= 1;
					}
				}
				else if (monsterY > playerY) {
					monster[i].rect.top -= 1;
					monster[i].rect.bottom -= 1;
					if (monster[i].rect.top <= 0) {
						monster[i].rect.top += 1;
						monster[i].rect.bottom += 1;
					}
				}
			}
		}
	}
}
void BombMonsterCheck(RECT monsterRect, int dieMonsterNum) {
	double x = (double)boardWidth / boardX;
	double y = (double)boardHeight / boardY;
	double monsterX = monsterRect.left + (double)(monsterRect.right - monsterRect.left) / 2;
	double monsterY = monsterRect.top + (double)(monsterRect.bottom - monsterRect.top) / 2;
	int X = monsterX / x;
	int Y = monsterY / y;
	if (X >= boardX || Y >= boardY) {
		if (X >= boardX) X = boardX - 1;
		if (Y >= boardY) Y = boardY - 1;
	}

	RECT hatchRect = { board[X][Y].rect.left - x * 1,board[X][Y].rect.top - y * 1,board[X][Y].rect.right + x * 1,board[X][Y].rect.bottom + y * 1 };      //빗금영역이라는 뜻

	if (hatchRect.left <= 0)hatchRect.left = 0;
	if (hatchRect.top <= 0)hatchRect.top = 0;
	if (hatchRect.right >= boardWidth)hatchRect.right = boardWidth - 1;
	if (hatchRect.bottom >= boardHeight)hatchRect.bottom = boardHeight - 1;
	for (int i = 0; i < monster[0].nowMonster; ++i) {
		if (monster[i].hp <= 0) continue;
		if (monster[i].state != LIVE)continue;
		RECT nullRect;
		if (IntersectRect(&nullRect, &monster[i].rect, &hatchRect)) {
			--monster[i].hp;
			shakeScreen = ShakeScreenTime;/*ㅁㄴㅇasd
			CreateEffect(monster[i].rect);*/
			BombMonsterCheck(monster[i].rect, i);
		}
	}
}
void CheckMonsterHp() {
	double x = (double)boardWidth / boardX;
	double y = (double)boardHeight / boardY;

	for (int i = 0; i < monster[0].nowMonster; ++i) {
		if (monster[i].hp <= 0) {
			{	//추가 탄환 추가
				double monsterX = monster[i].rect.left + (double)(monster[i].rect.right - monster[i].rect.left) / 2;
				double monsterY = monster[i].rect.top + (double)(monster[i].rect.bottom - monster[i].rect.top) / 2;
				int X = monsterX / x;
				int Y = monsterY / y;
				if (uid(rd) % 100 + PlusBulletRate > 100) {	//특수탄환 확률 20%
					int rand = uid(rd) % 100;
					if (rand < 75) {		//20%에서 75% (전체 중 12%)
						board[X][Y].plusBullet += 1;
					}
					else if (rand < 95) {	//20%에서 20%  (전체 중 6%)
						board[X][Y].plusBullet += 2;
					}
					else if (rand < 100) {	//20%에서 5%  
						board[X][Y].plusBullet += 3;
					}


					if (board[X][Y].plusBullet > 3) board[X][Y].plusBullet = 3;
				}


			}

			BoardBlackToWhite(monster[i].rect);
			CreateEffect(monster[i].rect);
			DieMonster(i);
			--i;
		}
	}
}
void MonsterToPlayer() {
	RECT nullRect;
	if (player.hp < 0 || player.state != LIVE) return;
	for (int i = 0; i < monster[0].nowMonster; ++i) {
		if (monster[i].state == SPAWN) continue;
		if (IntersectRect(&nullRect, &monster[i].rect, &player.rect)) {
			--player.hp;
			break;
		}
	}
	if (player.hp <= 0) {
		player.state = DIE;
		--player.life;
		shakeScreen = ShakeScreenTime;
		CreateEffect(player.rect);
		player.reSpawn = 1000;

		int randX = uid(rd) % boardWidth;
		int randY = uid(rd) % boardHeight;
		player.rect = { randX - 5,randY - 5,randX + 5,randY + 5 };
		player.nowBullet = 6;
		player.hp = 1;
	}
}
void MonsterChangeBoard() {
	double x = (double)boardWidth / boardX;
	double y = (double)boardHeight / boardY;
	for (int i = 0; i < monster[0].nowMonster; ++i) {
		if (monster[i].state == SPAWN) continue;
		if (monster[i].hp < 0) continue;
		double monsterX = monster[i].rect.left + (double)(monster[i].rect.right - monster[i].rect.left) / 2;
		double monsterY = monster[i].rect.top + (double)(monster[i].rect.bottom - monster[i].rect.top) / 2;
		int X = monsterX / x;
		int Y = monsterY / y;
		if (X >= boardX || Y >= boardY) {
			if (X >= boardX) X = boardX - 1;
			if (Y >= boardY) Y = boardY - 1;
		}
		if (board[X][Y].RGB == RGB(255, 255, 255)) {
			RECT nullRect;
			if (!(IntersectRect(&nullRect, &player.rect, &board[X][Y].rect))) {
				board[X][Y].RGB = RGB(0, 0, 0);
				board[X][Y].plusBullet = 0;
			}
		}
	}
}
void BoardBlackToWhite(RECT monsterRect) {
	double x = (double)boardWidth / boardX;
	double y = (double)boardHeight / boardY;
	double monsterX = monsterRect.left + (double)(monsterRect.right - monsterRect.left) / 2;
	double monsterY = monsterRect.top + (double)(monsterRect.bottom - monsterRect.top) / 2;
	int X = monsterX / x;
	int Y = monsterY / y;
	RECT hatchRect = { board[X][Y].rect.left - x,board[X][Y].rect.top - y,board[X][Y].rect.right + x,board[X][Y].rect.bottom + y };

	RECT nullRect;
	for (int i = 0; i < boardX; ++i) {
		for (int j = 0; j < boardY; ++j) {
			if (IntersectRect(&nullRect, &board[i][j].rect, &hatchRect)) {
				if (board[i][j].RGB == RGB(0, 0, 0))
					board[i][j].RGB = RGB(255, 255, 255);
			}
		}
	}

}
void DieMonster(int dieMonsterNum) {
	player.comboTime = ComboTime;
	++player.comboCount;
	score += MonsterDieScore * comboPlusScore;
	plusScore.plusScore += MonsterDieScore * comboPlusScore;
	plusScore.printTime = PlusScorePrintTime;
	monster[dieMonsterNum] = monster[--monster[0].nowMonster];
	monster[monster[0].nowMonster].InitMonster();
}

void CreateEffect(RECT createRect) {
	int now = effect[0].effectNum;
	if (now >= MaxEffectNum) return;
	int createWidth = createRect.right - createRect.left;
	int createHeight = createRect.bottom - createRect.top;
	effect[now].time = EffectTime;
	for (int i = 0; i < 5; ++i) {
		effect[now].bomb[i].x = uid(rd) % createWidth + createRect.left;
		effect[now].bomb[i].y = uid(rd) % createHeight + createRect.top;
		effect[now].bomb[i].radius = uid(rd) % 45 + 70;
	}
	++effect[0].effectNum;
}
void DieEffect(int dieNum) {
	effect[dieNum] = effect[--effect[0].effectNum];
	effect[effect[0].effectNum].InitEffect();
}

void CALLBACK GameUpdate() {
	{	//게임 끝내기 전용 지역
		if (player.life <= 0 || score == MaxScore) {
			if (pressESC) {
				start = false;
				setTimerTitle = true;
				InitGame();
			}
			else {
				//TODO: asd
				end = true;
				DrawGame();
			}
			InvalidateRect(hWnd, NULL, false);
			return;
		}
	}

	{	//중단 체크
		if (pause || pressLBUTTON || pressESC) {
			if (pressESC) {
				if (pause) pause = false;
				else pause = true;
				pressESC = false;
			}
			if (pressLBUTTON) {
				if (pause) {		//현재가 중단된 상태라면 중앙 3개밖에 없으니까.
					if (PtInRect(&pauseStart, { mx,my })) {
						pause = false;
					}
					else if (PtInRect(&pauseMenu, { mx,my })) {
						pause = false;
						start = false;
						setTimerTitle = true;
						InitGame();
					}
				}
				else {				//현재가 중단된 상태가 아니면 중단되려는지를 확인해아 하니까.
					if (PtInRect(&pauseRect, { mx,my })) pause = true;
				}
			}

			DrawGame();
			InvalidateRect(hWnd, NULL, false);
			return;
		}
	}

	{	//추가 점수 0점으로 초기화 및 시간 조정
		if (plusScore.printTime > 0) {
			--plusScore.printTime -= Game_Update_Speed;
			if (plusScore.printTime < 0) {
				plusScore.plusScore = 0;
				plusScore.printTime = 0;
			}
		}
	}
	{	//콤보 시간 체크 및 콤보 추가 점수 체크
		if (player.comboTime > 0) {
			player.comboTime -= Game_Update_Speed;
			if (player.comboTime < 0) {
				player.comboTime = 0;
				player.comboCount = 0;
			}
		}
		//5의 배수로 점수 배율이 1배~2배
		if (player.comboCount >= 49) comboPlusScore = 2;
		else if (player.comboCount >= 44) comboPlusScore = 1.9;
		else if (player.comboCount >= 39) comboPlusScore = 1.8;
		else if (player.comboCount >= 34) comboPlusScore = 1.7;
		else if (player.comboCount >= 29) comboPlusScore = 1.6;
		else if (player.comboCount >= 24) comboPlusScore = 1.5;
		else if (player.comboCount >= 19) comboPlusScore = 1.4;
		else if (player.comboCount >= 14) comboPlusScore = 1.3;
		else if (player.comboCount >= 9) comboPlusScore = 1.2;
		else if (player.comboCount >= 4) comboPlusScore = 1.1;
		else comboPlusScore = 1;
	}
	{	//점수 추가
		static int wait = 0;
		if (player.state == LIVE) {
			++wait;
			if (wait % 5 == 0) {
				++score;
				wait = 0;
			}
			if (score > MaxScore)score = MaxScore;
		}
	}
	{	//플레이어 총알 위치 변경
		player.bulletAngle += 5;
		if (player.bulletAngle > 179) player.bulletAngle -= 360;
	}
	{	//보드 추가 총알 위치 변경
		board[0][0].plusBulletAngle += 5;
		if (board[0][0].plusBulletAngle > 179) board[0][0].plusBulletAngle -= 360;
	}
	{	//차징
		if (pressSPACE) {
			player.bulletAngle += 10;
			if (player.bulletAngle > 179) player.bulletAngle -= 360;

			player.charge += Game_Update_Speed;
			if (player.charge >= ChargeTime) player.charge = ChargeTime;
		}
	}
	{	//총알 - 블럭 / 총알 - 적군 충돌체크 및 전체 총알 이동  
		BulletToSomething();
		BulletToMonster();
		BulletMove();
	}

	{	//피없는 몬스터 죽이기
		CheckMonsterHp();
	}

	{	//총알 날리기
		if (player.state == LIVE) {
			if (pressLEFT || pressRIGHT || pressUP || pressDOWN) shotBullet();
		}
	}
	{	//총알 추가
		++player.reload;
		if (player.nowBullet >= 6) player.reload = 0;
		if (player.reload > BulletReloadTime / Game_Update_Speed) {		//대략 BulletReloadTime당 한번이라는 뜻
			if (player.nowBullet < 6) {
				++player.nowBullet;
				player.reload = 0;
			}
			else {
				player.reload = 0;
			}
		}

	}

	{	//몬스터 - 플레이어 충돌 체크
		MonsterToPlayer();
	}

	{	//플레이어 부활 처리 및 플레이어 이동 처리
		PlayerMove();
		CheckPlusBullet();
	}

	{	//몬스터 이동 처리
		MonsterMove();
		MonsterChangeBoard();
	}
	{	//몬스터 추가
		if (++monster[0].reSpawn > monsterRespawnTime / Game_Update_Speed) {
			SpawnMonster();
		}
	}

	{	//이펙트 처리
		for (int i = 0; i < effect[0].effectNum; ++i) {
			if (effect[i].time <= 0) {
				DieEffect(i);
				--i;
				continue;
			}
			effect[i].time -= Game_Update_Speed;
			for (int j = 0; j < 5; ++j) {
				effect[i].bomb[j].radius -= 7;
				if (effect[i].bomb[j].radius < 0) effect[i].bomb[j].radius = 0;
			}
		}
	}

	DrawBoard();
	DrawGame();
	InvalidateRect(hWnd, NULL, false);
}
void SettingTimer() {
	if (start)KillTimer(hWnd, Timer_Title_Update);
	if (!start)KillTimer(hWnd, Timer_Game_Update);
	if (setTimerTitle) {
		SetTimer(hWnd, Timer_Title_Update, Title_Update_Speed, (TIMERPROC)DrawTitle);
		setTimerTitle = false;
	}
	if (setTimerGame) {
		SetTimer(hWnd, Timer_Game_Update, Game_Update_Speed, (TIMERPROC)GameUpdate);
		setTimerGame = false;
	}
}
void InitGame() {
	boardX = 0;
	boardY = 0;
	for (int i = 0; i < 20; ++i) {
		for (int j = 0; j < 14; ++j) {
			board[i][j].rect = { 0,0,0,0 };
			board[i][j].RGB = RGB(0, 0, 0);
			board[i][j].plusBullet = 0;
		}
	}

	for (int i = 0; i < monster[0].nowMonster; ++i) {
		monster[i].InitMonster();
	}
	monster[0].nowMonster = 0;
	monster[0].reSpawn = 0;

	player.life = PlayerLife;
	player.charge = 0;
	player.bulletAngle = 0;
	player.comboCount = 0;
	player.comboTime = 0;
	player.hp = 0;
	player.nowBullet = 0;
	player.rect = { 0,0,0,0 };
	player.reload = 0;
	player.reSpawn = 0;
	player.state = DIE;


	for (int i = 0; i < bullet[0].bulletCount; ++i) {
		bullet[i].InitBullet();
	}
	bullet[0].bulletCount = 0;

	for (int i = 0; i < effect[0].effectNum; ++i) {
		effect[i].InitEffect();
	}
	effect[0].effectNum = 0;

	monsterRespawnTime = MonsterSpawnTime;
	score = 0;
	shakeScreen = 0;

	oldBit1 = (HBITMAP)SelectObject(mainMemdc, boardBit);
	Rectangle(mainMemdc, -1, -1, boardWidth, boardHeight);
	SelectObject(mainMemdc, oldBit1);

	end = false;
}
void SetEasyGame() {
	boardX = 12;
	boardY = 8;
	double x = (double)boardWidth / boardX;
	double y = (double)boardHeight / boardY;
	for (int i = 0; i < boardX; ++i) {
		for (int j = 0; j < boardY; ++j) {
			board[i][j].rect = { (int)(x * i),(int)(y * j),(int)(x * (i + 1)),(int)(y * (j + 1)) };
			board[i][j].RGB = RGB(0, 0, 0);
			board[i][j].plusBullet = 0;
		}
	}

	PlayerSpawn();

	player.life = PlayerLife;
	monster[0].nowMonster = 0;
	bullet[0].bulletCount = 0;
	effect[0].effectNum = 0;
	monsterRespawnTime = MonsterSpawnTime * 1.5;
	score = 0;
	shakeScreen = 0;
}
void SetNormalGame() {
	boardX = 20;
	boardY = 14;
	double x = (double)boardWidth / boardX;
	double y = (double)boardHeight / boardY;
	for (int i = 0; i < boardX; ++i) {
		for (int j = 0; j < boardY; ++j) {
			board[i][j].rect = { (int)(x * i),(int)(y * j),(int)(x * (i + 1)),(int)(y * (j + 1)) };
			board[i][j].RGB = RGB(0, 0, 0);
			board[i][j].plusBullet = 0;
		}
	}

	PlayerSpawn();

	player.life = PlayerLife;
	monster[0].nowMonster = 0;
	bullet[0].bulletCount = 0;
	effect[0].effectNum = 0;
	monsterRespawnTime = MonsterSpawnTime;
	score = 0;
	shakeScreen = 0;
}
void SetHardGame() {
	boardX = 20;
	boardY = 14;
	double x = (double)boardWidth / boardX;
	double y = (double)boardHeight / boardY;
	for (int i = 0; i < boardX; ++i) {
		for (int j = 0; j < boardY; ++j) {
			board[i][j].rect = { (int)(x * i),(int)(y * j),(int)(x * (i + 1)),(int)(y * (j + 1)) };
			board[i][j].RGB = RGB(0, 0, 0);
			board[i][j].plusBullet = 0;
		}
	}
	for (int i = 0; i < 7; ++i) {
		board[uid(rd) % boardX][uid(rd) % boardY].RGB = RGB(255, 0, 0);
	}
	PlayerSpawn();

	player.life = PlayerLife;
	monster[0].nowMonster = 0;
	bullet[0].bulletCount = 0;
	effect[0].effectNum = 0;
	monsterRespawnTime = MonsterSpawnTime;
	score = 0;
	shakeScreen = 0;

}