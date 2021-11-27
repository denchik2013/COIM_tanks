#include <iostream>
#include <cstring>
#include <windows.h>
#include <ctime>

using namespace std;

#define width 80
#define height 25

#define field ' '
#define fbrick 176
#define fstone 206

#define duloW 179
#define duloH 205
#define tankC 219
#define catter '#'

        
typedef char mapHW[height][width];

void SetCurPos(int x, int y){
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

bool IsCross(RECT a, RECT b){
	return (a.right >= b.left) && (a.left <= b.right)
		&& (a.bottom >= b.top) && (a.top <= b.bottom);
};

struct Tmap {
	mapHW map;//Сама карта
	void Clear() { memset(map, field, sizeof(map)-1);}//заповнюємо карту зміною field
	void SetEnd() { map[height-1][width-1] = '\0'; }//вставляє в масив карти символ кінця стрічки
	void Show() { SetCurPos(0,0); SetEnd(); cout << map[0]; }//показує карту в консолі
};

enum Tdir {Rup = 0, Rdown, Rleft, Rright};//зміні для напрямку танку
POINT dirInc[] = {{0,-1},{0,1},{-1,0},{1,0}};//зміна кординат

//Клас танка
class Ttank {
	int x,y;
	int sX,sY;// startX, startY
public:
	Tdir dir;// куда дивиться танк
	Ttank(int startX, int startY)// Конструктор передаємо початкові координати
		{ dir = Rup; sX = startX; sY = startY; SetToStart(); }
	void Show(mapHW &map);//відображення танка
	void Move(char w, char s, char a, char d, char fire);//рух танка
	void SetToStart() { x = sX; y = sY; }// вертає танк на початок
	bool IsHoriz() { return (dir == Rright || dir == Rleft); }// ф-ція що танк дивиться по горизонталі
	RECT GetRect() { RECT r = {x-1, y-1, x+1, y+1}; return r; }// повертає область де знаходиться танк
};

enum Tmatter {ttStone, ttBrick};

class Tbrick{
	RECT rct;//область що займає приграда
public:
	bool use;
	Tmatter tp;// тип пригради
	Tbrick() { use = 0; tp = ttBrick; }// по замовчувані кірпіч
	void Show(mapHW &map);
	void SetPos(int px, int py) { RECT r = {px-1, py-1, px+1, py+1}; rct = r; use = 1; }//поміщаємо приграду в певту точку
	RECT GetRect() { return rct; }
};

class Tpula {
    int x,y;
    int speed;
    Tdir dir;
public:
    bool use;
    Tpula() { use = 0; speed = 5; }
    void SetPula(int px, int py, Tdir pdir)//створює пулу в заданій точці і задає напрямок
    { x = px; y = py; dir = pdir; use = 1; }
    void Move();
    void Show(mapHW &map) { if (!use) return; map[y][x] = '*'; }//відображаємо пулю якщо вона активна
};

Tmap scr;
#define tankCnt 2
Ttank tank[tankCnt] = { Ttank(1, 11), Ttank(78, 11) };
#define brickCnt 300
Tbrick brick[brickCnt];
#define pulaCnt 100
Tpula pula[pulaCnt];

Tpula &GetFreePula()
{
    for (int i = 0; i < pulaCnt; i++)
        if (!pula[i].use) return pula[i];
    return pula[0];
}

Ttank *CheckCrossAnyTank(RECT rct, Ttank *eccept){//область для провірки та танк який не потрібно провіряти
	for (int i = 0; i < tankCnt; i++)//проходим через усі танки 
		if ((eccept != tank+i) &&//крім того що не потрібно провіряти
		(IsCross(rct, tank[i].GetRect())) )//провіряємо перетинання танку
		return tank + i;// якщо вони перетикаються то повертаємо указатєль
	return 0;
}

Tbrick *CheckCrossAnyBrick(RECT rct){
	for (int i = 0; i < brickCnt; i++)
		if (brick[i].use && // провіряємо чи приграда активна
			IsCross(rct, brick[i].GetRect()) )// провіряємо чи танк столкнувся
			return brick + i;// повертаємо об'єкт 
	return 0;
}

void Ttank::Show(mapHW &map){
	if (IsHoriz())
		for (int i = -1; i < 2; map[y-1][x+i] = map[y+1][x+i] = catter, i++);//відмальовка гусинець горизонтально
	else
		for (int i = -1; i <2; map[y+i][x-1] = map[y+i][x+1] = catter, i++);//відмальовка гусенець вертикально
	map[y][x] = tankC;// відмальовка танка 
	POINT dt = dirInc[dir];//берем напрямлення танка
	map[y + dt.y][x + dt.x] = IsHoriz() ? duloH : duloW;// малюємо в цьому місці дуло
}

RECT area = {2,2, width-3, height-3};

void Ttank::Move(char w, char s, char a, char d, char fire){
	char wsad[4] = {w,s,a,d};
	for (int i = 0; i < 4; i++)//обробляєм кнопки
		if (GetKeyState(wsad[i]) < 0) dir = (Tdir)i;//провірка чи кнопка нажата і передаємо її в направлення
	POINT pt = dirInc[dir];
	Ttank old = *this;// указує на тимчасовий об'єкт
	x += pt.x;
	y += pt.y;
	// сохраряємо дані
	if (!IsCross(area, GetRect()) ||
		(CheckCrossAnyTank(GetRect(), this) != 0) ||
		(CheckCrossAnyBrick(GetRect()) != 0))
		*this = old;
	// провіряємо чи танк не перетнув границі карти якщо перетнув повертаємо минулі координати
	if (GetKeyState(fire) < 0)
		GetFreePula().SetPula(x + pt.x*2, y + pt.y*2, dir);
}

void Tbrick::Show(mapHW &map){
	if (!use) return;//якщо не використовується то не показуємо
	for (int i = rct.left; i <= rct.right; i++)//відображаємо їх
		for (int j = rct.top; j <= rct.bottom; j++)//
			if (tp == ttBrick)//
				map[j][i] = fbrick;//
			else//
				map[j][i] = fstone;//
}

RECT areaPula = {0,0, width-1,height-1};

void Tpula::Move(){
	if (!use) return;
	for (int i = 1; i < speed; i++){
		x += dirInc[dir].x;
		y += dirInc[dir].y;
		RECT rct = {x,y,x,y};
		if (!IsCross(rct, areaPula)) use = 0;
		
		Tbrick *brick = CheckCrossAnyBrick(rct);
		if (brick) use = 0, brick->use = (brick->tp == ttStone);//видаляє пулю якщо вона кудас попало і видаляємо куда попало якщо це не камінь
		
		Ttank *tank = CheckCrossAnyTank(rct, 0);
		if (tank) use = 0, tank->SetToStart();
		
		if (!use) return;
	}
}

void CreateBattleField(){
	int pos = 0;
	for (int i = 5; i < width-5; i += 3)
		for (int j = 1; j < height-1; j += 3){
			//проходим всю карту
			brick[pos].SetPos(i, j);//ставим блок кожних 3 клтнк
			if (rand() % 5 == 0) brick[pos].tp = ttStone;//шанс що це камінь
			if (rand() % 5 == 0) brick[pos].use = 0;//шанс чи там буде блок
			pos++;
		}
}

int main() {
	HWND console = GetConsoleWindow();
    RECT r;
    GetWindowRect(console, &r);

    MoveWindow(console, r.left, r.top, 620, 440, TRUE);//фіксуємо вікно

	srand(time(0));
	CreateBattleField();
	do{
		tank[0].Move('W','S','A','D', VK_SPACE);
		tank[1].Move(38,40,37,39, 13);
		for (int i = 0; i < pulaCnt; pula[i++].Move());
		
		scr.Clear();
		for (int i = 0; i < brickCnt; brick[i++].Show(scr.map));
		for (int i =0; i < tankCnt; tank[i++].Show(scr.map));
		for (int i =0; i < pulaCnt; pula[i++].Show(scr.map));
		scr.Show();
		
		Sleep(100);
	}
	while (GetKeyState(VK_ESCAPE) >= 0);
	 
        
	return 0;
	
	
};
