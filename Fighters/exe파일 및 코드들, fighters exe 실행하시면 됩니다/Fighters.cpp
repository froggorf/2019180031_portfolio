// no warning //
#pragma warning(disable : 4244)

// console //

#if defined(DEBUG) | defined(_DEBUG)
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif

// header //

#include <windows.h>
#include <tchar.h>
#include <atlimage.h>
#include <math.h>
#include <random>
#include <time.h>

// define (window) //

#define WIDTH 1200
#define HEIGHT 900

// define (useful) //

#define PI 3.141592

// define (player) //

#define CharacterMaxHp 100          
#define FallingDamage 7             
#define CharacterWidth 60
#define CharacterHeight 100      
#define CharacterGravitySpeed 9
#define CharacterSpeed 10
#define GravityAcceleration   1

#define ChangeToFallingSpeed 80

// define (game) //      //define 4 elevator

#define ELEVATOR 20
#define EMOVE 10

#define BULLETNUM 50
#define BULLETSPEED 90

#define AIMINGTIME 2
#define RELOADTIME 4

#define GRENADENUM 100
#define GRENADESIZE 15
#define GRENADESPEED 60
#define GRENADETIME 70

#define EXPLOSIONTNUM 10
#define EXPLOSIONSIZE 150

#define ITEMNUM 5
#define ITEMWIDTH 100
#define ITEMHEIGHT 50
#define ITEMTIME 300

#define PUNCHDAMAGE 5
#define PISTOLDAMAGE 7
#define UZIDAMAGE 3
#define GRENADEDAMAGE 30

#define SMALLKIT 30
#define BIGKIT 50

// enum (screen) //

enum class eScreen {
	main1,
	main2,
	help,
	select,
	map1,
	map2
};

// enum (hand to hand) //

enum class eHand {
	none,
	sword,
	axe
};

// enum (총) //

enum class eGun {
	none,
	pistol,
	uzi
};

// enum (throwing) //

enum class eThrowing {
	none,
	grenade
};

// enum (item) //

enum class eItem {
	none,
	axe,
	sword,
	pistol,
	uzi,
	grenade,
	smallkit,
	bigkit
};

// data (winmain) //

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"Window Programming Lab";

// callback //

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime);

// data (print) //

HWND hWnd;
HPEN oldpen{};
HBRUSH oldbrush{};
HFONT oldfont{};

// CImage (item) //

CImage Axe{};
CImage Sword{};
CImage Pistol{};
CImage Uzi{};
CImage Grenade{};
CImage Smallkit{};
CImage Bigkit{};

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
	hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, WIDTH, HEIGHT, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

// class //

enum CharacterStatus { LEFT_STAY, RIGHT_STAY, LEFT_RUN, RIGHT_RUN, LEFT_WALK, RIGHT_WALK, LEFT_JUMP, RIGHT_JUMP, LADDER_UP, LADDER_DOWN, SHOCK, LEFT_FALLING, RIGHT_FALLING, LEFT_STANDING, RIGHT_STANDING, LEFT_SIT, RIGHT_SIT, LEFT_ROLL, RIGHT_ROLL, LEFT_AIMING, RIGHT_AIMING, LEFT_PUNCH, RIGHT_PUNCH, LEFT_SHOCK, RIGHT_SHOCK };      //character status에 사용할 enum 향후 MELEE_LEFT,MELEE_RIGHT, 등등도 포함할 것
class Character {            //플레이어 클래스
public:
	int status;               //플레이어의 상태, 이 상태로 몇가지 처리 및 상태 및 애니메이션 이미지를 나타낼 것임

	int player{ 0 };      // 플레이어 넘버
	RECT playerRect;         //플레이어 RECT
	RECT imgRect;            //이미지 범위
	int GravitySpeed;         //플레이어 중력 속도(중력가속도 적용하여 변경 예정)
	int speed;               //플레이어 이동 속도
	int radian{};      // 라디안 변수 추가
	int aimingtimer{ -1 };      // 조준 해제 타이머
	int reloadtimer{ -1 };      // 장전 타이머
	int shoottime{ 0 };
	int hp = CharacterMaxHp;

	eHand hand = eHand::none;
	eGun gun = eGun::pistol;
	int gunnum{ 8 };
	eThrowing throwing = eThrowing::grenade;
	int throingnum{ 3 };

	BOOL usedPortal = FALSE;      //플레이어 포탈 변수 추가
	BOOL powerUp = FALSE;      //파워업 변수 추가   
	BOOL downJump = FALSE;      //밑점프 변수 추가
	BOOL Shoot = FALSE;      // 사격 상태
	BOOL Gunaming = FALSE;

	POINT Get_P() { POINT pt = { (playerRect.left + playerRect.right) / 2, (playerRect.top + playerRect.bottom) / 2 }; return pt; };
};

class Bullet {
private:
	int player{ 0 };
	POINT point{ 0 };
	POINT prepoint{ 0 };
	int type{ 0 };
	int rad{ 0 };

public:
	POINT points[BULLETSPEED / 5]{ 0 };

	Bullet() {};
	Bullet(int p, int t, POINT pt, int r) : player(p), type(t), rad(r), point({ pt }) {
		std::random_device rd;
		std::uniform_int_distribution<> uid(-2, 2);

		rad += uid(rd);
	};

	int Get_P() { return player; };
	int Get_T() { return type; };

	void Delete_() { player = 0; };

	void Move_();

	HDC Print_(HDC);
};

void Bullet::Move_() {
	prepoint = point;

	point.x += (int)((double)BULLETSPEED * cos((float)rad * PI / 180));
	point.y += (int)((double)BULLETSPEED * sin((float)rad * PI / 180));

	for (int i = 0; i < BULLETSPEED; i += 5) {
		points[i / 5].x = prepoint.x + (i * cos((float)rad * PI / 180));
		points[i / 5].y = prepoint.y + (i * sin((float)rad * PI / 180));
	}

	if (point.x < 0 || point.x > 2400 || point.y < 0 || point.y > 1800) {
		player = 0;
	}
}

HDC Bullet::Print_(HDC mdc) {
	HPEN hpen = CreatePen(1, 10, RGB(255, 255, 0));
	oldpen = (HPEN)SelectObject(mdc, hpen);

	MoveToEx(mdc, prepoint.x, prepoint.y, NULL);
	LineTo(mdc, point.x, point.y);

	SelectObject(mdc, oldpen);
	DeleteObject(hpen);
	return mdc;
}

class Throwing {
private:
	int player{ 0 };
	POINT point{ 0 };
	//int size{ 10 };       // define 대체
	int stat{ 0 };
	int timer{ -1 };
	int movex{ 0 };
	int movey{ 0 };
	int rad{ 0 };
	int gravity{ 0 };

public:
	Throwing() {};
	Throwing(int p, POINT pt) : player(p), point(pt) {
		stat = 1;
		timer = 0;
		movex = 0;
		movey = 0;
	}
	/*Throwing(POINT pt, int r) : point(pt) {
		stat = 2;

		movex = (int)((double)GRENADESPEED * cos((float)r * PI / 180));
		movey = (int)((double)GRENADESPEED * sin((float)r * PI / 180));

		gravity = 0;
	}*/

	int Get_P() { return player; };
	int Get_S() { return stat; };
	int Get_T() { return timer; };
	POINT Get_Pt() { return point; };
	RECT Get_R() { RECT rect = { point.x - GRENADESIZE, point.y - GRENADESIZE, point.x + GRENADESIZE, point.y + GRENADESIZE }; return rect; };

	int Get_My() { return movey; };

	void Set_R(int);
	void Set_X(int x) { point.x = x; };
	void Set_Y(int y) { point.y = y; };

	void Go_Timer() { ++timer; };
	void Move_X();
	void Move_Y();

	BOOL Drct_X() { return movex < 0 ? FALSE : TRUE; };
	BOOL Drct_Y() { return movey < 0 ? FALSE : TRUE; };

	void Crsh_X() { movex = -(movex / 4); };
	void Crsh_Y() { movey = -(movey / 8); movex = movex / 2; gravity = 0; };
	void Use_P() { movex = -movex; };

	HDC Print_(HDC);
};

void Throwing::Set_R(int r) {
	stat = 2;
	rad = r;

	movex = (int)((double)GRENADESPEED * cos((float)r * PI / 180));
	movey = (int)((double)GRENADESPEED * sin((float)r * PI / 180));

	gravity = 0;
}

void Throwing::Move_X() {
	point.x += movex;

	if (point.x < 0 || point.x > 2400) {
		player = 0;
		stat = 0;
	}
}

void Throwing::Move_Y() {
	point.y += movey;

	gravity += 1;
	movey += gravity;

	if (point.y < 0 || point.y > 1800) {
		player = 0;
		stat = 0;
	}

}

//void Throwing::Move_() {
//    if (stat == 2) {
//        point.x += movex;
//        point.y += movey;
//
//        gravity += 1;
//        movey += gravity;
//    }
//
//    if (point.x < 0 || point.x > 2400 || point.y < 0 || point.y > 1800) {
//        player = 0;
//        stat = 0;
//    }
//
//    ++timer;
//}

HDC Throwing::Print_(HDC mdc) {
	/*HPEN hpen = CreatePen(1, 10, RGB(255, 0, 0));
	oldpen = (HPEN)SelectObject(mdc, hpen);*/
	HBRUSH hbrush = CreateSolidBrush(RGB(0, 255, 0));
	oldbrush = (HBRUSH)SelectObject(mdc, hbrush);

	RECT rect = { point.x - GRENADESIZE - 5, point.y - GRENADESIZE - 5, point.x + GRENADESIZE + 5, point.y + GRENADESIZE + 5 };

	Ellipse(mdc, rect.left + 5, rect.top + 5, rect.right - 5, rect.bottom - 5);
	FrameRect(mdc, &rect, (HBRUSH)CreateSolidBrush(RGB(255, 0, 0)));

	/*SelectObject(mdc, oldpen);
	DeleteObject(hpen);*/
	SelectObject(mdc, oldbrush);
	DeleteObject(hbrush);
	return mdc;
}

class Explosion {
private:
	POINT point{ 0 };
	int stat{ 0 };

public:
	Explosion() {};
	Explosion(POINT pt) : point({ pt }), stat(1) {};

	int Get_S() { return stat; };
	RECT Get_R() { RECT rect = { point.x - EXPLOSIONSIZE, point.y - EXPLOSIONSIZE, point.x + EXPLOSIONSIZE, point.y + EXPLOSIONSIZE }; return rect; };

	HDC Print_(HDC);
};

HDC Explosion::Print_(HDC mdc) {
	HBRUSH hbrush = CreateSolidBrush(RGB(255, 255, 255));
	oldbrush = (HBRUSH)SelectObject(mdc, hbrush);

	RECT rect = { point.x - EXPLOSIONSIZE, point.y - EXPLOSIONSIZE, point.x + EXPLOSIONSIZE, point.y + EXPLOSIONSIZE };

	switch (stat) {
	case 1:
		Ellipse(mdc, rect.left, rect.top, rect.right, rect.bottom);

		++stat;
		break;
	case 2:
		++stat;
		break;
	case 3:
		Ellipse(mdc, rect.left, rect.top, rect.right, rect.bottom);

		++stat;
		break;
	case 4:
		++stat;
		break;
	case 5:
		Ellipse(mdc, rect.left, rect.top, rect.right, rect.bottom);

		stat = 0;
		break;
	default:
		break;
	}

	SelectObject(mdc, oldbrush);
	DeleteObject(hbrush);
	return mdc;
}

class Item {
private:
	POINT point{ 0 };
	eItem type{ eItem::none };

public:
	Item() {}
	Item(int x, int y) : point({ x, y }) {
		std::random_device rd;
		std::uniform_int_distribution<> uid(1, 7);

		type = (eItem)uid(rd);
	}
	Item(POINT pt) : point({ pt }) {
		std::random_device rd;
		std::uniform_int_distribution<> uid(1, 7);

		type = (eItem)uid(rd);
	}

	eItem Get_T() { return type; };
	RECT Get_R() { RECT rect = { point.x - ITEMWIDTH / 2, point.y - ITEMHEIGHT / 2, point.x + ITEMWIDTH / 2, point.y + ITEMHEIGHT / 2 }; return rect; };

	HDC Print_(HDC);
};

HDC hdc, mainMemdc, plusMemdc;
HDC Item::Print_(HDC mdc) {
	RECT drect = this->Get_R();
	if (mdc == NULL) {
		printf("엥?\n");
		system("pause");
	}
	switch (type) {
	case eItem::axe:
		Axe.Draw(mdc, drect);
		break;
	case eItem::sword:
		Sword.Draw(mdc, drect);

		break;
	case eItem::pistol:
		Pistol.Draw(mdc, drect);

		break;
	case eItem::uzi:
		Uzi.Draw(mdc, drect);

		break;
	case eItem::grenade:
		Grenade.Draw(mdc, drect);

		break;
	case eItem::smallkit:
		Smallkit.Draw(mdc, drect);

		break;
	case eItem::bigkit:
		Bigkit.Draw(mdc, drect);

		break;
	default:
		break;
	}

	return mdc;
}

// data (init) //

RECT clientRect;
int mWidth{}, mHeight{};
HBITMAP mainBit, oldBit1;
HBITMAP tempHDCBit, oldBit2;

// data (screen) //

static eScreen screen{ eScreen::main1 };
static int nowselect{ 0 };
static int nowmap{ 1 };
static int nowtime{ 2 };
static int nowpoint{ 2 };
CImage imgCharacter1;
CImage imgCharacter2;

CImage backGround_1;
CImage skyImg_1;

CImage backGround_2;

CImage ielevator{};

RECT printCharacterRect;

// data (Game) //

RECT map1GroundRect[21];
RECT map1BalconyRect[2];
RECT map1StairRect[6];
RECT map1UpPortalRect[2];
RECT map1DownPortalRect[2];
RECT map1LadderRect;

RECT map2GroundRect[18];
RECT map2BalconyRect[2];
RECT map2Stair1Rect[6];
RECT map2Stair2Rect[7];
RECT map2Stair3Rect[7];
RECT map2LeftPortalRect[2];
RECT map2LadderRect[2];

Character character1;
Character character2;

Bullet bullet[BULLETNUM]{};

Throwing grenade[GRENADENUM]{};

Explosion explosion[EXPLOSIONTNUM];

Item item[ITEMNUM];
int itemtime{ 0 };

int elevatordrct{ 1 };   //엘레베이터 상하 방향

BOOL Theend = FALSE;
int winner{ 0 };
int winpoint1{ 0 };
int winpoint2{ 0 };

int printscreen{ 4 };

// bool //

static BOOL bUp = FALSE;
static BOOL IspressedUp = FALSE;
static BOOL bDown = FALSE;
static BOOL IspressedDown = FALSE;
static BOOL bLeft = FALSE;
static BOOL IspressedLeft = FALSE;
static BOOL bRight = FALSE;
static BOOL IspressedRight = FALSE;

static BOOL bW = FALSE;
static BOOL IspressedW = FALSE;
static BOOL bS = FALSE;
static BOOL IspressedS = FALSE;
static BOOL bA = FALSE;
static BOOL IspressedA = FALSE;
static BOOL bD = FALSE;
static BOOL IspressedD = FALSE;

BOOL bN = FALSE;
BOOL IspressedN = FALSE;
BOOL IspressedM = FALSE;
BOOL IspressedComma = FALSE;

BOOL bNum1 = FALSE;
BOOL IspressedNum1 = FALSE;
BOOL IspressedNum2 = FALSE;
BOOL IspressedNum3 = FALSE;

static BOOL bEnter = FALSE;
static BOOL IspressedEnter = FALSE;
static BOOL bEscape = FALSE;
static BOOL IspressedEscape = FALSE;

BOOL gameStart = FALSE;

// function //

void InitWindow();
void InitMapRect();
void Update();
void GameUpdate();
void DrawTitle();
void DrawGame();
void PrintScreen(int);
void DrawStatusWindow();

// function (game) //

void Move_Elevator();
void CharacterElevatorCheck(Character&);

void Cal_Gravity(Character&);
void Chk_Portal(Character&);

void CheckUpDownButton_1();
void CheckUpDownButton_2();

void CheckLeftRightButton_1();
void CheckLeftRightButton_2();

void Punch_(Character&);

void Shoot_(Character&);
void Chk_BulletNMap();
void Chk_BulletNPlayer();

void Create_Throwing(Character&);
void Throwing_(Character&);
void Chk_ThrowingNMap_X(Throwing&);
void Chk_ThrowingNMap_Y(Throwing&);

void Explosion_();
void Chk_ExplosionNPlayer();

void Create_Item();
void Timer_Item();
void Get_Item(Character&);

void CheckFallingCharacter(Character&);
void ChangeCharacterMotion_1();
void ChangeCharacterMotion_2();

void Chk_End();

// reset //

void Rst_Object();
void Rst_Map1();
void Rst_Map2();

TCHAR aaaaaaaaaaaaaa[100];//TODO: 진짜 테스트용 나중에 지우기       //new -> new 에서 TODO: 로 바꿈
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (clientRect.right == 0) GetClientRect(hWnd, &clientRect);
	PAINTSTRUCT ps;

	//--- 메시지 처리하기

	{
		wsprintf(aaaaaaaaaaaaaa, L"현재 상태: ");
		switch (character1.status)
		{
		case LEFT_STAY:
			lstrcat(aaaaaaaaaaaaaa, L"왼쪽 대기");
			break;
		case RIGHT_STAY:
			lstrcat(aaaaaaaaaaaaaa, L"오른쪽 대기");
			break;
		case LEFT_RUN:
			lstrcat(aaaaaaaaaaaaaa, L"왼쪽 달리기");
			break;
		case RIGHT_RUN:
			lstrcat(aaaaaaaaaaaaaa, L"오른쪽 달리기");
			break;
		case LEFT_WALK:
			lstrcat(aaaaaaaaaaaaaa, L"왼쪽 걷기");
			break;
		case RIGHT_WALK:
			lstrcat(aaaaaaaaaaaaaa, L"오른쪽 걷기");
			break;
		case LEFT_JUMP:
			lstrcat(aaaaaaaaaaaaaa, L"왼쪽 점프");
			break;
		case RIGHT_JUMP:
			lstrcat(aaaaaaaaaaaaaa, L"오른쪽 점프");
			break;
		case LADDER_UP:
			lstrcat(aaaaaaaaaaaaaa, L"사다리 _ UP");
			break;
		case LADDER_DOWN:
			lstrcat(aaaaaaaaaaaaaa, L"사다리 _ DOWN");
			break;
		case SHOCK:
			lstrcat(aaaaaaaaaaaaaa, L"기절 중");
			break;
		case LEFT_FALLING:
			lstrcat(aaaaaaaaaaaaaa, L"왼쪽 FALLING");
			break;
		case RIGHT_FALLING:
			lstrcat(aaaaaaaaaaaaaa, L"오른쪽 FALLING");
			break;
		case LEFT_STANDING:
			lstrcat(aaaaaaaaaaaaaa, L"왼쪽 STANDING");
			break;
		case RIGHT_STANDING:
			lstrcat(aaaaaaaaaaaaaa, L"오른쪽 STANDING");
			break;
		case LEFT_SIT:
			lstrcat(aaaaaaaaaaaaaa, L"왼쪽 앉기");
			break;
		case RIGHT_SIT:
			lstrcat(aaaaaaaaaaaaaa, L"오른쪽 앉기");
			break;
		case LEFT_ROLL:
			lstrcat(aaaaaaaaaaaaaa, L"왼쪽 구르기");
			break;
		case RIGHT_ROLL:
			lstrcat(aaaaaaaaaaaaaa, L"오른쪽 구르기");
			break;
		}
	}

	switch (uMsg) {
	case WM_CREATE:
		InitWindow();
		SetTimer(hWnd, 1, 45, (TIMERPROC)TimerProc);
		break;
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_UP:
			if (!IspressedUp) {
				bUp = TRUE;
				IspressedUp = TRUE;

			}
			break;
		case VK_DOWN:
			if (!IspressedDown) {
				bDown = TRUE;
				IspressedDown = TRUE;

			}
			break;
		case VK_LEFT:
			if (!IspressedLeft) {
				bLeft = TRUE;
				IspressedLeft = TRUE;
			}
			break;
		case VK_RIGHT:
			if (!IspressedRight) {
				bRight = TRUE;
				IspressedRight = TRUE;
			}
			break;
		case 0x4E:
			if (!IspressedN) {
				if (character1.status == LEFT_STAY || character1.status == LEFT_WALK || character1.status == LEFT_RUN) {
					character1.status = LEFT_PUNCH;
					Punch_(character1);

					bN = TRUE;
					IspressedN = TRUE;
				}
				else if (character1.status == RIGHT_STAY || character1.status == RIGHT_WALK || character1.status == RIGHT_RUN) {
					character1.status = RIGHT_PUNCH;
					Punch_(character1);

					bN = TRUE;
					IspressedN = TRUE;
				}
				else if (character1.status == LEFT_SIT || character1.status == RIGHT_SIT) {
					Get_Item(character1);
				}
			}
			break;
		case 0x4D:
			switch (screen) {
			case eScreen::map1:
			case eScreen::map2:
				if (IspressedComma) {
					break;
				}

				if (character1.gun == eGun::none) {
					break;
				}

				if (character1.status == LEFT_STAY || character1.status == LEFT_WALK) {
					character1.status = LEFT_AIMING;

					character1.radian = 180;
					//character1.aimingtimer = -1;
					character1.Gunaming = TRUE;
					IspressedM = TRUE;
				}
				else if (character1.status == RIGHT_STAY || character1.status == RIGHT_WALK) {
					character1.status = RIGHT_AIMING;

					character1.radian = 720;
					//character1.aimingtimer = -1;
					character1.Gunaming = TRUE;
					IspressedM = TRUE;
				}

				if (character1.status == LEFT_AIMING || character1.status == RIGHT_AIMING) {
					character1.aimingtimer = -1;
					IspressedM = TRUE;
				}

				break;
			}
			break;
		case VK_OEM_COMMA:  //  투척 공격
			switch (screen) {
			case eScreen::map1:
			case eScreen::map2:
				if (IspressedM) {
					break;
				}

				if (character1.throwing == eThrowing::none) {
					break;
				}

				if (character1.status == LEFT_STAY || character1.status == LEFT_WALK) {
					Create_Throwing(character1);

					character1.status = LEFT_AIMING;

					character1.radian = 180;
					//character1.aimingtimer = -1;
					IspressedComma = TRUE;
				}
				else if (character1.status == RIGHT_STAY || character1.status == RIGHT_WALK) {
					Create_Throwing(character1);

					character1.status = RIGHT_AIMING;

					character1.radian = 720;
					//character1.aimingtimer = -1;
					IspressedComma = TRUE;
				}

				/*if (character1.status == LEFT_AIMING || character1.status == RIGHT_AIMING) {
					character1.aimingtimer = -1;
					IspressedComma = TRUE;
				}*/

				break;
			}
			break;
		case 190:      // 1p 파워업버튼
			if (character1.status == LEFT_STANDING || character1.status == RIGHT_STANDING) break;    //new
			if (bLeft || bRight) {
				character1.powerUp = TRUE;
			}
			if (bDown) {//
				RECT nullRect;
				character1.playerRect.bottom += character1.GravitySpeed;
				switch (screen) {
				case eScreen::map1:
					for (auto rect : map1BalconyRect) {
						if (IntersectRect(&nullRect, &rect, &character1.playerRect)) {
							character1.downJump = true;
						}
					}
					break;
				case eScreen::map2:
					for (auto rect : map2BalconyRect) {
						if (IntersectRect(&nullRect, &rect, &character1.playerRect)) {
							character1.downJump = true;
						}
					}
					break;
				}
				character1.playerRect.bottom -= character1.GravitySpeed;
			}
			break;
		case 'W':
		case 'w':
			if (!IspressedW) {
				bW = TRUE;
				IspressedW = TRUE;
			}
			break;
		case 'A':
		case 'a':
			if (!IspressedA) {
				bA = TRUE;
				IspressedA = TRUE;
			}
			break;
		case 'S':
		case 's':
			if (!IspressedS) {
				bS = TRUE;
				IspressedS = TRUE;
			}
			break;
		case 'D':
		case 'd':
			if (!IspressedD) {
				bD = TRUE;
				IspressedD = TRUE;
			}
			break;
		case 0x31:
			if (!IspressedNum1) {
				if (character2.status == LEFT_STAY || character2.status == LEFT_WALK || character2.status == LEFT_RUN) {
					character2.status = LEFT_PUNCH;
					Punch_(character2);

					bN = TRUE;
					IspressedNum1 = TRUE;
				}
				else if (character2.status == RIGHT_STAY || character2.status == RIGHT_WALK || character2.status == RIGHT_RUN) {
					character2.status = RIGHT_PUNCH;
					Punch_(character2);

					bN = TRUE;
					IspressedNum1 = TRUE;
				}
				else if (character2.status == LEFT_SIT || character2.status == RIGHT_SIT) {
					Get_Item(character2);
				}
			}
			break;
		case 0x32:
			switch (screen) {
			case eScreen::map1:
			case eScreen::map2:
				if (IspressedNum3) {
					break;
				}

				if (character2.gun == eGun::none) {
					break;
				}

				if (character2.status == LEFT_STAY || character2.status == LEFT_WALK) {
					character2.status = LEFT_AIMING;

					character2.radian = 180;
					//character2.aimingtimer = -1;
					character2.Gunaming = TRUE;
					IspressedNum2 = TRUE;
				}
				else if (character2.status == RIGHT_STAY || character2.status == RIGHT_WALK) {
					character2.status = RIGHT_AIMING;

					character2.radian = 720;
					//character2.aimingtimer = -1;
					character2.Gunaming = TRUE;
					IspressedNum2 = TRUE;
				}

				if (character2.status == LEFT_AIMING || character2.status == RIGHT_AIMING) {
					character2.aimingtimer = -1;
					IspressedNum2 = TRUE;
				}

				break;
			}
			break;
		case 0x33:
			switch (screen) {
			case eScreen::map1:
			case eScreen::map2:
				if (IspressedNum2) {
					break;
				}

				if (character2.throwing == eThrowing::none) {
					break;
				}

				if (character2.status == LEFT_STAY || character2.status == LEFT_WALK) {
					Create_Throwing(character2);

					character2.status = LEFT_AIMING;

					character2.radian = 180;
					//character2.aimingtimer = -1;
					IspressedNum3 = TRUE;
				}
				else if (character2.status == RIGHT_STAY || character2.status == RIGHT_WALK) {
					Create_Throwing(character2);

					character2.status = RIGHT_AIMING;

					character2.radian = 720;
					//character2.aimingtimer = -1;
					IspressedNum3 = TRUE;
				}

				/*if (character2.status == LEFT_AIMING || character2.status == RIGHT_AIMING) {
					character2.aimingtimer = -1;
					IspressedNum3 = TRUE;
				}*/

				break;
			}
			break;
		case '4':
			if (character2.status == LEFT_STANDING || character2.status == RIGHT_STANDING) break;   //new
			if (bA || bD) {
				character2.powerUp = true;
			}
			if (bS) {//
				RECT nullRect;
				character2.playerRect.bottom += character2.GravitySpeed;
				switch (screen) {
				case eScreen::map1:
					for (auto rect : map1BalconyRect) {
						if (IntersectRect(&nullRect, &rect, &character2.playerRect)) {
							character2.downJump = true;
						}
					}
					break;
				case eScreen::map2:
					for (auto rect : map2BalconyRect) {
						if (IntersectRect(&nullRect, &rect, &character2.playerRect)) {      //new
							character2.downJump = true;
						}
					}
					break;
				}
				character2.playerRect.bottom -= character2.GravitySpeed;
			}
			break;
		case VK_RETURN:
			if (!IspressedEnter) {
				bEnter = TRUE;
				IspressedEnter = TRUE;
			}
			break;
		case VK_ESCAPE:
			if (!IspressedEscape) {
				bEscape = TRUE;
				IspressedEscape = TRUE;
			}
		}
		break;
	case WM_KEYUP:
		switch (wParam) {
		case VK_UP:
			bUp = FALSE;
			IspressedUp = FALSE;
			break;
		case VK_DOWN:
			bDown = FALSE;
			IspressedDown = FALSE;
			if (gameStart) {
				if (character1.status == LEFT_SIT) character1.status = LEFT_STAY;
				else if (character1.status == RIGHT_SIT) character1.status = RIGHT_STAY;
			}
			break;
		case VK_LEFT:
			character1.powerUp = FALSE;         //키떼면 파워업 풀리게
			character1.usedPortal = FALSE;
			bLeft = FALSE;
			IspressedLeft = FALSE;
			if (character1.status == LEFT_WALK || character1.status == LEFT_RUN) character1.status = LEFT_STAY;
			else if (character1.status == RIGHT_WALK || character1.status == RIGHT_RUN)character1.status = RIGHT_STAY;
			break;
		case VK_RIGHT:
			character1.powerUp = FALSE;         //키떼면 파워업 풀리게
			character1.usedPortal = FALSE;
			bRight = FALSE;
			IspressedRight = FALSE;
			if (character1.status == LEFT_WALK || character1.status == LEFT_RUN) character1.status = LEFT_STAY;
			else if (character1.status == RIGHT_WALK || character1.status == RIGHT_RUN) character1.status = RIGHT_STAY;
			break;
		case 0x4E:
			bN = FALSE;
			IspressedN = FALSE;
			break;
		case 0x4D:      //원거리 공격
			if (IspressedComma) {
				break;
			}

			if (character1.gun == eGun::none) {
				IspressedM = FALSE;
				break;
			}

			if (character1.status == LEFT_AIMING || character1.status == RIGHT_AIMING) {
				if (!character1.Shoot) {
					character1.Shoot = TRUE;
				}

				//Shoot_(character1);
				//character1.aimingtimer = 0;
				IspressedM = FALSE;
			}

			IspressedM = FALSE;
			break;
		case VK_OEM_COMMA:
			if (IspressedM) {
				break;
			}

			if (character1.Gunaming) {
				break;
			}

			if (character1.status == LEFT_AIMING) {
				Throwing_(character1);

				character1.status = LEFT_STAY;
				IspressedComma = FALSE;
			}
			else if (character1.status == RIGHT_AIMING) {
				Throwing_(character1);

				character1.status = RIGHT_STAY;
				IspressedComma = FALSE;
			}

			IspressedComma = FALSE;
			break;
		case 'W':
		case 'w':
			bW = FALSE;
			IspressedW = FALSE;
			break;
		case 'A':
		case 'a':
			character2.powerUp = FALSE;         //키떼면 파워업 풀리게
			character2.usedPortal = FALSE;
			bA = FALSE;
			IspressedA = FALSE;
			if (character2.status == LEFT_WALK || character2.status == LEFT_RUN) character2.status = LEFT_STAY;
			else if (character2.status == RIGHT_WALK || character2.status == RIGHT_RUN)character2.status = RIGHT_STAY;
			break;
		case 'S':
		case 's':
			bS = FALSE;
			IspressedS = FALSE;
			if (gameStart) {
				if (character2.status == LEFT_SIT) character2.status = LEFT_STAY;
				else if (character2.status == RIGHT_SIT) character2.status = RIGHT_STAY;
			}
			break;
		case 'D':
		case 'd':
			character2.powerUp = FALSE;         //키떼면 파워업 풀리게
			character2.usedPortal = FALSE;
			bD = FALSE;
			IspressedD = FALSE;
			if (character2.status == LEFT_WALK || character2.status == LEFT_RUN) character2.status = LEFT_STAY;
			else if (character2.status == RIGHT_WALK || character2.status == RIGHT_RUN) character2.status = RIGHT_STAY;
			break;
		case 0x31:
			bNum1 = FALSE;
			IspressedNum1 = FALSE;
			break;
		case 0x32:
			if (IspressedNum3) {
				break;
			}

			if (character2.gun == eGun::none) {
				IspressedNum2 = FALSE;
				break;
			}

			if (character2.status == LEFT_AIMING || character2.status == RIGHT_AIMING) {
				if (!character2.Shoot) {
					character2.Shoot = TRUE;
				}

				//Shoot_(character2);
				//character2.aimingtimer = 0;
				IspressedNum2 = FALSE;
			}

			IspressedNum2 = FALSE;
			break;
		case 0x33:
			if (IspressedNum2) {
				break;
			}

			if (character2.Gunaming) {
				break;
			}

			if (character2.status == LEFT_AIMING) {
				Throwing_(character2);

				character2.status = LEFT_STAY;
				IspressedNum3 = FALSE;
			}
			else if (character2.status == RIGHT_AIMING) {
				Throwing_(character2);

				character2.status = RIGHT_STAY;
				IspressedNum3 = FALSE;
			}

			IspressedNum3 = FALSE;
			break;
		case '4':
			break;
		case VK_RETURN:
			//bEnter = FALSE;
			IspressedEnter = FALSE;
			break;
		case VK_ESCAPE:
			bEscape = FALSE;
			IspressedEscape = FALSE;
			break;
		}
		break;
	case WM_CHAR:
		switch (wParam) {
		case 'Q':
		case 'q':
			PostQuitMessage(0);
			break;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		if (!gameStart) DrawTitle();
		else DrawGame();
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
	if (!gameStart) Update();
	else GameUpdate();

	InvalidateRect(hWnd, NULL, FALSE);
}

void InitWindow() {
	GetClientRect(hWnd, &clientRect);
	mWidth = 2400;
	mHeight = 1800;

	hdc = GetDC(hWnd);
	mainMemdc = CreateCompatibleDC(hdc);
	plusMemdc = CreateCompatibleDC(hdc);
	if (mainBit == NULL) mainBit = CreateCompatibleBitmap(hdc, mWidth, mHeight);
	if (tempHDCBit == NULL)tempHDCBit = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
	ReleaseDC(hWnd, hdc);

	//맵 RECT 초기화 //
	InitMapRect();
	{
		//TODO: 플레이어 초기화 나중에 함수로 만들기 SpawnPlayer();
		character1.player = 1;
		character1.playerRect = { 394,1354,394 + CharacterWidth,1354 + CharacterHeight };
		character1.status = LEFT_STAY;
		character1.GravitySpeed = CharacterGravitySpeed;
		character1.speed = CharacterSpeed;
	}

	{//캐릭터 이미지 초기화
		imgCharacter1.Load(L"Character1 sprites.png");
		imgCharacter2.Load(L"Character2 sprites.png");
	}

	{//new - 맵 이미지 초기 설정
		skyImg_1.Load(L"map1 Sky.png");
		backGround_1.Load(L"map1 Ground.png");

		backGround_2.Load(L"map2 Ground.png");

		ielevator.Load(L"relevator.png");
	}

	// 아이템 이미지 초기 설정 //

	Axe.Load(L"axe.png");
	Sword.Load(L"sword.png");
	Pistol.Load(L"pistol.png");
	Uzi.Load(L"uzi.png");
	Grenade.Load(L"grenade.png");
	Smallkit.Load(L"smallkit.png");
	Bigkit.Load(L"bigkit.png");

}

// definition //

void InitMapRect() {
	{//map1 Init
		{//map1GroundRect
			map1GroundRect[0] = { 22,441,278,494 };
			map1GroundRect[1] = { 70,497,236,630 };
			map1GroundRect[2] = { 70,631,767,685 };
			map1GroundRect[3] = { 712,688,767,1289 };
			map1GroundRect[4] = { 182,1130,713,1179 };
			map1GroundRect[5] = { 150,1187,226,1348 };
			map1GroundRect[6] = { 145,1457,767,1505 };
			map1GroundRect[7] = { 1023,1130,1072,1317 };
			map1GroundRect[8] = { 1017,1480,1267,1534 };
			map1GroundRect[9] = { 1356,1563,2185,1615 };
			map1GroundRect[10] = { 2111,1132,2186,1457 };
			map1GroundRect[11] = { 1023,1077,1307,1131 };
			map1GroundRect[12] = { 1426,1075,1478,1101 };
			map1GroundRect[13] = { 1592,1076,1697,1131 };
			map1GroundRect[14] = { 1807,1075,1863,1101 };
			map1GroundRect[15] = { 1974,1075,2314,1132 };
			map1GroundRect[16] = { 2149,943,2313,1075 };
			map1GroundRect[17] = { 2110,885,2369,939 };
			map1GroundRect[18] = { 715,1510,766,1777 };
			map1GroundRect[19] = { 1024,1536,1075,1777 };
			map1GroundRect[ELEVATOR] = { 917, 1480, 1017, 1500 };   // 엘레베이터 렉트 추가
		}

		{//map1BalconyRect
			map1BalconyRect[0] = { 859,570,1392,585 };
			map1BalconyRect[1] = { 770,1187,892,1200 };
		}

		{//map1StairRect
			map1StairRect[0] = { 1232,1479,1268,1532 };
			map1StairRect[1] = { 1236,1500,1287,1549 };
			map1StairRect[2] = { 1263,1510,1304,1571 };
			map1StairRect[3] = { 1288,1521,1322,1588 };
			map1StairRect[4] = { 1304,1537,1341,1608 };
			map1StairRect[5] = { 1321,1550,1360,1620 };
		}

		{//portalRect
			map1UpPortalRect[0] = { 237,536,276,624 };
			map1UpPortalRect[1] = { 2111,980,2146,1074 };

			map1DownPortalRect[0] = { 187,1356,229,1451 };
			map1DownPortalRect[1] = { 2108,1461,2145,1559 };
		}

		{//ladderRect
			map1LadderRect = { 810, 630, 812,1184 - (int)(CharacterHeight * 1.5) };

		}
	}

	{//map2 Init
		{
			map2GroundRect[0] = { 140,607,140 + 646,607 + 51 };
			map2GroundRect[1] = { 732,658,732 + 54,658 + 204 };
			map2GroundRect[2] = { 137,1014,137 + 649,1014 + 50 };
			map2GroundRect[3] = { 732,1062,732 + 53,1062 + 95 };
			map2GroundRect[4] = { 177,1383,177 + 181,1383 + 47 };
			map2GroundRect[5] = { 421,1306,421 + 364,1306 + 51 };
			map2GroundRect[6] = { 948,1387,948 + 179,1387 + 77 };
			map2GroundRect[7] = { 1272,1303,1272 + 425,1303 + 52 };
			map2GroundRect[8] = { 1672,1253,1672 + 654,1253 + 51 };
			map2GroundRect[9] = { 2255,653,2255 + 72,653 + 602 };
			map2GroundRect[10] = { 1726,648,1726 + 530,648 + 53 };
			map2GroundRect[11] = { 1726,702,1726 + 51,702 + 176 };
			map2GroundRect[12] = { 1341,826,1341 + 385,826 + 52 };
			map2GroundRect[13] = { 1289,826,1289 + 52,826 + 93 };
			map2GroundRect[14] = { 50,608,50 + 91,608 + 308 };
			map2GroundRect[15] = { 141,1061,141 + 83,1061 + 210 };
			map2GroundRect[16] = { 2095,952,2095 + 160,952 + 78 };
			map2GroundRect[17] = { 1288,1081,1288 + 90,1081 + 85 };
		}

		{
			map2BalconyRect[0] = { 1378,1081,1378 + 103,1081 + 17 };
			map2BalconyRect[1] = { 1534,1081,1534 + 329,1081 + 17 };
		}

		{
			map2Stair1Rect[0] = { 419,1305,419 + 27,1305 + 57 };
			map2Stair1Rect[1] = { 403,1320,403 + 31,1320 + 55 };
			map2Stair1Rect[2] = { 387,1332,387 + 41,1332 + 52 };
			map2Stair1Rect[3] = { 369,1344,369 + 41,1344 + 52 };
			map2Stair1Rect[4] = { 354,1357,354 + 46,1357 + 51 };
			map2Stair1Rect[5] = { 335,1370,335 + 45,1370 + 52 };
		}

		{
			map2Stair2Rect[0] = { 772,1305,772 + 35,1305 + 52 };
			map2Stair2Rect[1] = { 802,1321,802 + 29,1321 + 59 };
			map2Stair2Rect[2] = { 825,1331,825 + 31,1331 + 61 };
			map2Stair2Rect[3] = { 849,1344,849 + 33,1344 + 64 };
			map2Stair2Rect[4] = { 875,1358,875 + 33,1358 + 64 };
			map2Stair2Rect[5] = { 897,1370,897 + 32,1370 + 70 };
			map2Stair2Rect[6] = { 916,1380,916 + 33,1380 + 79 };
		}

		{
			map2Stair3Rect[0] = { 1264,1300,1266 + 35,1305 + 52 };
			map2Stair3Rect[1] = { 1242,1321,1242 + 29,1321 + 59 };
			map2Stair3Rect[2] = { 1217,1331,1217 + 31,1331 + 61 };
			map2Stair3Rect[3] = { 1191,1344,1191 + 33,1344 + 64 };
			map2Stair3Rect[4] = { 1165,1358,1165 + 33,1358 + 64 };
			map2Stair3Rect[5] = { 1144,1370,1144 + 32,1370 + 70 };
			map2Stair3Rect[6] = { 1124,1380,1124 + 33,1380 + 79 };
		}

		{
			map2LeftPortalRect[0] = { 95,911,95 + 51,911 + 104 };
			map2LeftPortalRect[1] = { 175,1279,175 + 48,1279 + 103 };
		}

		{
			map2LadderRect[0] = { 1508,998,1509,998 + 264 - CharacterHeight };
			map2LadderRect[1] = { 2065,901,2066,901 + 306 - CharacterHeight };
		}
	}
}

void Update() {
	switch (screen) {
	case eScreen::main1:      // main1 //
		if (bEnter) {
			screen = eScreen::main2;

			bEnter = FALSE;
		}

		break;
	case eScreen::main2:      // main2 //
		if (bUp) {
			switch (nowselect) {
			case 0:
			case 1:
				nowselect = 2;
				break;
			default:
				--nowselect;
				break;
			}

			bUp = FALSE;
		}

		if (bDown) {
			switch (nowselect) {
			case 0:
			case 2:
				nowselect = 1;
				break;
			default:
				++nowselect;
				break;
			}

			bDown = FALSE;
		}

		if (bEnter) {
			switch (nowselect) {
			case 1:
				screen = eScreen::select;
				nowselect = 0;

				nowmap = 1;
				nowtime = 2;
				nowpoint = 2;
				break;
			case 2:
				screen = eScreen::help;
				nowselect = 0;
				break;
			default:
				break;
			}

			bEnter = FALSE;
		}

		if (bEscape) {
			screen = eScreen::main1;
			nowselect = 0;

			bEscape = FALSE;
		}

		break;
	case eScreen::help:      // help //
		if (bEscape) {
			screen = eScreen::main2;
			nowselect = 0;

			bEscape = FALSE;
		}

		break;
	case eScreen::select:
		if (bUp) {
			switch (nowselect) {
			case 0:
			case 1:
				nowselect = 3;
				break;
			default:
				--nowselect;
				break;
			}

			bUp = FALSE;
		}

		if (bDown) {
			switch (nowselect) {
			case 0:
			case 3:
				nowselect = 1;
				break;
			default:
				++nowselect;
				break;
			}

			bDown = FALSE;
		}

		if (bLeft) {
			switch (nowselect) {
			case 1:
				switch (nowmap) {
				case 1:
					nowmap = 2;
					break;
				default:
					--nowmap;
					break;
				}

				break;
			case 2:
				switch (nowtime) {
				case 1:
					nowtime = 3;
					break;
				default:
					--nowtime;
					break;
				}

				break;
			case 3:
				switch (nowpoint) {
				case 1:
					nowpoint = 3;
					break;
				default:
					--nowpoint;
					break;
				}

				break;
			default:
				break;
			}

			bLeft = FALSE;
		}

		if (bRight) {
			switch (nowselect) {
			case 1:
				switch (nowmap) {
				case 2:
					nowmap = 1;
					break;
				default:
					++nowmap;
					break;
				}

				break;
			case 2:
				switch (nowtime) {
				case 3:
					nowtime = 1;
					break;
				default:
					++nowtime;
					break;
				}

				break;
			case 3:
				switch (nowpoint) {
				case 3:
					nowpoint = 1;
					break;
				default:
					++nowpoint;
					break;
				}

				break;
			default:
				break;
			}

			bRight = FALSE;
		}

		if (bEnter) {
			switch (nowmap) {
			case 1:
				screen = eScreen::map1;
				nowselect = 0;

				printscreen = 4;
				Theend = FALSE;
				winpoint1 = 0;
				winpoint2 = 0;

				//{
				//	//TODO: 플레이어 초기화 나중에 함수로 만들기 SpawnPlayer();
				//	character1.player = 1;
				//	character1.playerRect = { 394,1354,394 + CharacterWidth,1354 + CharacterHeight };
				//	character1.status = LEFT_STAY;
				//	character1.GravitySpeed = CharacterGravitySpeed;
				//	character1.speed = CharacterSpeed;

				//	character1.imgRect.top = 90;
				//	character1.imgRect.bottom = character1.imgRect.top + 80;
				//	character1.imgRect.left = 0;
				//	character1.imgRect.right = character1.imgRect.left + 56;

				//	character2.player = 2;
				//	character2.playerRect = { 1750, 1563 - CharacterHeight, 1750 + CharacterWidth, 1563 };
				//	character2.status = LEFT_STAY;
				//	character2.GravitySpeed = CharacterGravitySpeed;
				//	character2.speed = CharacterSpeed;

				//	character2.imgRect.top = 90;
				//	character2.imgRect.bottom = character2.imgRect.top + 80;
				//	character2.imgRect.left = 0;
				//	character2.imgRect.right = character2.imgRect.left + 56;
				//	//TODO: 캐릭터 변수 추가될때마다 값 조정해주기
				//}

				Rst_Map1();
				Rst_Object();

				break;
			case 2:
				screen = eScreen::map2;
				nowselect = 0;

				{
					//TODO: 플레이어 초기화 나중에 함수로 만들기 SpawnPlayer();
					character1.playerRect = { 558,1203,558 + CharacterWidth,1203 + CharacterHeight };
					character1.status = LEFT_STAY;
					character1.GravitySpeed = CharacterGravitySpeed;
					character1.speed = CharacterSpeed;

					character1.imgRect.top = 90;
					character1.imgRect.bottom = character1.imgRect.top + 80;
					character1.imgRect.left = 0;
					character1.imgRect.right = character1.imgRect.left + 56;

					character2.playerRect = { 1945, 1149, 1945 + CharacterWidth, 1149 + CharacterHeight };
					character2.status = LEFT_STAY;
					character2.GravitySpeed = CharacterGravitySpeed;
					character2.speed = CharacterSpeed;

					character2.imgRect.top = 90;
					character2.imgRect.bottom = character2.imgRect.top + 80;
					character2.imgRect.left = 0;
					character2.imgRect.right = character2.imgRect.left + 56;
					//TODO: 캐릭터 변수 추가될때마다 값 조정해주기
				}

				Rst_Object();

				break;
			default:
				break;
			}

			gameStart = TRUE;
			bEnter = FALSE;
		}

		if (bEscape) {
			screen = eScreen::main2;
			nowselect = 0;

			bEscape = FALSE;
		}

		break;
	}
}

void GameUpdate() {
	switch (screen) {
	case eScreen::map1:

		// 아이템 생성 //

		Timer_Item();

		// 수류탄 변화 //

		for (int i = 0; i < GRENADENUM; ++i) {
			switch (grenade[i].Get_S()) {
			case 2:
				grenade[i].Move_X();
				Chk_ThrowingNMap_X(grenade[i]);
				grenade[i].Move_Y();
				Chk_ThrowingNMap_Y(grenade[i]);
			case 1:
				grenade[i].Go_Timer();
				break;
			default:
				break;
			}

			/* if (grenade[i].Get_S() != 0) {
				 grenade[i].Go_Timer();
			 }*/
		}

		// 수류탄 폭발 //

		Explosion_();

		// 총알 생성 //

		if (character1.Shoot == TRUE) {
			if (character1.reloadtimer == -1) {
				character1.aimingtimer = -1;

				switch (character1.gun) {
				case eGun::pistol:
					Shoot_(character1);

					character1.reloadtimer = 0;
					//character1.Shoot = FALSE;
					//character1.aimingtimer = 0;
					break;
				case eGun::uzi:
					if (character1.shoottime == 3) {
						Shoot_(character1);

						character1.reloadtimer = 0;
						//character1.Shoot = FALSE;
						//character1.aimingtimer = 0;
						character1.shoottime = 0;
					}
					else {
						Shoot_(character1);
						++character1.shoottime;
					}
					break;
				}
			}
		}

		if (character2.Shoot == TRUE) {
			if (character2.reloadtimer == -1) {
				character2.aimingtimer = -1;

				switch (character2.gun) {
				case eGun::pistol:
					Shoot_(character2);

					character2.reloadtimer = 0;
					//character2.Shoot = FALSE;
					//character2.aimingtimer = 0;
					break;
				case eGun::uzi:
					if (character2.shoottime == 3) {
						Shoot_(character2);

						character2.reloadtimer = 0;
						//character2.Shoot = FALSE;
						//character2.aimingtimer = 0;
						character2.shoottime = 0;
					}
					else {
						Shoot_(character2);
						++character2.shoottime;
					}
					break;
				}
			}
		}

		// 총알 움직임 //

		for (int i = 0; i < BULLETNUM; ++i) {
			if (bullet[i].Get_P() != 0) {
				bullet[i].Move_();
			}
		}


		{// 엘레베이터 //
			Move_Elevator();


			CharacterElevatorCheck(character1);
			CharacterElevatorCheck(character2);
		}


		// 중력 작용 //

		Cal_Gravity(character1);
		Cal_Gravity(character2);

		// 총 장전 //

		if (character1.reloadtimer >= 0) {
			if (character1.reloadtimer == RELOADTIME) {
				character1.Shoot = FALSE;
				character1.reloadtimer = -1;

				if (!IspressedM) {
					character1.aimingtimer = 0;
				}
				else {
					if (character1.gunnum != 0) {
						character1.aimingtimer = -1;
					}
					else {
						character1.aimingtimer = 0;
					}
				}
			}
			else {
				++character1.reloadtimer;
			}
		}

		if (character2.reloadtimer >= 0) {
			if (character2.reloadtimer == RELOADTIME) {
				character2.Shoot = FALSE;
				character2.reloadtimer = -1;

				if (!IspressedNum2) {
					character2.aimingtimer = 0;
				}
				else {
					if (character2.gunnum != 0) {
						character2.aimingtimer = -1;
					}
					else {
						character2.aimingtimer = 0;
					}
				}
			}
			else {
				++character2.reloadtimer;
			}
		}

		// 조준 해제 //

		if (character1.aimingtimer >= 0) {
			if (IspressedM) {
				if (character1.gunnum != 0) {
					character1.aimingtimer = -1;
				}
			}

			if (character1.aimingtimer == AIMINGTIME) {
				character1.aimingtimer = -1;

				if (character1.status == LEFT_AIMING) {
					character1.status = LEFT_STAY;
					character1.Gunaming = FALSE;
				}
				else {
					character1.status = RIGHT_STAY;
					character1.Gunaming = FALSE;
				}
			}
			else {
				++character1.aimingtimer;
			}
		}

		if (character2.aimingtimer >= 0) {
			if (IspressedNum2) {
				if (character2.gunnum != 0) {
					character2.aimingtimer = -1;
				}
			}

			if (character2.aimingtimer == AIMINGTIME) {
				character2.aimingtimer = -1;

				if (character2.status == LEFT_AIMING) {
					character2.status = LEFT_STAY;
					character2.Gunaming = FALSE;
				}
				else {
					character2.status = RIGHT_STAY;
					character2.Gunaming = FALSE;
				}
			}
			else {
				++character2.aimingtimer;
			}
		}


		{//위 아래 처리
			CheckUpDownButton_1();
			CheckUpDownButton_2();
		}

		{//방향키 처리
			CheckLeftRightButton_1();
			CheckLeftRightButton_2();
		}

		// 포탈 처리 //

		Chk_Portal(character1);
		Chk_Portal(character2);
		// 총알 충돌 체크 (플레이어) //

		Chk_BulletNPlayer();

		// 총알 충돌 체크 (맵) //

		Chk_BulletNMap();

		if (bEscape) {
			screen = eScreen::select;
			nowselect = 0;

			gameStart = FALSE;
			bEscape = FALSE;
		}


		CheckFallingCharacter(character1);
		CheckFallingCharacter(character2);


		ChangeCharacterMotion_1();
		ChangeCharacterMotion_2();

		// 라운드 종료 체크 //

		if (Theend) {
			if (bEnter) {
				if (winpoint1 == nowpoint || winpoint2 == nowpoint) {
					screen = eScreen::select;
					nowselect = 0;

					gameStart = FALSE;
					bEscape = FALSE;
				}
				else {
					Rst_Map1();
					Rst_Object();
				}

				printscreen = 4;
				Theend = FALSE;
				bEnter = FALSE;
			}
		}
		else {
			Chk_End();
		}

		break;
	case eScreen::map2:
		// 중력 작용 //

		Cal_Gravity(character1);
		Cal_Gravity(character2);


		{//위 아래 처리
			CheckUpDownButton_1();
			CheckUpDownButton_2();
		}

		{//방향키 처리
			CheckLeftRightButton_1();
			CheckLeftRightButton_2();
		}

		// 포탈 처리 //

		Chk_Portal(character1);
		Chk_Portal(character2);

		//게임중 ESC 처리
		if (bEscape) {
			screen = eScreen::select;
			nowselect = 0;

			gameStart = FALSE;
			bEscape = FALSE;
		}

		CheckFallingCharacter(character1);
		CheckFallingCharacter(character2);

		ChangeCharacterMotion_1();
		ChangeCharacterMotion_2();

		break;
	default:
		break;
	}
}

void DrawTitle() {
	oldBit1 = (HBITMAP)SelectObject(mainMemdc, mainBit);
	CImage background{};
	int iWidth{}, iHeight{};

	// 그리기 //

	switch (screen) {
	case eScreen::main1:      // main1 //
	{
		background.Load(L"main1.png");
		iWidth = background.GetWidth();
		iHeight = background.GetHeight();
		background.StretchBlt(mainMemdc, 0, 0, mWidth, mHeight, 0, 0, iWidth, iHeight);
	}
	break;
	case eScreen::main2:      // main2 //
	{
		background.Load(L"main2.png");
		iWidth = background.GetWidth();
		iHeight = background.GetHeight();
		background.StretchBlt(mainMemdc, 0, 0, mWidth, mHeight, 0, 0, iWidth, iHeight);

		CImage button1{};
		button1.Load(L"startb.png");
		iWidth = button1.GetWidth();
		iHeight = button1.GetHeight();

		if (nowselect == 1) {
			button1.StretchBlt(mainMemdc, mWidth / 2 - 200 - 50, mHeight / 2 - 250 - 25, 400 + 100, 200 + 50, 0, 0, iWidth, iHeight);
		}
		else {
			button1.StretchBlt(mainMemdc, mWidth / 2 - 200, mHeight / 2 - 250, 400, 200, 0, 0, iWidth, iHeight);
		}

		CImage button2{};
		button2.Load(L"helpb.png");
		iWidth = button2.GetWidth();
		iHeight = button2.GetHeight();

		if (nowselect == 2) {
			button2.StretchBlt(mainMemdc, mWidth / 2 - 200 - 50, mHeight / 2 + 50 - 25, 400 + 100, 200 + 50, 0, 0, iWidth, iHeight);
		}
		else {
			button2.StretchBlt(mainMemdc, mWidth / 2 - 200, mHeight / 2 + 50, 400, 200, 0, 0, iWidth, iHeight);
		}
	}
	break;
	case eScreen::help:      // help //
	{
		background.Load(L"help.png");
		iWidth = background.GetWidth();
		iHeight = background.GetHeight();
		background.StretchBlt(mainMemdc, 0, 0, mWidth, mHeight, 0, 0, iWidth, iHeight);
	}
	break;
	case eScreen::select:
	{
		background.Load(L"select.png");
		iWidth = background.GetWidth();
		iHeight = background.GetHeight();
		background.StretchBlt(mainMemdc, 0, 0, mWidth, mHeight, 0, 0, iWidth, iHeight);

		CImage button1{};

		if (nowmap == 1) {
			button1.Load(L"map1b.png");
		}
		else {
			button1.Load(L"map2b.png");
		}

		iWidth = button1.GetWidth();
		iHeight = button1.GetHeight();

		if (nowselect == 1) {
			button1.StretchBlt(mainMemdc, mWidth / 2 + 200, mHeight / 2 - 475 - 25, 400 + 100, 200 + 50, 0, 0, iWidth, iHeight);
		}
		else {
			button1.StretchBlt(mainMemdc, mWidth / 2 + 200, mHeight / 2 - 475, 400, 200, 0, 0, iWidth, iHeight);
		}

		CImage button2{};

		if (nowtime == 1) {
			button2.Load(L"time1b.png");
		}
		else if (nowtime == 2) {
			button2.Load(L"time2b.png");
		}
		else {
			button2.Load(L"time3b.png");
		}

		iWidth = button2.GetWidth();
		iHeight = button2.GetHeight();

		if (nowselect == 2) {
			button2.StretchBlt(mainMemdc, mWidth / 2 + 200, mHeight / 2 - 100 - 25, 400 + 100, 200 + 50, 0, 0, iWidth, iHeight);
		}
		else {
			button2.StretchBlt(mainMemdc, mWidth / 2 + 200, mHeight / 2 - 100, 400, 200, 0, 0, iWidth, iHeight);
		}

		CImage button3{};

		if (nowpoint == 1) {
			button3.Load(L"point1b.png");
		}
		else if (nowpoint == 2) {
			button3.Load(L"point2b.png");
		}
		else {
			button3.Load(L"point3b.png");
		}

		iWidth = button3.GetWidth();
		iHeight = button3.GetHeight();

		if (nowselect == 3) {
			button3.StretchBlt(mainMemdc, mWidth / 2 + 200, mHeight / 2 + 275 - 25, 400 + 100, 200 + 50, 0, 0, iWidth, iHeight);
		}
		else {
			button3.StretchBlt(mainMemdc, mWidth / 2 + 200, mHeight / 2 + 275, 400, 200, 0, 0, iWidth, iHeight);
		}
	}
	break;
	}

	// 최종 그리기 //

	StretchBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, mainMemdc, 0, 0, mWidth, mHeight, SRCCOPY);
	SelectObject(mainMemdc, oldBit1);
}


void DrawGame() {//테스트용 임시 함수
	oldBit1 = (HBITMAP)SelectObject(mainMemdc, mainBit);
	static int skyX = 1;

	switch (screen) {
	case eScreen::map1:
	{//new - 하늘 그리기
		//좌측 (이미지는 우측) 출력
		skyImg_1.Draw(mainMemdc, 0, 0, mWidth - skyX, mHeight, skyX, 0, mWidth - skyX, 1800);
		//우측 (이미지는 좌측) 출력
		skyImg_1.Draw(mainMemdc, mWidth - skyX, 0, skyX, mHeight, 0, 0, skyX, 1800);
		skyX += 5;
		if (skyX >= 2399) skyX = 1;
	}

	{//new - 배경그리기
		backGround_1.Draw(mainMemdc, 0, 0, mWidth, mHeight, 0, 0, mWidth, mHeight);
	}

	//backGround.Load(L"map1.png");
	//backGround.Draw(mainMemdc, 0, 0, mWidth, mHeight, 0, 0, backGround.GetWidth(), backGround.GetHeight());
	//printf("mainBit -> %d %d\n", backGround.GetHeight(), backGround.GetHeight());


	// bullet //

	for (int i = 0; i < BULLETNUM; ++i) {
		if (bullet[i].Get_P() != 0) {
			mainMemdc = bullet[i].Print_(mainMemdc);
		}
	}

	// aiming line //

	if (character1.status == LEFT_AIMING || character1.status == RIGHT_AIMING) {
		HPEN hpen = CreatePen(1, 10, RGB(255, 0, 0));
		oldpen = (HPEN)SelectObject(mainMemdc, hpen);

		POINT opt = character1.Get_P();

		MoveToEx(mainMemdc, opt.x, opt.y, NULL);
		LineTo(mainMemdc, opt.x + 200 * cos(character1.radian * PI / 180), opt.y + 200 * sin(character1.radian * PI / 180));

		SelectObject(mainMemdc, oldpen);
		DeleteObject(hpen);
	}

	if (character2.status == LEFT_AIMING || character2.status == RIGHT_AIMING) {
		HPEN hpen = CreatePen(1, 10, RGB(255, 0, 0));
		oldpen = (HPEN)SelectObject(mainMemdc, hpen);

		POINT opt = character2.Get_P();

		MoveToEx(mainMemdc, opt.x, opt.y, NULL);
		LineTo(mainMemdc, opt.x + 200 * cos(character2.radian * PI / 180), opt.y + 200 * sin(character2.radian * PI / 180));

		SelectObject(mainMemdc, oldpen);
		DeleteObject(hpen);
	}

	{//플레이어 캐릭터 출력
	//1p 출력
		if (character1.imgRect.right - character1.imgRect.left != 0) {
			if (character1.status == LEFT_PUNCH || character1.status == RIGHT_PUNCH) {
				if (character1.hand == eHand::axe) {
					switch (character1.imgRect.left) {
					case 0:
						imgCharacter1.Draw(mainMemdc, character1.playerRect.left, character1.playerRect.bottom - 116, 92, 116,
							character1.imgRect.left, character1.imgRect.top, character1.imgRect.right - character1.imgRect.left, character1.imgRect.bottom - character1.imgRect.top);
						break;
					case 92:
						imgCharacter1.Draw(mainMemdc, character1.playerRect.left + 10, character1.playerRect.bottom - 116, 92, 116,
							character1.imgRect.left, character1.imgRect.top, character1.imgRect.right - character1.imgRect.left, character1.imgRect.bottom - character1.imgRect.top);
						break;
					case 184:
						imgCharacter1.Draw(mainMemdc, character1.playerRect.left + 30, character1.playerRect.bottom - 116, 92, 116,
							character1.imgRect.left, character1.imgRect.top, character1.imgRect.right - character1.imgRect.left, character1.imgRect.bottom - character1.imgRect.top);
						break;
					case 276:
						imgCharacter1.Draw(mainMemdc, character1.playerRect.left + 50, character1.playerRect.bottom - 116, 92, 116,
							character1.imgRect.left, character1.imgRect.top, character1.imgRect.right - character1.imgRect.left, character1.imgRect.bottom - character1.imgRect.top);
						break;
					case 408:
						imgCharacter1.Draw(mainMemdc, character1.playerRect.left, character1.playerRect.bottom - 116, 92, 116,
							character1.imgRect.left, character1.imgRect.top, character1.imgRect.right - character1.imgRect.left, character1.imgRect.bottom - character1.imgRect.top);
						break;
					case 500:
						imgCharacter1.Draw(mainMemdc, character1.playerRect.left - 10, character1.playerRect.bottom - 116, 92, 116,
							character1.imgRect.left, character1.imgRect.top, character1.imgRect.right - character1.imgRect.left, character1.imgRect.bottom - character1.imgRect.top);
						break;
					case 592:
						imgCharacter1.Draw(mainMemdc, character1.playerRect.left - 40, character1.playerRect.bottom - 116, 92, 116,
							character1.imgRect.left, character1.imgRect.top, character1.imgRect.right - character1.imgRect.left, character1.imgRect.bottom - character1.imgRect.top);
						break;
					case 684:
						imgCharacter1.Draw(mainMemdc, character1.playerRect.left - 70, character1.playerRect.bottom - 116, 92, 116,
							character1.imgRect.left, character1.imgRect.top, character1.imgRect.right - character1.imgRect.left, character1.imgRect.bottom - character1.imgRect.top);
						break;
					}

				}
				else if (character1.hand == eHand::sword) {
					switch (character1.imgRect.left)
					{
					case 0:
						imgCharacter1.Draw(mainMemdc, character1.playerRect.left, character1.playerRect.bottom - 116, 114, 116,
							character1.imgRect.left, character1.imgRect.top, character1.imgRect.right - character1.imgRect.left, character1.imgRect.bottom - character1.imgRect.top);
						break;
					case 114:
						imgCharacter1.Draw(mainMemdc, character1.playerRect.left + 20, character1.playerRect.bottom - 116, 114, 116,
							character1.imgRect.left, character1.imgRect.top, character1.imgRect.right - character1.imgRect.left, character1.imgRect.bottom - character1.imgRect.top);
						break;
					case 228:
						imgCharacter1.Draw(mainMemdc, character1.playerRect.left + 60, character1.playerRect.bottom - 116, 114, 116,
							character1.imgRect.left, character1.imgRect.top, character1.imgRect.right - character1.imgRect.left, character1.imgRect.bottom - character1.imgRect.top);
						break;
					case 342:
						imgCharacter1.Draw(mainMemdc, character1.playerRect.left + 100, character1.playerRect.bottom - 116, 114, 116,
							character1.imgRect.left, character1.imgRect.top, character1.imgRect.right - character1.imgRect.left, character1.imgRect.bottom - character1.imgRect.top);
						break;

					case 500:
						imgCharacter1.Draw(mainMemdc, character1.playerRect.left, character1.playerRect.bottom - 116, 114, 116,
							character1.imgRect.left, character1.imgRect.top, character1.imgRect.right - character1.imgRect.left, character1.imgRect.bottom - character1.imgRect.top);
						break;
					case 614:
						imgCharacter1.Draw(mainMemdc, character1.playerRect.left - 20, character1.playerRect.bottom - 116, 114, 116,
							character1.imgRect.left, character1.imgRect.top, character1.imgRect.right - character1.imgRect.left, character1.imgRect.bottom - character1.imgRect.top);
						break;
					case 728:
						imgCharacter1.Draw(mainMemdc, character1.playerRect.left - 80, character1.playerRect.bottom - 116, 114, 116,
							character1.imgRect.left, character1.imgRect.top, character1.imgRect.right - character1.imgRect.left, character1.imgRect.bottom - character1.imgRect.top);
						break;
					case 842:
						imgCharacter1.Draw(mainMemdc, character1.playerRect.left - 140, character1.playerRect.bottom - 116, 114, 116,
							character1.imgRect.left, character1.imgRect.top, character1.imgRect.right - character1.imgRect.left, character1.imgRect.bottom - character1.imgRect.top);
						break;
					}

				}
				else {
					switch (character1.imgRect.left) {
					case 517:
						imgCharacter1.Draw(mainMemdc, character1.playerRect.left, character1.playerRect.top, character1.playerRect.right - character1.playerRect.left, character1.playerRect.bottom - character1.playerRect.top,
							character1.imgRect.left, character1.imgRect.top, character1.imgRect.right - character1.imgRect.left, character1.imgRect.bottom - character1.imgRect.top);
						break;
					case 585:
						imgCharacter1.Draw(mainMemdc, character1.playerRect.left + 10, character1.playerRect.top, character1.playerRect.right - character1.playerRect.left, character1.playerRect.bottom - character1.playerRect.top,
							character1.imgRect.left, character1.imgRect.top, character1.imgRect.right - character1.imgRect.left, character1.imgRect.bottom - character1.imgRect.top);
						break;
					case 653:
						imgCharacter1.Draw(mainMemdc, character1.playerRect.left + 20, character1.playerRect.top, character1.playerRect.right - character1.playerRect.left, character1.playerRect.bottom - character1.playerRect.top,
							character1.imgRect.left, character1.imgRect.top, character1.imgRect.right - character1.imgRect.left, character1.imgRect.bottom - character1.imgRect.top);
						break;
					case 731:
						imgCharacter1.Draw(mainMemdc, character1.playerRect.left, character1.playerRect.top, character1.playerRect.right - character1.playerRect.left, character1.playerRect.bottom - character1.playerRect.top,
							character1.imgRect.left, character1.imgRect.top, character1.imgRect.right - character1.imgRect.left, character1.imgRect.bottom - character1.imgRect.top);
						break;
					case 799:
						imgCharacter1.Draw(mainMemdc, character1.playerRect.left - 10, character1.playerRect.top, character1.playerRect.right - character1.playerRect.left, character1.playerRect.bottom - character1.playerRect.top,
							character1.imgRect.left, character1.imgRect.top, character1.imgRect.right - character1.imgRect.left, character1.imgRect.bottom - character1.imgRect.top);
						break;
					case 867:
						imgCharacter1.Draw(mainMemdc, character1.playerRect.left - 20, character1.playerRect.top, character1.playerRect.right - character1.playerRect.left, character1.playerRect.bottom - character1.playerRect.top,
							character1.imgRect.left, character1.imgRect.top, character1.imgRect.right - character1.imgRect.left, character1.imgRect.bottom - character1.imgRect.top);
						break;

					}

				}
			}
			else {
				imgCharacter1.Draw(mainMemdc, character1.playerRect.left, character1.playerRect.top, character1.playerRect.right - character1.playerRect.left, character1.playerRect.bottom - character1.playerRect.top,
					character1.imgRect.left, character1.imgRect.top, character1.imgRect.right - character1.imgRect.left, character1.imgRect.bottom - character1.imgRect.top);
			}

		}
		if (character2.imgRect.right - character2.imgRect.left > 0 && character2.imgRect.bottom - character2.imgRect.top > 0) {

			if (character2.status == LEFT_PUNCH || character2.status == RIGHT_PUNCH) {
				if (character2.hand == eHand::axe) {
					switch (character2.imgRect.left) {
					case 0:
						imgCharacter2.Draw(mainMemdc, character2.playerRect.left, character2.playerRect.bottom - 116, 92, 116,
							character2.imgRect.left, character2.imgRect.top, character2.imgRect.right - character2.imgRect.left, character2.imgRect.bottom - character2.imgRect.top);
						break;
					case 92:
						imgCharacter2.Draw(mainMemdc, character2.playerRect.left + 10, character2.playerRect.bottom - 116, 92, 116,
							character2.imgRect.left, character2.imgRect.top, character2.imgRect.right - character2.imgRect.left, character2.imgRect.bottom - character2.imgRect.top);
						break;
					case 184:
						imgCharacter2.Draw(mainMemdc, character2.playerRect.left + 30, character2.playerRect.bottom - 116, 92, 116,
							character2.imgRect.left, character2.imgRect.top, character2.imgRect.right - character2.imgRect.left, character2.imgRect.bottom - character2.imgRect.top);
						break;
					case 276:
						imgCharacter2.Draw(mainMemdc, character2.playerRect.left + 50, character2.playerRect.bottom - 116, 92, 116,
							character2.imgRect.left, character2.imgRect.top, character2.imgRect.right - character2.imgRect.left, character2.imgRect.bottom - character2.imgRect.top);
						break;
					case 408:
						imgCharacter2.Draw(mainMemdc, character2.playerRect.left, character2.playerRect.bottom - 116, 92, 116,
							character2.imgRect.left, character2.imgRect.top, character2.imgRect.right - character2.imgRect.left, character2.imgRect.bottom - character2.imgRect.top);
						break;
					case 500:
						imgCharacter2.Draw(mainMemdc, character2.playerRect.left - 10, character2.playerRect.bottom - 116, 92, 116,
							character2.imgRect.left, character2.imgRect.top, character2.imgRect.right - character2.imgRect.left, character2.imgRect.bottom - character2.imgRect.top);
						break;
					case 592:
						imgCharacter2.Draw(mainMemdc, character2.playerRect.left - 40, character2.playerRect.bottom - 116, 92, 116,
							character2.imgRect.left, character2.imgRect.top, character2.imgRect.right - character2.imgRect.left, character2.imgRect.bottom - character2.imgRect.top);
						break;
					case 684:
						imgCharacter2.Draw(mainMemdc, character2.playerRect.left - 70, character2.playerRect.bottom - 116, 92, 116,
							character2.imgRect.left, character2.imgRect.top, character2.imgRect.right - character2.imgRect.left, character2.imgRect.bottom - character2.imgRect.top);
						break;
					}

				}
				else if (character2.hand == eHand::sword) {
					switch (character2.imgRect.left)
					{
					case 0:
						imgCharacter2.Draw(mainMemdc, character2.playerRect.left, character2.playerRect.bottom - 116, 114, 116,
							character2.imgRect.left, character2.imgRect.top, character2.imgRect.right - character2.imgRect.left, character2.imgRect.bottom - character2.imgRect.top);
						break;
					case 114:
						imgCharacter2.Draw(mainMemdc, character2.playerRect.left + 20, character2.playerRect.bottom - 116, 114, 116,
							character2.imgRect.left, character2.imgRect.top, character2.imgRect.right - character2.imgRect.left, character2.imgRect.bottom - character2.imgRect.top);
						break;
					case 228:
						imgCharacter2.Draw(mainMemdc, character2.playerRect.left + 60, character2.playerRect.bottom - 116, 114, 116,
							character2.imgRect.left, character2.imgRect.top, character2.imgRect.right - character2.imgRect.left, character2.imgRect.bottom - character2.imgRect.top);
						break;
					case 342:
						imgCharacter2.Draw(mainMemdc, character2.playerRect.left + 100, character2.playerRect.bottom - 116, 114, 116,
							character2.imgRect.left, character2.imgRect.top, character2.imgRect.right - character2.imgRect.left, character2.imgRect.bottom - character2.imgRect.top);
						break;

					case 500:
						imgCharacter2.Draw(mainMemdc, character2.playerRect.left, character2.playerRect.bottom - 116, 114, 116,
							character2.imgRect.left, character2.imgRect.top, character2.imgRect.right - character2.imgRect.left, character2.imgRect.bottom - character2.imgRect.top);
						break;
					case 614:
						imgCharacter2.Draw(mainMemdc, character2.playerRect.left - 20, character2.playerRect.bottom - 116, 114, 116,
							character2.imgRect.left, character2.imgRect.top, character2.imgRect.right - character2.imgRect.left, character2.imgRect.bottom - character2.imgRect.top);
						break;
					case 728:
						imgCharacter2.Draw(mainMemdc, character2.playerRect.left - 80, character2.playerRect.bottom - 116, 114, 116,
							character2.imgRect.left, character2.imgRect.top, character2.imgRect.right - character2.imgRect.left, character2.imgRect.bottom - character2.imgRect.top);
						break;
					case 842:
						imgCharacter2.Draw(mainMemdc, character2.playerRect.left - 140, character2.playerRect.bottom - 116, 114, 116,
							character2.imgRect.left, character2.imgRect.top, character2.imgRect.right - character2.imgRect.left, character2.imgRect.bottom - character2.imgRect.top);
						break;
					}

				}
				else {
					printf("%d %d %d %d\n", character2.imgRect.left, character2.imgRect.top, character2.imgRect.right, character2.imgRect.bottom);//asd
					switch (character2.imgRect.left) {
					case 517:
						imgCharacter2.Draw(mainMemdc, character2.playerRect.left, character2.playerRect.top, character2.playerRect.right - character2.playerRect.left, character2.playerRect.bottom - character2.playerRect.top,
							character2.imgRect.left, character2.imgRect.top, character2.imgRect.right - character2.imgRect.left, character2.imgRect.bottom - character2.imgRect.top);
						break;
					case 585:
						imgCharacter2.Draw(mainMemdc, character2.playerRect.left + 10, character2.playerRect.top, character2.playerRect.right - character2.playerRect.left, character2.playerRect.bottom - character2.playerRect.top,
							character2.imgRect.left, character2.imgRect.top, character2.imgRect.right - character2.imgRect.left, character2.imgRect.bottom - character2.imgRect.top);
						break;
					case 653:
						imgCharacter2.Draw(mainMemdc, character2.playerRect.left + 20, character2.playerRect.top, character2.playerRect.right - character2.playerRect.left, character2.playerRect.bottom - character2.playerRect.top,
							character2.imgRect.left, character2.imgRect.top, character2.imgRect.right - character2.imgRect.left, character2.imgRect.bottom - character2.imgRect.top);
						break;
					case 731:
						imgCharacter2.Draw(mainMemdc, character2.playerRect.left, character2.playerRect.top, character2.playerRect.right - character2.playerRect.left, character2.playerRect.bottom - character2.playerRect.top,
							character2.imgRect.left, character2.imgRect.top, character2.imgRect.right - character2.imgRect.left, character2.imgRect.bottom - character2.imgRect.top);
						break;
					case 799:
						imgCharacter2.Draw(mainMemdc, character2.playerRect.left - 10, character2.playerRect.top, character2.playerRect.right - character2.playerRect.left, character2.playerRect.bottom - character2.playerRect.top,
							character2.imgRect.left, character2.imgRect.top, character2.imgRect.right - character2.imgRect.left, character2.imgRect.bottom - character2.imgRect.top);
						break;
					case 867:
						imgCharacter2.Draw(mainMemdc, character2.playerRect.left - 20, character2.playerRect.top, character2.playerRect.right - character2.playerRect.left, character2.playerRect.bottom - character2.playerRect.top,
							character2.imgRect.left, character2.imgRect.top, character2.imgRect.right - character2.imgRect.left, character2.imgRect.bottom - character2.imgRect.top);
						break;

					}

				}
			}
			else {
				imgCharacter2.Draw(mainMemdc, character2.playerRect.left, character2.playerRect.top, character2.playerRect.right - character2.playerRect.left, character2.playerRect.bottom - character2.playerRect.top,
					character2.imgRect.left, character2.imgRect.top, character2.imgRect.right - character2.imgRect.left, character2.imgRect.bottom - character2.imgRect.top);
			}
		}

		//Rectangle(mainMemdc, character1.playerRect.left, character1.playerRect.top, character1.playerRect.right, character1.playerRect.bottom);

		//2p 출력
		//Rectangle(mainMemdc, character2.playerRect.left, character2.playerRect.top, character2.playerRect.right, character2.playerRect.bottom);
	  /*  if (character2.imgRect.right - character2.imgRect.left > 0 && character2.imgRect.bottom - character2.imgRect.top > 0);
		else printf("무슨일?????????????\n");*/
	}

	//elevator //

	{
		ielevator.StretchBlt(mainMemdc, map1GroundRect[ELEVATOR].left, map1GroundRect[ELEVATOR].top, 100, 20, 0, 0, ielevator.GetWidth(), ielevator.GetHeight());
	}

	// grenade //

	for (int i = 0; i < GRENADENUM; ++i) {
		if (grenade[i].Get_S() == 2) {
			mainMemdc = grenade[i].Print_(mainMemdc);
		}
	}

	// explosion //

	for (int i = 0; i < EXPLOSIONTNUM; ++i) {
		if (explosion[i].Get_S() != 0) {
			mainMemdc = explosion[i].Print_(mainMemdc);
		}
	}

	// item //

	for (int i = 0; i < ITEMNUM; ++i) {
		mainMemdc = item[i].Print_(mainMemdc);
	}

	// 라운드 종료 알림 //

	if (Theend) {
		HFONT hfont = CreateFont(50, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0, 0, L"end");
		oldfont = (HFONT)SelectObject(mainMemdc, hfont);

		Rectangle(mainMemdc, mWidth / 4, mHeight / 4, mWidth * 3 / 4, mHeight * 3 / 4);

		RECT drawrect{};
		TCHAR text[30]{};

		drawrect = { mWidth / 4, mHeight / 4, mWidth * 3 / 4, mHeight / 4 + mHeight / 2 / 3 };
		if (winner == 3) {
			wsprintf(text, L"Draw !!");
		}
		else {
			wsprintf(text, L"Winner : Player %d !!", winner);
		}
		DrawText(mainMemdc, text, -1, &drawrect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

		drawrect = { mWidth / 4, mHeight / 4 + mHeight / 2 / 3, mWidth * 3 / 4, mHeight / 4 + mHeight / 2 * 2 / 3 };
		wsprintf(text, L"Player1 - %d : %d - Player2", winpoint1, winpoint2);
		DrawText(mainMemdc, text, -1, &drawrect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

		drawrect = { mWidth / 4, mHeight / 4 + mHeight / 2 * 2 / 3, mWidth * 3 / 4, mHeight / 4 + mHeight / 2 };
		wsprintf(text, L"Enter");
		DrawText(mainMemdc, text, -1, &drawrect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

		SelectObject(mainMemdc, oldfont);
		DeleteObject(hfont);
	}

	//{//테스트용 충돌 RECT 출력
	//	HBRUSH hbrush = CreateSolidBrush(RGB(0, 255, 0));    //발코니 렉트 추가, 색 추가
	//	SelectObject(mainMemdc, hbrush);

	//	for (auto rect : map1BalconyRect) {
	//		Rectangle(mainMemdc, rect.left, rect.top, rect.right, rect.bottom);
	//	}
	//	DeleteObject(hbrush);

	//	hbrush = CreateSolidBrush(RGB(255, 255, 255));
	//	SelectObject(mainMemdc, hbrush);

	//	for (auto rect : map1GroundRect) {
	//		Rectangle(mainMemdc, rect.left, rect.top, rect.right, rect.bottom);
	//	}

	//	DeleteObject(hbrush);

	//	hbrush = CreateSolidBrush(RGB(212, 37, 126));
	//	SelectObject(mainMemdc, hbrush);
	//	for (auto rect : map1StairRect) {
	//		Rectangle(mainMemdc, rect.left, rect.top, rect.right, rect.bottom);
	//	}
	//	DeleteObject(hbrush);

	//	hbrush = CreateSolidBrush(RGB(126, 126, 37));
	//	SelectObject(mainMemdc, hbrush);
	//	Rectangle(mainMemdc, map1LadderRect.left, map1LadderRect.top, map1LadderRect.right, map1LadderRect.bottom);
	//	DeleteObject(hbrush);
	//}

	//{
	//    //HFONT hFont = CreateFont(100, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	//    HFONT hFont = CreateFont(50, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	//    SelectObject(mainMemdc, hFont);
	//    //TextOut(mainMemdc, 0,0, aaaaaaaaaaaaaa, lstrlen(aaaaaaaaaaaaaa));
	//    TextOut(mainMemdc, character1.playerRect.left - 300, character1.playerRect.top - 300, aaaaaaaaaaaaaa, lstrlen(aaaaaaaaaaaaaa));
	//    DeleteObject(hFont);
	//}

	break;
	case eScreen::map2:
	{//new - 하늘 그리기
		//좌측 (이미지는 우측) 출력
		skyImg_1.Draw(mainMemdc, 0, 0, mWidth - skyX, mHeight, skyX, 0, mWidth - skyX, 1800);
		//우측 (이미지는 좌측) 출력
		skyImg_1.Draw(mainMemdc, mWidth - skyX, 0, skyX, mHeight, 0, 0, skyX, 1800);
		skyX += 5;
		if (skyX >= 2399) skyX = 1;
	}

	{//new - 배경그리기
		backGround_2.Draw(mainMemdc, 0, 0, mWidth, mHeight, 0, 0, mWidth, mHeight);
	}

	{//플레이어 캐릭터 출력
	//1p 출력
		if (character1.imgRect.right - character1.imgRect.left != 0)
			imgCharacter1.Draw(mainMemdc, character1.playerRect.left, character1.playerRect.top, character1.playerRect.right - character1.playerRect.left, character1.playerRect.bottom - character1.playerRect.top,
				character1.imgRect.left, character1.imgRect.top, character1.imgRect.right - character1.imgRect.left, character1.imgRect.bottom - character1.imgRect.top);
		if (character2.imgRect.right - character2.imgRect.left > 0 && character2.imgRect.bottom - character2.imgRect.top > 0)
			imgCharacter2.Draw(mainMemdc, character2.playerRect.left, character2.playerRect.top, character2.playerRect.right - character2.playerRect.left, character2.playerRect.bottom - character2.playerRect.top,
				character2.imgRect.left, character2.imgRect.top, character2.imgRect.right - character2.imgRect.left, character2.imgRect.bottom - character2.imgRect.top);

		//Rectangle(mainMemdc, character1.playerRect.left, character1.playerRect.top, character1.playerRect.right, character1.playerRect.bottom);

		//2p 출력
		//Rectangle(mainMemdc, character2.playerRect.left, character2.playerRect.top, character2.playerRect.right, character2.playerRect.bottom);
	  /*  if (character2.imgRect.right - character2.imgRect.left > 0 && character2.imgRect.bottom - character2.imgRect.top > 0);
		else printf("무슨일?????????????\n");*/
	}


	//{//test RECT 그리기
	//    HBRUSH hBrush;
	//    hBrush = CreateSolidBrush(RGB(255, 255, 255));
	//    SelectObject(mainMemdc, hBrush);
	//    for (auto rect : map2GroundRect) {
	//        Rectangle(mainMemdc, rect.left, rect.top, rect.right, rect.bottom);
	//    }
	//    DeleteObject(hBrush);

	//    hBrush = CreateSolidBrush(RGB(255,174,201));
	//    SelectObject(mainMemdc, hBrush);
	//    for (auto rect : map2BalconyRect) {
	//        Rectangle(mainMemdc, rect.left, rect.top, rect.right, rect.bottom);
	//    }
	//    DeleteObject(hBrush);

	//    hBrush = CreateSolidBrush(RGB(31, 197, 105));
	//    SelectObject(mainMemdc, hBrush);
	//    for (auto rect : map2Stair1Rect) {
	//        Rectangle(mainMemdc, rect.left, rect.top, rect.right, rect.bottom);
	//    }
	//    DeleteObject(hBrush);

	//    hBrush = CreateSolidBrush(RGB(0,128,255));
	//    SelectObject(mainMemdc, hBrush);
	//    for (auto rect : map2Stair2Rect) {
	//        Rectangle(mainMemdc, rect.left, rect.top, rect.right, rect.bottom);
	//    }
	//    DeleteObject(hBrush);

	//    hBrush = CreateSolidBrush(RGB(255,128,64));
	//    SelectObject(mainMemdc, hBrush);
	//    for (auto rect : map2Stair3Rect) {
	//        Rectangle(mainMemdc, rect.left, rect.top, rect.right, rect.bottom);
	//    }
	//    DeleteObject(hBrush);

	//    hBrush = CreateSolidBrush(RGB(255,255,0));
	//    SelectObject(mainMemdc, hBrush);
	//    for (auto rect : map2LeftPortalRect) {
	//        Rectangle(mainMemdc, rect.left, rect.top, rect.right, rect.bottom);
	//    }
	//    DeleteObject(hBrush);

	//    hBrush = CreateSolidBrush(RGB(255, 0, 255));
	//    SelectObject(mainMemdc, hBrush);
	//    for (auto rect : map2LadderRect) {
	//        Rectangle(mainMemdc, rect.left, rect.top, rect.right, rect.bottom);
	//    }
	//    DeleteObject(hBrush);
	//    
	//}
	break;
	default:
		break;
	}



	//new - 전체화면 - 1 // 플레이어 1 - 2 // 플레이어 2 - 3 // 플레이어 사이 거리 측정 - 4
	PrintScreen(printscreen);
	//new - 윈도우창 그리기
	DrawStatusWindow();

	BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, plusMemdc, 0, 0, SRCCOPY); //new
	SelectObject(plusMemdc, oldBit2); //new
	SelectObject(mainMemdc, oldBit1);
}

void PrintScreen(int select) { //new - 
	//전체화면 - 1 // 플레이어 1 - 2 // 플레이어 2 - 3 // 플레이어 사이 거리 측정 - 4
	switch (select) {
	case 1:
		StretchBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, mainMemdc, 0, 0, mWidth, mHeight, SRCCOPY);
		break;
	case 2:
		StretchBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, mainMemdc, character1.playerRect.left - 300, character1.playerRect.top - 300, 600, 600, SRCCOPY);
		break;
	case 3:
		StretchBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, mainMemdc, character2.playerRect.left - 300, character2.playerRect.top - 300, 600, 600, SRCCOPY);
		break;
	case 4:
	{
		int character1_x = character1.playerRect.left + CharacterWidth / 2, character1_y = character1.playerRect.top + CharacterHeight / 2;
		int character2_x = character2.playerRect.left + CharacterWidth / 2, character2_y = character2.playerRect.top + CharacterHeight / 2;
		int characterX = character2_x - character1_x;
		int characterY = character2_y - character1_y;
		if (characterX <= 0) characterX *= -1;
		if (characterY <= 0)characterY *= -1;

		int size = 0;
		if (characterX >= characterY) {
			size = characterX + 400;
			if (size < 800) size = 800;
		}
		else {
			size = characterY + 400;
			if (size < 800) size = 800;
		}

		int middleCharacterX, middleCharacterY;
		if (character1_x < character2_x) middleCharacterX = character1_x + characterX / 2;
		else middleCharacterX = character2_x + characterX / 2;

		if (character1_y < character2_y) middleCharacterY = character1_y + characterY / 2;
		else middleCharacterY = character2_y + characterY / 2;

		if (printCharacterRect.right == 0 && printCharacterRect.bottom == 0) {
			printCharacterRect = {
				middleCharacterX - size / 2,
				middleCharacterY - size / 2,
				middleCharacterX + size / 2,
				middleCharacterY + size / 2
			};
		}
		else {
			printCharacterRect = {
				 middleCharacterX - size / 2,
				 middleCharacterY - size / 2,
				 middleCharacterX + size / 2,
				 middleCharacterY + size / 2
			};
			if (printCharacterRect.left < 0) {
				printCharacterRect.right = printCharacterRect.right - printCharacterRect.left;
				printCharacterRect.left = 0;
			}
			if (printCharacterRect.top < 0) {
				printCharacterRect.bottom = printCharacterRect.bottom - printCharacterRect.top;
				printCharacterRect.top = 0;
			}
			if (printCharacterRect.right >= mWidth) {
				printCharacterRect.left = mWidth - 1 - (printCharacterRect.right - printCharacterRect.left);
				printCharacterRect.right = mWidth - 1;
			}
			if (printCharacterRect.bottom >= mHeight) {
				printCharacterRect.top = mHeight - 1 - (printCharacterRect.bottom - printCharacterRect.top);
				printCharacterRect.bottom = mHeight - 1;
			}
		}

		oldBit2 = (HBITMAP)SelectObject(plusMemdc, tempHDCBit);
		StretchBlt(plusMemdc, 0, 0, clientRect.right, clientRect.bottom, mainMemdc, printCharacterRect.left, printCharacterRect.top, printCharacterRect.right - printCharacterRect.left, printCharacterRect.bottom - printCharacterRect.top, SRCCOPY);
		break;
	}

	}
}

void DrawStatusWindow() {//new

	HBRUSH hBrush_GRAY = CreateSolidBrush(RGB(150, 150, 150));
	HBRUSH hBrush_RED = CreateSolidBrush(RGB(255, 0, 0));

	//플레이어 2
	RECT windowRect;
	oldbrush = (HBRUSH)SelectObject(plusMemdc, hBrush_GRAY);
	Rectangle(plusMemdc, 0, 0, 360, 80);
	SelectObject(plusMemdc, oldbrush);

	windowRect = { 10,10,70,70 };
	Rectangle(plusMemdc, windowRect.left, windowRect.top, windowRect.right, windowRect.bottom);

	if (character2.imgRect.right - character2.imgRect.left != 0)
		imgCharacter2.Draw(plusMemdc, 15, 15, 50, 50,
			character2.imgRect.left, character2.imgRect.top, character2.imgRect.right - character2.imgRect.left, character2.imgRect.bottom - character2.imgRect.top);

	{//2p - 근접 상태창
		windowRect = { 80,10,80 + 40,10 + 40 };
		Rectangle(plusMemdc, windowRect.left, windowRect.top, windowRect.right, windowRect.bottom);
		//DrawText(plusMemdc, L"1", lstrlen(L"1"), &windowRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
		if (character2.hand == eHand::sword) {
			Sword.Draw(plusMemdc, windowRect.left + 5, windowRect.top + 15, windowRect.right - windowRect.left - 10, windowRect.bottom - windowRect.top - 30,
				5, 15, 88, 22);
		}
		else if (character2.hand == eHand::axe) {
			Axe.Draw(plusMemdc, windowRect.left + 5, windowRect.top + 10, windowRect.right - windowRect.left - 10, windowRect.bottom - windowRect.top - 20,
				11, 8, 73, 34);
		}
	}

	{//2p - 총 상태창
		windowRect = { 130,10,130 + 40,10 + 40 };
		Rectangle(plusMemdc, windowRect.left, windowRect.top, windowRect.right, windowRect.bottom);
		//DrawText(plusMemdc, L"2", lstrlen(L"2"), &windowRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
		if (character2.gun == eGun::pistol) {
			Pistol.Draw(plusMemdc, windowRect.left + 5, windowRect.top + 5, windowRect.right - windowRect.left - 10, windowRect.bottom - windowRect.top - 10,
				35, 13, 31, 27);
		}
		else if (character2.gun == eGun::uzi) {
			Uzi.Draw(plusMemdc, windowRect.left + 5, windowRect.top + 5, windowRect.right - windowRect.left - 10, windowRect.bottom - windowRect.top - 10,
				32, 8, 41, 36);
		}
		windowRect = { 170,10,170 + 40,10 + 40 };
		Rectangle(plusMemdc, windowRect.left, windowRect.top, windowRect.right, windowRect.bottom);
		TCHAR gunnum[5];
		if (character2.gunnum > 0)wsprintf(gunnum, L"%d", character2.gunnum);
		else wsprintf(gunnum, L"...");
		DrawText(plusMemdc, gunnum, lstrlen(gunnum), &windowRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	}

	{//2p - 수류탄 상태창
		windowRect = { 220,10,220 + 40,10 + 40 };
		Rectangle(plusMemdc, windowRect.left, windowRect.top, windowRect.right, windowRect.bottom);
		//DrawText(plusMemdc, L"3", lstrlen(L"3"), &windowRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
		if (character2.throwing == eThrowing::grenade) {
			Grenade.Draw(plusMemdc, windowRect.left + 5, windowRect.top + 10, windowRect.right - windowRect.left - 10, windowRect.bottom - windowRect.top - 20,
				19, 8, 63, 34);
		}

		windowRect = { 260,10,260 + 40,10 + 40 };
		Rectangle(plusMemdc, windowRect.left, windowRect.top, windowRect.right, windowRect.bottom);
		TCHAR throwingnum[5];
		if (character2.throingnum > 0) wsprintf(throwingnum, L"%d", character2.throingnum);
		else wsprintf(throwingnum, L"...");
		DrawText(plusMemdc, throwingnum, lstrlen(throwingnum), &windowRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	}

	{//2p - 아이템창
		windowRect = { 310,10,310 + 40,10 + 40 };
		Rectangle(plusMemdc, windowRect.left, windowRect.top, windowRect.right, windowRect.bottom);
		DrawText(plusMemdc, L"", lstrlen(L""), &windowRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	}


	{//2p - 체력창
		windowRect = { 80,55,350,70 };
		Rectangle(plusMemdc, windowRect.left, windowRect.top, windowRect.right, windowRect.bottom);
		SelectObject(plusMemdc, hBrush_RED);
		Rectangle(plusMemdc, 80, 55, (int)(80 + (double)character2.hp / CharacterMaxHp * (350 - 80)), 70);
		SelectObject(plusMemdc, hBrush_GRAY);
	}




	{//플레이어 1
		windowRect = { 10,10,70,70 };
		Rectangle(plusMemdc, clientRect.right, 0, clientRect.right - 360, 80);
		SelectObject(plusMemdc, oldbrush);
	}

	{//1p - 캐릭터 출력
		windowRect = { clientRect.right - 10,10,clientRect.right - 10 - 60,10 + 60 };
		Rectangle(plusMemdc, windowRect.left, windowRect.top, windowRect.right, windowRect.bottom);
		if (character1.imgRect.right - character1.imgRect.left > 0 && character1.imgRect.bottom - character1.imgRect.top > 0)
			imgCharacter1.Draw(plusMemdc, windowRect.right + 5, 15, 50, 50,
				character1.imgRect.left, character1.imgRect.top, character1.imgRect.right - character1.imgRect.left, character1.imgRect.bottom - character1.imgRect.top);
	}


	{//1p - 근접 상태창
		windowRect = { clientRect.right - 310 - 40,10,clientRect.right - 310,10 + 40 };
		Rectangle(plusMemdc, windowRect.left, windowRect.top, windowRect.right, windowRect.bottom);
		//DrawText(plusMemdc, L"1", lstrlen(L"1"), &windowRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
		if (character1.hand == eHand::sword) {
			Sword.Draw(plusMemdc, windowRect.left + 5, windowRect.top + 15, windowRect.right - windowRect.left - 10, windowRect.bottom - windowRect.top - 30,
				5, 15, 88, 22);
		}
		else if (character1.hand == eHand::axe) {
			Axe.Draw(plusMemdc, windowRect.left + 5, windowRect.top + 10, windowRect.right - windowRect.left - 10, windowRect.bottom - windowRect.top - 20,
				11, 8, 73, 34);
		}

	}

	{//1p - 총 상태창
		windowRect = { clientRect.right - 260 - 40,10,clientRect.right - 260,10 + 40 };
		Rectangle(plusMemdc, windowRect.left, windowRect.top, windowRect.right, windowRect.bottom);
		//DrawText(plusMemdc, L"2", lstrlen(L"2"), &windowRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
		if (character1.gun == eGun::pistol) {
			Pistol.Draw(plusMemdc, windowRect.left + 5, windowRect.top + 5, windowRect.right - windowRect.left - 10, windowRect.bottom - windowRect.top - 10,
				35, 13, 31, 27);
		}
		else if (character1.gun == eGun::uzi) {
			Uzi.Draw(plusMemdc, windowRect.left + 5, windowRect.top + 5, windowRect.right - windowRect.left - 10, windowRect.bottom - windowRect.top - 10,
				32, 8, 41, 36);
		}

		windowRect = { clientRect.right - 220 - 40,10,clientRect.right - 220,10 + 40 };
		Rectangle(plusMemdc, windowRect.left, windowRect.top, windowRect.right, windowRect.bottom);
		//DrawText(plusMemdc, L"..", lstrlen(L".."), &windowRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
		TCHAR gunnum[5];
		if (character1.gunnum > 0)wsprintf(gunnum, L"%d", character1.gunnum);
		else wsprintf(gunnum, L"...");
		DrawText(plusMemdc, gunnum, lstrlen(gunnum), &windowRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	}

	{//1p - 수류탄 상태창
		windowRect = { clientRect.right - 170 - 40,10,clientRect.right - 170,10 + 40 };
		Rectangle(plusMemdc, windowRect.left, windowRect.top, windowRect.right, windowRect.bottom);
		//DrawText(plusMemdc, L"3", lstrlen(L"3"), &windowRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
		if (character1.throwing == eThrowing::grenade) {
			Grenade.Draw(plusMemdc, windowRect.left + 5, windowRect.top + 10, windowRect.right - windowRect.left - 10, windowRect.bottom - windowRect.top - 20,
				19, 8, 63, 34);
		}

		windowRect = { clientRect.right - 130,10,clientRect.right - 130 - 40,10 + 40 };
		Rectangle(plusMemdc, windowRect.left, windowRect.top, windowRect.right, windowRect.bottom);
		//DrawText(plusMemdc, L"..", lstrlen(L".."), &windowRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
		TCHAR throwingnum[5];
		if (character1.throingnum > 0) wsprintf(throwingnum, L"%d", character1.throingnum);
		else wsprintf(throwingnum, L"...");
		DrawText(plusMemdc, throwingnum, lstrlen(throwingnum), &windowRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

	}

	{//1p - 아이템 창
		windowRect = { clientRect.right - 80 - 40,10,clientRect.right - 80,10 + 40 };
		Rectangle(plusMemdc, windowRect.left, windowRect.top, windowRect.right, windowRect.bottom);
		DrawText(plusMemdc, L"", lstrlen(L""), &windowRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	}

	{//1p - 체력창
		windowRect = { clientRect.right - 350,55,clientRect.right - 80 ,70 };
		Rectangle(plusMemdc, windowRect.left, windowRect.top, windowRect.right, windowRect.bottom);
		oldbrush = (HBRUSH)SelectObject(plusMemdc, hBrush_RED);
		Rectangle(plusMemdc, windowRect.left, 55, (int)(windowRect.left + (double)character1.hp / CharacterMaxHp * (350 - 80)), 70);
		SelectObject(plusMemdc, oldbrush);
	}

	DeleteObject(hBrush_RED);
	DeleteObject(hBrush_GRAY);
}

// definition (game) //

void Move_Elevator() {
	if (elevatordrct == 1) {
		map1GroundRect[ELEVATOR].top -= EMOVE;
		map1GroundRect[ELEVATOR].bottom -= EMOVE;

		if (map1GroundRect[ELEVATOR].top == 1080) {
			elevatordrct = -1;
		}
	}
	else {
		map1GroundRect[ELEVATOR].top += EMOVE;
		map1GroundRect[ELEVATOR].bottom += EMOVE;

		if (map1GroundRect[ELEVATOR].bottom == 1500) {
			elevatordrct = 1;
		}
	}
}

void CharacterElevatorCheck(Character& character) {
	//엘리베이터에서 오른쪽 벽에 끼이게 될 경우에 대한 처리
	RECT nullRect;
	if (map1GroundRect[ELEVATOR].left <= character.playerRect.left && character.playerRect.left <= map1GroundRect[ELEVATOR].right && IntersectRect(&nullRect, &map1GroundRect[ELEVATOR], &character.playerRect)) {
		if (map1GroundRect[ELEVATOR].bottom >= character.playerRect.top + (character.playerRect.bottom - character.playerRect.top) / 2) {
			//printf("태울려는거임\n");
			character.playerRect.bottom = map1GroundRect[ELEVATOR].top;
			character.playerRect.top = character.playerRect.bottom - CharacterHeight;
		}
		else {
			//printf("옆으로 보내려는거임\n");
			character.playerRect.left = map1GroundRect[ELEVATOR].right;
			character.playerRect.right = character.playerRect.left + CharacterWidth;
		}

	}
	if (map1GroundRect[ELEVATOR].left <= character.playerRect.left && character.playerRect.left <= map1GroundRect[ELEVATOR].right && IntersectRect(&nullRect, &map1GroundRect[7], &character.playerRect)) {
		character.playerRect.right = map1GroundRect[7].left;
		character.playerRect.left = character.playerRect.right - CharacterWidth;
	}
	if (map1GroundRect[ELEVATOR].left <= character.playerRect.right && character.playerRect.right <= map1GroundRect[ELEVATOR].right && IntersectRect(&nullRect, &map1BalconyRect[1], &character.playerRect)) {
		character.playerRect.left = map1BalconyRect[1].right;
		character.playerRect.right = character.playerRect.left + CharacterWidth;
	}
}

void Cal_Gravity(Character& character) {
	if (character.status == LADDER_UP || character.status == LADDER_DOWN) {
		return;
	}

	character.playerRect.top += character.GravitySpeed;
	character.playerRect.bottom += character.GravitySpeed;
	RECT playerFootRect = { character.playerRect.left,character.playerRect.bottom - character.GravitySpeed,character.playerRect.right,character.playerRect.bottom };
	BOOL checkDrop = false;

	switch (screen) {
	case eScreen::map1:
		for (auto rect : map1StairRect) {
			RECT nullRect;
			if (IntersectRect(&nullRect, &rect, &character.playerRect)) {
				if (character.GravitySpeed <= 0) {
					character.playerRect.top = rect.bottom;
					character.playerRect.bottom = character.playerRect.top + CharacterHeight;
					character.GravitySpeed = CharacterGravitySpeed;
					break;
				}
				if (character.status == LEFT_JUMP)character.status = LEFT_STAY;
				else if (character.status == RIGHT_JUMP)character.status = RIGHT_STAY;
				character.playerRect.top = rect.top - CharacterHeight;
				character.playerRect.bottom = rect.top;
				if (character.GravitySpeed >= ChangeToFallingSpeed || character.status == LEFT_FALLING || character.status == RIGHT_FALLING) {
					printf("이건 낙뎀 들어가는 높이다.플레이어의 체력을 감소시키자.\n");
					character.hp -= FallingDamage;
					if (character.hp < 0) character.hp = 0;
					if (character.status == LEFT_FALLING) character.status = LEFT_STANDING;
					else if (character.status == RIGHT_FALLING)character.status = RIGHT_STANDING;
				}
				else {
					if (character.status == LEFT_JUMP) character.status = LEFT_STAY;
					else if (character.status == RIGHT_JUMP)character.status = RIGHT_STAY;
				}
				character.GravitySpeed = CharacterGravitySpeed;
				checkDrop = true;
				break;
			}
		}
		if (!checkDrop)for (auto rect : map1GroundRect) {
			RECT nullRect;
			if (IntersectRect(&nullRect, &rect, /*&playerFootRect*/&character.playerRect)) {
				if (character.GravitySpeed <= 0) {
					character.playerRect.top = rect.bottom;
					character.playerRect.bottom = character.playerRect.top + CharacterHeight;
					character.GravitySpeed = CharacterGravitySpeed;
					break;
				}
				if (character.status == LEFT_JUMP)character.status = LEFT_STAY;
				else if (character.status == RIGHT_JUMP)character.status = RIGHT_STAY;
				character.playerRect.top = rect.top - CharacterHeight;
				character.playerRect.bottom = rect.top;
				if (character.GravitySpeed >= ChangeToFallingSpeed || character.status == LEFT_FALLING || character.status == RIGHT_FALLING) {
					printf("이건 낙뎀 들어가는 높이다.플레이어의 체력을 감소시키자.\n");
					character.hp -= FallingDamage;
					if (character.hp < 0) character.hp = 0;
					if (character.status == LEFT_FALLING) character.status = LEFT_STANDING;
					else if (character.status == RIGHT_FALLING)character.status = RIGHT_STANDING;
				}
				else {
					if (character.status == LEFT_JUMP) character.status = LEFT_STAY;
					else if (character.status == RIGHT_JUMP)character.status = RIGHT_STAY;
				}
				character.GravitySpeed = CharacterGravitySpeed;
				checkDrop = true;
			}
		}
		if (!checkDrop)for (auto rect : map1BalconyRect) {
			RECT nullRect;
			if (IntersectRect(&nullRect, &rect, &playerFootRect)) {
				if (character.GravitySpeed <= 0) {
					character.playerRect.top = rect.bottom;
					character.playerRect.bottom = character.playerRect.top + CharacterHeight;
					character.GravitySpeed = CharacterGravitySpeed;
					break;
				}
				if (character.downJump) {
					character.playerRect.top = rect.bottom;
					character.playerRect.bottom = character.playerRect.top + CharacterHeight;
					character.GravitySpeed = CharacterGravitySpeed;
					checkDrop = true;
					character.downJump = false;
					break;
				}
				if (character.status == LEFT_JUMP)character.status = LEFT_STAY;
				else if (character.status == RIGHT_JUMP)character.status = RIGHT_STAY;
				character.playerRect.top = rect.top - CharacterHeight;
				character.playerRect.bottom = rect.top;
				if (character.GravitySpeed >= ChangeToFallingSpeed || character.status == LEFT_FALLING || character.status == RIGHT_FALLING) {
					printf("이건 낙뎀 들어가는 높이다.플레이어의 체력을 감소시키자.\n");
					character.hp -= FallingDamage;
					if (character.hp < 0) character.hp = 0;
					if (character.status == LEFT_FALLING) character.status = LEFT_STANDING;
					else if (character.status == RIGHT_FALLING)character.status = RIGHT_STANDING;
				}
				else {
					if (character.status == LEFT_JUMP) character.status = LEFT_STAY;
					else if (character.status == RIGHT_JUMP)character.status = RIGHT_STAY;
				}
				character.GravitySpeed = CharacterGravitySpeed;
				checkDrop = true;
			}
		}
		if (!checkDrop) {
			character.GravitySpeed += CharacterGravitySpeed * GravityAcceleration;
			if (character.playerRect.bottom > mHeight) {
				character.hp = 0;
				printf("나중엔 죽는거로 처리\n");
				//character.playerRect.top = 0;
				//character.playerRect.bottom = CharacterHeight;
			}
		}
		break;
	case eScreen::map2:
		//TODO: map2로 수정
		for (auto rect : map2Stair1Rect) {
			RECT nullRect;
			if (IntersectRect(&nullRect, &rect, &character.playerRect)) {
				if (character.GravitySpeed <= 0) {
					character.playerRect.top = rect.bottom;
					character.playerRect.bottom = character.playerRect.top + CharacterHeight;
					character.GravitySpeed = CharacterGravitySpeed;
					break;
				}
				if (character.status == LEFT_JUMP)character.status = LEFT_STAY;
				else if (character.status == RIGHT_JUMP)character.status = RIGHT_STAY;
				character.playerRect.top = rect.top - CharacterHeight;
				character.playerRect.bottom = rect.top;
				if (character.GravitySpeed >= ChangeToFallingSpeed || character.status == LEFT_FALLING || character.status == RIGHT_FALLING) {
					printf("이건 낙뎀 들어가는 높이다.플레이어의 체력을 감소시키자.\n");
					character.hp -= FallingDamage;
					if (character.hp < 0) character.hp = 0;
					if (character.status == LEFT_FALLING) character.status = LEFT_STANDING;
					else if (character.status == RIGHT_FALLING)character.status = RIGHT_STANDING;
				}
				else {
					if (character.status == LEFT_JUMP) character.status = LEFT_STAY;
					else if (character.status == RIGHT_JUMP)character.status = RIGHT_STAY;
				}
				character.GravitySpeed = CharacterGravitySpeed;
				checkDrop = true;
				break;
			}
		}
		if (!checkDrop)for (auto rect : map2Stair2Rect) {
			RECT nullRect;
			if (IntersectRect(&nullRect, &rect, &character.playerRect)) {
				if (character.GravitySpeed <= 0) {
					character.playerRect.top = rect.bottom;
					character.playerRect.bottom = character.playerRect.top + CharacterHeight;
					character.GravitySpeed = CharacterGravitySpeed;
					break;
				}
				if (character.status == LEFT_JUMP)character.status = LEFT_STAY;
				else if (character.status == RIGHT_JUMP)character.status = RIGHT_STAY;
				character.playerRect.top = rect.top - CharacterHeight;
				character.playerRect.bottom = rect.top;
				if (character.GravitySpeed >= ChangeToFallingSpeed || character.status == LEFT_FALLING || character.status == RIGHT_FALLING) {
					printf("이건 낙뎀 들어가는 높이다.플레이어의 체력을 감소시키자.\n");
					character.hp -= FallingDamage;
					if (character.hp < 0) character.hp = 0;
					if (character.status == LEFT_FALLING) character.status = LEFT_STANDING;
					else if (character.status == RIGHT_FALLING)character.status = RIGHT_STANDING;
				}
				else {
					if (character.status == LEFT_JUMP) character.status = LEFT_STAY;
					else if (character.status == RIGHT_JUMP)character.status = RIGHT_STAY;
				}
				character.GravitySpeed = CharacterGravitySpeed;
				checkDrop = true;
				break;
			}
		}
		if (!checkDrop)for (auto rect : map2Stair3Rect) {
			RECT nullRect;
			if (IntersectRect(&nullRect, &rect, &character.playerRect)) {
				if (character.GravitySpeed <= 0) {
					character.playerRect.top = rect.bottom;
					character.playerRect.bottom = character.playerRect.top + CharacterHeight;
					character.GravitySpeed = CharacterGravitySpeed;
					break;
				}
				if (character.status == LEFT_JUMP)character.status = LEFT_STAY;
				else if (character.status == RIGHT_JUMP)character.status = RIGHT_STAY;
				character.playerRect.top = rect.top - CharacterHeight;
				character.playerRect.bottom = rect.top;
				if (character.GravitySpeed >= ChangeToFallingSpeed || character.status == LEFT_FALLING || character.status == RIGHT_FALLING) {
					printf("이건 낙뎀 들어가는 높이다.플레이어의 체력을 감소시키자.\n");
					character.hp -= FallingDamage;
					if (character.hp < 0) character.hp = 0;
					if (character.status == LEFT_FALLING) character.status = LEFT_STANDING;
					else if (character.status == RIGHT_FALLING)character.status = RIGHT_STANDING;
				}
				else {
					if (character.status == LEFT_JUMP) character.status = LEFT_STAY;
					else if (character.status == RIGHT_JUMP)character.status = RIGHT_STAY;
				}
				character.GravitySpeed = CharacterGravitySpeed;
				checkDrop = true;
				break;
			}
		}
		if (!checkDrop)for (auto rect : map2GroundRect) {
			RECT nullRect;
			if (IntersectRect(&nullRect, &rect, /*&playerFootRect*/&character.playerRect)) {
				if (character.GravitySpeed <= 0) {
					character.playerRect.top = rect.bottom;
					character.playerRect.bottom = character.playerRect.top + CharacterHeight;
					character.GravitySpeed = CharacterGravitySpeed;
					break;
				}
				if (character.status == LEFT_JUMP)character.status = LEFT_STAY;
				else if (character.status == RIGHT_JUMP)character.status = RIGHT_STAY;
				character.playerRect.top = rect.top - CharacterHeight;
				character.playerRect.bottom = rect.top;
				if (character.GravitySpeed >= ChangeToFallingSpeed || character.status == LEFT_FALLING || character.status == RIGHT_FALLING) {
					printf("이건 낙뎀 들어가는 높이다.플레이어의 체력을 감소시키자.\n");
					character.hp -= FallingDamage;
					if (character.hp < 0) character.hp = 0;
					if (character.status == LEFT_FALLING) character.status = LEFT_STANDING;
					else if (character.status == RIGHT_FALLING)character.status = RIGHT_STANDING;
				}
				else {
					if (character.status == LEFT_JUMP) character.status = LEFT_STAY;
					else if (character.status == RIGHT_JUMP)character.status = RIGHT_STAY;
				}
				character.GravitySpeed = CharacterGravitySpeed;
				checkDrop = true;
			}
		}
		if (!checkDrop)for (auto rect : map2BalconyRect) {
			RECT nullRect;
			if (IntersectRect(&nullRect, &rect, &playerFootRect)) {
				if (character.GravitySpeed <= 0) {
					character.playerRect.top = rect.bottom;
					character.playerRect.bottom = character.playerRect.top + CharacterHeight;
					character.GravitySpeed = CharacterGravitySpeed;
					break;
				}
				if (character.downJump) {
					character.playerRect.top = rect.bottom;
					character.playerRect.bottom = character.playerRect.top + CharacterHeight;
					character.GravitySpeed = CharacterGravitySpeed;
					checkDrop = true;
					character.downJump = false;
					break;
				}
				if (character.status == LEFT_JUMP)character.status = LEFT_STAY;
				else if (character.status == RIGHT_JUMP)character.status = RIGHT_STAY;
				character.playerRect.top = rect.top - CharacterHeight;
				character.playerRect.bottom = rect.top;
				if (character.GravitySpeed >= ChangeToFallingSpeed) {
					printf("이건 낙뎀 들어가는 높이다.플레이어의 체력을 감소시키자.\n");
					character.hp -= FallingDamage;
					if (character.hp < 0) character.hp = 0;
					if (character.status == LEFT_FALLING) character.status = LEFT_STANDING;
					else if (character.status == RIGHT_FALLING)character.status = RIGHT_STANDING;
				}
				else {
					if (character.status == LEFT_JUMP) character.status = LEFT_STAY;
					else if (character.status == RIGHT_JUMP)character.status = RIGHT_STAY;
				}
				character.GravitySpeed = CharacterGravitySpeed;
				checkDrop = true;
			}
		}
		if (!checkDrop) {
			character.GravitySpeed += CharacterGravitySpeed * GravityAcceleration;
			if (character.playerRect.bottom > mHeight) {
				character.hp = 0;
				//character.playerRect.top = 0;
				//character.playerRect.bottom = CharacterHeight;
			}
		}
		break;
	}

}

void Chk_Portal(Character& character) {
	RECT nullRect;

	switch (screen) {
	case eScreen::map1:
		if (IntersectRect(&nullRect, &character.playerRect, &map1UpPortalRect[0])) {
			character.playerRect = { map1DownPortalRect[0].right,map1DownPortalRect[0].top,map1DownPortalRect[0].right + CharacterWidth,map1DownPortalRect[0].top + CharacterHeight };
			character.usedPortal = TRUE;
			if (character.status == LEFT_ROLL) character.status = RIGHT_ROLL;
		}
		else if (IntersectRect(&nullRect, &character.playerRect, &map1UpPortalRect[1])) {
			character.playerRect = { map1DownPortalRect[1].left - CharacterWidth,map1DownPortalRect[1].top,map1DownPortalRect[1].left,map1DownPortalRect[1].top + CharacterHeight };
			character.usedPortal = TRUE;
			if (character.status == RIGHT_ROLL) character.status = LEFT_ROLL;
		}
		else if (IntersectRect(&nullRect, &character.playerRect, &map1DownPortalRect[0])) {
			character.playerRect = { map1UpPortalRect[0].right,map1UpPortalRect[0].top,map1UpPortalRect[0].right + CharacterWidth,map1UpPortalRect[0].top + CharacterHeight };
			character.usedPortal = TRUE;

			if (character.status == LEFT_ROLL)character.status = RIGHT_ROLL;
		}
		else if (IntersectRect(&nullRect, &character.playerRect, &map1DownPortalRect[1])) {
			character.playerRect = { map1UpPortalRect[1].left - CharacterWidth,map1UpPortalRect[1].top,map1UpPortalRect[1].left,map1UpPortalRect[1].top + CharacterHeight };
			character.usedPortal = TRUE;
			if (character.status == RIGHT_ROLL) character.status = LEFT_ROLL;
		}
		break;
	case eScreen::map2:
		if (IntersectRect(&nullRect, &character.playerRect, &map2LeftPortalRect[0])) {
			character.playerRect = { map2LeftPortalRect[1].right,map2LeftPortalRect[1].top,map2LeftPortalRect[1].right + CharacterWidth,map2LeftPortalRect[1].top + CharacterHeight };
			character.usedPortal = TRUE;
			if (character.status == LEFT_ROLL) character.status = RIGHT_ROLL;
		}
		else if (IntersectRect(&nullRect, &character.playerRect, &map2LeftPortalRect[1])) {
			character.playerRect = { map2LeftPortalRect[0].right,map2LeftPortalRect[0].top,map2LeftPortalRect[0].right + CharacterWidth,map2LeftPortalRect[0].top + CharacterHeight };
			character.usedPortal = TRUE;
			if (character.status == LEFT_ROLL)character.status = RIGHT_ROLL;
		}
		break;
	}

}

void CheckUpDownButton_1() {
	if (character1.status == LEFT_SHOCK || character1.status == RIGHT_SHOCK) return;    //new
	else if (character1.status == LEFT_PUNCH || character1.status == RIGHT_PUNCH)return;
	if (!(character1.status == LEFT_AIMING || character1.status == RIGHT_AIMING)) {
		if (bUp) {
			RECT nullRect;
			switch (screen) {
			case eScreen::map1:
				if (IntersectRect(&nullRect, &map1LadderRect, &character1.playerRect)) {
					character1.status = LADDER_UP;
					if (IntersectRect(&nullRect, &map1GroundRect[2], &character1.playerRect)) {
						character1.playerRect.left = map1GroundRect[2].right;
						character1.playerRect.right = character1.playerRect.left + CharacterWidth;
					}
				}
				else {
					if (character1.status == LEFT_ROLL || character1.status == RIGHT_ROLL);
					else if (character1.status == LEFT_WALK || character1.status == LEFT_RUN) {
						character1.status = LEFT_JUMP;
					}
					else if (character1.status == RIGHT_WALK || character1.status == RIGHT_RUN) {
						character1.status = RIGHT_JUMP;
					}
					else if (character1.status == LADDER_UP || character1.status == LADDER_DOWN) {
						character1.status = RIGHT_STAY;
					}
				}

				if (character1.status == LADDER_UP) {
					//printf("사다리 타려고 하는중\n");
					character1.playerRect.top -= character1.speed;
					character1.playerRect.bottom -= character1.speed;
					character1.GravitySpeed = CharacterGravitySpeed - 1;
					if (character1.playerRect.bottom <= map1LadderRect.top) {
						character1.playerRect.bottom = map1LadderRect.top + 1;
						character1.playerRect.top = character1.playerRect.bottom - CharacterHeight;
					}
					RECT nullRect;
					if (IntersectRect(&nullRect, &map1BalconyRect[0], &character1.playerRect)) {
						character1.playerRect.top = map1BalconyRect[0].bottom;
						character1.playerRect.bottom = character1.playerRect.top + CharacterHeight;
					}
				}
				else {
					if (character1.GravitySpeed == CharacterGravitySpeed && (character1.status != LEFT_ROLL && character1.status != RIGHT_ROLL)) {
						if (character1.status != LEFT_STANDING && character1.status != RIGHT_STANDING)
							character1.GravitySpeed = CharacterGravitySpeed * -5 + 1;
						if (character1.status == LEFT_STAY || character1.status == LEFT_WALK || character1.status == LEFT_RUN) character1.status = LEFT_JUMP;
						else if (character1.status == RIGHT_STAY || character1.status == RIGHT_WALK || character1.status == RIGHT_RUN) character1.status = RIGHT_JUMP;
					}
					//printf("점프하려는중\n");
				}

				break;
			case eScreen::map2:
				if (IntersectRect(&nullRect, &map2LadderRect[0], &character1.playerRect)) {
					character1.status = LADDER_UP;
				}
				else if (IntersectRect(&nullRect, &map2LadderRect[1], &character1.playerRect)) {
					character1.status = LADDER_UP;
					if (IntersectRect(&nullRect, &map2GroundRect[16], &character1.playerRect)) {
						character1.playerRect.right = map2GroundRect[16].left;
						character1.playerRect.left = character1.playerRect.right - CharacterWidth;
					}
				}
				else {
					if (character1.status == LEFT_ROLL || character1.status == RIGHT_ROLL);
					else if (character1.status == LEFT_FALLING || character1.status == RIGHT_FALLING);
					else if (character1.status == LEFT_SHOCK || character1.status == RIGHT_SHOCK); //new
					else if (character1.status == LEFT_PUNCH || character1.status == RIGHT_PUNCH);
					else if (character1.status == LEFT_WALK || character1.status == LEFT_RUN) {
						character1.status = LEFT_JUMP;
					}
					else if (character1.status == RIGHT_WALK || character1.status == RIGHT_RUN) {
						character1.status = RIGHT_JUMP;
					}
					else if (character1.status == LADDER_UP || character1.status == LADDER_DOWN) {
						character1.status = RIGHT_STAY;
					}
				}

				if (character1.status == LADDER_UP) {
					//printf("사다리 타려고 하는중\n");
					if (IntersectRect(&nullRect, &map2LadderRect[0], &character1.playerRect) && character1.playerRect.bottom <= map2LadderRect[0].top) {
						character1.playerRect.bottom = map2LadderRect[0].top;
						character1.playerRect.top = character1.playerRect.bottom - CharacterHeight;
					}
					else if (IntersectRect(&nullRect, &map2LadderRect[1], &character1.playerRect) && character1.playerRect.bottom - character1.speed <= map2LadderRect[1].top) {
						character1.playerRect.bottom = map2LadderRect[1].top + 1;
						character1.playerRect.top = character1.playerRect.bottom - CharacterHeight;
					}
					else {
						character1.playerRect.top -= character1.speed;
						character1.playerRect.bottom -= character1.speed;
					}
					character1.GravitySpeed = CharacterGravitySpeed - 1;
					/*RECT nullRect;
					if (IntersectRect(&nullRect, &map1BalconyRect[0], &character1.playerRect)) {
						character1.playerRect.top = map1BalconyRect[0].bottom;
						character1.playerRect.bottom = character1.playerRect.top + CharacterHeight;
					}*/
				}
				else {
					if (character1.GravitySpeed == CharacterGravitySpeed && (character1.status != LEFT_ROLL && character1.status != RIGHT_ROLL)) {
						if (character1.status != LEFT_STANDING && character1.status != RIGHT_STANDING)
							character1.GravitySpeed = CharacterGravitySpeed * -5 + 1;
						if (character1.status == LEFT_STAY || character1.status == LEFT_WALK || character1.status == LEFT_RUN) character1.status = LEFT_JUMP;
						else if (character1.status == RIGHT_STAY || character1.status == RIGHT_WALK || character1.status == RIGHT_RUN) character1.status = RIGHT_JUMP;
					}
					//printf("점프하려는중\n");
				}
				break;
			}
		}

		if (bDown) {
			RECT nullRect;
			switch (screen) {
			case eScreen::map1:
				if (IntersectRect(&nullRect, &map1LadderRect, &character1.playerRect)) {
					if (character1.status != RIGHT_ROLL && character1.status != LEFT_ROLL) {
						character1.status = LADDER_DOWN;
						character1.playerRect.left = map1LadderRect.left - CharacterWidth / 2 - 5;
						character1.playerRect.right = character1.playerRect.left + CharacterWidth;
					}


					if (IntersectRect(&nullRect, &map1GroundRect[2], &character1.playerRect)) {
						character1.playerRect.left = map1GroundRect[2].right;
						character1.playerRect.right = character1.playerRect.left + CharacterWidth;
					}
					//   printf("둘이 접촉함\n");
				}
				else {
					//몸 숙여서 크기 줄이기  앉기 크기 처리 앉기처리 앉기 앉아서 크기 작아짐 크기작아짐
					if (character1.status != RIGHT_RUN && character1.status != LEFT_RUN && character1.status != LEFT_JUMP && character1.status != RIGHT_JUMP && character1.status != LADDER_DOWN && character1.status != LADDER_UP && !(character1.GravitySpeed >= CharacterGravitySpeed * 3) && character1.status != RIGHT_STANDING && character1.status != LEFT_STANDING && character1.status != LEFT_FALLING && character1.status != RIGHT_FALLING)
						character1.playerRect.top = character1.playerRect.bottom - CharacterHeight / 3 * 2;
					if (character1.status == LEFT_WALK) {
						character1.status = LEFT_ROLL;
					}
					else if (character1.status == RIGHT_WALK) {
						character1.status = RIGHT_ROLL;
					}
					else if (character1.status == LEFT_STAY) {
						character1.status = LEFT_SIT;
						//TODO: 크기 조정 할것
						//character1.playerRect.top = character1.playerRect.bottom
					}
					else if (character1.status == RIGHT_STAY) {
						character1.status = RIGHT_SIT;
					}
				}

				if (character1.status == LADDER_DOWN) {
					//   printf("사다리 내려가려고 하는중\n");
					character1.playerRect.top += character1.speed;
					character1.playerRect.bottom += character1.speed;
					character1.GravitySpeed = CharacterGravitySpeed - 1;
					RECT nullRect;
					if (!IntersectRect(&nullRect, &map1LadderRect, &character1.playerRect)) {
						character1.status = RIGHT_STAY;
					}
				}
				break;
			case eScreen::map2:
				if (IntersectRect(&nullRect, &map2LadderRect[0], &character1.playerRect)) {
					if (character1.status != RIGHT_ROLL && character1.status != LEFT_ROLL) {
						character1.status = LADDER_DOWN;
						character1.playerRect.left = map2LadderRect[0].left - CharacterWidth / 2 + 5;
						character1.playerRect.right = character1.playerRect.left + CharacterWidth;
					}
					/*if (IntersectRect(&nullRect, &map2BalconyRect[0], &character1.playerRect)) {
						character1.playerRect.left = map2BalconyRect[0].right;
						character1.playerRect.right = character1.playerRect.left + CharacterWidth;
					}
					else if (IntersectRect(&nullRect, &map2BalconyRect[1], &character1.playerRect)) {
						character1.playerRect.right = map2BalconyRect[1].left;
						character1.playerRect.left = character1.playerRect.right - CharacterWidth;
					}*/

				}
				else if (IntersectRect(&nullRect, &map2LadderRect[1], &character1.playerRect)) {
					if (character1.status != RIGHT_ROLL && character1.status != LEFT_ROLL) {
						character1.status = LADDER_DOWN;
						character1.playerRect.left = map2LadderRect[1].left - CharacterWidth / 2 - 5;
						character1.playerRect.right = character1.playerRect.left + CharacterWidth;
					}

					if (IntersectRect(&nullRect, &map2GroundRect[16], &character1.playerRect)) {
						character1.playerRect.right = map2GroundRect[16].left;
						character1.playerRect.left = character1.playerRect.right + CharacterWidth;
					}
				}
				else {
					//몸 숙여서 크기 줄이기  앉기 크기 처리 앉기처리 앉기 앉아서 크기 작아짐 크기작아짐
					if (character1.status != RIGHT_RUN && character1.status != LEFT_RUN && character1.status != LEFT_JUMP && character1.status != RIGHT_JUMP && character1.status != LADDER_DOWN && character1.status != LADDER_UP && !(character1.GravitySpeed >= CharacterGravitySpeed * 3) && character1.status != RIGHT_STANDING && character1.status != LEFT_STANDING)
						character1.playerRect.top = character1.playerRect.bottom - CharacterHeight / 3 * 2;
					if (character1.status == LEFT_WALK) {
						character1.status = LEFT_ROLL;
					}
					else if (character1.status == RIGHT_WALK) {
						character1.status = RIGHT_ROLL;
					}
					else if (character1.status == LEFT_STAY) {
						character1.status = LEFT_SIT;
						//TODO: 크기 조정 할것
						//character1.playerRect.top = character1.playerRect.bottom
					}
					else if (character1.status == RIGHT_STAY) {
						character1.status = RIGHT_SIT;
					}
				}

				if (character1.status == LADDER_DOWN) {
					//   printf("사다리 내려가려고 하는중\n");
					character1.playerRect.top += character1.speed;
					character1.playerRect.bottom += character1.speed;
					character1.GravitySpeed = CharacterGravitySpeed - 1;
					RECT nullRect;
					if (!IntersectRect(&nullRect, &map2LadderRect[0], &character1.playerRect) && !IntersectRect(&nullRect, &map2LadderRect[1], &character1.playerRect)) {
						character1.status = RIGHT_STAY;
					}
				}
				break;
			}


		}
	}
}

void CheckUpDownButton_2() {
	if (character2.status == LEFT_SHOCK || character2.status == RIGHT_SHOCK) return;    //new
	else if (character2.status == LEFT_PUNCH || character2.status == RIGHT_PUNCH)return;

	if (!(character2.status == LEFT_AIMING || character2.status == RIGHT_AIMING)) {
		if (bW) {
			RECT nullRect;
			switch (screen) {
			case eScreen::map1:
				if (IntersectRect(&nullRect, &map1LadderRect, &character2.playerRect)) {
					character2.status = LADDER_UP;
					if (IntersectRect(&nullRect, &map1GroundRect[2], &character2.playerRect)) {
						character2.playerRect.left = map1GroundRect[2].right;
						character2.playerRect.right = character2.playerRect.left + CharacterWidth;
					}
				}
				else {
					if (character2.status == LEFT_ROLL || character2.status == RIGHT_ROLL);
					else if (character2.status == LEFT_FALLING || character2.status == RIGHT_FALLING);//new
					else if (character2.status == LEFT_SHOCK || character2.status == RIGHT_SHOCK);
					else if (character2.status == LEFT_PUNCH || character2.status == RIGHT_PUNCH);
					else if (character2.status == LEFT_WALK || character2.status == LEFT_RUN) {
						character2.status = LEFT_JUMP;
					}
					else if (character2.status == RIGHT_WALK || character2.status == RIGHT_RUN) {
						character2.status = RIGHT_JUMP;
					}
					else if (character2.status == LADDER_UP || character2.status == LADDER_DOWN) {
						character2.status = RIGHT_STAY;
					}
				}

				if (character2.status == LADDER_UP) {
					//printf("사다리 타려고 하는중\n");
					character2.playerRect.top -= character2.speed;
					character2.playerRect.bottom -= character2.speed;
					character2.GravitySpeed = CharacterGravitySpeed - 1;
					if (character2.playerRect.bottom <= map1LadderRect.top) {
						character2.playerRect.bottom = map1LadderRect.top + 1;
						character2.playerRect.top = character2.playerRect.bottom - CharacterHeight;
					}
					RECT nullRect;
					if (IntersectRect(&nullRect, &map1BalconyRect[0], &character2.playerRect)) {
						character2.playerRect.top = map1BalconyRect[0].bottom;
						character2.playerRect.bottom = character2.playerRect.top + CharacterHeight;
					}
				}
				else {
					if (character2.GravitySpeed == CharacterGravitySpeed && (character2.status != LEFT_ROLL && character2.status != RIGHT_ROLL)) {
						if (character2.status != LEFT_STANDING && character2.status != RIGHT_STANDING)
							character2.GravitySpeed = CharacterGravitySpeed * -5 + 1;
						if (character2.status == LEFT_STAY || character2.status == LEFT_WALK || character2.status == LEFT_RUN) character2.status = LEFT_JUMP;
						else if (character2.status == RIGHT_STAY || character2.status == RIGHT_WALK || character2.status == RIGHT_RUN) character2.status = RIGHT_JUMP;
					}
					//printf("점프하려는중\n");
				}

				break;
			case eScreen::map2:
				if (IntersectRect(&nullRect, &map2LadderRect[0], &character2.playerRect)) {
					character2.status = LADDER_UP;
				}
				else if (IntersectRect(&nullRect, &map2LadderRect[1], &character2.playerRect)) {
					character2.status = LADDER_UP;
					if (IntersectRect(&nullRect, &map2GroundRect[16], &character2.playerRect)) {
						character2.playerRect.right = map2GroundRect[16].left;
						character2.playerRect.left = character2.playerRect.right - CharacterWidth;
					}
				}
				else {
					if (character2.status == LEFT_ROLL || character2.status == RIGHT_ROLL);
					else if (character2.status == LEFT_FALLING || character2.status == RIGHT_FALLING); //new
					else if (character2.status == LEFT_SHOCK || character2.status == RIGHT_SHOCK);
					else if (character2.status == LEFT_PUNCH || character2.status == RIGHT_PUNCH);
					else if (character2.status == LEFT_WALK || character2.status == LEFT_RUN) {
						character2.status = LEFT_JUMP;
					}
					else if (character2.status == RIGHT_WALK || character2.status == RIGHT_RUN) {
						character2.status = RIGHT_JUMP;
					}
					else if (character2.status == LADDER_UP || character2.status == LADDER_DOWN) {
						character2.status = RIGHT_STAY;
					}
				}

				if (character2.status == LADDER_UP) {
					//printf("사다리 타려고 하는중\n");
					if (IntersectRect(&nullRect, &map2LadderRect[0], &character2.playerRect) && character2.playerRect.bottom <= map2LadderRect[0].top) {
						character2.playerRect.bottom = map2LadderRect[0].top;
						character2.playerRect.top = character2.playerRect.bottom - CharacterHeight;
					}
					else if (IntersectRect(&nullRect, &map2LadderRect[1], &character2.playerRect) && character2.playerRect.bottom - character2.speed <= map2LadderRect[1].top) {
						character2.playerRect.bottom = map2LadderRect[1].top + 1;
						character2.playerRect.top = character2.playerRect.bottom - CharacterHeight;
					}
					else {
						character2.playerRect.top -= character2.speed;
						character2.playerRect.bottom -= character2.speed;
					}
					character2.GravitySpeed = CharacterGravitySpeed - 1;
					/*RECT nullRect;
					if (IntersectRect(&nullRect, &map1BalconyRect[0], &character2.playerRect)) {
						character2.playerRect.top = map1BalconyRect[0].bottom;
						character2.playerRect.bottom = character2.playerRect.top + CharacterHeight;
					}*/
				}
				else {
					if (character2.GravitySpeed == CharacterGravitySpeed && (character2.status != LEFT_ROLL && character2.status != RIGHT_ROLL)) {
						if (character2.status != LEFT_STANDING && character2.status != RIGHT_STANDING)
							character2.GravitySpeed = CharacterGravitySpeed * -5 + 1;
						if (character2.status == LEFT_STAY || character2.status == LEFT_WALK || character2.status == LEFT_RUN) character2.status = LEFT_JUMP;
						else if (character2.status == RIGHT_STAY || character2.status == RIGHT_WALK || character2.status == RIGHT_RUN) character2.status = RIGHT_JUMP;
					}
					//printf("점프하려는중\n");
				}
				break;
			}
		}

		if (bS) {
			RECT nullRect;
			switch (screen) {
			case eScreen::map1:
				if (IntersectRect(&nullRect, &map1LadderRect, &character2.playerRect)) {
					if (character2.status != RIGHT_ROLL && character2.status != LEFT_ROLL) {
						character2.status = LADDER_DOWN;
						character2.playerRect.left = map1LadderRect.left - CharacterWidth / 2 - 5;
						character2.playerRect.right = character2.playerRect.left + CharacterWidth;
					}


					if (IntersectRect(&nullRect, &map1GroundRect[2], &character2.playerRect)) {
						character2.playerRect.left = map1GroundRect[2].right;
						character2.playerRect.right = character2.playerRect.left + CharacterWidth;
					}
					//   printf("둘이 접촉함\n");
				}
				else {
					//몸 숙여서 크기 줄이기  앉기 크기 처리 앉기처리 앉기 앉아서 크기 작아짐 크기작아짐
					if (character2.status != RIGHT_RUN && character2.status != LEFT_RUN && character2.status != LEFT_JUMP && character2.status != RIGHT_JUMP && character2.status != LADDER_DOWN && character2.status != LADDER_UP && !(character2.GravitySpeed >= CharacterGravitySpeed * 3) && character2.status != RIGHT_STANDING && character2.status != LEFT_STANDING)
						character2.playerRect.top = character2.playerRect.bottom - CharacterHeight / 3 * 2;
					if (character2.status == LEFT_WALK) {
						character2.status = LEFT_ROLL;
					}
					else if (character2.status == RIGHT_WALK) {
						character2.status = RIGHT_ROLL;
					}
					else if (character2.status == LEFT_STAY) {
						character2.status = LEFT_SIT;
						//TODO: 크기 조정 할것
						//character2.playerRect.top = character2.playerRect.bottom
					}
					else if (character2.status == RIGHT_STAY) {
						character2.status = RIGHT_SIT;
					}
				}

				if (character2.status == LADDER_DOWN) {
					//   printf("사다리 내려가려고 하는중\n");
					character2.playerRect.top += character2.speed;
					character2.playerRect.bottom += character2.speed;
					character2.GravitySpeed = CharacterGravitySpeed - 1;
					RECT nullRect;
					if (!IntersectRect(&nullRect, &map1LadderRect, &character2.playerRect)) {
						character2.status = RIGHT_STAY;
					}
				}
				break;
			case eScreen::map2:
				if (IntersectRect(&nullRect, &map2LadderRect[0], &character2.playerRect)) {
					if (character2.status != RIGHT_ROLL && character2.status != LEFT_ROLL) {
						character2.status = LADDER_DOWN;
						character2.playerRect.left = map2LadderRect[0].left - CharacterWidth / 2 + 5;
						character2.playerRect.right = character2.playerRect.left + CharacterWidth;
					}
					/*if (IntersectRect(&nullRect, &map2BalconyRect[0], &character2.playerRect)) {
						character2.playerRect.left = map2BalconyRect[0].right;
						character2.playerRect.right = character2.playerRect.left + CharacterWidth;
					}
					else if (IntersectRect(&nullRect, &map2BalconyRect[1], &character2.playerRect)) {
						character2.playerRect.right = map2BalconyRect[1].left;
						character2.playerRect.left = character2.playerRect.right - CharacterWidth;
					}*/

				}
				else if (IntersectRect(&nullRect, &map2LadderRect[1], &character2.playerRect)) {
					if (character2.status != RIGHT_ROLL && character2.status != LEFT_ROLL) {
						character2.status = LADDER_DOWN;
						character2.playerRect.left = map2LadderRect[1].left - CharacterWidth / 2 - 5;
						character2.playerRect.right = character2.playerRect.left + CharacterWidth;
					}

					if (IntersectRect(&nullRect, &map2GroundRect[16], &character2.playerRect)) {
						character2.playerRect.right = map2GroundRect[16].left;
						character2.playerRect.left = character2.playerRect.right + CharacterWidth;
					}
				}
				else {
					//몸 숙여서 크기 줄이기  앉기 크기 처리 앉기처리 앉기 앉아서 크기 작아짐 크기작아짐
					if (character2.status != RIGHT_RUN && character2.status != LEFT_RUN && character2.status != LEFT_JUMP && character2.status != RIGHT_JUMP && character2.status != LADDER_DOWN && character2.status != LADDER_UP && !(character2.GravitySpeed >= CharacterGravitySpeed * 3) && character2.status != RIGHT_STANDING && character2.status != LEFT_STANDING)
						character2.playerRect.top = character2.playerRect.bottom - CharacterHeight / 3 * 2;
					if (character2.status == LEFT_WALK) {
						character2.status = LEFT_ROLL;
					}
					else if (character2.status == RIGHT_WALK) {
						character2.status = RIGHT_ROLL;
					}
					else if (character2.status == LEFT_STAY) {
						character2.status = LEFT_SIT;
						//TODO: 크기 조정 할것
						//character2.playerRect.top = character2.playerRect.bottom
					}
					else if (character2.status == RIGHT_STAY) {
						character2.status = RIGHT_SIT;
					}
				}

				if (character2.status == LADDER_DOWN) {
					//   printf("사다리 내려가려고 하는중\n");
					character2.playerRect.top += character2.speed;
					character2.playerRect.bottom += character2.speed;
					character2.GravitySpeed = CharacterGravitySpeed - 1;
					RECT nullRect;
					if (!IntersectRect(&nullRect, &map2LadderRect[0], &character2.playerRect) && !IntersectRect(&nullRect, &map2LadderRect[1], &character2.playerRect)) {
						character2.status = RIGHT_STAY;
					}
				}
				break;
			}


		}
	}
}

void CheckLeftRightButton_1() {
	if (character1.status == LEFT_FALLING || character1.status == RIGHT_FALLING) return;
	else if (character1.status == LEFT_SHOCK || character1.status == RIGHT_SHOCK)return; //new
	else if (character1.status == LEFT_PUNCH || character1.status == RIGHT_PUNCH)return;
	if (character1.status == LEFT_AIMING || character1.status == RIGHT_AIMING) {
		if (!character1.Shoot) {
			if (bLeft) {
				character1.radian = 180;
				character1.status = LEFT_AIMING;
			}
			else if (bRight) {
				character1.radian = 720;
				character1.status = RIGHT_AIMING;
			}
		}

		if (bUp) {
			if (character1.radian >= 90 && character1.radian < 270) {
				character1.radian += 5;
			}
			else if (character1.radian <= 810 && character1.radian > 630) {
				character1.radian -= 5;
			}
		}
		else if (bDown) {
			if (character1.radian <= 270 && character1.radian > 90) {
				character1.radian -= 5;
			}
			else if (character1.radian >= 630 && character1.radian < 810) {
				character1.radian += 5;
			}
		}
	}
	else {
		if (character1.usedPortal) {
			if (((bRight && character1.status != RIGHT_ROLL) || character1.status == LEFT_ROLL) && character1.status != LEFT_STANDING && character1.status != RIGHT_STANDING) {
				switch (screen) {
				case eScreen::map1:
				{
					RECT nullRect;
					if (!IntersectRect(&nullRect, &map1LadderRect, &character1.playerRect)) {
						if (character1.status == LEFT_ROLL);
						else if (character1.status == RIGHT_JUMP)
							character1.status = LEFT_JUMP;
						else if (character1.powerUp && character1.status != LEFT_JUMP && character1.status != RIGHT_JUMP)
							character1.status = LEFT_RUN;
						else if (character1.status == LEFT_SIT || character1.status == RIGHT_SIT)
							character1.status = LEFT_ROLL;
						else if (character1.status != LEFT_RUN && character1.status != LEFT_JUMP && character1.status != RIGHT_JUMP)
							character1.status = LEFT_WALK;
						else if (character1.status == LADDER_UP || character1.status == LADDER_DOWN) {
							character1.status = LEFT_STAY;
						}

					}
					character1.playerRect.left -= character1.speed;
					character1.playerRect.right -= character1.speed;
					if (character1.powerUp) {         //스프린트(+character1.speed * character1.powerUp 으로 처리할지 고민
						character1.playerRect.left -= character1.speed;
						character1.playerRect.right -= character1.speed;
					}
					for (auto rect : map1GroundRect) {
						RECT nullRect;
						if (IntersectRect(&nullRect, &rect, &character1.playerRect)) {
							character1.playerRect.left = rect.right;
							character1.playerRect.right = rect.right + CharacterWidth;
						}
					}
					for (auto rect : map1BalconyRect) {
						RECT nullRect;
						if (IntersectRect(&nullRect, &rect, &character1.playerRect)) {
							character1.playerRect.left = rect.right;
							character1.playerRect.right = rect.right + CharacterWidth;
						}
					}
				}
				break;
				case eScreen::map2:
				{
					RECT nullRect;
					if (!IntersectRect(&nullRect, &map2LadderRect[0], &character1.playerRect) && !IntersectRect(&nullRect, &map2LadderRect[1], &character1.playerRect)) {
						if (character1.status == LEFT_ROLL);
						else if (character1.status == RIGHT_JUMP)
							character1.status = LEFT_JUMP;
						else if (character1.powerUp && character1.status != LEFT_JUMP && character1.status != RIGHT_JUMP)
							character1.status = LEFT_RUN;
						else if (character1.status == LEFT_SIT || character1.status == RIGHT_SIT)
							character1.status = LEFT_ROLL;
						else if (character1.status != LEFT_RUN && character1.status != LEFT_JUMP && character1.status != RIGHT_JUMP)
							character1.status = LEFT_WALK;
						else if (character1.status == LADDER_UP || character1.status == LADDER_DOWN) {
							character1.status = LEFT_STAY;
						}

					}
					character1.playerRect.left -= character1.speed;
					character1.playerRect.right -= character1.speed;
					if (character1.powerUp) {         //스프린트(+character1.speed * character1.powerUp 으로 처리할지 고민
						character1.playerRect.left -= character1.speed;
						character1.playerRect.right -= character1.speed;
					}
					for (auto rect : map2GroundRect) {
						RECT nullRect;
						if (rect.left == map2GroundRect[16].left && rect.top == map2GroundRect[16].top) continue;
						if (IntersectRect(&nullRect, &rect, &character1.playerRect)) {
							character1.playerRect.left = rect.right;
							character1.playerRect.right = rect.right + CharacterWidth;
						}
					}
					/*for (auto rect : map2BalconyRect) {
						RECT nullRect;
						if (IntersectRect(&nullRect, &rect, &character1.playerRect)) {
							character1.playerRect.left = rect.right;
							character1.playerRect.right = rect.right + CharacterWidth;
						}
					}*/
				}
				break;
				}

				character1.radian = 180;
			}


			if (((bLeft && character1.status != LEFT_ROLL) || character1.status == RIGHT_ROLL) && character1.status != LEFT_STANDING && character1.status != RIGHT_STANDING) {
				{//사다리 확인
					RECT nullRect;
					switch (screen) {
					case eScreen::map1:
						if (!IntersectRect(&nullRect, &map1LadderRect, &character1.playerRect)) {
							if (character1.status == RIGHT_ROLL);
							else if (character1.status == LEFT_JUMP)
								character1.status = RIGHT_JUMP;
							else if (character1.powerUp && character1.status != RIGHT_JUMP && character1.status != LEFT_JUMP) {
								character1.status = RIGHT_RUN;
							}

							else if (character1.status == LEFT_SIT || character1.status == RIGHT_SIT)
								character1.status = RIGHT_ROLL;
							else if (character1.status != RIGHT_RUN && character1.status != RIGHT_JUMP && character1.status != LEFT_JUMP)
								character1.status = RIGHT_WALK;
							else if (character1.status == LADDER_UP || character1.status == LADDER_DOWN) {
								character1.status = RIGHT_STAY;
							}
						}
						character1.playerRect.left += character1.speed;
						character1.playerRect.right += character1.speed;
						if (character1.powerUp) {         //스프린트(+character1.speed * character1.powerUp 으로 처리할지 고민
							character1.playerRect.left += character1.speed;
							character1.playerRect.right += character1.speed;
						}
						for (auto rect : map1GroundRect) {
							RECT nullRect;
							if (IntersectRect(&nullRect, &rect, &character1.playerRect)) {
								character1.playerRect.right = rect.left;
								character1.playerRect.left = rect.left - CharacterWidth;
							}
						}
						/*for (auto rect : map1BalconyRect) {
						   RECT nullRect;
						   if (IntersectRect(&nullRect, &rect, &character1.playerRect)) {
							 character1.playerRect.right = rect.left;
							 character1.playerRect.left = rect.left - CharacterWidth;
						   }
						}*/
						{
							RECT nullRect;
							if (IntersectRect(&nullRect, &map1BalconyRect[0], &character1.playerRect)) {
								character1.playerRect.right = map1BalconyRect[0].left;
								character1.playerRect.left = map1BalconyRect[0].left - CharacterWidth;
							}
						}
						break;
					case eScreen::map2:
						if (!IntersectRect(&nullRect, &map2LadderRect[0], &character1.playerRect) && !IntersectRect(&nullRect, &map2LadderRect[1], &character1.playerRect)) {
							if (character1.status == RIGHT_ROLL);
							else if (character1.status == LEFT_JUMP)
								character1.status = RIGHT_JUMP;
							else if (character1.powerUp && character1.status != RIGHT_JUMP && character1.status != LEFT_JUMP) {
								character1.status = RIGHT_RUN;
							}

							else if (character1.status == LEFT_SIT || character1.status == RIGHT_SIT)
								character1.status = RIGHT_ROLL;
							else if (character1.status != RIGHT_RUN && character1.status != RIGHT_JUMP && character1.status != LEFT_JUMP)
								character1.status = RIGHT_WALK;
							else if (character1.status == LADDER_UP || character1.status == LADDER_DOWN) {
								character1.status = RIGHT_STAY;
							}
						}
						character1.playerRect.left += character1.speed;
						character1.playerRect.right += character1.speed;
						if (character1.powerUp) {         //스프린트(+character1.speed * character1.powerUp 으로 처리할지 고민
							character1.playerRect.left += character1.speed;
							character1.playerRect.right += character1.speed;
						}
						for (auto rect : map2GroundRect) {
							RECT nullRect;
							if (IntersectRect(&nullRect, &rect, &character1.playerRect)) {
								character1.playerRect.right = rect.left;
								character1.playerRect.left = rect.left - CharacterWidth;
							}
						}
						/*for (auto rect : map2BalconyRect) {
						   RECT nullRect;
						   if (IntersectRect(&nullRect, &rect, &character1.playerRect)) {
							 character1.playerRect.right = rect.left;
							 character1.playerRect.left = rect.left - CharacterWidth;
						   }
						}*/
						break;
					}
				}



				character1.radian = 720;
			}
		}
		else {
			if (((bLeft && character1.status != RIGHT_ROLL) || character1.status == LEFT_ROLL) && character1.status != LEFT_STANDING && character1.status != RIGHT_STANDING) {
				switch (screen) {
				case eScreen::map1:
				{
					RECT nullRect;
					if (!IntersectRect(&nullRect, &map1LadderRect, &character1.playerRect)) {
						if (character1.status == LEFT_ROLL || character1.status == RIGHT_ROLL);
						else if (character1.status == RIGHT_RUN || character1.status == RIGHT_WALK);
						else if (character1.status == RIGHT_JUMP)
							character1.status = LEFT_JUMP;
						else if ((character1.powerUp && character1.status != LEFT_JUMP && character1.status != RIGHT_JUMP) || character1.status == LEFT_RUN) {
							character1.status = LEFT_RUN;
						}

						else if (character1.status == LEFT_SIT || character1.status == RIGHT_SIT)
							character1.status = LEFT_ROLL;
						else if (character1.status != LEFT_RUN && character1.status != LEFT_JUMP && character1.status != RIGHT_JUMP)
							character1.status = LEFT_WALK;
						else if (character1.status == LADDER_UP || character1.status == LADDER_DOWN) {
							character1.status = LEFT_STAY;
						}
					}
					character1.playerRect.left -= character1.speed;
					character1.playerRect.right -= character1.speed;
					if (character1.powerUp) {         //스프린트(+character1.speed * character1.powerUp 으로 처리할지 고민
						character1.playerRect.left -= character1.speed;
						character1.playerRect.right -= character1.speed;
					}
					for (auto rect : map1GroundRect) {
						RECT nullRect;
						if (IntersectRect(&nullRect, &rect, &character1.playerRect)) {
							character1.playerRect.left = rect.right;
							character1.playerRect.right = rect.right + CharacterWidth;
						}
					}
					for (auto rect : map1BalconyRect) {
						RECT nullRect;
						if (IntersectRect(&nullRect, &rect, &character1.playerRect)) {
							character1.playerRect.left = rect.right;
							character1.playerRect.right = rect.right + CharacterWidth;
						}
					}
				}

				break;
				case eScreen::map2:
				{
					RECT nullRect;
					if (!IntersectRect(&nullRect, &map2LadderRect[0], &character1.playerRect) && !IntersectRect(&nullRect, &map2LadderRect[1], &character1.playerRect)) {
						if (character1.status == LEFT_ROLL || character1.status == RIGHT_ROLL);
						else if (character1.status == RIGHT_RUN || character1.status == RIGHT_WALK);
						else if (character1.status == RIGHT_JUMP)
							character1.status = LEFT_JUMP;
						else if ((character1.powerUp && character1.status != LEFT_JUMP && character1.status != RIGHT_JUMP) || character1.status == LEFT_RUN) {
							character1.status = LEFT_RUN;
						}

						else if (character1.status == LEFT_SIT || character1.status == RIGHT_SIT)
							character1.status = LEFT_ROLL;
						else if (character1.status != LEFT_RUN && character1.status != LEFT_JUMP && character1.status != RIGHT_JUMP)
							character1.status = LEFT_WALK;
						else if (character1.status == LADDER_UP || character1.status == LADDER_DOWN) {
							character1.status = LEFT_STAY;
						}
					}
					character1.playerRect.left -= character1.speed;
					character1.playerRect.right -= character1.speed;
					if (character1.powerUp) {         //스프린트(+character1.speed * character1.powerUp 으로 처리할지 고민
						character1.playerRect.left -= character1.speed;
						character1.playerRect.right -= character1.speed;
					}
					for (auto rect : map2GroundRect) {
						if (rect.left == map2GroundRect[16].left && rect.top == map2GroundRect[16].top) continue;
						RECT nullRect;
						if (IntersectRect(&nullRect, &rect, &character1.playerRect)) {
							character1.playerRect.left = rect.right;
							character1.playerRect.right = rect.right + CharacterWidth;
						}
					}
					/*for (auto rect : map2BalconyRect) {
						RECT nullRect;
						if (IntersectRect(&nullRect, &rect, &character1.playerRect)) {
							character1.playerRect.left = rect.right;
							character1.playerRect.right = rect.right + CharacterWidth;
						}
					}*/
				}
				break;
				}


				character1.radian = 180;
			}

			if (((bRight && character1.status != LEFT_ROLL) || character1.status == RIGHT_ROLL) && character1.status != LEFT_STANDING && character1.status != RIGHT_STANDING) {

				{
					RECT nullRect;
					switch (screen) {
					case eScreen::map1:
						if (!IntersectRect(&nullRect, &map1LadderRect, &character1.playerRect)) {
							if (character1.status == RIGHT_ROLL || character1.status == LEFT_ROLL);
							else if (character1.status == LEFT_RUN || character1.status == LEFT_WALK);
							else if (character1.status == LEFT_JUMP) {
								character1.status = RIGHT_JUMP;
							}

							else if ((character1.powerUp && character1.status != RIGHT_JUMP && character1.status != LEFT_JUMP) || character1.status == RIGHT_RUN) {
								character1.status = RIGHT_RUN;
							}

							else if (character1.status == LEFT_SIT || character1.status == RIGHT_SIT)
								character1.status = RIGHT_ROLL;
							else if (character1.status != RIGHT_RUN && character1.status != RIGHT_JUMP && character1.status != LEFT_JUMP)
								character1.status = RIGHT_WALK;
							else if (character1.status == LADDER_UP || character1.status == LADDER_DOWN) {
								character1.status = RIGHT_STAY;
							}

						}
						character1.playerRect.left += character1.speed;
						character1.playerRect.right += character1.speed;
						if (character1.powerUp) {         //스프린트(+character1.speed * character1.powerUp 으로 처리할지 고민
							character1.playerRect.left += character1.speed;
							character1.playerRect.right += character1.speed;
						}
						for (auto rect : map1GroundRect) {
							RECT nullRect;
							if (IntersectRect(&nullRect, &rect, &character1.playerRect)) {
								character1.playerRect.right = rect.left;
								character1.playerRect.left = rect.left - CharacterWidth;
							}
						}
						/*   for (auto rect : map1BalconyRect) {
							 RECT nullRect;
							 if (IntersectRect(&nullRect, &rect, &character1.playerRect)) {
							   character1.playerRect.right = rect.left;
							   character1.playerRect.left = rect.left - CharacterWidth;
							 }
						   }*/
						{
							RECT nullRect;
							if (IntersectRect(&nullRect, &map1BalconyRect[0], &character1.playerRect)) {
								character1.playerRect.right = map1BalconyRect[0].left;
								character1.playerRect.left = map1BalconyRect[0].left - CharacterWidth;
							}
						}
						break;
					case eScreen::map2:
						if (!IntersectRect(&nullRect, &map2LadderRect[0], &character1.playerRect) && !IntersectRect(&nullRect, &map2LadderRect[1], &character1.playerRect)) {
							if (character1.status == RIGHT_ROLL || character1.status == LEFT_ROLL);
							else if (character1.status == LEFT_RUN || character1.status == LEFT_WALK);
							else if (character1.status == LEFT_JUMP) {
								character1.status = RIGHT_JUMP;
							}

							else if ((character1.powerUp && character1.status != RIGHT_JUMP && character1.status != LEFT_JUMP) || character1.status == RIGHT_RUN) {
								character1.status = RIGHT_RUN;
							}

							else if (character1.status == LEFT_SIT || character1.status == RIGHT_SIT)
								character1.status = RIGHT_ROLL;
							else if (character1.status != RIGHT_RUN && character1.status != RIGHT_JUMP && character1.status != LEFT_JUMP)
								character1.status = RIGHT_WALK;
							else if (character1.status == LADDER_UP || character1.status == LADDER_DOWN) {
								character1.status = RIGHT_STAY;
							}

						}
						character1.playerRect.left += character1.speed;
						character1.playerRect.right += character1.speed;
						if (character1.powerUp) {         //스프린트(+character1.speed * character1.powerUp 으로 처리할지 고민
							character1.playerRect.left += character1.speed;
							character1.playerRect.right += character1.speed;
						}
						for (auto rect : map2GroundRect) {
							RECT nullRect;
							if (IntersectRect(&nullRect, &rect, &character1.playerRect)) {
								character1.playerRect.right = rect.left;
								character1.playerRect.left = rect.left - CharacterWidth;
							}
						}
						/* for (auto rect : map2BalconyRect) {
						   RECT nullRect;
						   if (IntersectRect(&nullRect, &rect, &character1.playerRect)) {
							 character1.playerRect.right = rect.left;
							 character1.playerRect.left = rect.left - CharacterWidth;
						   }
						 }*/

						break;
					}
				}



				character1.radian = 720;
			}
		}
	}
}

void CheckLeftRightButton_2() {
	if (character2.status == LEFT_FALLING || character2.status == RIGHT_FALLING) return;
	else if (character2.status == LEFT_SHOCK || character2.status == RIGHT_SHOCK)return; //new
	else if (character2.status == LEFT_PUNCH || character2.status == RIGHT_PUNCH)return;
	if (character2.status == LEFT_AIMING || character2.status == RIGHT_AIMING) {
		if (!character2.Shoot) {
			if (bA) {
				character2.radian = 180;
				character2.status = LEFT_AIMING;
			}
			else if (bD) {
				character2.radian = 720;
				character2.status = RIGHT_AIMING;
			}
		}

		if (bW) {
			if (character2.radian >= 90 && character2.radian < 270) {
				character2.radian += 5;
			}
			else if (character2.radian <= 810 && character2.radian > 630) {
				character2.radian -= 5;
			}
		}
		else if (bS) {
			if (character2.radian <= 270 && character2.radian > 90) {
				character2.radian -= 5;
			}
			else if (character2.radian >= 630 && character2.radian < 810) {
				character2.radian += 5;
			}
		}
	}
	else {
		if (character2.usedPortal) {
			if (((bD && character2.status != RIGHT_ROLL) || character2.status == LEFT_ROLL) && character2.status != LEFT_STANDING && character2.status != RIGHT_STANDING) {
				switch (screen) {
				case eScreen::map1:
				{
					RECT nullRect;
					if (!IntersectRect(&nullRect, &map1LadderRect, &character2.playerRect)) {
						if (character2.status == LEFT_ROLL);
						else if (character2.status == RIGHT_JUMP)
							character2.status = LEFT_JUMP;
						else if (character2.powerUp && character2.status != LEFT_JUMP && character2.status != RIGHT_JUMP)
							character2.status = LEFT_RUN;
						else if (character2.status == LEFT_SIT || character2.status == RIGHT_SIT)
							character2.status = LEFT_ROLL;
						else if (character2.status != LEFT_RUN && character2.status != LEFT_JUMP && character2.status != RIGHT_JUMP)
							character2.status = LEFT_WALK;
						else if (character2.status == LADDER_UP || character2.status == LADDER_DOWN) {
							character2.status = LEFT_STAY;
						}

					}
					character2.playerRect.left -= character2.speed;
					character2.playerRect.right -= character2.speed;
					if (character2.powerUp) {         //스프린트(+character2.speed * character2.powerUp 으로 처리할지 고민
						character2.playerRect.left -= character2.speed;
						character2.playerRect.right -= character2.speed;
					}
					for (auto rect : map1GroundRect) {
						RECT nullRect;
						if (IntersectRect(&nullRect, &rect, &character2.playerRect)) {
							character2.playerRect.left = rect.right;
							character2.playerRect.right = rect.right + CharacterWidth;
						}
					}
					for (auto rect : map1BalconyRect) {
						RECT nullRect;
						if (IntersectRect(&nullRect, &rect, &character2.playerRect)) {
							character2.playerRect.left = rect.right;
							character2.playerRect.right = rect.right + CharacterWidth;
						}
					}
				}
				break;
				case eScreen::map2:
				{
					RECT nullRect;
					if (!IntersectRect(&nullRect, &map2LadderRect[0], &character2.playerRect) && !IntersectRect(&nullRect, &map2LadderRect[1], &character2.playerRect)) {
						if (character2.status == LEFT_ROLL);
						else if (character2.status == RIGHT_JUMP)
							character2.status = LEFT_JUMP;
						else if (character2.powerUp && character2.status != LEFT_JUMP && character2.status != RIGHT_JUMP)
							character2.status = LEFT_RUN;
						else if (character2.status == LEFT_SIT || character2.status == RIGHT_SIT)
							character2.status = LEFT_ROLL;
						else if (character2.status != LEFT_RUN && character2.status != LEFT_JUMP && character2.status != RIGHT_JUMP)
							character2.status = LEFT_WALK;
						else if (character2.status == LADDER_UP || character2.status == LADDER_DOWN) {
							character2.status = LEFT_STAY;
						}

					}
					character2.playerRect.left -= character2.speed;
					character2.playerRect.right -= character2.speed;
					if (character2.powerUp) {         //스프린트(+character2.speed * character2.powerUp 으로 처리할지 고민
						character2.playerRect.left -= character2.speed;
						character2.playerRect.right -= character2.speed;
					}
					for (auto rect : map2GroundRect) {
						RECT nullRect;
						if (rect.left == map2GroundRect[16].left && rect.top == map2GroundRect[16].top) continue;
						if (IntersectRect(&nullRect, &rect, &character2.playerRect)) {
							character2.playerRect.left = rect.right;
							character2.playerRect.right = rect.right + CharacterWidth;
						}
					}
					/*for (auto rect : map2BalconyRect) {
						RECT nullRect;
						if (IntersectRect(&nullRect, &rect, &character2.playerRect)) {
							character2.playerRect.left = rect.right;
							character2.playerRect.right = rect.right + CharacterWidth;
						}
					}*/
				}
				break;
				break;
				}

				character2.radian = 180;
			}


			if (((bA && character2.status != LEFT_ROLL) || character2.status == RIGHT_ROLL) && character2.status != LEFT_STANDING && character2.status != RIGHT_STANDING) {
				{//사다리 확인
					RECT nullRect;
					switch (screen) {
					case eScreen::map1:
						if (!IntersectRect(&nullRect, &map1LadderRect, &character2.playerRect)) {
							if (character2.status == RIGHT_ROLL);
							else if (character2.status == LEFT_JUMP)
								character2.status = RIGHT_JUMP;
							else if (character2.powerUp && character2.status != RIGHT_JUMP && character2.status != LEFT_JUMP) {
								character2.status = RIGHT_RUN;
							}

							else if (character2.status == LEFT_SIT || character2.status == RIGHT_SIT)
								character2.status = RIGHT_ROLL;
							else if (character2.status != RIGHT_RUN && character2.status != RIGHT_JUMP && character2.status != LEFT_JUMP)
								character2.status = RIGHT_WALK;
							else if (character2.status == LADDER_UP || character2.status == LADDER_DOWN) {
								character2.status = RIGHT_STAY;
							}
						}
						character2.playerRect.left += character2.speed;
						character2.playerRect.right += character2.speed;
						if (character2.powerUp) {         //스프린트(+character2.speed * character2.powerUp 으로 처리할지 고민
							character2.playerRect.left += character2.speed;
							character2.playerRect.right += character2.speed;
						}
						for (auto rect : map1GroundRect) {
							RECT nullRect;
							if (IntersectRect(&nullRect, &rect, &character2.playerRect)) {
								character2.playerRect.right = rect.left;
								character2.playerRect.left = rect.left - CharacterWidth;
							}
						}
						/*for (auto rect : map1BalconyRect) {
						   RECT nullRect;
						   if (IntersectRect(&nullRect, &rect, &character2.playerRect)) {
							 character2.playerRect.right = rect.left;
							 character2.playerRect.left = rect.left - CharacterWidth;
						   }
						}*/
						{
							RECT nullRect;
							if (IntersectRect(&nullRect, &map1BalconyRect[0], &character2.playerRect)) {
								character2.playerRect.right = map1BalconyRect[0].left;
								character2.playerRect.left = map1BalconyRect[0].left - CharacterWidth;
							}
						}
						break;
					case eScreen::map2:
						if (!IntersectRect(&nullRect, &map2LadderRect[0], &character2.playerRect) && !IntersectRect(&nullRect, &map2LadderRect[1], &character2.playerRect)) {
							if (character2.status == RIGHT_ROLL);
							else if (character2.status == LEFT_JUMP)
								character2.status = RIGHT_JUMP;
							else if (character2.powerUp && character2.status != RIGHT_JUMP && character2.status != LEFT_JUMP) {
								character2.status = RIGHT_RUN;
							}

							else if (character2.status == LEFT_SIT || character2.status == RIGHT_SIT)
								character2.status = RIGHT_ROLL;
							else if (character2.status != RIGHT_RUN && character2.status != RIGHT_JUMP && character2.status != LEFT_JUMP)
								character2.status = RIGHT_WALK;
							else if (character2.status == LADDER_UP || character2.status == LADDER_DOWN) {
								character2.status = RIGHT_STAY;
							}
						}
						character2.playerRect.left += character2.speed;
						character2.playerRect.right += character2.speed;
						if (character2.powerUp) {         //스프린트(+character2.speed * character2.powerUp 으로 처리할지 고민
							character2.playerRect.left += character2.speed;
							character2.playerRect.right += character2.speed;
						}
						for (auto rect : map2GroundRect) {
							RECT nullRect;
							if (IntersectRect(&nullRect, &rect, &character2.playerRect)) {
								character2.playerRect.right = rect.left;
								character2.playerRect.left = rect.left - CharacterWidth;
							}
						}
						/*for (auto rect : map2BalconyRect) {
						   RECT nullRect;
						   if (IntersectRect(&nullRect, &rect, &character2.playerRect)) {
							 character2.playerRect.right = rect.left;
							 character2.playerRect.left = rect.left - CharacterWidth;
						   }
						}*/
						break;
					}
				}



				character2.radian = 720;
			}
		}
		else {
			if (((bA && character2.status != RIGHT_ROLL) || character2.status == LEFT_ROLL) && character2.status != LEFT_STANDING && character2.status != RIGHT_STANDING) {
				switch (screen) {
				case eScreen::map1:
				{
					RECT nullRect;
					if (!IntersectRect(&nullRect, &map1LadderRect, &character2.playerRect)) {
						if (character2.status == LEFT_ROLL || character2.status == RIGHT_ROLL);
						else if (character2.status == RIGHT_RUN || character2.status == RIGHT_WALK);
						else if (character2.status == RIGHT_JUMP)
							character2.status = LEFT_JUMP;
						else if ((character2.powerUp && character2.status != LEFT_JUMP && character2.status != RIGHT_JUMP) || character2.status == LEFT_RUN) {
							character2.status = LEFT_RUN;
						}

						else if (character2.status == LEFT_SIT || character2.status == RIGHT_SIT)
							character2.status = LEFT_ROLL;
						else if (character2.status != LEFT_RUN && character2.status != LEFT_JUMP && character2.status != RIGHT_JUMP)
							character2.status = LEFT_WALK;
						else if (character2.status == LADDER_UP || character2.status == LADDER_DOWN) {
							character2.status = LEFT_STAY;
						}
					}
					character2.playerRect.left -= character2.speed;
					character2.playerRect.right -= character2.speed;
					if (character2.powerUp) {         //스프린트(+character2.speed * character2.powerUp 으로 처리할지 고민
						character2.playerRect.left -= character2.speed;
						character2.playerRect.right -= character2.speed;
					}
					for (auto rect : map1GroundRect) {
						RECT nullRect;
						if (IntersectRect(&nullRect, &rect, &character2.playerRect)) {
							character2.playerRect.left = rect.right;
							character2.playerRect.right = rect.right + CharacterWidth;
						}
					}
					for (auto rect : map1BalconyRect) {
						RECT nullRect;
						if (IntersectRect(&nullRect, &rect, &character2.playerRect)) {
							character2.playerRect.left = rect.right;
							character2.playerRect.right = rect.right + CharacterWidth;
						}
					}
				}

				break;
				case eScreen::map2:
				{
					RECT nullRect;
					if (!IntersectRect(&nullRect, &map2LadderRect[0], &character2.playerRect) && !IntersectRect(&nullRect, &map2LadderRect[1], &character2.playerRect)) {
						if (character2.status == LEFT_ROLL || character2.status == RIGHT_ROLL);
						else if (character2.status == RIGHT_RUN || character2.status == RIGHT_WALK);
						else if (character2.status == RIGHT_JUMP)
							character2.status = LEFT_JUMP;
						else if ((character2.powerUp && character2.status != LEFT_JUMP && character2.status != RIGHT_JUMP) || character2.status == LEFT_RUN) {
							character2.status = LEFT_RUN;
						}

						else if (character2.status == LEFT_SIT || character2.status == RIGHT_SIT)
							character2.status = LEFT_ROLL;
						else if (character2.status != LEFT_RUN && character2.status != LEFT_JUMP && character2.status != RIGHT_JUMP)
							character2.status = LEFT_WALK;
						else if (character2.status == LADDER_UP || character2.status == LADDER_DOWN) {
							character2.status = LEFT_STAY;
						}
					}
					character2.playerRect.left -= character2.speed;
					character2.playerRect.right -= character2.speed;
					if (character2.powerUp) {         //스프린트(+character2.speed * character2.powerUp 으로 처리할지 고민
						character2.playerRect.left -= character2.speed;
						character2.playerRect.right -= character2.speed;
					}
					for (auto rect : map2GroundRect) {
						if (rect.left == map2GroundRect[16].left && rect.top == map2GroundRect[16].top) continue;
						RECT nullRect;
						if (IntersectRect(&nullRect, &rect, &character2.playerRect)) {
							character2.playerRect.left = rect.right;
							character2.playerRect.right = rect.right + CharacterWidth;
						}
					}
					/*for (auto rect : map2BalconyRect) {
						RECT nullRect;
						if (IntersectRect(&nullRect, &rect, &character2.playerRect)) {
							character2.playerRect.left = rect.right;
							character2.playerRect.right = rect.right + CharacterWidth;
						}
					}*/
				}
				break;
				}


				character2.radian = 180;
			}

			if (((bD && character2.status != LEFT_ROLL) || character2.status == RIGHT_ROLL) && character2.status != LEFT_STANDING && character2.status != RIGHT_STANDING) {

				{
					RECT nullRect;
					switch (screen) {
					case eScreen::map1:
						if (!IntersectRect(&nullRect, &map1LadderRect, &character2.playerRect)) {
							if (character2.status == RIGHT_ROLL || character2.status == LEFT_ROLL);
							else if (character2.status == LEFT_RUN || character2.status == LEFT_WALK);
							else if (character2.status == LEFT_JUMP) {
								character2.status = RIGHT_JUMP;
							}

							else if ((character2.powerUp && character2.status != RIGHT_JUMP && character2.status != LEFT_JUMP) || character2.status == RIGHT_RUN) {
								character2.status = RIGHT_RUN;
							}

							else if (character2.status == LEFT_SIT || character2.status == RIGHT_SIT)
								character2.status = RIGHT_ROLL;
							else if (character2.status != RIGHT_RUN && character2.status != RIGHT_JUMP && character2.status != LEFT_JUMP)
								character2.status = RIGHT_WALK;
							else if (character2.status == LADDER_UP || character2.status == LADDER_DOWN) {
								character2.status = RIGHT_STAY;
							}

						}
						character2.playerRect.left += character2.speed;
						character2.playerRect.right += character2.speed;
						if (character2.powerUp) {         //스프린트(+character2.speed * character2.powerUp 으로 처리할지 고민
							character2.playerRect.left += character2.speed;
							character2.playerRect.right += character2.speed;
						}
						for (auto rect : map1GroundRect) {
							RECT nullRect;
							if (IntersectRect(&nullRect, &rect, &character2.playerRect)) {
								character2.playerRect.right = rect.left;
								character2.playerRect.left = rect.left - CharacterWidth;
							}
						}
						/*   for (auto rect : map1BalconyRect) {
							 RECT nullRect;
							 if (IntersectRect(&nullRect, &rect, &character2.playerRect)) {
							   character2.playerRect.right = rect.left;
							   character2.playerRect.left = rect.left - CharacterWidth;
							 }
						   }*/
						{
							RECT nullRect;
							if (IntersectRect(&nullRect, &map1BalconyRect[0], &character2.playerRect)) {
								character2.playerRect.right = map1BalconyRect[0].left;
								character2.playerRect.left = map1BalconyRect[0].left - CharacterWidth;
							}
						}
						break;
					case eScreen::map2:
						if (!IntersectRect(&nullRect, &map2LadderRect[0], &character2.playerRect) && !IntersectRect(&nullRect, &map2LadderRect[1], &character2.playerRect)) {
							if (character2.status == RIGHT_ROLL || character2.status == LEFT_ROLL);
							else if (character2.status == LEFT_RUN || character2.status == LEFT_WALK);
							else if (character2.status == LEFT_JUMP) {
								character2.status = RIGHT_JUMP;
							}

							else if ((character2.powerUp && character2.status != RIGHT_JUMP && character2.status != LEFT_JUMP) || character2.status == RIGHT_RUN) {
								character2.status = RIGHT_RUN;
							}

							else if (character2.status == LEFT_SIT || character2.status == RIGHT_SIT)
								character2.status = RIGHT_ROLL;
							else if (character2.status != RIGHT_RUN && character2.status != RIGHT_JUMP && character2.status != LEFT_JUMP)
								character2.status = RIGHT_WALK;
							else if (character2.status == LADDER_UP || character2.status == LADDER_DOWN) {
								character2.status = RIGHT_STAY;
							}

						}
						character2.playerRect.left += character2.speed;
						character2.playerRect.right += character2.speed;
						if (character2.powerUp) {         //스프린트(+character2.speed * character2.powerUp 으로 처리할지 고민
							character2.playerRect.left += character2.speed;
							character2.playerRect.right += character2.speed;
						}
						for (auto rect : map2GroundRect) {
							RECT nullRect;
							if (IntersectRect(&nullRect, &rect, &character2.playerRect)) {
								character2.playerRect.right = rect.left;
								character2.playerRect.left = rect.left - CharacterWidth;
							}
						}
						/* for (auto rect : map2BalconyRect) {
						   RECT nullRect;
						   if (IntersectRect(&nullRect, &rect, &character2.playerRect)) {
							 character2.playerRect.right = rect.left;
							 character2.playerRect.left = rect.left - CharacterWidth;
						   }
						 }*/

						break;
					}
				}



				character2.radian = 720;
			}
		}
	}
}

void Punch_(Character& character) {
	int length = 2;

	if (character.player == 1) {
		if (character.status == LEFT_PUNCH) {
			RECT rect = { character.playerRect };
			RECT nullrect{};

			switch (character.hand) {
			case eHand::sword:
				rect.left -= CharacterWidth / length;
				rect.right -= CharacterWidth / length;
			case eHand::axe:
				rect.left -= CharacterWidth / length;
				rect.right -= CharacterWidth / length;
			case eHand::none:
				rect.left -= CharacterWidth / length;
				rect.right -= CharacterWidth / length;

				if (IntersectRect(&nullrect, &rect, &character2.playerRect)) {
					character2.hp -= PUNCHDAMAGE;

					if (character2.hp < 0) {
						character2.hp = 0;
					}

					switch (character2.status) {
					case LEFT_AIMING:
						// 떨어뜨리기
					case LEFT_PUNCH:
					case LEFT_ROLL:
					case LEFT_RUN:
					case LEFT_SIT:
					case LEFT_STAY:
					case LEFT_WALK:
						character2.status = LEFT_SHOCK;
						break;
					case LEFT_JUMP:
					case LADDER_DOWN:
					case LADDER_UP:
						character2.status = LEFT_FALLING;
						break;
					case RIGHT_AIMING:
						// 떨어뜨리기
					case RIGHT_PUNCH:
					case RIGHT_ROLL:
					case RIGHT_RUN:
					case RIGHT_SIT:
					case RIGHT_STAY:
					case RIGHT_WALK:
						character2.status = RIGHT_SHOCK;
						break;
					case RIGHT_JUMP:
						character2.status = RIGHT_FALLING;
						break;
					}

					character2.Gunaming = FALSE;
				}

				break;
			default:
				break;
			}
		}
		else if (character.status == RIGHT_PUNCH) {
			RECT rect = { character.playerRect };

			RECT nullrect{};

			switch (character.hand) {
			case eHand::sword:
				rect.left += CharacterWidth / length;
				rect.right += CharacterWidth / length;
			case eHand::axe:
				rect.left += CharacterWidth / length;
				rect.right += CharacterWidth / length;
			case eHand::none:
				rect.left += CharacterWidth / length;
				rect.right += CharacterWidth / length;

				if (IntersectRect(&nullrect, &rect, &character2.playerRect)) {
					character2.hp -= PUNCHDAMAGE;

					if (character2.hp < 0) {
						character2.hp = 0;
					}

					switch (character2.status) {
					case LEFT_AIMING:
						// 떨어뜨리기
					case LEFT_PUNCH:
					case LEFT_ROLL:
					case LEFT_RUN:
					case LEFT_SIT:
					case LEFT_STAY:
					case LEFT_WALK:
						character2.status = LEFT_SHOCK;
						break;
					case LEFT_JUMP:
					case LADDER_DOWN:
					case LADDER_UP:
						character2.status = LEFT_FALLING;
						break;
					case RIGHT_AIMING:
						// 떨어뜨리기
					case RIGHT_PUNCH:
					case RIGHT_ROLL:
					case RIGHT_RUN:
					case RIGHT_SIT:
					case RIGHT_STAY:
					case RIGHT_WALK:
						character2.status = RIGHT_SHOCK;
						break;
					case RIGHT_JUMP:
						character2.status = RIGHT_FALLING;
						break;
					}

					character2.Gunaming = FALSE;
				}

				break;
			default:
				break;
			}
		}
	}
	else {
		if (character.status == LEFT_PUNCH) {
			RECT rect = { character.playerRect };
			RECT nullrect{};

			switch (character.hand) {
			case eHand::sword:
				rect.left -= CharacterWidth / length;
				rect.right -= CharacterWidth / length;
			case eHand::axe:
				rect.left -= CharacterWidth / length;
				rect.right -= CharacterWidth / length;
			case eHand::none:
				rect.left -= CharacterWidth / length;
				rect.right -= CharacterWidth / length;

				if (IntersectRect(&nullrect, &rect, &character1.playerRect)) {
					character1.hp -= PUNCHDAMAGE;

					if (character1.hp < 0) {
						character1.hp = 0;
					}

					switch (character1.status) {
					case LEFT_AIMING:
						// 떨어뜨리기
					case LEFT_PUNCH:
					case LEFT_ROLL:
					case LEFT_RUN:
					case LEFT_SIT:
					case LEFT_STAY:
					case LEFT_WALK:
						character1.status = LEFT_SHOCK;
						break;
					case LEFT_JUMP:
					case LADDER_DOWN:
					case LADDER_UP:
						character1.status = LEFT_FALLING;
						break;
					case RIGHT_AIMING:
						// 떨어뜨리기
					case RIGHT_PUNCH:
					case RIGHT_ROLL:
					case RIGHT_RUN:
					case RIGHT_SIT:
					case RIGHT_STAY:
					case RIGHT_WALK:
						character1.status = RIGHT_SHOCK;
						break;
					case RIGHT_JUMP:
						character1.status = RIGHT_FALLING;
						break;
					}

					character1.Gunaming = FALSE;
				}

				break;
			default:
				break;
			}
		}
		else if (character.status == RIGHT_PUNCH) {
			RECT rect = { character.playerRect };

			RECT nullrect{};

			switch (character.hand) {
			case eHand::sword:
				rect.left += CharacterWidth / length;
				rect.right += CharacterWidth / length;
			case eHand::axe:
				rect.left += CharacterWidth / length;
				rect.right += CharacterWidth / length;
			case eHand::none:
				rect.left += CharacterWidth / length;
				rect.right += CharacterWidth / length;

				if (IntersectRect(&nullrect, &rect, &character1.playerRect)) {
					character1.hp -= PUNCHDAMAGE;

					if (character1.hp < 0) {
						character1.hp = 0;
					}

					switch (character1.status) {
					case LEFT_AIMING:
						// 떨어뜨리기
					case LEFT_PUNCH:
					case LEFT_ROLL:
					case LEFT_RUN:
					case LEFT_SIT:
					case LEFT_STAY:
					case LEFT_WALK:
						character1.status = LEFT_SHOCK;
						break;
					case LEFT_JUMP:
					case LADDER_DOWN:
					case LADDER_UP:
						character1.status = LEFT_FALLING;
						break;
					case RIGHT_AIMING:
						// 떨어뜨리기
					case RIGHT_PUNCH:
					case RIGHT_ROLL:
					case RIGHT_RUN:
					case RIGHT_SIT:
					case RIGHT_STAY:
					case RIGHT_WALK:
						character1.status = RIGHT_SHOCK;
						break;
					case RIGHT_JUMP:
						character1.status = RIGHT_FALLING;
						break;
					}

					character1.Gunaming = FALSE;
				}

				break;
			default:
				break;
			}
		}
	}
}

void Shoot_(Character& character) {
	//if (character.player == 1) {
	if (character.gunnum != 0) {
		for (int i = 0; i < BULLETNUM; ++i) {
			if (bullet[i].Get_P() == 0) {
				if (character.gun == eGun::pistol) {
					bullet[i] = { character.player, 1, character.Get_P(), character.radian };
				}
				else {
					bullet[i] = { character.player, 2, character.Get_P(), character.radian };
				}
				--character.gunnum;

				if (character.gunnum == 0) {
					character.gun = eGun::none;
				}
				return;
			}
		}
	}
	//}
}

void Chk_BulletNMap() {
	switch (screen) {
	case eScreen::map1:
		for (int i = 0; i < BULLETNUM; ++i) {
			if (bullet[i].Get_P() != 0) {
				int chk{ 0 };

				for (int j = 0; j < BULLETSPEED / 5; ++j) {
					for (auto rect : map1GroundRect) {
						if (PtInRect(&rect, bullet[i].points[j])) {
							bullet[i].Delete_();

							chk = 1;
							break;
						}
					}

					if (chk == 1) {
						break;
					}

					for (auto rect : map1StairRect) {
						if (PtInRect(&rect, bullet[i].points[j])) {
							bullet[i].Delete_();

							chk = 1;
							break;
						}
					}

					if (chk == 1) {
						break;
					}

					for (auto rect : map1UpPortalRect) {
						if (PtInRect(&rect, bullet[i].points[j])) {
							bullet[i].Delete_();

							chk = 1;
							break;
						}
					}

					if (chk == 1) {
						break;
					}

					for (auto rect : map1DownPortalRect) {
						if (PtInRect(&rect, bullet[i].points[j])) {
							bullet[i].Delete_();

							chk = 1;
							break;
						}
					}

					if (chk == 1) {
						break;
					}
				}
			}
		}

		break;
	case eScreen::map2:
		break;
	default:
		break;
	}
}

void Chk_BulletNPlayer() {
	for (int i = 0; i < BULLETNUM; ++i) {
		if (bullet[i].Get_P() == 1) {
			for (int j = 0; j < BULLETSPEED / 5; ++j) {
				if (PtInRect(&character2.playerRect, bullet[i].points[j])) {
					if (bullet[i].Get_T() == 1) {
						character2.hp -= PISTOLDAMAGE;
					}
					else {
						character2.hp -= UZIDAMAGE;
					}

					if (character2.hp < 0) {
						character2.hp = 0;
					}

					switch (character2.status) {
					case LEFT_AIMING:
						// 떨어뜨리기
					case LEFT_PUNCH:
					case LEFT_ROLL:
					case LEFT_RUN:
					case LEFT_SIT:
					case LEFT_STAY:
					case LEFT_WALK:
						character2.status = LEFT_SHOCK;
						break;
					case LEFT_JUMP:
					case LADDER_DOWN:
					case LADDER_UP:
						character2.status = LEFT_FALLING;
						break;
					case RIGHT_AIMING:
						// 떨어뜨리기
					case RIGHT_PUNCH:
					case RIGHT_ROLL:
					case RIGHT_RUN:
					case RIGHT_SIT:
					case RIGHT_STAY:
					case RIGHT_WALK:
						character2.status = RIGHT_SHOCK;
						break;
					case RIGHT_JUMP:
						character2.status = RIGHT_FALLING;
						break;
					}

					character2.Gunaming = FALSE;

					bullet[i].Delete_();
					return;
				}
			}
		}
		else if (bullet[i].Get_P() == 2) {
			for (int j = 0; j < BULLETSPEED / 5; ++j) {
				if (PtInRect(&character1.playerRect, bullet[i].points[j])) {
					if (bullet[i].Get_T() == 1) {
						character1.hp -= PISTOLDAMAGE;
					}
					else {
						character1.hp -= UZIDAMAGE;
					}

					if (character1.hp < 0) {
						character1.hp = 0;
					}

					switch (character1.status) {
					case LEFT_AIMING:
						// 떨어뜨리기
					case LEFT_PUNCH:
					case LEFT_ROLL:
					case LEFT_RUN:
					case LEFT_SIT:
					case LEFT_STAY:
					case LEFT_WALK:
						character1.status = LEFT_SHOCK;
						break;
					case LEFT_JUMP:
					case LADDER_DOWN:
					case LADDER_UP:
						character1.status = LEFT_FALLING;
						break;
					case RIGHT_AIMING:
						// 떨어뜨리기
					case RIGHT_PUNCH:
					case RIGHT_ROLL:
					case RIGHT_RUN:
					case RIGHT_SIT:
					case RIGHT_STAY:
					case RIGHT_WALK:
						character1.status = RIGHT_SHOCK;
						break;
					case RIGHT_JUMP:
						character1.status = RIGHT_FALLING;
						break;
					}

					character1.Gunaming = FALSE;

					bullet[i].Delete_();
					return;
				}
			}
		}
	}
}

void Create_Throwing(Character& character) {
	for (int i = 0; i < GRENADENUM; ++i) {
		if (grenade[i].Get_S() == 0) {
			grenade[i] = { character.player, character.Get_P() };
			--character.throingnum;

			if (character.throingnum == 0) {
				character.throwing = eThrowing::none;
			}
			return;
		}
	}
}

void Throwing_(Character& character) {
	for (int i = 0; GRENADENUM; ++i) {
		if (grenade[i].Get_S() == 1) {
			if (grenade[i].Get_P() == character.player) {
				grenade[i].Set_X(character.Get_P().x);
				grenade[i].Set_Y(character.Get_P().y);
				grenade[i].Set_R(character.radian);
				return;
			}
		}
	}
}

void Chk_ThrowingNMap_X(Throwing& grnd) {
	RECT threct = grnd.Get_R();

	switch (screen) {
	case eScreen::map1:
	{
		if (grnd.Drct_X()) {
			for (auto rect : map1GroundRect) {
				RECT nullRect;
				if (IntersectRect(&nullRect, &rect, &threct)) {
					grnd.Set_X(rect.left - GRENADESIZE);
					grnd.Crsh_X();

					return;
				}
			}

			for (auto rect : map1BalconyRect) {
				RECT nullRect;
				if (IntersectRect(&nullRect, &rect, &threct)) {
					grnd.Set_X(rect.left - GRENADESIZE);
					grnd.Crsh_X();

					return;
				}
			}

			/*for (auto rect : map1StairRect) {
				RECT nullRect;
				if (IntersectRect(&nullRect, &rect, &threct)) {
					grnd.Set_X(rect.left - GRENADESIZE);
					grnd.Crsh_X();
				}
			}*/
		}
		else {
			for (auto rect : map1GroundRect) {
				RECT nullRect;
				if (IntersectRect(&nullRect, &rect, &threct)) {
					grnd.Set_X(rect.right + GRENADESIZE);
					grnd.Crsh_X();

					return;
				}
			}

			for (auto rect : map1BalconyRect) {
				RECT nullRect;
				if (IntersectRect(&nullRect, &rect, &threct)) {
					grnd.Set_X(rect.right + GRENADESIZE);
					grnd.Crsh_X();

					return;
				}
			}

			for (auto rect : map1StairRect) {
				RECT nullRect;
				if (IntersectRect(&nullRect, &rect, &threct)) {
					grnd.Set_X(rect.right + GRENADESIZE);
					grnd.Crsh_X();

					return;
				}
			}
		}

		RECT nullrect{};

		if (IntersectRect(&nullrect, &threct, &map1UpPortalRect[0])) {
			grnd.Set_X(map1DownPortalRect[0].right + GRENADESIZE);
			grnd.Set_Y((map1DownPortalRect[0].top + map1DownPortalRect[0].bottom) / 2);
			grnd.Use_P();
		}
		else if (IntersectRect(&nullrect, &threct, &map1UpPortalRect[1])) {
			grnd.Set_X(map1DownPortalRect[1].left - GRENADESIZE);
			grnd.Set_Y((map1DownPortalRect[1].top + map1DownPortalRect[1].bottom) / 2);
			grnd.Use_P();
		}
		else if (IntersectRect(&nullrect, &threct, &map1DownPortalRect[0])) {
			grnd.Set_X(map1UpPortalRect[0].right + GRENADESIZE);
			grnd.Set_Y((map1UpPortalRect[0].top + map1UpPortalRect[0].bottom) / 2);
			grnd.Use_P();
		}
		else if (IntersectRect(&nullrect, &threct, &map1DownPortalRect[1])) {
			grnd.Set_X(map1UpPortalRect[1].left - GRENADESIZE);
			grnd.Set_Y((map1UpPortalRect[1].top + map1UpPortalRect[1].bottom) / 2);
			grnd.Use_P();
		}
	}

	break;
	case eScreen::map2:
		break;
	default:
		break;
	}
}

void Chk_ThrowingNMap_Y(Throwing& grnd) {
	RECT threct = grnd.Get_R();
	/*int plus = 10;
	threct = { threct.left, threct.top - plus, threct.right, threct.bottom };*/
	/*threct = { threct.left - plus, threct.top - plus, threct.right + plus, threct.bottom + plus };*/

	switch (screen) {
	case eScreen::map1:
	{
		if (grnd.Drct_Y()) {
			threct.bottom += grnd.Get_My();

			for (auto rect : map1StairRect) {
				RECT nullRect;
				if (IntersectRect(&nullRect, &rect, &threct)) {
					grnd.Set_Y(rect.top - GRENADESIZE);
					grnd.Crsh_Y();

					return;
				}
			}

			for (auto rect : map1GroundRect) {
				RECT nullRect;
				if (IntersectRect(&nullRect, &rect, &threct)) {
					if (rect.left == map1GroundRect[ELEVATOR].left && rect.top == map1GroundRect[ELEVATOR].top) {
						grnd.Set_Y(rect.top - GRENADESIZE - EMOVE - 25);
						grnd.Crsh_Y();
					}
					else {
						grnd.Set_Y(rect.top - GRENADESIZE);
						grnd.Crsh_Y();
					}

					return;
				}
			}

			for (auto rect : map1BalconyRect) {
				RECT nullRect;
				if (IntersectRect(&nullRect, &rect, &threct)) {
					grnd.Set_Y(rect.top - GRENADESIZE);
					grnd.Crsh_Y();

					return;
				}
			}
		}
		else {
			for (auto rect : map1StairRect) {
				RECT nullRect;
				if (IntersectRect(&nullRect, &rect, &threct)) {
					grnd.Set_Y(rect.bottom + GRENADESIZE);
					grnd.Crsh_Y();

					return;
				}
			}

			for (auto rect : map1GroundRect) {
				RECT nullRect;
				if (IntersectRect(&nullRect, &rect, &threct)) {
					grnd.Set_Y(rect.bottom + GRENADESIZE);
					grnd.Crsh_Y();

					return;
				}
			}
		}
	}

	break;
	case eScreen::map2:
		break;
	default:
		break;
	}
}

void Explosion_() {
	for (int i = 0; i < GRENADENUM; ++i) {
		if (grenade[i].Get_S() != 0) {
			if (grenade[i].Get_T() == GRENADETIME) {
				for (int j = 0; j < EXPLOSIONTNUM; ++j) {
					if (explosion[j].Get_S() == 0) {
						explosion[j] = { grenade[i].Get_Pt() };

						RECT rect1{}, rect2{};
						rect2 = explosion[j].Get_R();

						if (IntersectRect(&rect1, &rect2, &character1.playerRect)) {
							character1.hp -= GRENADEDAMAGE;

							if (character1.hp < 0) {
								character1.hp = 0;
							}

							switch (character1.status) {
							case LEFT_AIMING:
								// 떨어뜨리기
								if (IspressedComma) {
									for (int i = 0; GRENADENUM; ++i) {
										if (grenade[i].Get_S() == 1) {
											if (grenade[i].Get_P() == character1.player) {
												grenade[i].Set_X(character1.Get_P().x);
												grenade[i].Set_Y(character1.Get_P().y);
												grenade[i].Set_R(90);
												break;
											}
										}
									}
								}
							case LEFT_PUNCH:
							case LEFT_ROLL:
							case LEFT_RUN:
							case LEFT_SIT:
							case LEFT_STAY:
							case LEFT_WALK:
							case LEFT_SHOCK:
								character1.status = LEFT_FALLING;
								break;
							case LEFT_JUMP:
							case LADDER_DOWN:
							case LADDER_UP:
								character1.status = LEFT_FALLING;
								break;
							case RIGHT_AIMING:
								// 떨어뜨리기
								if (IspressedComma) {
									for (int i = 0; GRENADENUM; ++i) {
										if (grenade[i].Get_S() == 1) {
											if (grenade[i].Get_P() == character1.player) {
												grenade[i].Set_X(character1.Get_P().x);
												grenade[i].Set_Y(character1.Get_P().y);
												grenade[i].Set_R(90);
												break;
											}
										}
									}
								}
							case RIGHT_PUNCH:
							case RIGHT_ROLL:
							case RIGHT_RUN:
							case RIGHT_SIT:
							case RIGHT_STAY:
							case RIGHT_WALK:
							case RIGHT_SHOCK:
								character1.status = RIGHT_FALLING;
								break;
							case RIGHT_JUMP:
								character1.status = RIGHT_FALLING;
								break;
							}

							character1.Gunaming = FALSE;
						}

						if (IntersectRect(&rect1, &rect2, &character2.playerRect)) {
							character2.hp -= GRENADEDAMAGE;

							if (character2.hp < 0) {
								character2.hp = 0;
							}

							switch (character2.status) {
							case LEFT_AIMING:
								// 떨어뜨리기
								if (IspressedNum3) {
									for (int i = 0; GRENADENUM; ++i) {
										if (grenade[i].Get_S() == 1) {
											if (grenade[i].Get_P() == character2.player) {
												grenade[i].Set_X(character2.Get_P().x);
												grenade[i].Set_Y(character2.Get_P().y);
												grenade[i].Set_R(90);
												break;
											}
										}
									}
								}
							case LEFT_PUNCH:
							case LEFT_ROLL:
							case LEFT_RUN:
							case LEFT_SIT:
							case LEFT_STAY:
							case LEFT_WALK:
							case LEFT_SHOCK:
								character2.status = LEFT_FALLING;
								break;
							case LEFT_JUMP:
							case LADDER_DOWN:
							case LADDER_UP:
								character2.status = LEFT_FALLING;
								break;
							case RIGHT_AIMING:
								// 떨어뜨리기
								if (IspressedNum3) {
									for (int i = 0; GRENADENUM; ++i) {
										if (grenade[i].Get_S() == 1) {
											if (grenade[i].Get_P() == character2.player) {
												grenade[i].Set_X(character2.Get_P().x);
												grenade[i].Set_Y(character2.Get_P().y);
												grenade[i].Set_R(90);
												break;
											}
										}
									}
								}
							case RIGHT_PUNCH:
							case RIGHT_ROLL:
							case RIGHT_RUN:
							case RIGHT_SIT:
							case RIGHT_STAY:
							case RIGHT_WALK:
							case RIGHT_SHOCK:
								character2.status = RIGHT_FALLING;
								break;
							case RIGHT_JUMP:
								character2.status = RIGHT_FALLING;
								break;
							}

							character2.Gunaming = FALSE;
						}

						grenade[i] = {};
						return;
					}
				}
			}
		}
	}
}

void Create_Item() {
	switch (screen) {
	case eScreen::map1:
		item[0] = { 1130, 500 };
		item[1] = { 500, 560 };
		item[2] = { 500, 1380 };
		item[3] = { 1160, 1000 };
		item[4] = { 1700, 1480 };

		break;
	case eScreen::map2:
		break;
	default:
		break;
	}
}

void Timer_Item() {
	if (itemtime == ITEMTIME) {
		Create_Item();

		itemtime = 0;
	}
	else if (itemtime == ITEMTIME * 3 / 4) {
		for (int i = 0; i < ITEMNUM; ++i) {
			item[i] = {};
		}

		++itemtime;
	}
	else {
		++itemtime;
	}
}

void Get_Item(Character& character) {
	RECT nullrect{}, irect{};

	for (int i = 0; i < ITEMNUM; ++i) {
		irect = item[i].Get_R();

		if (IntersectRect(&nullrect, &irect, &character.playerRect)) {
			switch (item[i].Get_T()) {
			case eItem::axe:
				character.hand = eHand::axe;
				break;
			case eItem::sword:
				character.hand = eHand::sword;
				break;
			case eItem::pistol:
				character.gun = eGun::pistol;
				character.gunnum = 8;
				break;
			case eItem::uzi:
				character.gun = eGun::uzi;
				character.gunnum = 24;
				break;
			case eItem::grenade:
				character.throwing = eThrowing::grenade;
				character.throingnum = 3;
				break;
			case eItem::smallkit:
				character.hp += SMALLKIT;

				if (character.hp > 100) {
					character.hp = 100;
				}
				break;
			case eItem::bigkit:
				character.hp += BIGKIT;

				if (character.hp > 100) {
					character.hp = 100;
				}
				break;
			default:
				break;
			}

			item[i] = {};
			return;
		}
	}

}

void Chk_End() {
	if (character1.hp == 0 && character2.hp == 0) {
		printscreen = 1;
		Theend = TRUE;
		winner = 3;

		++winpoint1;
		++winpoint2;
	}
	else if (character1.hp == 0) {
		printscreen = 1;
		Theend = TRUE;
		winner = 1;

		++winpoint1;
	}
	else if (character2.hp == 0) {
		printscreen = 1;
		Theend = TRUE;
		winner = 2;

		++winpoint2;
	}
}

void Rst_Object() {
	// bullet //

	for (int i = 0; i < BULLETNUM; ++i) {
		bullet[i] = {};
	}

	// grenade //

	for (int i = 0; i < GRENADENUM; ++i) {
		grenade[i] = {};
	}

	// explosion //

	for (int i = 0; i < EXPLOSIONTNUM; ++i) {
		explosion[i] = {};
	}

	// item //

	for (int i = 0; i < ITEMNUM; ++i) {
		item[i] = {};
	}

	itemtime = 0;
}

void Rst_Map1() {
	// elevator //

	map1GroundRect[ELEVATOR] = { 917, 1480, 1017, 1500 };
	elevatordrct = 1;

	// character //

	character1 = {};
	character2 = {};

	character1.player = 1;
	character1.playerRect = { 1750, 1563 - CharacterHeight, 1750 + CharacterWidth, 1563 };
	character1.status = LEFT_STAY;
	character1.GravitySpeed = CharacterGravitySpeed;
	character1.speed = CharacterSpeed;

	character1.imgRect.top = 90;
	character1.imgRect.bottom = character1.imgRect.top + 80;
	character1.imgRect.left = 0;
	character1.imgRect.right = character1.imgRect.left + 56;

	character2.player = 2;
	character2.playerRect = { 394,1354,394 + CharacterWidth,1354 + CharacterHeight };
	character2.status = LEFT_STAY;
	character2.GravitySpeed = CharacterGravitySpeed;
	character2.speed = CharacterSpeed;

	character2.imgRect.top = 90;
	character2.imgRect.bottom = character2.imgRect.top + 80;
	character2.imgRect.left = 0;
	character2.imgRect.right = character2.imgRect.left + 56;
}

void Rst_Map2() {

}

void CheckFallingCharacter(Character& character) {
	if (character.GravitySpeed >= CharacterGravitySpeed * 3) {
		if (character.status == LEFT_SIT || character.status == RIGHT_SIT || character.status == LEFT_ROLL || character.status == RIGHT_ROLL) character.playerRect.top = character.playerRect.bottom - CharacterHeight;
		if (character.GravitySpeed >= ChangeToFallingSpeed) {
			if (character.status == LEFT_WALK || character.status == LEFT_STAY || character.status == LEFT_RUN || character.status == LEFT_ROLL || character.status == LEFT_SIT || character.status == LEFT_JUMP) character.status = LEFT_FALLING;
			else if (character.status == RIGHT_WALK || character.status == RIGHT_STAY || character.status == RIGHT_RUN || character.status == RIGHT_ROLL || character.status == RIGHT_SIT || character.status == RIGHT_JUMP) character.status = RIGHT_FALLING;
		}
		else {
			if (character.status == LEFT_WALK || character.status == LEFT_STAY || character.status == LEFT_RUN || character.status == LEFT_ROLL || character.status == LEFT_SIT) character.status = LEFT_JUMP;
			else if (character.status == RIGHT_WALK || character.status == RIGHT_STAY || character.status == RIGHT_RUN || character.status == RIGHT_ROLL || character.status == RIGHT_SIT) character.status = RIGHT_JUMP;
		}
	}
}

void ChangeCharacterMotion_1() {//new
	int offsetWidth = 0;
	int offsetHeight = 0;
	static int wait_1 = 1;
	switch (character1.status)
	{
	case LEFT_STAY:
		offsetWidth = 56;
		offsetHeight = 80;
		if ((character1.imgRect.top != 90 && character1.imgRect.left != 0) || character1.imgRect.top != 90) {
			wait_1 = 1;
			character1.imgRect.top = 90;
			character1.imgRect.bottom = character1.imgRect.top + offsetHeight;
			character1.imgRect.left = 0;
			character1.imgRect.right = character1.imgRect.left + offsetWidth;
		}
		else {
			if (wait_1 % 20 == 0) {
				character1.imgRect.left += (offsetWidth);
				character1.imgRect.right += (offsetWidth);
				if (character1.imgRect.left >= 0 + offsetWidth * 4) {
					character1.imgRect.left = 0;
					character1.imgRect.right = character1.imgRect.left + offsetWidth;
				}
				wait_1 = 1;
			}
			else ++wait_1;
		}


		break;
	case RIGHT_STAY:
		offsetWidth = 56;
		offsetHeight = 80;
		if ((character1.imgRect.top != 0 && character1.imgRect.left != 0) || character1.imgRect.top != 0) {
			wait_1 = 1;
			character1.imgRect.top = 0;
			character1.imgRect.bottom = character1.imgRect.top + offsetHeight;
			character1.imgRect.left = 0;
			character1.imgRect.right = character1.imgRect.left + offsetWidth;
		}
		else {
			if (wait_1 % 20 == 0) {
				character1.imgRect.left += (offsetWidth);
				character1.imgRect.right += (offsetWidth);
				if (character1.imgRect.left >= 0 + offsetWidth * 4) {
					character1.imgRect.left = 0;
					character1.imgRect.right = character1.imgRect.left + offsetWidth;
				}
				wait_1 = 1;
			}
			else ++wait_1;
		}
		break;
	case LEFT_RUN:
		offsetWidth = 76;
		offsetHeight = 80;
		if ((character1.imgRect.top != 368 && character1.imgRect.left != 465) || !(465 <= character1.imgRect.left && character1.imgRect.left <= 465 + offsetWidth * 4)) {
			wait_1 = 1;
			character1.imgRect.top = 368;
			character1.imgRect.bottom = character1.imgRect.top + offsetHeight;
			character1.imgRect.left = 465;
			character1.imgRect.right = character1.imgRect.left + offsetWidth;
		}
		else {
			if (wait_1 % 2 == 0) {
				character1.imgRect.left += offsetWidth;
				character1.imgRect.right += offsetWidth;
				if (character1.imgRect.left >= 465 + offsetWidth * 4) {
					character1.imgRect.left = 465;
					character1.imgRect.right = character1.imgRect.left + offsetWidth;
				}
				wait_1 = 1;
			}
			else ++wait_1;
		}
		break;
	case RIGHT_RUN:
		offsetWidth = 76;
		offsetHeight = 80;
		if ((character1.imgRect.top != 270 && character1.imgRect.left != 465) || !(465 <= character1.imgRect.left && character1.imgRect.left <= 465 + offsetWidth * 4)) {
			wait_1 = 1;
			character1.imgRect.top = 270;
			character1.imgRect.bottom = character1.imgRect.top + offsetHeight;
			character1.imgRect.left = 465;
			character1.imgRect.right = character1.imgRect.left + offsetWidth;
		}
		else {
			if (wait_1 % 2 == 0) {
				character1.imgRect.left += offsetWidth;
				character1.imgRect.right += offsetWidth;
				if (character1.imgRect.left >= 465 + offsetWidth * 4) {
					character1.imgRect.left = 465;
					character1.imgRect.right = character1.imgRect.left + offsetWidth;
				}
				wait_1 = 1;
			}
			else ++wait_1;
		}
		break;
	case LEFT_WALK:
		offsetWidth = 64;
		offsetHeight = 80;
		if ((character1.imgRect.top != 367 && character1.imgRect.left != 0) || character1.imgRect.top != 367) {
			wait_1 = 1;
			character1.imgRect.top = 367;
			character1.imgRect.bottom = character1.imgRect.top + offsetHeight;
			character1.imgRect.left = 0;
			character1.imgRect.right = character1.imgRect.left + offsetWidth;
		}
		else {
			if (wait_1 % 3 == 0) {
				character1.imgRect.left += offsetWidth;
				character1.imgRect.right += offsetWidth;
				if (character1.imgRect.left >= 0 + offsetWidth * 4) {
					character1.imgRect.left = 0;
					character1.imgRect.right = character1.imgRect.left + offsetWidth;
				}
				wait_1 = 1;
			}
			else ++wait_1;
		}
		break;
	case RIGHT_WALK:
		offsetWidth = 64;
		offsetHeight = 80;
		if ((character1.imgRect.top != 270 && character1.imgRect.left != 0) || character1.imgRect.top != 270) {
			wait_1 = 1;

			character1.imgRect.top = 270;
			character1.imgRect.bottom = character1.imgRect.top + offsetHeight;
			character1.imgRect.left = 0;
			character1.imgRect.right = character1.imgRect.left + offsetWidth;
		}
		else {
			if (wait_1 % 3 == 0) {
				character1.imgRect.left += offsetWidth;
				character1.imgRect.right += offsetWidth;
				if (character1.imgRect.left >= 0 + offsetWidth * 4) {
					character1.imgRect.left = 0;
					character1.imgRect.right = character1.imgRect.left + offsetWidth;
				}
				wait_1 = 1;
			}
			else ++wait_1;
		}
		break;
	case LEFT_JUMP:
		if ((character1.imgRect.top != 544 && character1.imgRect.left != 264) || character1.imgRect.left != 544) {
			character1.imgRect.top = 544;
			character1.imgRect.bottom = character1.imgRect.top + 80;
			character1.imgRect.left = 264;
			character1.imgRect.right = character1.imgRect.left + 60;
		}
		break;
	case RIGHT_JUMP:
		if ((character1.imgRect.top != 544 && character1.imgRect.left != 0) || character1.imgRect.left != 544) {
			character1.imgRect.top = 544;
			character1.imgRect.bottom = character1.imgRect.top + 80;
			character1.imgRect.left = 0;
			character1.imgRect.right = character1.imgRect.left + 60;
		}
		break;
	case LADDER_UP:
		offsetWidth = 60;
		offsetHeight = 84;
		if ((character1.imgRect.top != 174 && character1.imgRect.left != 0) || !(0 <= character1.imgRect.left && character1.imgRect.left <= 0 + offsetWidth * 4)) {
			wait_1 = 1;
			character1.imgRect.top = 174;
			character1.imgRect.bottom = character1.imgRect.top + offsetHeight;
			character1.imgRect.left = 0;
			character1.imgRect.right = character1.imgRect.left + offsetWidth;
		}
		else {
			if (bUp) {
				if (wait_1 % 3 == 0) {
					character1.imgRect.left += offsetWidth;
					character1.imgRect.right += offsetWidth;
					if (character1.imgRect.left == 0 + offsetWidth * 4) {
						character1.imgRect.left = 0;
						character1.imgRect.right = character1.imgRect.left + offsetWidth;
					}
					wait_1 = 1;
				}
				else ++wait_1;
			}
		}
		break;
	case LADDER_DOWN:
		offsetWidth = 52;
		offsetHeight = 80;
		if (!(character1.imgRect.left == 502 && character1.imgRect.top == 178)) {
			character1.imgRect.top = 178;
			character1.imgRect.bottom = character1.imgRect.top + offsetHeight;
			character1.imgRect.left = 502;
			character1.imgRect.right = character1.imgRect.left + offsetWidth;
		}
		break;
	case SHOCK:
		break;
	case LEFT_FALLING:
		offsetWidth = 68;
		offsetHeight = 68;
		if (!(character1.imgRect.left == 340 && character1.imgRect.right == 736)) {
			character1.imgRect.top = 736;
			character1.imgRect.bottom = character1.imgRect.top + offsetHeight;
			character1.imgRect.left = 340;
			character1.imgRect.right = character1.imgRect.left + offsetWidth;
		}
		break;
	case RIGHT_FALLING:
		offsetWidth = 68;
		offsetHeight = 68;
		if (!(character1.imgRect.left == 0 && character1.imgRect.right == 736)) {
			character1.imgRect.top = 736;
			character1.imgRect.bottom = character1.imgRect.top + offsetHeight;
			character1.imgRect.left = 0;
			character1.imgRect.right = character1.imgRect.left + offsetWidth;
		}
		break;
	case LEFT_STANDING:
		offsetWidth = 76;
		offsetHeight = 64;
		if ((character1.imgRect.top != 652 && character1.imgRect.left != 462) || !(462 <= character1.imgRect.left && character1.imgRect.left <= 462 + offsetWidth * 2)) {
			wait_1 = 1;
			character1.imgRect.top = 652;
			character1.imgRect.bottom = character1.imgRect.top + offsetHeight;
			character1.imgRect.left = 462;
			character1.imgRect.right = character1.imgRect.left + offsetWidth;
		}
		else {
			if (wait_1 % 10 == 0) {
				character1.imgRect.left += offsetWidth;
				character1.imgRect.right += offsetWidth;
				if (character1.imgRect.left == 462 + offsetWidth * 2) {
					character1.status = LEFT_STAY;
					character1.imgRect.top = 90;
					character1.imgRect.bottom = character1.imgRect.top + 80;
					character1.imgRect.left = 0;
					character1.imgRect.right = character1.imgRect.left + 56;
				}
				wait_1 = 1;
			}
			else ++wait_1;
		}
		break;
	case RIGHT_STANDING:
		offsetWidth = 76;
		offsetHeight = 64;
		if ((character1.imgRect.top != 652 && character1.imgRect.left != 0) || !(0 <= character1.imgRect.left && character1.imgRect.left <= 0 + offsetWidth * 2) || (character1.imgRect.top == 736 && character1.imgRect.left == 0)) {

			wait_1 = 1;
			character1.imgRect.top = 652;
			character1.imgRect.bottom = character1.imgRect.top + offsetHeight;
			character1.imgRect.left = 0;
			character1.imgRect.right = character1.imgRect.left + offsetWidth;
		}
		else {
			if (wait_1 % 10 == 0) {
				character1.imgRect.left += offsetWidth;
				character1.imgRect.right += offsetWidth;
				if (character1.imgRect.left == 0 + offsetWidth * 2) {
					character1.status = RIGHT_STAY;
					character1.imgRect.top = 0;
					character1.imgRect.bottom = character1.imgRect.top + 80;
					character1.imgRect.left = 0;
					character1.imgRect.right = character1.imgRect.left + 56;
				}
				wait_1 = 1;
			}
			else ++wait_1;
		}
		break;
	case LEFT_SIT:
		if (character1.imgRect.top != 106 && character1.imgRect.left != 415) {
			character1.imgRect.top = 106;
			character1.imgRect.bottom = character1.imgRect.top + 64;
			character1.imgRect.left = 415;
			character1.imgRect.right = character1.imgRect.left + 52;
		}
		break;
	case RIGHT_SIT:
		if (character1.imgRect.top != 16 && character1.imgRect.left != 415) {
			character1.imgRect.top = 16;
			character1.imgRect.bottom = character1.imgRect.top + 64;
			character1.imgRect.left = 415;
			character1.imgRect.right = character1.imgRect.left + 52;
		}
		break;
	case LEFT_ROLL:
		offsetWidth = 60;
		offsetHeight = 64;
		if ((character1.imgRect.top != 465 && character1.imgRect.left != 465) || !(465 <= character1.imgRect.left && character1.imgRect.left <= 465 + offsetWidth * 4)) {
			wait_1 = 1;
			character1.imgRect.top = 465;
			character1.imgRect.bottom = character1.imgRect.top + offsetHeight;
			character1.imgRect.left = 465;
			character1.imgRect.right = character1.imgRect.left + offsetWidth;
		}
		else {
			if (wait_1 % 3 == 0) {
				character1.imgRect.left += offsetWidth;
				character1.imgRect.right += offsetWidth;
				if (character1.imgRect.left == 465 + offsetWidth * 4) {
					character1.playerRect.top = character1.playerRect.bottom - CharacterHeight;
					if (character1.usedPortal) {
						character1.usedPortal = false;
					}
					character1.status = LEFT_STAY;
					character1.imgRect.top = 90;
					character1.imgRect.bottom = character1.imgRect.top + offsetHeight;
					character1.imgRect.left = 0;
					character1.imgRect.right = character1.imgRect.left + offsetWidth;
				}
				wait_1 = 1;
			}
			else ++wait_1;
		}
		break;
	case RIGHT_ROLL:
		offsetWidth = 60;
		offsetHeight = 64;
		if ((character1.imgRect.top != 465 && character1.imgRect.left != 0) || !(0 <= character1.imgRect.left && character1.imgRect.left <= 0 + offsetWidth * 4)) {
			wait_1 = 1;
			character1.imgRect.top = 465;
			character1.imgRect.bottom = character1.imgRect.top + offsetHeight;
			character1.imgRect.left = 0;
			character1.imgRect.right = character1.imgRect.left + offsetWidth;
		}
		else {
			if (wait_1 % 3 == 0) {
				character1.imgRect.left += offsetWidth;
				character1.imgRect.right += offsetWidth;
				if (character1.imgRect.left == 0 + offsetWidth * 4) {
					character1.playerRect.top = character1.playerRect.bottom - CharacterHeight;
					if (character1.usedPortal) {
						character1.usedPortal = false;
					}
					character1.status = RIGHT_STAY;
					character1.imgRect.top = 0;
					character1.imgRect.bottom = character1.imgRect.top + offsetHeight;
					character1.imgRect.left = 0;
					character1.imgRect.right = character1.imgRect.left + offsetWidth;
				}
				wait_1 = 1;
			}
			else ++wait_1;
		}
		break;
	case LEFT_AIMING:  //new - 
		if (IspressedComma) {//수류탄 처리
			if (character1.imgRect.top != 944 || character1.imgRect.left != 767) {
				character1.imgRect.top = 944;
				character1.imgRect.bottom = character1.imgRect.top + 80;
				character1.imgRect.left = 767;
				character1.imgRect.right = character1.imgRect.left + 52;
			}
		}
		else {//총 화면 처리
			if (character1.imgRect.top != 944 || character1.imgRect.left != 520) {
				character1.imgRect.top = 944;
				character1.imgRect.bottom = character1.imgRect.top + 80;
				character1.imgRect.left = 520;
				character1.imgRect.right = character1.imgRect.left + 68;
			}
		}
		break;
	case RIGHT_AIMING:
		if (IspressedComma) {//수류탄 처리
			if (character1.imgRect.top != 944 || character1.imgRect.left != 247) {
				character1.imgRect.top = 944;
				character1.imgRect.bottom = character1.imgRect.top + 80;
				character1.imgRect.left = 247;
				character1.imgRect.right = character1.imgRect.left + 52;
			}
		}
		else {//총 화면 처리
			if (character1.imgRect.top != 944 || character1.imgRect.left != 0) {
				character1.imgRect.top = 944;
				character1.imgRect.bottom = character1.imgRect.top + 80;
				character1.imgRect.left = 0;
				character1.imgRect.right = character1.imgRect.left + 68;
			}
		}
		break;
	case LEFT_PUNCH:
		if (character1.hand == eHand::none) {
			offsetWidth = 68;
			offsetHeight = 80;
			if ((character1.imgRect.top != 551 && character1.imgRect.left != 731) || !(731 <= character1.imgRect.left && character1.imgRect.left <= 731 + offsetWidth * 3)) {
				wait_1 = 1;
				character1.imgRect.top = 551;
				character1.imgRect.bottom = character1.imgRect.top + offsetHeight;
				character1.imgRect.left = 731;
				character1.imgRect.right = character1.imgRect.left + offsetWidth;
			}
			else {
				if (wait_1 % 3 == 0) {
					character1.imgRect.left += offsetWidth;
					character1.imgRect.right += offsetWidth;
					if (character1.imgRect.left == 731 + offsetWidth * 3) {
						character1.status = LEFT_STAY;
						character1.imgRect.top = 90;
						character1.imgRect.bottom = character1.imgRect.top + 80;
						character1.imgRect.left = 0;
						character1.imgRect.right = character1.imgRect.left + 56;
					}
					wait_1 = 1;
				}
				else ++wait_1;
			}
		}
		else if (character1.hand == eHand::sword) {
			offsetWidth = 114;
			offsetHeight = 116;
			if ((character1.imgRect.top != 1040 && character1.imgRect.left != 500) || !(500 <= character1.imgRect.left && character1.imgRect.left <= 500 + offsetWidth * 4)) {
				wait_1 = 1;
				character1.imgRect.top = 1040;
				character1.imgRect.bottom = character1.imgRect.top + offsetHeight;
				character1.imgRect.left = 500;
				character1.imgRect.right = character1.imgRect.left + offsetWidth;
			}
			else {
				if (wait_1 % 3 == 0) {
					character1.imgRect.left += offsetWidth;
					character1.imgRect.right += offsetWidth;
					if (character1.imgRect.left == 500 + offsetWidth * 4) {
						character1.status = LEFT_STAY;
						character1.imgRect.top = 90;
						character1.imgRect.bottom = character1.imgRect.top + 80;
						character1.imgRect.left = 0;
						character1.imgRect.right = character1.imgRect.left + 56;
					}
					wait_1 = 1;
				}
				else ++wait_1;
			}
		}
		else if (character1.hand == eHand::axe) {
			offsetWidth = 92;
			offsetHeight = 116;
			if ((character1.imgRect.top != 1194 && character1.imgRect.left != 408) || !(408 <= character1.imgRect.left && character1.imgRect.left <= 408 + offsetWidth * 4)) {
				wait_1 = 1;
				character1.imgRect.top = 1194;
				character1.imgRect.bottom = character1.imgRect.top + offsetHeight;
				character1.imgRect.left = 408;
				character1.imgRect.right = character1.imgRect.left + offsetWidth;
			}
			else {
				if (wait_1 % 3 == 0) {
					character1.imgRect.left += offsetWidth;
					character1.imgRect.right += offsetWidth;
					if (character1.imgRect.left == 408 + offsetWidth * 4) {
						character1.status = LEFT_STAY;
						character1.imgRect.top = 90;
						character1.imgRect.bottom = character1.imgRect.top + 80;
						character1.imgRect.left = 0;
						character1.imgRect.right = character1.imgRect.left + 56;
					}
					wait_1 = 1;
				}
				else ++wait_1;
			}
		}
		break;
	case RIGHT_PUNCH:
		if (character1.hand == eHand::none) {
			offsetWidth = 68;
			offsetHeight = 80;
			if ((character1.imgRect.top != 551 && character1.imgRect.left != 517) || !(517 <= character1.imgRect.left && character1.imgRect.left <= 517 + offsetWidth * 3)
				|| character1.imgRect.top != 551) {
				wait_1 = 1;
				character1.imgRect.top = 551;
				character1.imgRect.bottom = character1.imgRect.top + offsetHeight;
				character1.imgRect.left = 517;
				character1.imgRect.right = character1.imgRect.left + offsetWidth;
			}
			else {
				if (wait_1 % 3 == 0) {
					character1.imgRect.left += offsetWidth;
					character1.imgRect.right += offsetWidth;
					if (character1.imgRect.left == 517 + offsetWidth * 3) {
						character1.status = RIGHT_STAY;
						character1.imgRect.top = 0;
						character1.imgRect.bottom = character1.imgRect.top + 80;
						character1.imgRect.left = 0;
						character1.imgRect.right = character1.imgRect.left + 56;
					}
					wait_1 = 1;
				}
				else ++wait_1;
			}
		}
		else if (character1.hand == eHand::sword) {
			offsetWidth = 114;
			offsetHeight = 116;
			if ((character1.imgRect.top != 1040 && character1.imgRect.left != 0) || !(0 <= character1.imgRect.left && character1.imgRect.left <= 0 + offsetWidth * 4)
				|| character1.imgRect.top != 1040) {
				wait_1 = 1;
				character1.imgRect.top = 1040;
				character1.imgRect.bottom = character1.imgRect.top + offsetHeight;
				character1.imgRect.left = 0;
				character1.imgRect.right = character1.imgRect.left + offsetWidth;
			}
			else {
				if (wait_1 % 3 == 0) {
					character1.imgRect.left += offsetWidth;
					character1.imgRect.right += offsetWidth;
					if (character1.imgRect.left == 0 + offsetWidth * 4) {
						character1.status = RIGHT_STAY;
						character1.imgRect.top = 0;
						character1.imgRect.bottom = character1.imgRect.top + 80;
						character1.imgRect.left = 0;
						character1.imgRect.right = character1.imgRect.left + 56;
					}
					wait_1 = 1;
				}
				else ++wait_1;
			}
		}
		else if (character1.hand == eHand::axe) {
			offsetWidth = 92;
			offsetHeight = 116;
			if ((character1.imgRect.top != 1194 && character1.imgRect.left != 0) || !(0 <= character1.imgRect.left && character1.imgRect.left <= 0 + offsetWidth * 4)
				|| character1.imgRect.top != 1194) {
				wait_1 = 1;
				character1.imgRect.top = 1194;
				character1.imgRect.bottom = character1.imgRect.top + offsetHeight;
				character1.imgRect.left = 0;
				character1.imgRect.right = character1.imgRect.left + offsetWidth;
			}
			else {
				if (wait_1 % 3 == 0) {
					character1.imgRect.left += offsetWidth;
					character1.imgRect.right += offsetWidth;
					if (character1.imgRect.left == 0 + offsetWidth * 4) {
						character1.status = RIGHT_STAY;
						character1.imgRect.top = 0;
						character1.imgRect.bottom = character1.imgRect.top + 80;
						character1.imgRect.left = 0;
						character1.imgRect.right = character1.imgRect.left + 56;
					}
					wait_1 = 1;
				}
				else ++wait_1;
			}
		}
		break;
	case LEFT_SHOCK:
		if (character1.imgRect.top != 844 || character1.imgRect.left != 294) {
			character1.imgRect.top = 844;
			character1.imgRect.bottom = character1.imgRect.top + 76;
			character1.imgRect.left = 294;
			character1.imgRect.right = character1.imgRect.left + 60;
		}
		else {
			++wait_1;
			if (wait_1 % 10 == 0) {
				character1.status = LEFT_STAY;
				character1.imgRect.top = 90;
				character1.imgRect.bottom = character1.imgRect.top + 80;
				character1.imgRect.left = 0;
				character1.imgRect.right = character1.imgRect.left + 56;
			}
		}
		break;
	case RIGHT_SHOCK:
		if (character1.imgRect.top != 844 || character1.imgRect.left != 0) {
			wait_1 = 1;
			character1.imgRect.top = 844;
			character1.imgRect.bottom = character1.imgRect.top + 76;
			character1.imgRect.left = 0;
			character1.imgRect.right = character1.imgRect.left + 60;
		}
		else {
			++wait_1;
			if (wait_1 % 10 == 0) {
				character1.status = RIGHT_STAY;
				character1.imgRect.top = 0;
				character1.imgRect.bottom = character1.imgRect.top + 80;
				character1.imgRect.left = 0;
				character1.imgRect.right = character1.imgRect.left + 56;
			}
		}
		break;
	default:
		break;
	}
}

void ChangeCharacterMotion_2() {//new
	int offsetWidth = 0;
	int offsetHeight = 0;
	static int wait_2 = 1;
	switch (character2.status)
	{
	case LEFT_STAY:
		offsetWidth = 56;
		offsetHeight = 80;
		if ((character2.imgRect.top != 90 && character2.imgRect.left != 0) || character2.imgRect.top != 90) {
			wait_2 = 1;
			character2.imgRect.top = 90;
			character2.imgRect.bottom = character2.imgRect.top + offsetHeight;
			character2.imgRect.left = 0;
			character2.imgRect.right = character2.imgRect.left + offsetWidth;
		}
		else {
			if (wait_2 % 20 == 0) {
				character2.imgRect.left += (offsetWidth);
				character2.imgRect.right += (offsetWidth);
				if (character2.imgRect.left >= 0 + offsetWidth * 4) {
					character2.imgRect.left = 0;
					character2.imgRect.right = character2.imgRect.left + offsetWidth;
				}
				wait_2 = 1;
			}
			else ++wait_2;
		}


		break;
	case RIGHT_STAY:
		offsetWidth = 56;
		offsetHeight = 80;
		if ((character2.imgRect.top != 0 && character2.imgRect.left != 0) || character2.imgRect.top != 0) {
			wait_2 = 1;
			character2.imgRect.top = 0;
			character2.imgRect.bottom = character2.imgRect.top + offsetHeight;
			character2.imgRect.left = 0;
			character2.imgRect.right = character2.imgRect.left + offsetWidth;
		}
		else {
			if (wait_2 % 20 == 0) {
				character2.imgRect.left += (offsetWidth);
				character2.imgRect.right += (offsetWidth);
				if (character2.imgRect.left >= 0 + offsetWidth * 4) {
					character2.imgRect.left = 0;
					character2.imgRect.right = character2.imgRect.left + offsetWidth;
				}
				wait_2 = 1;
			}
			else ++wait_2;
		}
		break;
	case LEFT_RUN:
		offsetWidth = 76;
		offsetHeight = 80;
		if ((character2.imgRect.top != 368 && character2.imgRect.left != 465) || !(465 <= character2.imgRect.left && character2.imgRect.left <= 465 + offsetWidth * 4)) {
			wait_2 = 1;
			character2.imgRect.top = 368;
			character2.imgRect.bottom = character2.imgRect.top + offsetHeight;
			character2.imgRect.left = 465;
			character2.imgRect.right = character2.imgRect.left + offsetWidth;
		}
		else {
			if (wait_2 % 2 == 0) {
				character2.imgRect.left += offsetWidth;
				character2.imgRect.right += offsetWidth;
				if (character2.imgRect.left >= 465 + offsetWidth * 4) {
					character2.imgRect.left = 465;
					character2.imgRect.right = character2.imgRect.left + offsetWidth;
				}
				wait_2 = 1;
			}
			else ++wait_2;
		}
		break;
	case RIGHT_RUN:
		offsetWidth = 76;
		offsetHeight = 80;
		if ((character2.imgRect.top != 270 && character2.imgRect.left != 465) || !(465 <= character2.imgRect.left && character2.imgRect.left <= 465 + offsetWidth * 4)) {
			wait_2 = 1;
			character2.imgRect.top = 270;
			character2.imgRect.bottom = character2.imgRect.top + offsetHeight;
			character2.imgRect.left = 465;
			character2.imgRect.right = character2.imgRect.left + offsetWidth;
		}
		else {
			if (wait_2 % 2 == 0) {
				character2.imgRect.left += offsetWidth;
				character2.imgRect.right += offsetWidth;
				if (character2.imgRect.left >= 465 + offsetWidth * 4) {
					character2.imgRect.left = 465;
					character2.imgRect.right = character2.imgRect.left + offsetWidth;
				}
				wait_2 = 1;
			}
			else ++wait_2;
		}
		break;
	case LEFT_WALK:
		offsetWidth = 64;
		offsetHeight = 80;
		if ((character2.imgRect.top != 367 && character2.imgRect.left != 0) || character2.imgRect.top != 367) {
			wait_2 = 1;
			character2.imgRect.top = 367;
			character2.imgRect.bottom = character2.imgRect.top + offsetHeight;
			character2.imgRect.left = 0;
			character2.imgRect.right = character2.imgRect.left + offsetWidth;
		}
		else {
			if (wait_2 % 3 == 0) {
				character2.imgRect.left += offsetWidth;
				character2.imgRect.right += offsetWidth;
				if (character2.imgRect.left >= 0 + offsetWidth * 4) {
					character2.imgRect.left = 0;
					character2.imgRect.right = character2.imgRect.left + offsetWidth;
				}
				wait_2 = 1;
			}
			else ++wait_2;
		}
		break;
	case RIGHT_WALK:
		offsetWidth = 64;
		offsetHeight = 80;
		if ((character2.imgRect.top != 270 && character2.imgRect.left != 0) || character2.imgRect.top != 270) {
			wait_2 = 1;

			character2.imgRect.top = 270;
			character2.imgRect.bottom = character2.imgRect.top + offsetHeight;
			character2.imgRect.left = 0;
			character2.imgRect.right = character2.imgRect.left + offsetWidth;
		}
		else {
			if (wait_2 % 3 == 0) {
				character2.imgRect.left += offsetWidth;
				character2.imgRect.right += offsetWidth;
				if (character2.imgRect.left >= 0 + offsetWidth * 4) {
					character2.imgRect.left = 0;
					character2.imgRect.right = character2.imgRect.left + offsetWidth;
				}
				wait_2 = 1;
			}
			else ++wait_2;
		}
		break;
	case LEFT_JUMP:
		if ((character2.imgRect.top != 544 && character2.imgRect.left != 264) || character2.imgRect.left != 544) {
			// printf("왼쪽 점프 모션으로 바꿈\n");
			character2.imgRect.top = 544;
			character2.imgRect.bottom = character2.imgRect.top + 80;
			character2.imgRect.left = 264;
			character2.imgRect.right = character2.imgRect.left + 60;
		}
		break;
	case RIGHT_JUMP:
		if ((character2.imgRect.top != 544 && character2.imgRect.left != 0) || character2.imgRect.left != 544) {
			//  printf("오른쪽 점프 모션으로 바꿈\n");
			character2.imgRect.top = 544;
			character2.imgRect.bottom = character2.imgRect.top + 80;
			character2.imgRect.left = 0;
			character2.imgRect.right = character2.imgRect.left + 60;
		}
		break;
	case LADDER_UP:
		offsetWidth = 60;
		offsetHeight = 84;
		if ((character2.imgRect.top != 174 && character2.imgRect.left != 0) || !(0 <= character2.imgRect.left && character2.imgRect.left <= 0 + offsetWidth * 4)) {
			wait_2 = 1;
			character2.imgRect.top = 174;
			character2.imgRect.bottom = character2.imgRect.top + offsetHeight;
			character2.imgRect.left = 0;
			character2.imgRect.right = character2.imgRect.left + offsetWidth;
		}
		else {
			if (bW) {
				if (wait_2 % 3 == 0) {
					character2.imgRect.left += offsetWidth;
					character2.imgRect.right += offsetWidth;
					if (character2.imgRect.left == 0 + offsetWidth * 4) {
						character2.imgRect.left = 0;
						character2.imgRect.right = character2.imgRect.left + offsetWidth;
					}
					wait_2 = 1;
				}
				else ++wait_2;
			}
		}
		break;
	case LADDER_DOWN:
		offsetWidth = 52;
		offsetHeight = 80;
		if (!(character2.imgRect.left == 502 && character2.imgRect.top == 178)) {
			character2.imgRect.top = 178;
			character2.imgRect.bottom = character2.imgRect.top + offsetHeight;
			character2.imgRect.left = 502;
			character2.imgRect.right = character2.imgRect.left + offsetWidth;
		}
		break;
	case SHOCK:
		break;
	case LEFT_FALLING:
		offsetWidth = 68;
		offsetHeight = 68;
		if (!(character2.imgRect.left == 340 && character2.imgRect.right == 736)) {
			character2.imgRect.top = 736;
			character2.imgRect.bottom = character2.imgRect.top + offsetHeight;
			character2.imgRect.left = 340;
			character2.imgRect.right = character2.imgRect.left + offsetWidth;
		}
		break;
	case RIGHT_FALLING:
		offsetWidth = 68;
		offsetHeight = 68;
		if (!(character2.imgRect.left == 0 && character2.imgRect.right == 736)) {
			character2.imgRect.top = 736;
			character2.imgRect.bottom = character2.imgRect.top + offsetHeight;
			character2.imgRect.left = 0;
			character2.imgRect.right = character2.imgRect.left + offsetWidth;
		}
		break;
	case LEFT_STANDING:
		offsetWidth = 76;
		offsetHeight = 64;
		if ((character2.imgRect.top != 652 && character2.imgRect.left != 462) || !(462 <= character2.imgRect.left && character2.imgRect.left <= 462 + offsetWidth * 2)) {
			//  printf("왼쪽 바뀜\n");
			wait_2 = 1;
			character2.imgRect.top = 652;
			character2.imgRect.bottom = character2.imgRect.top + offsetHeight;
			character2.imgRect.left = 462;
			character2.imgRect.right = character2.imgRect.left + offsetWidth;
		}
		else {
			if (wait_2 % 10 == 0) {
				character2.imgRect.left += offsetWidth;
				character2.imgRect.right += offsetWidth;
				if (character2.imgRect.left == 462 + offsetWidth * 2) {
					character2.status = LEFT_STAY;
				}
				wait_2 = 1;
			}
			else ++wait_2;
		}
		break;
	case RIGHT_STANDING:
		offsetWidth = 76;
		offsetHeight = 64;
		if ((character2.imgRect.top != 652 && character2.imgRect.left != 0) || !(0 <= character2.imgRect.left && character2.imgRect.left <= 0 + offsetWidth * 2) || (character2.imgRect.top == 736 && character2.imgRect.left == 0)) {
			//printf("오른쪽 바뀜\n");
			wait_2 = 1;
			character2.imgRect.top = 652;
			character2.imgRect.bottom = character2.imgRect.top + offsetHeight;
			character2.imgRect.left = 0;
			character2.imgRect.right = character2.imgRect.left + offsetWidth;
		}
		else {
			if (wait_2 % 10 == 0) {
				character2.imgRect.left += offsetWidth;
				character2.imgRect.right += offsetWidth;
				if (character2.imgRect.left == 0 + offsetWidth * 2) {
					character2.status = RIGHT_STAY;
				}
				wait_2 = 1;
			}
			else ++wait_2;
		}
		break;
	case LEFT_SIT:
		if (character2.imgRect.top != 106 && character2.imgRect.left != 415) {
			character2.imgRect.top = 106;
			character2.imgRect.bottom = character2.imgRect.top + 64;
			character2.imgRect.left = 415;
			character2.imgRect.right = character2.imgRect.left + 52;
		}
		break;
	case RIGHT_SIT:
		if (character2.imgRect.top != 16 && character2.imgRect.left != 415) {
			character2.imgRect.top = 16;
			character2.imgRect.bottom = character2.imgRect.top + 64;
			character2.imgRect.left = 415;
			character2.imgRect.right = character2.imgRect.left + 52;
		}
		break;
	case LEFT_ROLL:
		offsetWidth = 60;
		offsetHeight = 64;
		if ((character2.imgRect.top != 465 && character2.imgRect.left != 465) || !(465 <= character2.imgRect.left && character2.imgRect.left <= 465 + offsetWidth * 4)) {
			wait_2 = 1;
			character2.imgRect.top = 465;
			character2.imgRect.bottom = character2.imgRect.top + offsetHeight;
			character2.imgRect.left = 465;
			character2.imgRect.right = character2.imgRect.left + offsetWidth;
		}
		else {
			if (wait_2 % 3 == 0) {
				character2.imgRect.left += offsetWidth;
				character2.imgRect.right += offsetWidth;
				if (character2.imgRect.left == 465 + offsetWidth * 4) {
					character2.playerRect.top = character2.playerRect.bottom - CharacterHeight;
					if (character2.usedPortal) {
						character2.usedPortal = false;
					}
					character2.status = LEFT_STAY;
					character2.imgRect.top = 90;
					character2.imgRect.bottom = character2.imgRect.top + offsetHeight;
					character2.imgRect.left = 0;
					character2.imgRect.right = character2.imgRect.left + offsetWidth;
				}
				wait_2 = 1;
			}
			else ++wait_2;
		}
		break;
	case RIGHT_ROLL:
		offsetWidth = 60;
		offsetHeight = 64;
		if ((character2.imgRect.top != 465 && character2.imgRect.left != 0) || !(0 <= character2.imgRect.left && character2.imgRect.left <= 0 + offsetWidth * 4)) {
			wait_2 = 1;
			character2.imgRect.top = 465;
			character2.imgRect.bottom = character2.imgRect.top + offsetHeight;
			character2.imgRect.left = 0;
			character2.imgRect.right = character2.imgRect.left + offsetWidth;
		}
		else {
			if (wait_2 % 3 == 0) {
				character2.imgRect.left += offsetWidth;
				character2.imgRect.right += offsetWidth;
				if (character2.imgRect.left == 0 + offsetWidth * 4) {
					character2.playerRect.top = character2.playerRect.bottom - CharacterHeight;
					if (character2.usedPortal) {
						character2.usedPortal = false;
					}
					character2.status = RIGHT_STAY;
					character2.imgRect.top = 0;
					character2.imgRect.bottom = character2.imgRect.top + offsetHeight;
					character2.imgRect.left = 0;
					character2.imgRect.right = character2.imgRect.left + offsetWidth;
				}
				wait_2 = 1;
			}
			else ++wait_2;
		}
		break;
	case LEFT_AIMING:  //new - asd
		if (IspressedNum3) {//수류탄 처리
			if (character2.imgRect.top != 944 || character2.imgRect.left != 767) {
				character2.imgRect.top = 944;
				character2.imgRect.bottom = character2.imgRect.top + 80;
				character2.imgRect.left = 767;
				character2.imgRect.right = character2.imgRect.left + 52;
			}
		}
		else {//총 화면 처리
			if (character2.imgRect.top != 944 || character2.imgRect.left != 520) {
				character2.imgRect.top = 944;
				character2.imgRect.bottom = character2.imgRect.top + 80;
				character2.imgRect.left = 520;
				character2.imgRect.right = character2.imgRect.left + 68;
			}
		}
		break;
	case RIGHT_AIMING:
		if (IspressedNum3) {//수류탄 처리
			if (character2.imgRect.top != 944 || character2.imgRect.left != 247) {
				character2.imgRect.top = 944;
				character2.imgRect.bottom = character2.imgRect.top + 80;
				character2.imgRect.left = 247;
				character2.imgRect.right = character2.imgRect.left + 52;
			}
		}
		else {//총 화면 처리
			if (character2.imgRect.top != 944 || character2.imgRect.left != 0) {
				character2.imgRect.top = 944;
				character2.imgRect.bottom = character2.imgRect.top + 80;
				character2.imgRect.left = 0;
				character2.imgRect.right = character2.imgRect.left + 68;
			}
		}
		break;
	case LEFT_PUNCH:
		if (character2.hand == eHand::none) {//asd
			offsetWidth = 68;
			offsetHeight = 80;
			if ((character2.imgRect.top != 551 && character2.imgRect.left != 731) || !(731 <= character2.imgRect.left && character2.imgRect.left <= 731 + offsetWidth * 3)) {
				wait_2 = 1;
				character2.imgRect.top = 551;
				character2.imgRect.bottom = character2.imgRect.top + offsetHeight;
				character2.imgRect.left = 731;
				character2.imgRect.right = character2.imgRect.left + offsetWidth;
			}
			else {
				if (wait_2 % 3 == 0) {
					character2.imgRect.left += offsetWidth;
					character2.imgRect.right += offsetWidth;
					if (character2.imgRect.left == 731 + offsetWidth * 3) {
						character2.status = LEFT_STAY;
						character2.imgRect.top = 90;
						character2.imgRect.bottom = character2.imgRect.top + 80;
						character2.imgRect.left = 0;
						character2.imgRect.right = character2.imgRect.left + 56;
					}
					wait_2 = 1;
				}
				else ++wait_2;
			}
		}
		else if (character2.hand == eHand::sword) {
			offsetWidth = 114;
			offsetHeight = 116;
			if ((character2.imgRect.top != 1040 && character2.imgRect.left != 500) || !(500 <= character2.imgRect.left && character2.imgRect.left <= 500 + offsetWidth * 4)) {
				wait_2 = 1;
				character2.imgRect.top = 1040;
				character2.imgRect.bottom = character2.imgRect.top + offsetHeight;
				character2.imgRect.left = 500;
				character2.imgRect.right = character2.imgRect.left + offsetWidth;
			}
			else {
				if (wait_2 % 3 == 0) {
					character2.imgRect.left += offsetWidth;
					character2.imgRect.right += offsetWidth;
					if (character2.imgRect.left == 500 + offsetWidth * 4) {
						character2.status = LEFT_STAY;
						character2.imgRect.top = 90;
						character2.imgRect.bottom = character2.imgRect.top + 80;
						character2.imgRect.left = 0;
						character2.imgRect.right = character2.imgRect.left + 56;
					}
					wait_2 = 1;
				}
				else ++wait_2;
			}
		}
		else if (character2.hand == eHand::axe) {
			offsetWidth = 92;
			offsetHeight = 116;
			if ((character2.imgRect.top != 1194 && character2.imgRect.left != 408) || !(408 <= character2.imgRect.left && character2.imgRect.left <= 408 + offsetWidth * 4)) {
				wait_2 = 1;
				character2.imgRect.top = 1194;
				character2.imgRect.bottom = character2.imgRect.top + offsetHeight;
				character2.imgRect.left = 408;
				character2.imgRect.right = character2.imgRect.left + offsetWidth;
			}
			else {
				if (wait_2 % 3 == 0) {
					character2.imgRect.left += offsetWidth;
					character2.imgRect.right += offsetWidth;
					if (character2.imgRect.left == 408 + offsetWidth * 4) {
						character2.status = LEFT_STAY;
						character2.imgRect.top = 90;
						character2.imgRect.bottom = character2.imgRect.top + 80;
						character2.imgRect.left = 0;
						character2.imgRect.right = character2.imgRect.left + 56;
					}
					wait_2 = 1;
				}
				else ++wait_2;
			}
		}
		break;
	case RIGHT_PUNCH:
		if (character2.hand == eHand::none) {
			offsetWidth = 68;
			offsetHeight = 80;
			if ((character2.imgRect.top != 551 && character2.imgRect.left != 517) || !(517 <= character2.imgRect.left && character2.imgRect.left <= 517 + offsetWidth * 3)
				|| character2.imgRect.top != 551) {
				wait_2 = 1;
				character2.imgRect.top = 551;
				character2.imgRect.bottom = character2.imgRect.top + offsetHeight;
				character2.imgRect.left = 517;
				character2.imgRect.right = character2.imgRect.left + offsetWidth;
			}
			else {
				if (wait_2 % 3 == 0) {
					character2.imgRect.left += offsetWidth;
					character2.imgRect.right += offsetWidth;
					if (character2.imgRect.left == 517 + offsetWidth * 3) {
						character2.status = RIGHT_STAY;
						character2.imgRect.top = 0;
						character2.imgRect.bottom = character2.imgRect.top + 80;
						character2.imgRect.left = 0;
						character2.imgRect.right = character2.imgRect.left + 56;
					}
					wait_2 = 1;
				}
				else ++wait_2;
			}
		}
		else if (character2.hand == eHand::sword) {
			offsetWidth = 114;
			offsetHeight = 116;
			if ((character2.imgRect.top != 1040 && character2.imgRect.left != 0) || !(0 <= character2.imgRect.left && character2.imgRect.left <= 0 + offsetWidth * 4)
				|| character2.imgRect.top != 1040) {
				wait_2 = 1;
				character2.imgRect.top = 1040;
				character2.imgRect.bottom = character2.imgRect.top + offsetHeight;
				character2.imgRect.left = 0;
				character2.imgRect.right = character2.imgRect.left + offsetWidth;
			}
			else {
				if (wait_2 % 3 == 0) {
					character2.imgRect.left += offsetWidth;
					character2.imgRect.right += offsetWidth;
					if (character2.imgRect.left == 0 + offsetWidth * 4) {
						character2.status = RIGHT_STAY;
						character2.imgRect.top = 0;
						character2.imgRect.bottom = character2.imgRect.top + 80;
						character2.imgRect.left = 0;
						character2.imgRect.right = character2.imgRect.left + 56;
					}
					wait_2 = 1;
				}
				else ++wait_2;
			}
		}
		else if (character2.hand == eHand::axe) {
			offsetWidth = 92;
			offsetHeight = 116;
			if ((character2.imgRect.top != 1194 && character2.imgRect.left != 0) || !(0 <= character2.imgRect.left && character2.imgRect.left <= 0 + offsetWidth * 4)
				|| character2.imgRect.top != 1194) {
				wait_2 = 1;
				character2.imgRect.top = 1194;
				character2.imgRect.bottom = character2.imgRect.top + offsetHeight;
				character2.imgRect.left = 0;
				character2.imgRect.right = character2.imgRect.left + offsetWidth;
			}
			else {
				if (wait_2 % 3 == 0) {
					character2.imgRect.left += offsetWidth;
					character2.imgRect.right += offsetWidth;
					if (character2.imgRect.left == 0 + offsetWidth * 4) {
						character2.status = RIGHT_STAY;
						character2.imgRect.top = 0;
						character2.imgRect.bottom = character2.imgRect.top + 80;
						character2.imgRect.left = 0;
						character2.imgRect.right = character2.imgRect.left + 56;
					}
					wait_2 = 1;
				}
				else ++wait_2;
			}
		}
		break;
	case LEFT_SHOCK:
		if (character2.imgRect.top != 844 || character2.imgRect.left != 294) {
			character2.imgRect.top = 844;
			character2.imgRect.bottom = character2.imgRect.top + 76;
			character2.imgRect.left = 294;
			character2.imgRect.right = character2.imgRect.left + 60;
		}
		else {
			++wait_2;
			if (wait_2 % 10 == 0) {
				character2.status = LEFT_STAY;
				character2.imgRect.top = 90;
				character2.imgRect.bottom = character2.imgRect.top + 80;
				character2.imgRect.left = 0;
				character2.imgRect.right = character2.imgRect.left + 56;
			}
		}
		break;
	case RIGHT_SHOCK:
		if (character2.imgRect.top != 844 || character2.imgRect.left != 0) {
			wait_2 = 1;
			character2.imgRect.top = 844;
			character2.imgRect.bottom = character2.imgRect.top + 76;
			character2.imgRect.left = 0;
			character2.imgRect.right = character2.imgRect.left + 60;
		}
		else {
			++wait_2;
			if (wait_2 % 10 == 0) {
				character2.status = RIGHT_STAY;
				character2.imgRect.top = 0;
				character2.imgRect.bottom = character2.imgRect.top + 80;
				character2.imgRect.left = 0;
				character2.imgRect.right = character2.imgRect.left + 56;
			}
		}
		break;
	default:
		break;
	}
}