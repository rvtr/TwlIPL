// ResourceTransition.cpp : メイン プロジェクト ファイルです。

#include "stdafx.h"

using namespace System;

#define ROOT_PATH "..\\"

int main(array<System::String ^> ^args)
{
	// 作業ルートを引数からゲット
	System::String ^root;
	if( (args->Length <= 0) || (System::String::IsNullOrEmpty( args[0] )) )
	{
		root = gcnew System::String(ROOT_PATH);		// ない場合はデフォルト
	}
	else
	{
		root = args[0];
		if( !root->EndsWith( "\\" ) )
		{
			root += "\\";
		}
	}
	Console::WriteLine( "Root path: " + root );

	// 新しいフォルダ/jp/言語リソース.dll を jp/ に移す(コピー)
	try
	{
		if( System::IO::File::Exists( root + "新しいフォルダ\\ja\\MasterEditorTWL.resources.dll" ) )
		{
			Console::WriteLine( "File Exists." );
			if( !System::IO::Directory::Exists( root + "ja" ) )
			{
				System::IO::Directory::CreateDirectory( root + "ja" );
				Console::WriteLine( "Create New Directory." );
			}
			System::IO::File::Copy( root + "新しいフォルダ\\ja\\MasterEditorTWL.resources.dll", 
									root + "ja\\MasterEditorTWL.resources.dll", true );	// 上書き
			Console::WriteLine( "Copy Resource DLL." );
		}
		else
		{
			Console::WriteLine( "No File Exists." );
		}
	}
	catch( System::Exception ^ex )
	{
		Console::WriteLine( "error: " + ex->Message );
	}

	//Console::ReadKey();

	return 0;
}
