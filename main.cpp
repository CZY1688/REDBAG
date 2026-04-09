#include "resource.h"   
#include "BForm.h"     
#include "RedPacket.h"     
using namespace std;


// ��������״̬��������ǵ�ǰ�������ڲ鿴�ĸ����������
enum ECurrentPacket
{
    CurrentPacketA = 0,
    CurrentPacketB = 1,
    CurrentPacketC = 2
};

//�ı�ͳһ�������﷽���Ժ��޸�
#define PACKET_LABEL_PREFIX TEXT("���")
static const LPCTSTR TCN_WindowTitle = TEXT("ģ��΢�������");
static const LPCTSTR TCN_GroupA = TEXT("���A��Ǯ�����ã�ֱ�ӿ�����");
static const LPCTSTR TCN_GroupB = TEXT("���B��Ǯ�����ã�ֱ�ӿ�����");
static const LPCTSTR TCN_GroupC = TEXT("���C������Ǯ����������");
static const LPCTSTR TCN_Grab = TEXT("�����");
static const LPCTSTR TCN_View = TEXT("�鿴");
static const LPCTSTR TCN_RobotGrab = TEXT("�����������");
static const LPCTSTR TCN_CMoney = TEXT("Ǯ��(Ԫ)��");
static const LPCTSTR TCN_CNum = TEXT("�ּ��������");
static const LPCTSTR TCN_CFill = TEXT("��Ǯ�����");
static const LPCTSTR TCN_ResultDefault = TEXT("");
static const LPCTSTR TCN_TitleInfo = TEXT("��ʾ");
static const LPCTSTR TCN_TitleWarn = TEXT("����");
static const LPCTSTR TCN_AnonymousUser = TEXT("�����û�");

static const LPCTSTR PacketLabelA = PACKET_LABEL_PREFIX TEXT("A");
static const LPCTSTR PacketLabelB = PACKET_LABEL_PREFIX TEXT("B");
static const LPCTSTR PacketLabelC = PACKET_LABEL_PREFIX TEXT("C");


//ȫ�ֱ���

CBForm form(ID_form1);                             // ���������
RedPacket packetA(30.0, 5, TEXT("Owner A"));             // Ԥ����A��30Ԫ����5����
RedPacket packetB(50.0, 8, TEXT("Owner B"));             // Ԥ����B��50Ԫ����8����
RedPacket packetC(0.0, 1, TEXT("Owner C"));              // ���C����ʼ���0���ȴ��û�����
bool packetCReady = false;                         // ��Ǻ��C�Ƿ��Ѿ�����Ǯ
ECurrentPacket currentPacket = CurrentPacketA;     // ��¼��ǰ������Ҫ��ʾ�ĺ�� (Ĭ����A)
int robotIndex = 1;                                // ���������Ƶ��ۼ����

//�¼���������

void UpdateLeftFooter(const RedPacket& packet, LPCTSTR packetLabel);
void ShowPacketLog(const RedPacket& packet, LPCTSTR packetLabel);
void DoGrabWithName(RedPacket& packet, const tstring& who, LPCTSTR packetLabel, bool showResultText);
void DoGrab(RedPacket& packet, unsigned short idNameEdit, LPCTSTR packetLabel, bool checkReady, bool showResultText);




// �Զ����ɻ���������
tstring NextRobotName()
{
    TCHAR robotName[64] = { 0 };
    _stprintf(robotName, TEXT("������%d"), robotIndex++);
    return tstring(robotName);
}

//��ͨ��Ϣ��ʾ
int ShowInfoBox(LPCTSTR msg)
{
    return MsgBox(msg, TCN_TitleInfo, mb_OK, mb_IconInformation);
}

//����������ʾ
int ShowWarnBox(LPCTSTR msg)
{
    return MsgBox(msg, TCN_TitleWarn, mb_OK, mb_IconExclamation);
}

//��ָ����������ж�ȡ�û�������ı�
tstring ReadText(unsigned short idEdit)
{
    return form.Control(idEdit, false).Text();
}

//��ȡ�û���������û�ʲô��û���Ĭ�ϸ����������û���
tstring ReadNameOrDefault(unsigned short idEdit)
{
    tstring s = ReadText(idEdit);
    if (s.empty()) return TCN_AnonymousUser;
    return s;
}

//�����ײ�ļ�¼�ַ������� "����:���" ��ֿ���
bool ParseNameMoney(const tstring& src, tstring& outName, tstring& outMoney)
{
    size_t pos = src.rfind(TEXT(":"));
    if (pos == tstring::npos) return false;
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
    LPCTSTR textGrab = TCN_Grab;
    LPCTSTR textView = TCN_View;

    form.Control(ID_grpA, false).TextSet(TCN_GroupA);
    form.Control(ID_grpB, false).TextSet(TCN_GroupB);
    form.Control(ID_grpC, false).TextSet(TCN_GroupC);

    form.Control(ID_btnAGrab, false).TextSet(textGrab);
    form.Control(ID_btnAShow, false).TextSet(textView);
    form.Control(ID_btnBGrab, false).TextSet(textGrab);
    form.Control(ID_btnBShow, false).TextSet(textView);

    form.Control(ID_txtCMoney, false).TextSet(TCN_CMoney);
    form.Control(ID_txtCNum, false).TextSet(TCN_CNum);
    form.Control(ID_btnCFill, false).TextSet(TCN_CFill);
    form.Control(ID_btnCGrab, false).TextSet(textGrab);
    form.Control(ID_btnCShow, false).TextSet(textView);

    form.Control(ID_btnRobotGrab, false).TextSet(TCN_RobotGrab);
    form.Control(ID_txtResult, false).TextSet(TCN_ResultDefault);
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
    tstring best = packet.bestLuckRecord();
    TCHAR msg[256] = { 0 };
    if (best.empty())
    {
        _stprintf(msg, TEXT("%s������ѣ����ޡ�"), packetLabel);
    }
    else
    {
        tstring who, money;
        if (ParseNameMoney(best, who, money))
        {
            _stprintf(msg, TEXT("%s������ѣ�%s��%s Ԫ��"),
                packetLabel, who.c_str(), money.c_str());
        }
        else
        {
            _stprintf(msg, TEXT("%s������ѣ�%s��"), packetLabel, best.c_str());
        }
    }
    ShowInfoBox(msg);
}

// �ڽ��涥������������ļ�̽��
void UpdateResultText(const tstring& grabber, double money)
{
    TCHAR line[256] = { 0 };
    _stprintf(line, TEXT("��ϲ��%s ���� %.2f Ԫ�����"), grabber.c_str(), money);
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

    const tstring* pRecs = packet.records();
    int count = packet.grabbedCount();
    tstring best = packet.bestLuckRecord();

    AppendLog(TEXT("�������¼��"));

    // ��������������¼
    for (int i = 0; i < count; ++i)
    {
        tstring who, money;
        TCHAR line[256] = { 0 };

        if (ParseNameMoney(pRecs[i], who, money))
        {
            // ���������¼��������ѣ��Ӹ�������
            if (!best.empty() && pRecs[i] == best)
            {
                _stprintf(line, TEXT("%s ������ %s Ԫ��������ѣ�"), who.c_str(), money.c_str());
            }
            else
            {
                _stprintf(line, TEXT("%s ������ %s Ԫ"), who.c_str(), money.c_str());
            }
        }
        else
        {
            _stprintf(line, TEXT("%s"), pRecs[i].c_str());
        }
        AppendLog(line);
    }

    if (count == 0) AppendLog(TEXT("����������¼"));

    // ͬ���������½ǵ�״̬��ʾ
    UpdateLeftFooter(packet, packetLabel);
}


// ���������
void DoGrabWithName(RedPacket& packet, const tstring& who, LPCTSTR packetLabel, bool showResultText)
{
    int status = RedPacket::GrabEmpty;
    double got = packet.grab(who, &status); // ִ�����������

    // ���1�����û��Ѿ������˺��
    if (status == RedPacket::GrabDuplicate)
    {
        TCHAR msg[256] = { 0 };
        _stprintf(msg, TEXT("%s �Ѿ�����%s��ÿ���û�ֻ����һ�Ρ�"), who.c_str(), packetLabel);
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
    _stprintf(msg, TEXT("��ϲ��%s ��%s���� %.2f Ԫ��"), who.c_str(), packetLabel, got);
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

    tstring who = ReadNameOrDefault(idNameEdit);
    DoGrabWithName(packet, who, packetLabel, showResultText);
}

// �����������
void DoGrabCurrentWithName(const tstring& who)
{
    if (currentPacket == CurrentPacketA)
    {
        DoGrabWithName(packetA, who, PacketLabelA, false);
    }
    else if (currentPacket == CurrentPacketB)
    {
        DoGrabWithName(packetB, who, PacketLabelB, true);
    }
    else // ��ǰ�Ǻ��C
    {
        if (!packetCReady)
        {
            UpdateLeftFooter(packetC, PacketLabelC);
            ShowWarnBox(TEXT("���C��û��Ǯ�����������Ǯ������"));
            return;
        }
        DoGrabWithName(packetC, who, PacketLabelC, false);
    }
}

// �������ͼʱ�Ŀ�������߼�
void DoGrabCurrentFromInput()
{
    if (currentPacket == CurrentPacketA)
    {
        DoGrab(packetA, ID_editAName, PacketLabelA, false, false);
    }
    else if (currentPacket == CurrentPacketB)
    {
        DoGrab(packetB, ID_editBName, PacketLabelB, false, true);
    }
    else
    {
        DoGrab(packetC, ID_editCName, PacketLabelC, true, false);
    }
}

// ���ڳ�ʼ��
void OnFormLoad()
{
    form.IconSet(IDI_ICON1);
    form.TextSet(TCN_WindowTitle);
    form.MoveToScreenCenter();
    form.BackColorSet(RGB(236, 236, 236));
    form.KeyPreview = true;

    // ������Դ�ļ��еķ���ͼƬ
    form.Control(ID_picCover, false).PictureSet(IDB_PACKET_COVER);

    ApplyRuntimeTexts();
    ResetUIState();

    // ��ʼ״̬�£��ײ�״̬����ʾ���A����Ϣ
    UpdateLeftFooter(packetA, PacketLabelA);
}

// ��������ġ����������ť����¼�
void OnAGrab() { DoGrab(packetA, ID_editAName, PacketLabelA, false, false); }
void OnBGrab() { DoGrab(packetB, ID_editBName, PacketLabelB, false, true); }
void OnCGrab() { DoGrab(packetC, ID_editCName, PacketLabelC, true, false); }

// ��������ġ��鿴���顱��ť����¼�
void OnAShow() { currentPacket = CurrentPacketA; ShowPacketLog(packetA, PacketLabelA); }
void OnBShow() { currentPacket = CurrentPacketB; ShowPacketLog(packetB, PacketLabelB); }
void OnCShow() { currentPacket = CurrentPacketC; ShowPacketLog(packetC, PacketLabelC); }

// �������Cר���ġ���Ǯ���߼�
void OnCFill()
{
    // ��ȡ���͸���
    double money = form.Control(ID_editCMoney, false).TextVal();
    int count = static_cast<int>(form.Control(ID_editCNum, false).TextVal());

    // ���ݺϷ���У��
    if (money <= 0.0 || count <= 0)
    {
        UpdateLeftFooter(packetC, PacketLabelC);
        ShowWarnBox(TEXT("��Ǯʧ�ܣ����͸������������0��"));
        return;
    }

    // ״̬У�飺��ֹ��;������Ǯ������Ŀ����
    if (!packetC.canSetMoney())
    {
        UpdateLeftFooter(packetC, PacketLabelC);
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
    ShowPacketLog(packetC, PacketLabelC);
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
