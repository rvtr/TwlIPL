// MasterEditorTWL_Launcher.cpp : メイン プロジェクト ファイルです。

#include "stdafx.h"
#include "../../MasterEditorTWL/MasterEditorTWL/Form1.h"

//using namespace MasterEditorTWL_Launcher;
using namespace MasterEditorTWL;

[STAThreadAttribute]
int main(array<System::String ^> ^args)
{
	// コントロールが作成される前に、Windows XP ビジュアル効果を有効にします
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 

	// メイン ウィンドウを作成して、実行します
	Application::Run(gcnew Form1());
	return 0;
}
