#include <iostream>
#include <vector>
#include <string>
#include <conio.h>
#include <windows.h>
#include <algorithm>
using namespace std;

const int N = 10;
const char WALL = '#';
const char EMPTY = ' ';
const char PLAYER = 'P';
const char LAVAED_PLAYER='p';
const char DOOR = 'D';
const char LAVA = 'L';
const char STAIRS_to_1 = '1';
const char STAIRS_to_2 = '2';
const char STAIRS_to_3 = '3';
const char SMALL_HP = 'h';
const char LARGE_HP = 'H';
const char KEY = 'K';
const char ARMOR = 'A';
const char WEAPON = 'W';
const char GEM = 'G';
const char SLIME = 's';
const char SKELETON = 'k';
const char BAT = 'b';
const char APOSTLE = 'a';
const char BOSS = 'M';
const char BONE = 'U';//实验性功能，重开后有尸骨，目前暂且关闭
const int LEVELS = 3; // 楼层数量
int current_level = 0; // 当前楼层


struct Monster {
    string name;
    int hp;
    int atk;
    int def;
    int score;
    string special;
    int count;
    bool reborn;
};


struct Player {
    int x, y;
    int hp = 50;
    int max_hp = 50;
    int atk = 20;
    int def = 5;
    int score = 0;
    int keys = 0;
};

Player player;
vector<vector<char>> previous_map(N, vector<char>(N, EMPTY));
vector<vector<vector<char>>> maps(LEVELS, vector<vector<char>>(N, vector<char>(N, EMPTY)));
vector<Monster> monsters = {
    {"史莱姆酱", 10, 5, 5, 5, "【毒液】：怪物额外造成(玩家当前血量的20%+5)的高额伤害", 0,false},
    {"骷髅士兵", 16, 8, 8, 8, "【不死族】：首次被击杀后复活并拥有初始状态25%的血量", 0,true},
    {"吸血蝙蝠", 25, 15, 0, 10, "【吸血】：怪物可以吸收造成直接伤害大小的血量", 0,false},
    {"深渊使徒", 50, 30, 15, 25, "【精神污染】：战斗进行每三回合，勇士会受干扰，停滞一次无法攻击，期间仍可以受到伤害", 0,false},
    {"魔王", 100, 25, 15, 100, "【统御之力】：魔王拥有所有怪物的全部特性，除非相应的怪物全部死亡", 1,true}
};
vector<Monster> monsters_Copy = monsters;


// 设置光标位置
void set_cursor_position(int x, int y) {
    COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// 初始化地图
void initialize_map() {
    for (int level = 0; level < LEVELS; ++level) {
        for (int i = 0; i < N; ++i) {
            maps[level][0][i] = maps[level][N - 1][i] = WALL;
            maps[level][i][0] = maps[level][i][N - 1] = WALL;
        }
    }

    // 初始化第一层
    maps[0][1][1] = PLAYER;
    player.x = 1;
    player.y = 1;
    maps[0][1][4] = SLIME;
    maps[0][1][7] = SLIME;
    maps[0][1][8] = KEY;
    maps[0][2][8] = SLIME;
    maps[0][3][6] = SLIME;
    for (int i = 5;i <= 8;i++) maps[0][5][i] = LAVA;
    for (int i = 5;i <= 7;i++) maps[0][i][5] = LAVA;
    maps[0][6][8] = STAIRS_to_2;
    maps[0][7][1] = SKELETON;
    maps[0][8][2] = SKELETON;
    maps[0][8][1] = KEY;
    maps[0][8][5] = DOOR;

    // 初始化第二层
    maps[1][1][2] = STAIRS_to_3;
    maps[1][6][7] = STAIRS_to_1;
    maps[1][2][6] = SKELETON;
    maps[1][3][3] = SKELETON;
    maps[1][4][1] = BAT;
    maps[1][7][5] = BAT;
    for (int i = 6;i <= 8;i++) maps[1][7][i] = WALL;
    maps[1][8] = {WALL, EMPTY,EMPTY,EMPTY,BAT,DOOR,WEAPON,ARMOR,GEM,WALL };

    // 初始化第三层（可拓展）
    maps[2][1][1] = STAIRS_to_2;
    
    maps[2][1][8] = BAT;
    for (int i = 3;i <= 6;i++) maps[2][i] = {WALL, LAVA,LAVA,WALL,LAVA,LAVA,WALL,LAVA,LAVA ,WALL};
    maps[2][3][4] = DOOR;
    maps[2][3][5] = DOOR;
    maps[2][5][4] = APOSTLE;
    maps[2][5][5] = APOSTLE;
    maps[2][7] = {WALL,LAVA,LAVA,LAVA,LAVA,WEAPON,LARGE_HP,LAVA,LAVA,WALL };
    maps[2][8] = {WALL, LAVA,LAVA,LAVA,LAVA,APOSTLE,EMPTY,EMPTY,BOSS,WALL };

    for (int i = 0; i < monsters.size(); ++i) {
        monsters[i].count=0;
        monsters_Copy[i].count = 0;
    }
    for (int level = 0; level < LEVELS; ++level) {
        for (int i = 1; i < N-1; ++i) {
            for (int j = 1;j < N - 1;j++) {
                if (maps[level][i][j] == SLIME) {
                    monsters[0].count += 1;
                    monsters_Copy[0].count += 1;
                }
                else if (maps[level][i][j] == SKELETON) {
                    monsters[1].count += 1;
                    monsters_Copy[1].count += 1;
                }
                else if (maps[level][i][j] == BAT) {
                    monsters[2].count += 1;
                    monsters_Copy[2].count += 1;
                }
                else if (maps[level][i][j] == APOSTLE) {
                    monsters[3].count += 1;
                    monsters_Copy[3].count += 1;
                }
            }
        }
    }
}

// 绘制地图，仅更新变化的部分
    void display_map() {
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                if (maps[current_level][i][j] != previous_map[i][j]) {
                    set_cursor_position(j * 2, i);
                    cout << maps[current_level][i][j];
                    previous_map[i][j] = maps[current_level][i][j];
                }
            }
        }
        set_cursor_position(0, N + 1);
        cout << "Floor: " << current_level + 1 << " HP: " << player.hp << "/" << player.max_hp
            << " ATK: " << player.atk
            << " DEF: " << player.def
            << " SCORE: " << player.score
            << " KEYS: " << player.keys 
            << "         " << endl;
        cout << " #:墙; P:玩家; D:门; L:岩浆; 1/2/3:到第i层的楼梯; \n h:小血瓶; H:大血瓶; K:钥匙; A:板甲; W:铁剑; G:生命之石; \n s:史莱姆酱; k:骷髅士兵; b:吸血蝙蝠; a:深渊使徒; M:魔王(拥有未死亡怪物的全部属性！);" << endl;
        fflush(stdout);
    }

    void reset_game() {
        // 重置玩家属性
        player = { 1, 1, 50, 50, 20, 5, 0, 0 };
        current_level = 0;

        // 重置地图和怪物数据
        initialize_map();
        for (int i = 0; i < monsters.size(); ++i) {
            monsters[i] = monsters_Copy[i];
        }
        fill(previous_map.begin(), previous_map.end(), vector<char>(N, EMPTY));
    }

    void display_monster_manual() {
        system("cls"); // 清屏，展示怪物手册
        cout << "怪物手册 (按Z返回游戏):\n";
        for (int i = 0;i < 5;i++) {
            cout << "名字：" << monsters_Copy[i].name << endl;
            cout << "生命值：" << monsters_Copy[i].hp << endl;
            cout << "防御力：" << monsters_Copy[i].def << endl;
            cout << "特殊属性：" << (monsters_Copy[i].special.empty() ? "None" : monsters_Copy[i].special) << endl;
            cout << "击杀得分：" << monsters_Copy[i].score << endl;
            cout << "目前数量：" << monsters[i].count << endl;
            cout << endl;
        }
        cout << "血瓶：有两种血瓶，小血瓶增加血量 10，大血瓶增加血量 25，加血不可超过血量上限。\n钥匙：简化版游戏只有一种大门，因此也只需要设计一种钥匙即可。\n防具：“板甲”，防御力增加 10。 武器：“铁剑”，攻击力增加 20。\n宝石：“生命之石”，增加血量50。\n岩浆：经过造成10点真实伤害。\n\n";
        cout << "Press Z to return to the game." << endl;

        // 等待用户按 Z 返回
        char input;
        do {
            input = _getch();
        } while (input != 'Z' && input != 'z');

        system("cls"); // 清屏，返回游戏
        fill(previous_map.begin(), previous_map.end(), vector<char>(N, EMPTY));
        display_map();
    }

    extern void game_loop();

    void are_dead() {
        set_cursor_position(0, N +20);
        for (int i = 0;i < 10;i++) cout << string(50, ' ') << endl; // 清除上次战斗记录
        maps[current_level][player.x][player.y] = EMPTY;
        cout << "胜败乃兵家常事，大侠请重新来过\n";
        cout << "总分：" << player.score << "\n";
        cout << "Press R to restart or Q to quit.\n";

        char input;
        while (true) {
            input = _getch();
            if (input == 'R' || input == 'r') {
                system("cls"); // 清屏
                reset_game();
                game_loop(); // 重新开始游戏
                break;
            }
            else if (input == 'Q' || input == 'q') {
                exit(0); // 退出游戏
            }
        }
    }

void move_away() {
    if (maps[current_level][player.x][player.y] == LAVAED_PLAYER) maps[current_level][player.x][player.y] = LAVA;
    else maps[current_level][player.x][player.y] = EMPTY;
}

void battle(int mtype) {
    set_cursor_position(0, N + 6);
    for (int i = 0;i < 10;i++) cout << string(50, ' ') << endl; // 清除上次战斗记录
    int turns = 0;
    int help_turns = 0;
    while (min(player.hp, monsters[mtype].hp) > 0) {
        if (turns % 2 == 0) {
            if (not(mtype == 3 && turns % 3 == 0 && turns != 0)) monsters[mtype].hp -= max(0, player.atk - monsters[mtype].def);
            else {
                cout << monsters[mtype].name << "触发了额外效果：精神污染！" << endl;
                help_turns++;
            }
        }
        else {
            if (mtype == 2)
            {
                monsters[mtype].hp += max(0, monsters[mtype].atk - player.def);
                cout << monsters[mtype].name << "触发了额外效果：吸血！" << endl;
                help_turns++;
            }
            if (mtype == 0) {
                player.hp -= (player.hp / 5+5);
                cout << monsters[mtype].name << "触发了额外效果：毒液！" << endl;
                help_turns++;
            }
            player.hp -= max(0, monsters[mtype].atk - player.def);
        }
        if (monsters[mtype].hp <= 0) {
            if (monsters[mtype].reborn == true)
            {
                monsters[mtype].hp = monsters_Copy[mtype].hp * 0.2;
                monsters[mtype].reborn = false;
                cout << monsters[mtype].name << "触发了额外效果：不死族！" << endl;
                help_turns++;
            }
            else monsters[mtype].hp = 0;
        }
        if (player.hp < 0) player.hp = 0;

        set_cursor_position(0, N + (help_turns++) + 6);
        cout << "第" << ++turns << "回合：" << "你的血量： " << player.hp << "  " << monsters[mtype].name << "的血量：" << monsters[mtype].hp << endl;
    }
}

void move_player(int dx, int dy) {
    int nx = player.x + dx, ny = player.y + dy;
    if (maps[current_level][nx][ny] == WALL) return;
    else if (maps[current_level][nx][ny] == STAIRS_to_1) {
        if (current_level < 0) {
            maps[current_level][player.x][player.y] = EMPTY;
            current_level++;
        }
        else if (current_level > 0) {
            maps[current_level][player.x][player.y] = EMPTY;
            current_level--;
        }
        player.x = nx;
        player.y = ny;
        maps[current_level][nx][ny] = PLAYER;
    }
    else if (maps[current_level][nx][ny] == STAIRS_to_2) {
        if (current_level < 1) {
            maps[current_level][player.x][player.y] = EMPTY;
            current_level++;
        }
        else if (current_level > 1) {
            maps[current_level][player.x][player.y] = EMPTY;
            current_level--;
        }
        player.x = nx;
        player.y = ny;
        maps[current_level][nx][ny] = PLAYER;
    }
    else if (maps[current_level][nx][ny] == STAIRS_to_3) {
        if (current_level < 2) {
            maps[current_level][player.x][player.y] = EMPTY;
            current_level++;
        }
        else if (current_level > 2) {
            maps[current_level][player.x][player.y] = EMPTY;
            current_level--;
        }
        player.x = nx;
        player.y = ny;
        maps[current_level][nx][ny] = PLAYER;
    }
    else if (maps[current_level][nx][ny] == EMPTY || maps[current_level][nx][ny] == LAVA) {
        move_away();
        player.x = nx;
        player.y = ny;
        const char temp = maps[current_level][nx][ny];
        maps[current_level][nx][ny] = PLAYER;
        if (temp == LAVA) {
            player.hp = max(player.hp-10,0);
            maps[current_level][nx][ny] = LAVAED_PLAYER;
        }
    }
    else if (maps[current_level][nx][ny] == SMALL_HP) {
        player.hp = min(player.hp + 10, player.max_hp);
        maps[current_level][nx][ny] = EMPTY;
    }
    else if (maps[current_level][nx][ny] == LARGE_HP) {
        player.hp = min(player.hp + 25, player.max_hp);
        maps[current_level][nx][ny] = EMPTY;
    }
    else if (maps[current_level][nx][ny] == GEM) {
        player.hp =min(player.hp+50,player.max_hp);
        maps[current_level][nx][ny] = EMPTY;
    }
    else if (maps[current_level][nx][ny] == KEY) {
        player.keys++;
        maps[current_level][nx][ny] = EMPTY;
    }
    else if (maps[current_level][nx][ny] == DOOR) {
        if (player.keys > 0) {
            player.keys--;
            maps[current_level][nx][ny] = EMPTY;
        }
    }
    else if (maps[current_level][nx][ny] == WEAPON) {
        if (player.atk < 30) player.atk += 10;//第一把刀20伤害
        player.atk += 10;//第二把刀10伤害
        maps[current_level][nx][ny] = EMPTY;
    }
    else if (maps[current_level][nx][ny] == ARMOR) {
        player.def += 10;
        maps[current_level][nx][ny] = EMPTY;
    }
    else if ((maps[current_level][nx][ny] == SLIME) || (maps[current_level][nx][ny] == SKELETON) || (maps[current_level][nx][ny] == BAT) || (maps[current_level][nx][ny] == APOSTLE)) {
        int mtype = 0;
        switch (maps[current_level][nx][ny]) {
            case SLIME:mtype = 0;break;
            case SKELETON:mtype = 1;break;
            case BAT:mtype = 2;break;
            case APOSTLE:mtype = 3;break;
        }
        monsters[mtype] = monsters_Copy[mtype];//可复用性
        int turns=0;
        battle(mtype);
        if (player.hp <= 0) are_dead();
        else {
            player.score += monsters[mtype].score;
            monsters[mtype].count--;
            monsters_Copy[mtype].count--;
            maps[current_level][nx][ny] = EMPTY;
        }
    }
    else if (maps[current_level][nx][ny] == BOSS) {
        set_cursor_position(0, N + 6);
        for (int i = 0;i < 10;i++) cout << string(50, ' ') << endl; // 清除上次战斗记录
        int mtype = 4;
        monsters[mtype] = monsters_Copy[mtype];
        int turns = 0;
        int help_turns = 0;
        if (monsters[1].count == 0) monsters[mtype].reborn = false;
        while (min(player.hp, monsters[mtype].hp) > 0) {
            if (turns % 2 == 0) {
                if (not(monsters[3].count>0 && turns % 3 == 0 && turns!=0)) monsters[mtype].hp -= max(0, player.atk - monsters[mtype].def);
                else {
                    cout << monsters[mtype].name << "触发了额外效果：精神污染！" << endl;
                    help_turns++;
                }
            }
            else {
                if (monsters[2].count > 0)
                {
                    monsters[mtype].hp += max(0, monsters[mtype].atk - player.def);
                    cout << monsters[mtype].name << "触发了额外效果：吸血！" << endl;
                    help_turns++;
                }
                if (monsters[0].count > 0) {
                    player.hp -= (player.hp / 5+5);
                    cout << monsters[mtype].name << "触发了额外效果：毒液！" << endl;
                    help_turns++;
                }
                player.hp -= max(0, monsters[mtype].atk - player.def);
            }
            if (monsters[mtype].hp <= 0) {
                if (monsters[mtype].reborn == true)
                {
                    monsters[mtype].hp = monsters_Copy[mtype].hp * 0.2;
                    monsters[mtype].reborn = false;
                    cout << monsters[mtype].name << "触发了额外效果：不死族！" << endl;
                    help_turns++;
                }
                else monsters[mtype].hp = 0;
            }
            if (player.hp < 0) player.hp = 0;
            set_cursor_position(0, N + (help_turns++) + 6);
            cout << "第" << ++turns << "回合：" << "你的血量： " << player.hp << "  " << monsters[mtype].name << "的血量：" << monsters[mtype].hp << endl;
        }
        if (player.hp <= 0) are_dead();
        else {
            player.score += monsters[mtype].score;
            maps[current_level][nx][ny] = EMPTY;
            monsters[mtype].count--;
            monsters_Copy[mtype].count--;
            cout << "你击败了魔王，你赢了！\n";
            cout << "Press R to restart or Q to quit.\n";
            char input;
            while (true) {
                input = _getch();
                if (input == 'R' || input == 'r') {
                    system("cls"); // 清屏
                    reset_game();
                    game_loop(); // 重新开始游戏
                    break;
                }
                else if (input == 'Q' || input == 'q') {
                    exit(0); // 退出游戏
                }
            }
        }
    }
}

// 游戏主循环
void game_loop() {
    initialize_map();
    system("cls"); // 清屏，防止闪烁
    display_map();

    while (player.hp > 0) {
        if (_kbhit()) {
            int input = _getch();
            if (input == 'X' || input == 'x') {
                display_monster_manual();
            }
            else if (input == 224) { // 方向键
                char arrow_key = _getch();
                switch (arrow_key) {
                case 72: move_player(-1, 0); break; // 上箭头
                case 80: move_player(1, 0); break;  // 下箭头
                case 75: move_player(0, -1); break; // 左箭头
                case 77: move_player(0, 1); break;  // 右箭头
                }
            }
            else {
                switch (input) {
                case 'W': case 'w': move_player(-1, 0); break;
                case 'S': case 's': move_player(1, 0); break;
                case 'A': case 'a': move_player(0, -1); break;
                case 'D': case 'd': move_player(0, 1); break;
                }
            }
            display_map();
        }
    }

    are_dead();
}

int main() {
    game_loop();
    return 0;
}
