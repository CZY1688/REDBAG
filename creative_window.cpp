#include "resource.h"
#include "BForm.h"
#include <strsafe.h>

// ============================================================================
// 创意窗口示例（基于现有模板）
// ----------------------------------------------------------------------------
// 设计目标：
// 1) 保持模板风格：仍然使用 CBForm + EventAdd + main() 入口。
// 2) 注释详细：每个关键状态、事件与行为都给出解释。
// 3) 富有新意：做一个“灵感雷达”窗口——鼠标像雷达扫描点，点击切换配色，
//    空格键切换“自动诗句模式”，回车键切换主题，Esc 退出程序。
//
// 使用方式：
// - 本文件是“新示例程序”，默认不替换当前 main.cpp。
// - 若你要运行本示例，可在工程中将本文件作为入口，或暂时替换 main.cpp 内容。
// ============================================================================

// 全局窗体对象：沿用模板资源 ID_form1（来自 template.rc）
static CBForm g_form(ID_form1);

// ------------------------- 全局状态（用于交互展示） --------------------------
// 鼠标当前位置（窗口客户区坐标）
static int g_mouseX = 0;
static int g_mouseY = 0;

// 点击计数：每次鼠标按下增加，用作“创意能量值”的一部分
static int g_clickCount = 0;

// 当前主题序号（0~4），控制前景/背景色
static int g_themeIndex = 0;

// 是否启用“自动诗句模式”
static bool g_autoPoem = true;

// 诗句资源：放在文件级，便于维护，也避免在高频 Paint 逻辑中声明。
static LPCTSTR g_poems[5] =
{
	TEXT("灵感诗句：把问题拆小，答案会主动靠近。"),
	TEXT("灵感诗句：先做可运行版本，再做优雅版本。"),
	TEXT("灵感诗句：复杂系统的起点，常是一个清晰按钮。"),
	TEXT("灵感诗句：当你能解释它，你就快实现它了。"),
	TEXT("灵感诗句：每次重绘，都是一次重新组织思路。")
};

// ----------------------------------------------------------------------------
// 按主题应用颜色。
// 说明：
// - 为了避免依赖外部素材，这里只使用模板可直接设置的窗口颜色属性。
// - 在 Paint 事件开始时调用，保证每次重绘都和主题同步。
// ----------------------------------------------------------------------------
static void ApplyTheme(int idx)
{
	switch (idx % 5)
	{
	case 0:
		g_form.BackColorSet(RGB(18, 24, 40));
		g_form.ForeColorSet(RGB(215, 235, 255));
		break;
	case 1:
		g_form.BackColorSet(RGB(30, 14, 36));
		g_form.ForeColorSet(RGB(255, 215, 235));
		break;
	case 2:
		g_form.BackColorSet(RGB(14, 38, 34));
		g_form.ForeColorSet(RGB(220, 255, 224));
		break;
	case 3:
		g_form.BackColorSet(RGB(42, 28, 10));
		g_form.ForeColorSet(RGB(255, 236, 200));
		break;
	default:
		g_form.BackColorSet(RGB(22, 22, 22));
		g_form.ForeColorSet(RGB(245, 245, 245));
		break;
	}
}

// ----------------------------------------------------------------------------
// 窗体加载事件：只做“初始配置”。
// ----------------------------------------------------------------------------
static void Form_Load()
{
	// 设定窗口标题（体现示例主题）
	g_form.TextSet(TEXT("灵感雷达 / Creative Radar (Template Demo)"));

	// 设置默认字体，便于文本展示（使用模板已有字体接口）
	g_form.FontNameSet(TEXT("Microsoft YaHei UI"));
	g_form.FontSizeSet(11.0f);

	// 让窗口出现在屏幕中央，并设置一个舒适尺寸
	g_form.MoveToScreenCenter(760, 470);

	// 打开 KeyPreview：让窗体先接收按键事件，便于做全局快捷键
	g_form.KeyPreview = true;

	// 应用初始主题
	ApplyTheme(g_themeIndex);

	// 主动刷新一次，确保初始 UI 即刻绘制
	g_form.Refresh();
}

// ----------------------------------------------------------------------------
// 鼠标移动事件：
// - 持续更新“雷达坐标”。
// - 在自动模式下，移动时立即刷新，让界面有“活的”感觉。
// ----------------------------------------------------------------------------
static void Form_MouseMove(int /*button*/, int /*shift*/, int x, int y)
{
	g_mouseX = x;
	g_mouseY = y;

	if (g_autoPoem)
	{
		g_form.Refresh();
	}
}

// ----------------------------------------------------------------------------
// 鼠标按下事件：
// - 左键：切换主题；
// - 右键：切换自动诗句模式；
// - 任意键：计数 +1，影响“创意能量值”。
// ----------------------------------------------------------------------------
static void Form_MouseDown(int button, int /*shift*/, int /*x*/, int /*y*/)
{
	++g_clickCount;

	// 按模板定义：button=1 左键，2 右键，4 中键
	if (button == 1)
	{
		g_themeIndex = (g_themeIndex + 1) % 5;
	}
	else if (button == 2)
	{
		g_autoPoem = !g_autoPoem;
	}

	g_form.Refresh();
}

// ----------------------------------------------------------------------------
// 按键事件：
// - Space：切换自动诗句模式；
// - Enter：切换主题；
// - Esc：退出程序。
// ----------------------------------------------------------------------------
static void Form_KeyDown(int keyCode, int /*shift*/, int /*cancelFlag*/)
{
	switch (keyCode)
	{
	case VK_SPACE:
		g_autoPoem = !g_autoPoem;
		g_form.Refresh();
		break;
	case VK_RETURN:
		g_themeIndex = (g_themeIndex + 1) % 5;
		g_form.Refresh();
		break;
	case VK_ESCAPE:
		// 结束消息循环，退出程序。
		// End() 是模板在 BForm.h 中声明的全局函数（封装了退出消息逻辑）。
		End(0);
		break;
	default:
		break;
	}
}

// ----------------------------------------------------------------------------
// 绘制事件（核心创意展示）：
// - 将鼠标位置 + 点击次数映射为“创意能量值”；
// - 根据能量值从文本库中选择一句“灵感诗句”；
// - 持续显示交互状态，形成可玩的小面板。
// ----------------------------------------------------------------------------
static void Form_Paint()
{
	ApplyTheme(g_themeIndex);
	g_form.Cls();

	// 能量值：纯演示算法，强调“输入行为 -> 可见反馈”
	const long long energyRaw =
		(static_cast<long long>(g_mouseX) * 3LL) +
		(static_cast<long long>(g_mouseY) * 5LL) +
		(static_cast<long long>(g_clickCount) * 11LL);
	const int energy = static_cast<int>(energyRaw % 1000LL);
	const int poemIndex = (energy / 200) % 5;

	TCHAR line[512] = {0};
	int y = 18;

	// 标题
	g_form.FontBoldSet(true);
	g_form.PrintText(TEXT("【创意窗口示例】灵感雷达"), 20, y);
	g_form.FontBoldSet(false);
	y += 34;

	// 动态状态区
	StringCchPrintf(line, _countof(line), TEXT("鼠标坐标：(%d, %d)"), g_mouseX, g_mouseY);
	g_form.PrintText(line, 20, y); y += 24;

	StringCchPrintf(line, _countof(line), TEXT("点击次数：%d"), g_clickCount);
	g_form.PrintText(line, 20, y); y += 24;

	StringCchPrintf(line, _countof(line), TEXT("创意能量值：%d / 999"), energy);
	g_form.PrintText(line, 20, y); y += 24;

	StringCchPrintf(line, _countof(line), TEXT("当前主题：#%d"), g_themeIndex + 1);
	g_form.PrintText(line, 20, y); y += 24;

	g_form.PrintText(g_autoPoem ? TEXT("自动诗句模式：ON") : TEXT("自动诗句模式：OFF"), 20, y);
	y += 30;

	// 创意文本区
	g_form.FontItalicSet(true);
	g_form.PrintText(g_poems[poemIndex], 20, y);
	g_form.FontItalicSet(false);
	y += 36;

	// 操作说明区（强调可探索性）
	g_form.PrintText(TEXT("操作提示："), 20, y); y += 22;
	g_form.PrintText(TEXT("1) 鼠标左键：切换主题"), 32, y); y += 22;
	g_form.PrintText(TEXT("2) 鼠标右键 / Space：切换自动诗句"), 32, y); y += 22;
	g_form.PrintText(TEXT("3) Enter：切换主题；Esc：退出"), 32, y); y += 22;

	// 底部彩蛋：根据能量值给一句“调试风格提示”
	y += 10;
	if (energy < 333)
	{
		g_form.PrintText(TEXT("状态建议：现在适合“写注释 + 搭骨架”。"), 20, y);
	}
	else if (energy < 666)
	{
		g_form.PrintText(TEXT("状态建议：现在适合“补交互 + 跑验证”。"), 20, y);
	}
	else
	{
		g_form.PrintText(TEXT("状态建议：现在适合“重构细节 + 打磨体验”。"), 20, y);
	}
}

int main()
{
	// 设置应用图标（沿用模板资源）
	g_form.IconSet(IDI_ICON1);

	// 注册窗体事件（idResControl=0 表示窗体本身事件）
	g_form.EventAdd(0, eForm_Load, Form_Load);
	g_form.EventAdd(0, ePaint, Form_Paint);
	g_form.EventAdd(0, eMouseMove, Form_MouseMove);
	g_form.EventAdd(0, eMouseDown, Form_MouseDown);
	g_form.EventAdd(0, eKeyDown, Form_KeyDown);

	// 显示主窗体
	g_form.Show();
	return 0;
}
