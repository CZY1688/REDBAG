#include "resource.h"   
#include "BForm.h"     
#include "RedPacket.h"     
#include <string>

using namespace std;


// ��������״̬��������ǵ�ǰ�������ڲ鿴�ĸ����������
enum ECurrentPacket
{
    CurrentPacketA = 0,
    CurrentPacketB = 1,
    CurrentPacketC = 2
};

//�ı�ͳһ�������﷽���Ժ��޸�
static const TCHAR kWindowTitle[] = TEXT("ģ��΢�������");
static const TCHAR kGroupA[] = TEXT("���A��Ǯ�����ã�ֱ�ӿ�����");
static const TCHAR kGroupB[] = TEXT("���B��Ǯ�����ã�ֱ�ӿ�����");
static const TCHAR kGroupC[] = TEXT("���C������Ǯ����������");
static const TCHAR kGrab[] = TEXT("�����");
static const TCHAR kView[] = TEXT("�鿴");
static const TCHAR kRobotGrab[] = TEXT("�����������");
static const TCHAR kCMoney[] = TEXT("Ǯ��(Ԫ)��");
static const TCHAR kCNum[] = TEXT("�ּ��������");
static const TCHAR kCFill[] = TEXT("��Ǯ�����");
static const TCHAR kResultDefault[] = TEXT("");
static const TCHAR kTitleInfo[] = TEXT("��ʾ");
static const TCHAR kTitleWarn[] = TEXT("����");
static const TCHAR kAnonymousUser[] = TEXT("�����û�");
static const TCHAR kRobotNamePrefix[] = TEXT("机器人");
static const TCHAR kPacketLabelA[] = TEXT("���A");
static const TCHAR kPacketLabelB[] = TEXT("���B");
static const TCHAR kPacketLabelC[] = TEXT("���C");


//ȫ�ֱ���

CBForm form(ID_form1);                             // ���������
RedPacket packetA(30.0, 5, "Owner A");             // Ԥ����A��30Ԫ����5����
RedPacket packetB(50.0, 8, "Owner B");             // Ԥ����B��50Ԫ����8����
RedPacket packetC(0.0, 1, "Owner C");              // ���C����ʼ���0���ȴ��û�����
bool packetCReady = false;                         // ��Ǻ��C�Ƿ��Ѿ�����Ǯ
ECurrentPacket currentPacket = CurrentPacketA;     // ��¼��ǰ������Ҫ��ʾ�ĺ�� (Ĭ����A)
int robotIndex = 1;                                // ���������Ƶ��ۼ����

//�¼���������

void UpdateLeftFooter(const RedPacket& packet, LPCTSTR packetLabel);
void ShowPacketLog(const RedPacket& packet, LPCTSTR packetLabel);
void DoGrabWithName(RedPacket& packet, const string& who, LPCTSTR packetLabel, bool showResultText);
void DoGrab(RedPacket& packet, unsigned short idNameEdit, LPCTSTR packetLabel, bool checkReady, bool showResultText);


// �����ַ����룺����׼ string תΪ Windows API ��Ҫ�Ŀ��ַ� tstring
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

// �����ַ����룺�����ַ� tstring ת�ر�׼ string
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

// �Զ����ɻ���������
string NextRobotName()
{
    TCHAR robotName[64] = { 0 };
    _stprintf(robotName, TEXT("%s%d"), kRobotNamePrefix, robotIndex++);
    return ToString(robotName);
}

//��ͨ��Ϣ��ʾ
void ShowInfoBox(LPCTSTR msg)
{
    MsgBox(msg, kTitleInfo, mb_OK, mb_IconInformation);
}

//����������ʾ
void ShowWarnBox(LPCTSTR msg)
{
    MsgBox(msg, kTitleWarn, mb_OK, mb_IconExclamation);
}

//��ָ����������ж�ȡ�û�������ı�
tstring ReadText(unsigned short idEdit)
{
    return form.Control(idEdit, false).Text();
}

//��ȡ�û���������û�ʲô��û���Ĭ�ϸ����������û���
string ReadNameOrDefault(unsigned short idEdit)
{
    tstring s = ReadText(idEdit);
    if (s.empty()) return ToString(kAnonymousUser);
    return ToString(s);
}

//�����ײ�ļ�¼�ַ������� "����:���" ��ֿ���
bool ParseNameMoney(const string& src, string& outName, string& outMoney)
{
    size_t pos = src.rfind(':');
    if (pos == string::npos) return false;
    outName = src.substr(0, pos);
    outMoney = src.substr(pos + 1);
    return true;
}

//����־��׷��һ������
void AppendLog(LPCTSTR s)
{
    form.Control(ID_editLog, false).TextAdd(s);
    form.Control(ID_editLog, false).TextAdd(TEXT("\r\n"));
}


// ��ʼ�����пؼ��Ĺ̶��ı�����
void ApplyRuntimeTexts()
{
    LPCTSTR textGrab = kGrab;
    LPCTSTR textView = kView;

    form.Control(ID_grpA, false).TextSet(kGroupA);
    form.Control(ID_grpB, false).TextSet(kGroupB);
    form.Control(ID_grpC, false).TextSet(kGroupC);

    form.Control(ID_btnAGrab, false).TextSet(textGrab);
    form.Control(ID_btnAShow, false).TextSet(textView);
    form.Control(ID_btnBGrab, false).TextSet(textGrab);
    form.Control(ID_btnBShow, false).TextSet(textView);

    form.Control(ID_txtCMoney, false).TextSet(kCMoney);
    form.Control(ID_txtCNum, false).TextSet(kCNum);
    form.Control(ID_btnCFill, false).TextSet(kCFill);
    form.Control(ID_btnCGrab, false).TextSet(textGrab);
    form.Control(ID_btnCShow, false).TextSet(textView);

    form.Control(ID_btnRobotGrab, false).TextSet(kRobotGrab);
    form.Control(ID_txtResult, false).TextSet(kResultDefault);
    form.Control(ID_txtSep, false).VisibleSet(false);
}

// ���ý��浽���ʼ��״̬
void ResetUIState()
{
    form.Control(ID_editLog, false).TextSet(TEXT("")); // �����־��

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

    // ���C��δ��Ǯ֮ǰ���������ť�豣�ֽ���״̬
    form.Control(ID_btnCGrab, false).EnabledSet(false);

    // ȷ�����ֿؼ��Ĳ㼶������ʾ
    form.Control(ID_picCover, false).ZOrder(1);
    form.Control(ID_grpA, false).ZOrder(1);
    form.Control(ID_grpB, false).ZOrder(1);
    form.Control(ID_grpC, false).ZOrder(1);
}

// ��鵱ǰ������������������������չʾ
void ShowBestLuckMsg(const RedPacket& packet, LPCTSTR packetLabel)
{
    string best = packet.bestLuckRecord();
    TCHAR msg[256] = { 0 };
    if (best.empty())
    {
        _stprintf(msg, TEXT("%s������ѣ����ޡ�"), packetLabel);
    }
    else
    {
        string who, money;
        if (ParseNameMoney(best, who, money))
        {
            _stprintf(msg, TEXT("%s������ѣ�%s��%s Ԫ��"),
                packetLabel, ToTString(who).c_str(), ToTString(money).c_str());
        }
        else
        {
            _stprintf(msg, TEXT("%s������ѣ�%s��"), packetLabel, ToTString(best).c_str());
        }
    }
    ShowInfoBox(msg);
}

// �ڽ��涥������������ļ�̽��
void UpdateResultText(const string& grabber, double money)
{
    TCHAR line[256] = { 0 };
    _stprintf(line, TEXT("��ϲ��%s ���� %.2f Ԫ�����"), ToTString(grabber).c_str(), money);
    form.Control(ID_txtResult, false).TextSet(line);
}

// ���½������½���ʾ�ĺ��ʣ�����
void UpdateLeftFooter(const RedPacket& packet, LPCTSTR packetLabel)
{
    TCHAR line[256] = { 0 };
    int grabbed = packet.grabbedCount();
    int total = packet.totalCount();
    int left = total - grabbed;
    if (left < 0) left = 0;

    _stprintf(line, TEXT("%s������ %d ����ʣ�� %d ��"), packetLabel, grabbed, left);
    form.Control(ID_txtLeftStatic, false).TextSet(line);
}

// ˢ����־������ָ���������ϸ������¼��ӡ����
void ShowPacketLog(const RedPacket& packet, LPCTSTR packetLabel)
{
    form.Control(ID_editLog, false).TextSet(TEXT("")); // ����վ���־

    const string* pRecs = packet.records();
    int count = packet.grabbedCount();
    string best = packet.bestLuckRecord();

    AppendLog(TEXT("�������¼��"));

    // ��������������¼
    for (int i = 0; i < count; ++i)
    {
        string who, money;
        TCHAR line[256] = { 0 };

        if (ParseNameMoney(pRecs[i], who, money))
        {
            // ���������¼��������ѣ��Ӹ�������
            if (!best.empty() && pRecs[i] == best)
            {
                _stprintf(line, TEXT("%s ������ %s Ԫ��������ѣ�"), ToTString(who).c_str(), ToTString(money).c_str());
            }
            else
            {
                _stprintf(line, TEXT("%s ������ %s Ԫ"), ToTString(who).c_str(), ToTString(money).c_str());
            }
        }
        else
        {
            _stprintf(line, TEXT("%s"), ToTString(pRecs[i]).c_str());
        }
        AppendLog(line);
    }

    if (count == 0) AppendLog(TEXT("����������¼"));

    // ͬ���������½ǵ�״̬��ʾ
    UpdateLeftFooter(packet, packetLabel);
}


// ���������
void DoGrabWithName(RedPacket& packet, const string& who, LPCTSTR packetLabel, bool showResultText)
{
    int status = RedPacket::GrabEmpty;
    double got = packet.grab(who, &status); // ִ�����������

    // ���1�����û��Ѿ������˺��
    if (status == RedPacket::GrabDuplicate)
    {
        TCHAR msg[256] = { 0 };
        _stprintf(msg, TEXT("%s �Ѿ�����%s��ÿ���û�ֻ����һ�Ρ�"), ToTString(who).c_str(), packetLabel);
        UpdateLeftFooter(packet, packetLabel);
        ShowWarnBox(msg);
        return;
    }

    // ���2������Ѿ���������
    if (got <= 0.0)
    {
        TCHAR msg[256] = { 0 };
        _stprintf(msg, TEXT("%s�����꣬�����ˣ�"), packetLabel);
        UpdateLeftFooter(packet, packetLabel);
        ShowWarnBox(msg);

        if (showResultText) form.Control(ID_txtResult, false).TextSet(TEXT("�����ˣ���������ꡣ"));
        ShowBestLuckMsg(packet, packetLabel); // ������ʱ������һ���������
        return;
    }

    // ���3���ɹ��������
    TCHAR msg[256] = { 0 };
    _stprintf(msg, TEXT("��ϲ��%s ��%s���� %.2f Ԫ��"), ToTString(who).c_str(), packetLabel, got);
    ShowInfoBox(msg);

    if (showResultText) UpdateResultText(who, got);

    // �����ˢ����־���
    ShowPacketLog(packet, packetLabel);
}

// ��������
void DoGrab(RedPacket& packet, unsigned short idNameEdit, LPCTSTR packetLabel, bool checkReady, bool showResultText)
{
    // ����Ǳ�������Ǯ�ĺ����������C��������Ƿ���׼����
    if (checkReady && !packetCReady)
    {
        UpdateLeftFooter(packet, packetLabel);
        ShowWarnBox(TEXT("�˺����δ��Ǯ���������ý��͸�����"));
        return;
    }

    string who = ReadNameOrDefault(idNameEdit);
    DoGrabWithName(packet, who, packetLabel, showResultText);
}

// �����������
void DoGrabCurrentWithName(const string& who)
{
    if (currentPacket == CurrentPacketA)
    {
        DoGrabWithName(packetA, who, kPacketLabelA, false);
    }
    else if (currentPacket == CurrentPacketB)
    {
        DoGrabWithName(packetB, who, kPacketLabelB, true);
    }
    else // ��ǰ�Ǻ��C
    {
        if (!packetCReady)
        {
            UpdateLeftFooter(packetC, kPacketLabelC);
            ShowWarnBox(TEXT("���C��û��Ǯ�����������Ǯ������"));
            return;
        }
        DoGrabWithName(packetC, who, kPacketLabelC, false);
    }
}

// �������ͼʱ�Ŀ�������߼�
void DoGrabCurrentFromInput()
{
    if (currentPacket == CurrentPacketA)
    {
        DoGrab(packetA, ID_editAName, kPacketLabelA, false, false);
    }
    else if (currentPacket == CurrentPacketB)
    {
        DoGrab(packetB, ID_editBName, kPacketLabelB, false, true);
    }
    else
    {
        DoGrab(packetC, ID_editCName, kPacketLabelC, true, false);
    }
}

// ���ڳ�ʼ��
void OnFormLoad()
{
    form.IconSet(IDI_ICON1);
    form.TextSet(kWindowTitle);
    form.MoveToScreenCenter();
    form.BackColorSet(RGB(236, 236, 236));
    form.KeyPreview = true;

    // ������Դ�ļ��еķ���ͼƬ
    form.Control(ID_picCover, false).PictureSet(IDB_PACKET_COVER);

    ApplyRuntimeTexts();
    ResetUIState();

    // ��ʼ״̬�£��ײ�״̬����ʾ���A����Ϣ
    UpdateLeftFooter(packetA, kPacketLabelA);
}

// ��������ġ����������ť����¼�
void OnAGrab() { DoGrab(packetA, ID_editAName, kPacketLabelA, false, false); }
void OnBGrab() { DoGrab(packetB, ID_editBName, kPacketLabelB, false, true); }
void OnCGrab() { DoGrab(packetC, ID_editCName, kPacketLabelC, true, false); }

// ��������ġ��鿴���顱��ť����¼�
void OnAShow() { currentPacket = CurrentPacketA; ShowPacketLog(packetA, kPacketLabelA); }
void OnBShow() { currentPacket = CurrentPacketB; ShowPacketLog(packetB, kPacketLabelB); }
void OnCShow() { currentPacket = CurrentPacketC; ShowPacketLog(packetC, kPacketLabelC); }

// �������Cר���ġ���Ǯ���߼�
void OnCFill()
{
    // ��ȡ���͸���
    double money = form.Control(ID_editCMoney, false).TextVal();
    int count = static_cast<int>(form.Control(ID_editCNum, false).TextVal());

    // ���ݺϷ���У��
    if (money <= 0.0 || count <= 0)
    {
        UpdateLeftFooter(packetC, kPacketLabelC);
        ShowWarnBox(TEXT("��Ǯʧ�ܣ����͸������������0��"));
        return;
    }

    // ״̬У�飺��ֹ��;������Ǯ������Ŀ����
    if (!packetC.canSetMoney())
    {
        UpdateLeftFooter(packetC, kPacketLabelC);
        ShowWarnBox(TEXT("���C�����û�����������������Ǯ��"));
        return;
    }

    // ���õײ����ݣ������Ž��������Ȩ��
    packetC.setMoney(money, count);
    packetCReady = true;
    form.Control(ID_btnCGrab, false).EnabledSet(true);

    TCHAR msg[128] = { 0 };
    _stprintf(msg, TEXT("��Ǯ�ɹ��������� %.2f Ԫ���� %d �������"), money, count);
    ShowInfoBox(msg);
    ShowPacketLog(packetC, kPacketLabelC);
}

// �����������������������
void OnRobotGrab()
{
    DoGrabCurrentWithName(NextRobotName());
}

// �����������ͼƬʱ���¼�����
void OnCoverClickGrabCurrent()
{
    DoGrabCurrentFromInput();
}


int main()
{
    //�¼���
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

    //��������
    form.Show();

    return 0;
}
