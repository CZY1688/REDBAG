#include "resource.h"   
#include "BForm.h"     
#include "RedPacket.h"     
#include <string>

using namespace std;


// 定义三种状态，用来标记当前界面正在查看哪个红包的详情
enum ECurrentPacket
{
    CurrentPacketA = 0,
    CurrentPacketB = 1,
    CurrentPacketC = 2
};

//文本统一放在这里方便以后修改
#define PACKET_LABEL_PREFIX TEXT("红包")
inline LPCTSTR TCN_WindowTitle() { return TEXT("模拟微信抢红包"); }
inline LPCTSTR TCN_GroupA() { return TEXT("红包A（钱已塞好，直接开抢）"); }
inline LPCTSTR TCN_GroupB() { return TEXT("红包B（钱已塞好，直接开抢）"); }
inline LPCTSTR TCN_GroupC() { return TEXT("红包C（先塞钱，才能抢）"); }
inline LPCTSTR TCN_Grab() { return TEXT("抢红包"); }
inline LPCTSTR TCN_View() { return TEXT("查看"); }
inline LPCTSTR TCN_RobotGrab() { return TEXT("机器人抢红包"); }
inline LPCTSTR TCN_CMoney() { return TEXT("钱数(元)："); }
inline LPCTSTR TCN_CNum() { return TEXT("分几个红包："); }
inline LPCTSTR TCN_CFill() { return TEXT("塞钱进红包"); }
inline LPCTSTR TCN_ResultDefault() { return TEXT(""); }
inline LPCTSTR TCN_TitleInfo() { return TEXT("提示"); }
inline LPCTSTR TCN_TitleWarn() { return TEXT("警告"); }
inline LPCTSTR TCN_AnonymousUser() { return TEXT("匿名用户"); }

inline LPCTSTR PacketLabelA() { return PACKET_LABEL_PREFIX TEXT("A"); }
inline LPCTSTR PacketLabelB() { return PACKET_LABEL_PREFIX TEXT("B"); }
inline LPCTSTR PacketLabelC() { return PACKET_LABEL_PREFIX TEXT("C"); }


//全局变量

CBForm form(ID_form1);                             // 主窗体对象
RedPacket packetA(30.0, 5, "Owner A");             // 预设红包A：30元，分5个包
RedPacket packetB(50.0, 8, "Owner B");             // 预设红包B：50元，分8个包
RedPacket packetC(0.0, 1, "Owner C");              // 红包C：初始金额0，等待用户输入
bool packetCReady = false;                         // 标记红包C是否已经塞过钱
ECurrentPacket currentPacket = CurrentPacketA;     // 记录当前界面主要显示的红包 (默认是A)
int robotIndex = 1;                                // 机器人名称的累加序号

//事件函数声明

void UpdateLeftFooter(const RedPacket& packet, LPCTSTR packetLabel);
void ShowPacketLog(const RedPacket& packet, LPCTSTR packetLabel);
void DoGrabWithName(RedPacket& packet, const string& who, LPCTSTR packetLabel, bool showResultText);
void DoGrab(RedPacket& packet, unsigned short idNameEdit, LPCTSTR packetLabel, bool checkReady, bool showResultText);


// 处理字符编码：将标准 string 转为 Windows API 需要的宽字符 tstring
tstring ToTString(const string& s)
{
#ifdef UNICODE
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, 0, 0);
    if (len <= 0) return TEXT("");
    wchar_t buf[256];
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, buf, len);
    return tstring(buf);
#else
    return s;
#endif
}

// 处理字符编码：将宽字符 tstring 转回标准 string
string ToString(const tstring& s)
{
#ifdef UNICODE
    int len = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, 0, 0, 0, 0);
    if (len <= 0) return string();
    char buf[256];
    WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, buf, len, 0, 0);
    return string(buf);
#else
    return s;
#endif
}

// 自动生成机器人名称
string NextRobotName()
{
    TCHAR robotName[64] = { 0 };
    _stprintf(robotName, TEXT("机器人%d"), robotIndex++);
    return ToString(tstring(robotName));
}

//普通信息提示
int ShowInfoBox(LPCTSTR msg)
{
    return MessageBox(form.hWnd(), msg, TCN_TitleInfo(), MB_OK | MB_ICONINFORMATION);
}

//警告或错误提示
int ShowWarnBox(LPCTSTR msg)
{
    return MessageBox(form.hWnd(), msg, TCN_TitleWarn(), MB_OK | MB_ICONWARNING);
}

//从指定的输入框中读取用户输入的文本
tstring ReadText(unsigned short idEdit)
{
    return form.Control(idEdit, false).Text();
}

//读取用户名，如果用户什么都没填，就默认给个“匿名用户”
string ReadNameOrDefault(unsigned short idEdit)
{
    tstring s = ReadText(idEdit);
    if (s.empty()) return ToString(TCN_AnonymousUser());
    return ToString(s);
}

//解析底层的记录字符串，将 "名字:金额" 拆分开来
bool ParseNameMoney(const string& src, string& outName, string& outMoney)
{
    size_t pos = src.rfind(':');
    if (pos == string::npos) return false;
    outName = src.substr(0, pos);
    outMoney = src.substr(pos + 1);
    return true;
}

//向日志区追加一行内容
void AppendLog(LPCTSTR s)
{
    form.Control(ID_editLog, false).TextAdd(s);
    form.Control(ID_editLog, false).TextAdd(TEXT("\r\n"));
}


// 初始化所有控件的固定文本内容
void ApplyRuntimeTexts()
{
    LPCTSTR textGrab = TCN_Grab();
    LPCTSTR textView = TCN_View();

    form.Control(ID_grpA, false).TextSet(TCN_GroupA());
    form.Control(ID_grpB, false).TextSet(TCN_GroupB());
    form.Control(ID_grpC, false).TextSet(TCN_GroupC());

    form.Control(ID_btnAGrab, false).TextSet(textGrab);
    form.Control(ID_btnAShow, false).TextSet(textView);
    form.Control(ID_btnBGrab, false).TextSet(textGrab);
    form.Control(ID_btnBShow, false).TextSet(textView);

    form.Control(ID_txtCMoney, false).TextSet(TCN_CMoney());
    form.Control(ID_txtCNum, false).TextSet(TCN_CNum());
    form.Control(ID_btnCFill, false).TextSet(TCN_CFill());
    form.Control(ID_btnCGrab, false).TextSet(textGrab);
    form.Control(ID_btnCShow, false).TextSet(textView);

    form.Control(ID_btnRobotGrab, false).TextSet(TCN_RobotGrab());
    form.Control(ID_txtResult, false).TextSet(TCN_ResultDefault());
    form.Control(ID_txtSep, false).VisibleSet(false);
}

// 重置界面到最初始的状态
void ResetUIState()
{
    form.Control(ID_editLog, false).TextSet(TEXT("")); // 清空日志区

    form.Control(ID_editAName, false).TextSet(TEXT(""));
    form.Control(ID_btnAGrab, false).VisibleSet(true);
    form.Control(ID_btnAShow, false).VisibleSet(true);
    form.Control(ID_editAName, false).VisibleSet(true);

    form.Control(ID_editBName, false).TextSet(TEXT(""));
    form.Control(ID_editBName, false).VisibleSet(true);
    form.Control(ID_btnBGrab, false).VisibleSet(true);
    form.Control(ID_btnBShow, false).VisibleSet(true);
    form.Control(ID_txtResult, false).VisibleSet(true);

    form.Control(ID_editCMoney, false).TextSet(TEXT(""));
    form.Control(ID_editCNum, false).TextSet(TEXT(""));
    form.Control(ID_editCName, false).TextSet(TEXT(""));

    // 红包C在未塞钱之前，抢红包按钮需保持禁用状态
    form.Control(ID_btnCGrab, false).EnabledSet(false);

    // 确保各种控件的层级正常显示
    form.Control(ID_picCover, false).ZOrder(1);
    form.Control(ID_grpA, false).ZOrder(1);
    form.Control(ID_grpB, false).ZOrder(1);
    form.Control(ID_grpC, false).ZOrder(1);
}

// 检查当前红包的最佳手气，并弹出弹窗展示
void ShowBestLuckMsg(const RedPacket& packet, LPCTSTR packetLabel)
{
    string best = packet.bestLuckRecord();
    TCHAR msg[256] = { 0 };
    if (best.empty())
    {
        _stprintf(msg, TEXT("%s手气最佳：暂无。"), packetLabel);
    }
    else
    {
        string who, money;
        if (ParseNameMoney(best, who, money))
        {
            _stprintf(msg, TEXT("%s手气最佳：%s，%s 元。"),
                packetLabel, ToTString(who).c_str(), ToTString(money).c_str());
        }
        else
        {
            _stprintf(msg, TEXT("%s手气最佳：%s。"), packetLabel, ToTString(best).c_str());
        }
    }
    ShowInfoBox(msg);
}

// 在界面顶部更新抢红包的简短结果
void UpdateResultText(const string& grabber, double money)
{
    TCHAR line[256] = { 0 };
    _stprintf(line, TEXT("恭喜！%s 抢到 %.2f 元红包！"), ToTString(grabber).c_str(), money);
    form.Control(ID_txtResult, false).TextSet(line);
}

// 更新界面左下角显示的红包剩余情况
void UpdateLeftFooter(const RedPacket& packet, LPCTSTR packetLabel)
{
    TCHAR line[256] = { 0 };
    int grabbed = packet.grabbedCount();
    int total = packet.totalCount();
    int left = total - grabbed;
    if (left < 0) left = 0;

    _stprintf(line, TEXT("%s：已抢 %d 个，剩余 %d 个"), packetLabel, grabbed, left);
    form.Control(ID_txtLeftStatic, false).TextSet(line);
}

// 刷新日志区，将指定红包的详细抢包记录打印出来
void ShowPacketLog(const RedPacket& packet, LPCTSTR packetLabel)
{
    form.Control(ID_editLog, false).TextSet(TEXT("")); // 先清空旧日志

    const string* pRecs = packet.records();
    int count = packet.grabbedCount();
    string best = packet.bestLuckRecord();

    AppendLog(TEXT("抢红包记录："));

    // 遍历所有抢包记录
    for (int i = 0; i < count; ++i)
    {
        string who, money;
        TCHAR line[256] = { 0 };

        if (ParseNameMoney(pRecs[i], who, money))
        {
            // 如果这条记录是手气最佳，加个特殊标记
            if (!best.empty() && pRecs[i] == best)
            {
                _stprintf(line, TEXT("%s 抢到了 %s 元，手气最佳！"), ToTString(who).c_str(), ToTString(money).c_str());
            }
            else
            {
                _stprintf(line, TEXT("%s 抢到了 %s 元"), ToTString(who).c_str(), ToTString(money).c_str());
            }
        }
        else
        {
            _stprintf(line, TEXT("%s"), ToTString(pRecs[i]).c_str());
        }
        AppendLog(line);
    }

    if (count == 0) AppendLog(TEXT("暂无抢包记录"));

    // 同步更新左下角的状态提示
    UpdateLeftFooter(packet, packetLabel);
}


// 抢红包操作
void DoGrabWithName(RedPacket& packet, const string& who, LPCTSTR packetLabel, bool showResultText)
{
    int status = RedPacket::GrabEmpty;
    double got = packet.grab(who, &status); // 执行抢红包动作

    // 情况1：该用户已经抢过此红包
    if (status == RedPacket::GrabDuplicate)
    {
        TCHAR msg[256] = { 0 };
        _stprintf(msg, TEXT("%s 已经抢过%s，每个用户只能抢一次。"), ToTString(who).c_str(), packetLabel);
        UpdateLeftFooter(packet, packetLabel);
        ShowWarnBox(msg);
        return;
    }

    // 情况2：红包已经被抢光了
    if (got <= 0.0)
    {
        TCHAR msg[256] = { 0 };
        _stprintf(msg, TEXT("%s已抢完，手慢了！"), packetLabel);
        UpdateLeftFooter(packet, packetLabel);
        ShowWarnBox(msg);

        if (showResultText) form.Control(ID_txtResult, false).TextSet(TEXT("手慢了，红包已抢完。"));
        ShowBestLuckMsg(packet, packetLabel); // 包抢完时，公布一下手气最佳
        return;
    }

    // 情况3：成功抢到红包
    TCHAR msg[256] = { 0 };
    _stprintf(msg, TEXT("恭喜！%s 在%s抢到 %.2f 元。"), ToTString(who).c_str(), packetLabel, got);
    ShowInfoBox(msg);

    if (showResultText) UpdateResultText(who, got);

    // 抢完后刷新日志面板
    ShowPacketLog(packet, packetLabel);
}

// 抢红包入口
void DoGrab(RedPacket& packet, unsigned short idNameEdit, LPCTSTR packetLabel, bool checkReady, bool showResultText)
{
    // 如果是必须先塞钱的红包（比如红包C），检查是否已准备好
    if (checkReady && !packetCReady)
    {
        UpdateLeftFooter(packet, packetLabel);
        ShowWarnBox(TEXT("此红包还未塞钱，请先设置金额和个数。"));
        return;
    }

    string who = ReadNameOrDefault(idNameEdit);
    DoGrabWithName(packet, who, packetLabel, showResultText);
}

// 机器人抢红包
void DoGrabCurrentWithName(const string& who)
{
    if (currentPacket == CurrentPacketA)
    {
        DoGrabWithName(packetA, who, PacketLabelA(), false);
    }
    else if (currentPacket == CurrentPacketB)
    {
        DoGrabWithName(packetB, who, PacketLabelB(), true);
    }
    else // 当前是红包C
    {
        if (!packetCReady)
        {
            UpdateLeftFooter(packetC, PacketLabelC());
            ShowWarnBox(TEXT("红包C还没塞钱，请先完成塞钱操作。"));
            return;
        }
        DoGrabWithName(packetC, who, PacketLabelC(), false);
    }
}

// 点击封面图时的快捷抢包逻辑
void DoGrabCurrentFromInput()
{
    if (currentPacket == CurrentPacketA)
    {
        DoGrab(packetA, ID_editAName, PacketLabelA(), false, false);
    }
    else if (currentPacket == CurrentPacketB)
    {
        DoGrab(packetB, ID_editBName, PacketLabelB(), false, true);
    }
    else
    {
        DoGrab(packetC, ID_editCName, PacketLabelC(), true, false);
    }
}

// 窗口初始化
void OnFormLoad()
{
    form.IconSet(IDI_ICON1);
    form.TextSet(TCN_WindowTitle());
    form.MoveToScreenCenter();
    form.BackColorSet(RGB(236, 236, 236));
    form.KeyPreview = true;

    // 加载资源文件中的封面图片
    form.Control(ID_picCover, false).PictureSet(IDB_PACKET_COVER);

    ApplyRuntimeTexts();
    ResetUIState();

    // 初始状态下，底部状态栏显示红包A的信息
    UpdateLeftFooter(packetA, PacketLabelA());
}

// 三个红包的“抢红包”按钮点击事件
void OnAGrab() { DoGrab(packetA, ID_editAName, PacketLabelA(), false, false); }
void OnBGrab() { DoGrab(packetB, ID_editBName, PacketLabelB(), false, true); }
void OnCGrab() { DoGrab(packetC, ID_editCName, PacketLabelC(), true, false); }

// 三个红包的“查看详情”按钮点击事件
void OnAShow() { currentPacket = CurrentPacketA; ShowPacketLog(packetA, PacketLabelA()); }
void OnBShow() { currentPacket = CurrentPacketB; ShowPacketLog(packetB, PacketLabelB()); }
void OnCShow() { currentPacket = CurrentPacketC; ShowPacketLog(packetC, PacketLabelC()); }

// 处理红包C专属的“塞钱”逻辑
void OnCFill()
{
    // 读取金额和个数
    double money = form.Control(ID_editCMoney, false).TextVal();
    int count = static_cast<int>(form.Control(ID_editCNum, false).TextVal());

    // 数据合法性校验
    if (money <= 0.0 || count <= 0)
    {
        UpdateLeftFooter(packetC, PacketLabelC());
        ShowWarnBox(TEXT("塞钱失败：金额和个数都必须大于0。"));
        return;
    }

    // 状态校验：防止中途重新塞钱导致账目混乱
    if (!packetC.canSetMoney())
    {
        UpdateLeftFooter(packetC, PacketLabelC());
        ShowWarnBox(TEXT("红包C已有用户抢过，不能重新塞钱。"));
        return;
    }

    // 设置底层数据，并开放界面的抢包权限
    packetC.setMoney(money, count);
    packetCReady = true;
    form.Control(ID_btnCGrab, false).EnabledSet(true);

    TCHAR msg[128] = { 0 };
    _stprintf(msg, TEXT("塞钱成功！已设置 %.2f 元，共 %d 个红包。"), money, count);
    ShowInfoBox(msg);
    ShowPacketLog(packetC, PacketLabelC());
}

// 触发“机器人抢红包”功能
void OnRobotGrab()
{
    DoGrabCurrentWithName(NextRobotName());
}

// 点击背景封面图片时的事件处理
void OnCoverClickGrabCurrent()
{
    DoGrabCurrentFromInput();
}


int main()
{
    //事件绑定
    form.EventAdd(0, eForm_Load, OnFormLoad);
    form.EventAdd(ID_picCover, eStatic_Click, OnCoverClickGrabCurrent);

    form.EventAdd(ID_btnAGrab, eCommandButton_Click, OnAGrab);
    form.EventAdd(ID_btnAShow, eCommandButton_Click, OnAShow);

    form.EventAdd(ID_btnBGrab, eCommandButton_Click, OnBGrab);
    form.EventAdd(ID_btnBShow, eCommandButton_Click, OnBShow);

    form.EventAdd(ID_btnCFill, eCommandButton_Click, OnCFill);
    form.EventAdd(ID_btnCGrab, eCommandButton_Click, OnCGrab);
    form.EventAdd(ID_btnCShow, eCommandButton_Click, OnCShow);

    form.EventAdd(ID_btnRobotGrab, eCommandButton_Click, OnRobotGrab);

    //启动窗体
    form.Show();

    return 0;
}