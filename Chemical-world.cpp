#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>//一个一个导入!
#include <iostream>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <random>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <ctime>
#include <map>
#include <iomanip>
#include <vector> //依旧 
#include <string>
#include <thread> //多线程 
#include <chrono>
#include "json.hpp"
#include "cfs.h"
#include <functional>

using json = nlohmann::json;
namespace fs = std::filesystem;

//本游戏内容会非常庞大，所以存档系统必须写……
//但是存档系统需要很多麻烦的东西，比如sqite和nlohmann json,不过可以学嘛，总之需要把代码写的灵活一点，防止根本不知道怎么改
//对了，不知道为什么，我的代码报错了，什么abort()
//……
//实在不行考虑考虑……sqlite?

using namespace std;

#ifdef _WIN32
#include<mmsystem.h>
#pragma comment(lib,"winmm.lib")
#endif


#define BLACK 0
#define DARK_BLUE 1
#define DARK_GREEN 2
#define DARK_CYAN 3 //这个其实是青色的深色版本，别被名字骗了 
#define DARK_RED 4
#define DARK_PURPLE 5
#define DARK_YELLOW 6
#define RESET 7
#define GREY 8
#define BLUE 9
#define GREEN 10
#define CYAN 11 //说是青色，其实是……淡浅绿色 
#define RED 12
#define PURPLE 13
#define YELLOW 14
#define LIGHT_WHITE 15

// ======================== 编码转换助手 ========================

class EncodingHelper {
public:
    // GBK 转 UTF-8（保存时用）
    static string gbkToUtf8(const string& gbkStr) {
        if (gbkStr.empty()) return "";

        int len = MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, NULL, 0);
        if (len == 0) return gbkStr;

        wstring wstr(len, L'\0');
        MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, &wstr[0], len);

        len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
        if (len == 0) return gbkStr;

        string utf8Str(len, '\0');
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &utf8Str[0], len, NULL, NULL);

        // 去掉末尾的 '\0'
        if (!utf8Str.empty() && utf8Str.back() == '\0') {
            utf8Str.pop_back();
        }

        return utf8Str;
    }

    // UTF-8 转 GBK（显示用）
    static string utf8ToGbk(const string& utf8Str) {
        if (utf8Str.empty()) return "";

        int len = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
        if (len == 0) return utf8Str;

        wstring wstr(len, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, &wstr[0], len);

        len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
        if (len == 0) return utf8Str;

        string gbkStr(len, '\0');
        WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &gbkStr[0], len, NULL, NULL);

        if (!gbkStr.empty() && gbkStr.back() == '\0') {
            gbkStr.pop_back();
        }

        return gbkStr;
    }

    // 清理字符串，确保是有效的 UTF-8
    static string sanitizeUtf8(const string& str) {
        string result;
        result.reserve(str.length());

        for (size_t i = 0; i < str.length(); ) {
            unsigned char c = str[i];

            // ASCII 直接保留
            if (c < 0x80) {
                if (c >= 0x20 || c == '\n' || c == '\r' || c == '\t') {
                    result += c;
                }
                i++;
            }
            // 2字节 UTF-8
            else if ((c & 0xE0) == 0xC0 && i + 1 < str.length()) {
                if ((str[i + 1] & 0xC0) == 0x80) {
                    result += str.substr(i, 2);
                    i += 2;
                }
                else {
                    result += '?';
                    i++;
                }
            }
            // 3字节 UTF-8
            else if ((c & 0xF0) == 0xE0 && i + 2 < str.length()) {
                if ((str[i + 1] & 0xC0) == 0x80 && (str[i + 2] & 0xC0) == 0x80) {
                    result += str.substr(i, 3);
                    i += 3;
                }
                else {
                    result += '?';
                    i++;
                }
            }
            // 4字节 UTF-8
            else if ((c & 0xF8) == 0xF0 && i + 3 < str.length()) {
                if ((str[i + 1] & 0xC0) == 0x80 && (str[i + 2] & 0xC0) == 0x80 && (str[i + 3] & 0xC0) == 0x80) {
                    result += str.substr(i, 4);
                    i += 4;
                }
                else {
                    result += '?';
                    i++;
                }
            }
            // 非法字节
            else {
                result += '?';
                i++;
            }
        }

        return result;
    }
};


inline void cls() {
	system("cls");
}
inline void delay(int ms) {
	Sleep(ms);
}
inline void setcolor(int color) {
#ifdef _WIN32
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(h, color);
#endif
}

inline void typewriter(const string& text, int ms = 30) {
	for (char c : text) {
		cout << c << flush;
		delay(ms);
	}
}

//开场动画，帅到爆炸
void cppwelcome() {
	setcolor(DARK_GREEN);
	typewriter("#include<bits/stdc++.h>\n", 40);
	delay(100);
	typewriter("using namespace std;\n", 40);
	delay(150);
	typewriter("int main(){\n", 40);
	delay(88);
	typewriter("	cout<<''hello wrold'';\n", 40);
	delay(200);
	typewriter("	cout<<''Hello,player!'';\n", 40);
	delay(100);
	typewriter("	string c='' WELCOME TO CHEMICAL-WORLD!'';\n",40);
	delay(88);
	typewriter("	//infinite、creative！;\n", 40);
	delay(150);
	typewriter("	return 0;\n", 40);
	delay(300);
	typewriter("}\n",40);
	delay(2000);
	setcolor(RESET);
}

void showlogo() {
	setcolor(CYAN);
	cout << R"( ╔════════════════════════════════════════════════════════════════════════════╗
 ║                                                                            ║
 ║     ██████╗██╗  ██╗███████╗███╗   ███╗██╗ ██████╗ █████╗ ██╗               ║
 ║    ██╔════╝██║  ██║██╔════╝████╗ ████║██║██╔════╝██╔══██╗██║               ║
 ║    ██║     ███████║█████╗  ██╔████╔██║██║██║     ███████║██║               ║
 ║    ██║     ██╔══██║██╔══╝  ██║╚██╔╝██║██║██║     ██╔══██║██║               ║
 ║    ╚██████╗██║  ██║███████╗██║ ╚═╝ ██║██║╚██████╗██║  ██║███████╗          ║ 
 ║     ╚═════╝╚═╝  ╚═╝╚══════╝╚═╝     ╚═╝╚═╝ ╚═════╝╚═╝  ╚═╝╚══════╝          ║
 ║                                                                            ║
 ║                        ██╗    ██╗ ██████╗ ██████╗ ██ ╗                     ║
 ║                        ██║    ██║██╔═══██╗██╔══██╗██║                      ║
 ║                        ██║ █╗ ██║██║   ██║██████╔╝██║                      ║
 ║                        ██║███╗██║██║   ██║██╔══██╗██║                      ║
 ║                        ╚███╔███╔╝╚██████╔╝██║  ██║███████╗                 ║ 
 ║                         ╚══╝╚══╝  ╚═════╝ ╚═╝  ╚═╝╚══════╝                 ║
 ║                                                                            ║
 ║                     ╔══════════════════════════════════════╗               ║
 ║                     ║   从 原 子 到 星 辰 ， 一 手 掌 控   ║               ║
 ║                     ╚══════════════════════════════════════╝               ║
 ║                                                                            ║
 ╚════════════════════════════════════════════════════════════════════════════╝)";
	delay(5000);

	cls();
	setcolor(RED);
	cout << R"(╔════════════════════════════════════════════════════════════════════════════╗
║                                                                            ║
║                                    .&@&&&&@,.                              ║
║                              ,&@@@(/////(#@@@#.                            ║
║                           .@@@/(/////////////#@@.                          ║
║                         .@@(/////////&@&//////(@@.                         ║
║                        @@/,,,,,,,,,,,,,,,,,,,,,*@@                         ║
║                      .@#,,,,,,,,,,,,,,,,,,,,,,,,(@.                        ║
║                     .@/,,,,,,,,,,,,,,,,,,,,,,,,,,,@                        ║
║                     @&,,,,,,,,,,,,,,,,,,,,,,,,,,,,&@                       ║
║                    @@,,,,,,,,,,,,,,,,,,,,,,,,,,,,,%@                       ║
║      ██████╗       @&,,,,,,,,,,,,,,,,,,,,,,,,,,,,,&@     ██████╗           ║
║     ██╔════╝       @@,,,,,,,,,,,,,,,,,,,,,,,,,,,,,%@    ██╔═████╗          ║
║     ██║            @&,,,,,,,,,,,,,,,,,,,,,,,,,,,,&@    ██║██╔██║           ║
║     ██║            &@,,,,,,,,,,,,,,,,,,,,,,,,,,,%@     ████╔╝██║           ║
║     ╚██████╗       .@/,,,,,,,,,,,,,,,,,,,,,,,,,#@      ╚██████╔╝           ║
║      ╚═════╝        (@,,,,,,,,,,,,,,,,,,,,,,,,,&@        ╚═════╝           ║
║                      %@,,,,,,,,,,,,,,,,,,,,,,,@&                           ║
║                       @&,,,,,,,,,,,,,,,,,,,,,%@                            ║
║                        @@*,,,,,,,,,,,,,,,,,#@&                             ║
║                         .@@/,,,,,,,,,,,,,#@@                               ║
║                           ,@@@(,,,,,,(&@@&                                 ║
║                              ,#@@@@@@&*                                    ║
║                                                                            ║
║                    ╔══════════════════════════════════════╗                ║
║                    ║  工 业  |  化 学  |  星 际  |  无 限 ║                ║
║                    ╚══════════════════════════════════════╝                ║
║                                                                            ║
╚════════════════════════════════════════════════════════════════════════════╝)";
	delay(3000);

	cls();
	setcolor(GREEN);
	cout << R"(╔════════════════════════════════════════════════════════════════════════════╗
║                                                                            ║
║                                    ╔╗                                      ║
║                                   ╔╝╚╗                                     ║
║                                  ╔╝  ╚╗                                    ║
║                                 ╔╝    ╚╗                                   ║
║    ██████╗██╗  ██╗███████╗     ╚╗    ╔╝     ██╗    ██╗ ██████╗ ██████╗     ║
║   ██╔════╝██║  ██║██╔════╝      ╚╗  ╔╝      ██║    ██║██╔═══██╗██╔══██╗    ║
║   ██║     ███████║█████╗         ╚╗╔╝       ██║ █╗ ██║██║   ██║██████╔╝    ║
║   ██║     ██╔══██║██╔══╝          ╚╝        ██║███╗██║██║   ██║██╔══██╗    ║
║   ╚██████╗██║  ██║███████╗                  ╚███╔███╔╝╚██████╔╝██║  ██║    ║
║    ╚═════╝╚═╝  ╚═╝╚══════╝                   ╚══╝╚══╝  ╚═════╝ ╚═╝  ╚═╝    ║
║                                                                            ║
║   ┌─────────────────────────────────────────────────────────────────────┐  ║
║   │                                                                     │  ║
║   │   "The only way to do great work is to love what you do."           │  ║
║   │                                          — Steve Jobs               │  ║
║   │                                                                     │  ║
║   └─────────────────────────────────────────────────────────────────────┘  ║
║                                                                            ║
║                         v1.0  |  BUILDING THE FUTURE                       ║
║                                                                            ║
╚════════════════════════════════════════════════════════════════════════════╝)";

	delay(3000);

	cls();
	setcolor(YELLOW);
	cout << R"(╔════════════════════════════════════════════════════════════════════════════╗
║                                                                            ║
║                               .....                                        ║
║                            .+*********+.                                   ║
║                          .+**+++***+++**+.                                 ║
║                        .+++***********+++.                                 ║
║                       .++******+ +******++..                               ║
║                      .+++******   ******++++.                              ║
║     ██████╗██╗  ██╗   .+++****       ****+++..    ██╗    ██╗ ██████╗       ║
║    ██╔════╝██║  ██║    .++**+         +**+++.     ██║    ██║██╔══██╗       ║
║    ██║     ███████║      .+++           +++..      ██║ █╗ ██║██████╔╝      ║
║    ██║     ██╔══██║        .+           +.         ██║███╗██║██╔══██╗      ║
║    ╚██████╗██║  ██║          .         .           ╚███╔███╔╝██║  ██║      ║
║     ╚═════╝╚═╝  ╚═╝                                   ╚══╝╚══╝ ╚═╝  ╚═╝    ║
║                                                                            ║
║                          ┌─────────────────────┐                           ║
║                          │   H    He           │                           ║
║                          │   Li   Be    B   C  │                           ║
║                          │   N    O     F   Ne │                           ║
║                          │   Na   Mg    Al  Si │                           ║
║                          │   P    S     Cl  Ar │                           ║
║                          └─────────────────────┘                           ║
║                                                                            ║
║                    ╔══════════════════════════════════════╗                ║
║                    ║    元 素  |  工 业  |  宇 宙         ║                ║
║                    ╚══════════════════════════════════════╝                ║
║                                                                            ║
╚════════════════════════════════════════════════════════════════════════════╝)";
	delay(3000);
	cls();
	setcolor(RESET);
}

void setupExceptionHandler() {
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    _set_abort_behavior(0, _WRITE_ABORT_MSG);
}

// ======================== 存档数据结构 ========================
struct InventoryItem {
    string name;
    int quantity;
    string category;   // ore, ingot, chemical, fuel, part, etc.
    double value;      // 基础价值
};

struct PlayerData {
    string name;
    int level;
    long long coins;
    int goldBars;
    int exp;
    time_t createdAt;
    time_t lastPlayed;
    //我去，差点没加上inventory
    //inventory以及一些功能
	//inventory是一个vector，里面是InventoryItem结构体，包含物品名称、数量、类别（矿石、零件、化学品等）和基础价值（用于交易和计算利润）
    
    vector<InventoryItem> inventory;

    void addItem(const string& itemName, int quantity, const string& category="misc", double value = 0) {
        for (auto& item : inventory) {
            if (item.name == itemName) {
                item.quantity += quantity;
                return;
            }
        }
        inventory.push_back({ itemName, quantity, category, value });
	}

    bool hasItem(const string& itemName, int quantity = 1) {
        for (auto& item : inventory) {
            if (item.name == itemName && item.quantity >= quantity) {
                return true;
            }
        }
        return false;
    }

};



struct Machine {
    string id;
    string type;       // 粉碎机、电解机、车床等
    int level;
    bool isRunning;
    int x, y;          // 工厂布局坐标
    map<string, int> inventory;  // 机器内部库存
    int powerConsumption;
};

struct Factory {
    string name;
    vector<Machine> machines;
    int totalPower;
    int usedPower;
    map<string, int> centralStorage;  // 中央仓库
};

struct ChemicalKnowledge {
    string formula;
    bool discovered;
    int discoveryTime;
};

struct Quest {
    int id;
    string name;
    int progress;
    bool completed;
};

struct Planet {
    string name;
    bool unlocked;
    vector<string> resources;
    int miningRigs;           // 采矿设备数量
    long long extractedAmount;
};


//===========教程
struct TutorialStep {
    string id;
    string title;
    string description;
    string triggerEvent;
    string requiredItem;
    string targetMachine;
    bool isCompleted;
    function<bool(PlayerData&, Factory&)> condition;
};

class TutorialManager {
private:
    vector<TutorialStep> steps;
    map<string, bool> completedFlags;
    int currentStepIndex;
    bool tutorialActive;

    void showStep(const TutorialStep& step) {
        system("cls");
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14);
        cout << "\n══════════════ 新 手 教 程 ══════════════\n";
        cout << "||" << step.title << string(50 - step.title.length(), ' ') << "|| \n";
        cout << "═════════════════════════════════════\n";
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11);
        cout << "\n" << step.description << "\n" << endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
        cout << "\n [按任意键继续]";
        cin.get();
    }
public:


    TutorialManager() {
        tutorialActive = true;
        currentStepIndex = 0;
        initSteps();
    }

    void initSteps() {
        steps = {
            {"first_sale","第一桶金","完成你的第一笔销售！去卖掉一些东西吧!\n提示:考虑考虑水？量大管饱!","OPEN_SHOP","","",false,nullptr},
            {"buy_electrolyzer","购买电解机","现在你有了一些钱，可以购买你的第一台机器了！去商店买一台电解机吧！\n提示:电解机可以把水分解成氢气和氧气！","BUY_MACHINE","","电解机",false,nullptr},
            {"power_up","工厂通电","对了，你的工厂需要电力才能运行！去商店买个电池箱，给你的机器供电吧！\n提示:电池箱可以储存电力并为机器供电！","CONNECT_POWER","","",false,nullptr},
            {"electrolysis_salt","电解食盐","wow! 你已经有了电解机了！现在试着把食盐电解成氢气、氯气,还可以……弄些氢氧化钠！\n提示:食盐的化学式是NaCl！","USE_MACHINE","盐","电解机",false,
            [](PlayerData& p,Factory& f)->bool {
                return p.hasItem("sodium") && p.hasItem("cloride");
            }},
            {"sodium_is_so_explosive","活泼的钠","你成功电解出了钠！现在试着把它放在水里看看会发生什么吧！\n提示:钠+水=氢气+氢氧化钠,你需要很多！","CHEMICAL_REACTION ","钠","水",false,nullptr}
        };

    }

    void checkEvent(const string& event, const string& itemName, PlayerData& player, Factory& factory) {
        if (!tutorialActive) return;
        for (auto& step : steps) {
            if (step.isCompleted) continue;

            if (step.triggerEvent == event) {
                if (step.requiredItem.empty() || step.requiredItem == itemName) {
                    showStep(step);
                    step.isCompleted = true;
                    completedFlags[step.id] = true;

                }
            }
        }
    }
    //根据步骤ID给予奖励，奖励可以是经验、金币、物品等
    void giveReward(const string& stepId, PlayerData& player) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
        cout << "\ngood job! 奖励给你:\n";
        if (stepId == "first_sale") {
            player.coins += 50;
            cout << "+50 coins!\n";
        }
        else if (stepId == "buy_electrolyzer") {
            player.coins += 100;
            cout << "+100 coins!\n";
        }
        else if (stepId == "power_up") {
            player.addItem("raw coal", 1, "item", 15);
            cout << "+1 粗煤炭\n";
        }
        else if (stepId == "electrolysis_salt") {
            player.exp += 100;
            cout << "+100 exp\n";
        }
        else if (stepId == "sodium_is_so_explosive") {
            player.exp += 200;
            cout << "+200 exp\n";
        }

        delay(2000);
    }

    bool isStepCompleted(const string& stepId) {
        return completedFlags[stepId];
    }

    void skipTutorial() {
        tutorialActive = false;
        for (auto& step : steps) {
            step.isCompleted = true;
            completedFlags[step.id] = true;
        }
        cout << "教程已跳过！祝你在化学世界玩得开心！\n";
        cout << "看来你很有自信！\n";
        delay(2000);
    }
    //教程启动
    void startTutorial() {
        if (!tutorialActive) return;
        for (const auto& step : steps) {
            if (!step.isCompleted) {
                showStep(step);
                break;
            }
        }
    }
};


// ======================== 存档管理器 ========================

class SaveManager {
private:
    TutorialManager tutorial;
    string saveDirectory = "saves/";
    string currentSaveFile = "";
    PlayerData player;
    Factory factory;
    vector<InventoryItem> inventory;
    vector<ChemicalKnowledge> chemistry;
    vector<Quest> quests;
    vector<Planet> planets;

    // 辅助函数：创建存档目录
    void ensureSaveDirectory() {
        if (!fs::exists(saveDirectory)) {
            fs::create_directory(saveDirectory);
        }
    }
    // 辅助函数：获取当前时间字符串
    string getTimeString(time_t t) {
        char buffer[80];
        struct tm* timeinfo = localtime(&t);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        return string(buffer);
    }

    string prepareString(const string& raw) {
        string utf8 = EncodingHelper::gbkToUtf8(raw);
        return EncodingHelper::sanitizeUtf8(utf8);
    }

    string displayString(const string& utf8Str) {
        return EncodingHelper::utf8ToGbk(utf8Str);
    }


public:

    SaveManager() {
        ensureSaveDirectory();
    }

    

    // ========== 玩家数据操作 ==========
    void createNewGame() {
        system("cls");
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11);
        cout << R"(
╔═══════════════════════════════════════════════════════════════╗
║                      新 游 戏 向 导                           ║
╚═══════════════════════════════════════════════════════════════╝
)";
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);

        cout << "\n* 请输入你的名字(为了避免一些奇怪的BUG，请不要输入A-Z,a-z,0-9,_,-以外的字符！敬请谅解……): ";
        cout << "\n*如果不输入，默认是Chemist";
        cout << "\n!!!非上述的字符会被过滤掉！";
        string rawName;
        getline(cin, rawName);
        string cleanName;
        for (char c : rawName) {
            if ((c >= 'a' && c <= 'z') || (c>='A' && c<='Z') || (c>='0' && c<='9') || c=='_' || c=='-') {
                cleanName += c;
            }
        }
        if (cleanName.empty()) {
            player.name = "Chemist";
        }
        else {
            player.name = cleanName;
        }

        // 初始化玩家数据
        player.level = 1;
        player.coins = 100;
        player.goldBars = 0;
        player.exp = 0;
        player.createdAt = time(nullptr);
        player.lastPlayed = time(nullptr);

        // 初始化库存（新手包）
        inventory = {
            {"water", 10, "chemical", 5},
            {"raw rich pyrite", 5, "ore", 10},
            {"raw coal", 3, "fuel", 15}
        };

        // 初始化工厂
        factory.name = player.name + "'s Factory";
        factory.machines = {
};
        factory.totalPower = 0;
        factory.usedPower = 0;

        // 初始化化学知识（已发现的配方）
        chemistry = {
            {"H2O", true, (int)time(nullptr)},
        };

        // 初始化任务
        quests = {
            {1, "First_sale", 0, false},
            {2, "ores go GRRR", 0, false},
            {3, "electrolysis_salt", 0, false}
        };

        // 初始化星球
        planets = {
            {"Earth", true, {"pyrite", "coal", "oil"}, 0, 0}
        };

        cout << "\nhi!   欢迎，" << player.name << "！你的化学世界之旅开始了！\n";
        cout << "hi!正在创建存档...\n";

        saveGame("auto");

        cout << "wow! 存档创建成功！\n";
        Sleep(1500);
        cls();
        cout << "欢迎！" << player.name << "！准备好开始你的化学世界之旅吧！\n";
        cout<<"你需要新手教程吗? (y/n): ";
        char choice;
        cin >> choice;
		cin.ignore();
        if (choice == 'n' || choice == 'N') {
            tutorial.skipTutorial();
        }
        else {
			tutorial.startTutorial();
        }

    }

	void triggerEvent(const string& event, const string& itemName = "") {
        tutorial.checkEvent(event, itemName, player, factory);
    }

    TutorialManager& getTutorial() {
        return tutorial;
	}

    // 保存游戏
    bool saveGame(const string& slotName) {
        json saveData;

        // 更新时间戳
        player.lastPlayed = time(nullptr);

        // === 玩家数据 ===
        saveData["player"]["name"] = player.name;
        saveData["player"]["level"] = player.level;
        saveData["player"]["coins"] = player.coins;
        saveData["player"]["goldBars"] = player.goldBars;
        saveData["player"]["exp"] = player.exp;
        saveData["player"]["createdAt"] = player.createdAt;
        saveData["player"]["lastPlayed"] = player.lastPlayed;

        // === 库存数据 ===
        for (const auto& item : inventory) {
            json itemJson;
            itemJson["name"] = item.name;
            itemJson["quantity"] = item.quantity;
            itemJson["category"] = item.category;
            itemJson["value"] = item.value;
            saveData["inventory"].push_back(itemJson);
        }

        // === 工厂数据 ===
        saveData["factory"]["name"] = factory.name;
        saveData["factory"]["totalPower"] = factory.totalPower;
        saveData["factory"]["usedPower"] = factory.usedPower;

        for (const auto& machine : factory.machines) {
            json machineJson;
            machineJson["id"] = machine.id;
            machineJson["type"] = machine.type;
            machineJson["level"] = machine.level;
            machineJson["isRunning"] = machine.isRunning;
            machineJson["x"] = machine.x;
            machineJson["y"] = machine.y;
            machineJson["powerConsumption"] = machine.powerConsumption;

            // 机器内部库存
            for (const auto& [item, qty] : machine.inventory) {
                machineJson["inventory"][item] = qty;
            }
            saveData["factory"]["machines"].push_back(machineJson);
        }

        // 中央仓库
        for (const auto& [item, qty] : factory.centralStorage) {
            saveData["factory"]["centralStorage"][item] = qty;
        }

        // === 化学知识 ===
        for (const auto& chem : chemistry) {
            json chemJson;
            chemJson["formula"] = chem.formula;
            chemJson["discovered"] = chem.discovered;
            chemJson["discoveryTime"] = chem.discoveryTime;
            saveData["chemistry"].push_back(chemJson);
        }

        // === 任务系统 ===
        for (const auto& quest : quests) {
            json questJson;
            questJson["id"] = quest.id;
            questJson["name"] = quest.name;
            questJson["progress"] = quest.progress;
            questJson["completed"] = quest.completed;
            saveData["quests"].push_back(questJson);
        }

        // === 星球数据 ===
        for (const auto& planet : planets) {
            json planetJson;
            planetJson["name"] = planet.name;
            planetJson["unlocked"] = planet.unlocked;
            planetJson["resources"] = planet.resources;
            planetJson["miningRigs"] = planet.miningRigs;
            planetJson["extractedAmount"] = planet.extractedAmount;
            saveData["planets"].push_back(planetJson);
        }

        // === 元数据 ===
        saveData["metadata"]["version"] = "1.0";
        saveData["metadata"]["saveTime"] = time(nullptr);

        // 写入文件
        string filename = saveDirectory + slotName + ".json";
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "X 无法保存游戏！" << endl;
            return false;
        }

        file << saveData.dump(4);  
        file.close();

        currentSaveFile = slotName;
        cout << "wow! 游戏已保存到 " << filename << endl;
        return true;
    }

    // 加载游戏
    bool loadGame(const string& slotName) {
        string filename = saveDirectory + slotName + ".json";
        ifstream file(filename);

        if (!file.is_open()) {
            cerr << "X 找不到存档文件！" << endl;
            return false;
        }

        json saveData;
        file >> saveData;
        file.close();

        try {
            // === 加载玩家数据 ===
            player.name = saveData["player"]["name"];
            player.level = saveData["player"]["level"];
            player.coins = saveData["player"]["coins"];
            player.goldBars = saveData["player"]["goldBars"];
            player.exp = saveData["player"]["exp"];
            player.createdAt = saveData["player"]["createdAt"];
            player.lastPlayed = saveData["player"]["lastPlayed"];

            // === 加载库存 ===
            inventory.clear();
            for (const auto& item : saveData["inventory"]) {
                inventory.push_back({
                    item["name"],
                    item["quantity"],
                    item["category"],
                    item["value"]
                    });
            }

            // === 加载工厂 ===
            factory.name = saveData["factory"]["name"];
            factory.totalPower = saveData["factory"]["totalPower"];
            factory.usedPower = saveData["factory"]["usedPower"];

            factory.machines.clear();
            for (const auto& machineJson : saveData["factory"]["machines"]) {
                Machine m;
                m.id = machineJson["id"];
                m.type = machineJson["type"];
                m.level = machineJson["level"];
                m.isRunning = machineJson["isRunning"];
                m.x = machineJson["x"];
                m.y = machineJson["y"];
                m.powerConsumption = machineJson["powerConsumption"];

                if (machineJson.contains("inventory")) {
                    for (auto& [item, qty] : machineJson["inventory"].items()) {
                        m.inventory[item] = qty;
                    }
                }
                factory.machines.push_back(m);
            }

            // 中央仓库
            factory.centralStorage.clear();
            if (saveData["factory"].contains("centralStorage")) {
                for (auto& [item, qty] : saveData["factory"]["centralStorage"].items()) {
                    factory.centralStorage[item] = qty;
                }
            }

            // === 加载化学知识 ===
            chemistry.clear();
            for (const auto& chem : saveData["chemistry"]) {
                chemistry.push_back({
                    chem["formula"],
                    chem["discovered"],
                    chem["discoveryTime"]
                    });
            }

            // === 加载任务 ===
            quests.clear();
            for (const auto& quest : saveData["quests"]) {
                quests.push_back({
                    quest["id"],
                    quest["name"],
                    quest["progress"],
                    quest["completed"]
                    });
            }

            // === 加载星球 ===
            planets.clear();
            for (const auto& planet : saveData["planets"]) {
                Planet p;
                p.name = planet["name"];
                p.unlocked = planet["unlocked"];
                p.resources = planet["resources"].get<vector<string>>();
                p.miningRigs = planet["miningRigs"];
                p.extractedAmount = planet["extractedAmount"];
                planets.push_back(p);
            }

            currentSaveFile = slotName;
            cout << " wow! 存档加载成功！欢迎回来，" << player.name << "！\n";
            cout << " hi! 上次游戏时间: " << getTimeString(player.lastPlayed) << endl;
            return true;

        }
        catch (const exception& e) {
            cerr << "X 存档损坏！错误: " << e.what() << endl;
            return false;
        }
    }

    // 列出所有存档
    void listSaves() {
        ensureSaveDirectory();

        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14);
        cout << "\n╔═══════════════════════════════════════════════════════════════╗\n";
        cout << "║                        存 档 列 表                            ║\n";
        cout << "╚═══════════════════════════════════════════════════════════════╝\n";
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11);

        int index = 1;
        vector<string> saveFiles;

        for (const auto& entry : fs::directory_iterator(saveDirectory)) {
            if (entry.path().extension() == ".json") {
                saveFiles.push_back(entry.path().stem().string());
                cout << index++ << ". " << entry.path().stem().string() << endl;
            }
        }

        if (saveFiles.empty()) {
            cout << "暂无存档，请先开始新游戏！\n";
        }

        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    }

    // 获取玩家数据（供其他模块使用）
    PlayerData& getPlayerData() { return player; }
    vector<InventoryItem>& getInventory() { return inventory; }
    Factory& getFactory() { return factory; }

    // 添加物品到库存
    void addItem(const string& name, int quantity, const string& category, double value) {
        for (auto& item : inventory) {
            if (item.name == name) {
                item.quantity += quantity;
                return;
            }
        }
        inventory.push_back({ name, quantity, category, value });
    }

    // 显示玩家状态（调试用）
    void showPlayerStats() {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
        cout << "\n══════════════ 玩家状态 ══════════════\n";
        cout << " 名字: " << player.name << endl;
        cout << " 等级: " << player.level << endl;
        cout << " 硬币: " << player.coins << endl;
        cout << " 金条: " << player.goldBars << endl;
        cout << " 经验: " << player.exp << endl;
        cout << " 库存物品: " << inventory.size() << " 种\n";
        cout << " 机器数量: " << factory.machines.size() << endl;
        cout << "═════════════════════════════════════\n";
        cout << "金条是一种高级货币!事实上，你可以制作它们！";
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
        delay(4000);
        cls();
    }

    void handleShopAction(const string& action, const string& itemName) {
        if (action == "buy") {
            if (itemName == "electrolyzer") {
                if (player.coins >= 100) {
                    player.coins -= 100;
                    addItem("electrolyzer", 1, "machine", 0);
                    cout << "购买成功！你现在有了电解机！\n";
                    triggerEvent("BUY_MACHINE", "electrolyzer");
                }
                else {
                    cout << "你的钱不够！\n";
                }
            }
            else if (itemName == "battery box") {
                if (player.coins >= 150) {
                    player.coins -= 150;
                    addItem("battery box", 1, "machine", 0);
                    cout << "购买成功！你现在有了电池箱！\n";
                    triggerEvent("BUY_MACHINE", "battery box");
                }
                else {
                    cout << "你的钱不够！\n";
                }
            }
            else {
                cout << "商店里没有这个物品！\n";
            }
        }
	}

};

// ======================== 主菜单 ========================

class Start {
private:
    SaveManager saveManager;

    void showMainMenu() {
        cls();
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11);
        cout << R"(
╔═══════════════════════════════════════════════════════════════╗
║                   化 学 世 界 - 主 菜 单                      ║
╚═══════════════════════════════════════════════════════════════╝
)";
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
        cout << "\n  1. 新游戏\n";
        cout << "  2. 加载存档\n";
        cout << "  3. 查看存档列表\n";
        cout << "  4. 退出\n";
        cout << "\n  请选择: ";
    }

    void gameLoop() {
        int choice;
        while (true) {
            cls();
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14);
            cout << "\n══════════════ 游戏主界面 ══════════════\n";
            saveManager.showPlayerStats();
            cout << "\n  1. 查看库存\n";
            cout << "  2. 开始游戏!!!\n";
            cout << "  3. 保存游戏\n";
            cout << "  4. 返回主菜单\n";
            cout << "  请选择: ";

            cin >> choice;
            cin.ignore();

            switch (choice) {
            case 1: {
                // 查看库存（示例）
                cout << "\n当前库存:\n";
                for (const auto& item : saveManager.getInventory()) {
                    cout << "  - " << item.name << " x" << item.quantity << " (" << item.category << ")\n";
                }
                cout << "\n非常抱歉的通知……由于技术原因，这里只能使用英文来展示，orz";
                system("pause");
                break;
            }
            case 2: {
                //开始教程
				// 这里直接进入教程，后续可以扩展成真正的游戏界面
				TutorialManager& tutorial = saveManager.getTutorial();
                tutorial.startTutorial();
                break;
            }
            case 3: {
                string saveName;
                cout << "输入存档名称: ";
                getline(cin, saveName);
                if (saveManager.saveGame(saveName)) {
                    cout << "保存成功！\n";
                }
                system("pause");
                break;
            }
            case 4:{
                  return;
            }
            }
        }
    }
    
public:
    void run() {
        int choice;
        while (true) {
            showMainMenu();
            cin >> choice;
            cin.ignore();

            switch (choice) {
            case 1:
                saveManager.createNewGame();
                gameLoop();
                break;

            case 2: {
                string saveName;
                cout << "输入存档名称: ";
                getline(cin, saveName);
                if (saveManager.loadGame(saveName)) {
                    gameLoop();
                }
                else {
                    cout << "加载失败！\n";
                    system("pause");
                }
                break;
            }

            case 3:
                saveManager.listSaves();
                system("pause");
                break;

            case 4:
                cout << "感谢游玩！\n";
                return;

            default:
                cout << "无效选择！\n";
                system("pause");
            }
        }
    }
};



int main()
{
	SetConsoleTitle(L"化学·世界");
	cppwelcome();
	cls();
	showlogo();
    // 为了调试方便这里可以注释掉
    
    //下面的那一部分有调试代码
    
    try {

        // 测试存档目录是否可创建
        if (!fs::exists("saves")) {
            if (!fs::create_directory("saves")) {
                cerr << "X 无法创建 saves 目录！" << endl;
                system("pause");
                return 1;
            }
            cout << " wow! 创建 saves 目录成功" << endl;
        }
        Start start;
        start.run();

    }
    catch (const exception& e) {
        cerr << "X 致命错误: " << e.what() << endl;
        system("pause");
        return 1;
    }
    catch (...) {
        cerr << "X 未知错误！" << endl;
        system("pause");
        return 1;
    }
    //你真是个超人
}
