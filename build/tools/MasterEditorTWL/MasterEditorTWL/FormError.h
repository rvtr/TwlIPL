#pragma once

#include <apptype.h>
#include "common.h"
#include "srl.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace MasterEditorTWL {

	/// <summary>
	/// FormError の概要
	///
	/// 警告: このクラスの名前を変更する場合、このクラスが依存するすべての .resx ファイルに関連付けられた
	///          マネージ リソース コンパイラ ツールに対して 'Resource File Name' プロパティを
	///          変更する必要があります。この変更を行わないと、
	///          デザイナと、このフォームに関連付けられたローカライズ済みリソースとが、
	///          正しく相互に利用できなくなります。
	/// </summary>
	public ref class FormError : public System::Windows::Forms::Form
	{
	public:
		FormError(void)
		{
			InitializeComponent();
			//
			//TODO: ここにコンストラクタ コードを追加します
			//
		}

	protected:
		/// <summary>
		/// 使用中のリソースをすべてクリーンアップします。
		/// </summary>
		~FormError()
		{
			if (components)
			{
				delete components;
			}
		}
	public: System::Windows::Forms::DataGridView^  gridError;
	protected: 

	protected: 





	private: System::Windows::Forms::Label^  labError;
	public: System::Windows::Forms::DataGridView^  gridWarn;
	private: 


	public: 



	private: System::Windows::Forms::DataGridViewTextBoxColumn^  colWarnName;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  colWarnBegin;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  colWarnEnd;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  colWarnCause;
	private: System::Windows::Forms::Label^  labWarn;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  colErrorName;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  colErrorBegin;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  colErrorEnd;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  colErrorCause;
	private: System::Windows::Forms::Button^  butClose;

	private: 

	public: 







	protected: 

	private:
		/// <summary>
		/// 必要なデザイナ変数です。
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// デザイナ サポートに必要なメソッドです。このメソッドの内容を
		/// コード エディタで変更しないでください。
		/// </summary>
		void InitializeComponent(void)
		{
			this->gridError = (gcnew System::Windows::Forms::DataGridView());
			this->colErrorName = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->colErrorBegin = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->colErrorEnd = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->colErrorCause = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->labError = (gcnew System::Windows::Forms::Label());
			this->gridWarn = (gcnew System::Windows::Forms::DataGridView());
			this->colWarnName = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->colWarnBegin = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->colWarnEnd = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->colWarnCause = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->labWarn = (gcnew System::Windows::Forms::Label());
			this->butClose = (gcnew System::Windows::Forms::Button());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->gridError))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->gridWarn))->BeginInit();
			this->SuspendLayout();
			// 
			// gridError
			// 
			this->gridError->BackgroundColor = System::Drawing::SystemColors::Control;
			this->gridError->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
			this->gridError->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(4) {this->colErrorName, 
				this->colErrorBegin, this->colErrorEnd, this->colErrorCause});
			this->gridError->GridColor = System::Drawing::SystemColors::Control;
			this->gridError->Location = System::Drawing::Point(28, 36);
			this->gridError->Name = L"gridError";
			this->gridError->ReadOnly = true;
			this->gridError->RowHeadersVisible = false;
			this->gridError->RowTemplate->Height = 21;
			this->gridError->Size = System::Drawing::Size(661, 172);
			this->gridError->TabIndex = 0;
			// 
			// colErrorName
			// 
			this->colErrorName->HeaderText = L"項目名";
			this->colErrorName->Name = L"colErrorName";
			this->colErrorName->ReadOnly = true;
			this->colErrorName->Width = 150;
			// 
			// colErrorBegin
			// 
			this->colErrorBegin->HeaderText = L"開始";
			this->colErrorBegin->Name = L"colErrorBegin";
			this->colErrorBegin->ReadOnly = true;
			this->colErrorBegin->Width = 60;
			// 
			// colErrorEnd
			// 
			this->colErrorEnd->HeaderText = L"終了";
			this->colErrorEnd->Name = L"colErrorEnd";
			this->colErrorEnd->ReadOnly = true;
			this->colErrorEnd->Width = 60;
			// 
			// colErrorCause
			// 
			this->colErrorCause->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::Fill;
			this->colErrorCause->HeaderText = L"要因";
			this->colErrorCause->Name = L"colErrorCause";
			this->colErrorCause->ReadOnly = true;
			// 
			// labError
			// 
			this->labError->AutoSize = true;
			this->labError->ForeColor = System::Drawing::SystemColors::ActiveCaption;
			this->labError->Location = System::Drawing::Point(26, 21);
			this->labError->Name = L"labError";
			this->labError->Size = System::Drawing::Size(137, 12);
			this->labError->TabIndex = 1;
			this->labError->Text = L"エラー(必ず修正してください)";
			// 
			// gridWarn
			// 
			this->gridWarn->BackgroundColor = System::Drawing::SystemColors::Control;
			this->gridWarn->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
			this->gridWarn->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(4) {this->colWarnName, 
				this->colWarnBegin, this->colWarnEnd, this->colWarnCause});
			this->gridWarn->GridColor = System::Drawing::SystemColors::Control;
			this->gridWarn->Location = System::Drawing::Point(28, 245);
			this->gridWarn->Name = L"gridWarn";
			this->gridWarn->ReadOnly = true;
			this->gridWarn->RowHeadersVisible = false;
			this->gridWarn->RowTemplate->Height = 21;
			this->gridWarn->Size = System::Drawing::Size(661, 172);
			this->gridWarn->TabIndex = 2;
			// 
			// colWarnName
			// 
			this->colWarnName->HeaderText = L"項目名";
			this->colWarnName->Name = L"colWarnName";
			this->colWarnName->ReadOnly = true;
			this->colWarnName->Width = 150;
			// 
			// colWarnBegin
			// 
			this->colWarnBegin->HeaderText = L"開始";
			this->colWarnBegin->Name = L"colWarnBegin";
			this->colWarnBegin->ReadOnly = true;
			this->colWarnBegin->Width = 60;
			// 
			// colWarnEnd
			// 
			this->colWarnEnd->HeaderText = L"終了";
			this->colWarnEnd->Name = L"colWarnEnd";
			this->colWarnEnd->ReadOnly = true;
			this->colWarnEnd->Width = 60;
			// 
			// colWarnCause
			// 
			this->colWarnCause->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::Fill;
			this->colWarnCause->HeaderText = L"要因";
			this->colWarnCause->Name = L"colWarnCause";
			this->colWarnCause->ReadOnly = true;
			// 
			// labWarn
			// 
			this->labWarn->AutoSize = true;
			this->labWarn->ForeColor = System::Drawing::SystemColors::ActiveCaption;
			this->labWarn->Location = System::Drawing::Point(26, 230);
			this->labWarn->Name = L"labWarn";
			this->labWarn->Size = System::Drawing::Size(331, 12);
			this->labWarn->TabIndex = 3;
			this->labWarn->Text = L"警告(修正は必須ではありませんが情報に誤りがないかご確認ください)";
			// 
			// butClose
			// 
			this->butClose->Location = System::Drawing::Point(557, 429);
			this->butClose->Name = L"butClose";
			this->butClose->Size = System::Drawing::Size(132, 23);
			this->butClose->TabIndex = 4;
			this->butClose->Text = L"閉じる";
			this->butClose->UseVisualStyleBackColor = true;
			this->butClose->Click += gcnew System::EventHandler(this, &FormError::butClose_Click);
			// 
			// FormError
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(722, 464);
			this->Controls->Add(this->butClose);
			this->Controls->Add(this->labWarn);
			this->Controls->Add(this->gridWarn);
			this->Controls->Add(this->labError);
			this->Controls->Add(this->gridError);
			this->Name = L"FormError";
			this->Text = L"Error Information";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->gridError))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->gridWarn))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

		// --------------------------------------------------------
		// エラー情報の登録
		// --------------------------------------------------------
	public:
		void setGridError( System::Collections::Generic::List<RCMrcError ^> ^list, System::Boolean isJapanese )
		{
			this->gridError->Rows->Clear();
			if( list != nullptr )
			{
				for each( RCMrcError ^err in list )
				{
					this->gridError->Rows->Add( err->getAll( isJapanese ) );
				}
			}
		}

		void setGridWarn( System::Collections::Generic::List<RCMrcError ^> ^list, System::Boolean isJapanese )
		{
			this->gridWarn->Rows->Clear();
			if( list != nullptr )
			{
				for each( RCMrcError ^err in list )
				{
					this->gridWarn->Rows->Add( err->getAll( isJapanese ) );
				}
			}
		}

		// --------------------------------------------------------
		// エラー情報の重ね合わせ
		// --------------------------------------------------------
	public:
		void overloadGridError( 
			System::Collections::Generic::List<RCMrcError ^> ^listLo,
			System::Collections::Generic::List<RCMrcError ^> ^listHi, System::Boolean isJapanese )
		{
			// listLoの修正可能な情報をlistHiで上書きする(listHiは修正可能な情報のみであることが前提)
			this->gridError->Rows->Clear();
			if( listLo != nullptr )
			{
				for each( RCMrcError ^err in listLo )
				{
					if( !err->EnableModify )	// 修正可能な情報は表示しない
						this->gridError->Rows->Add( err->getAll( isJapanese ) );
				}
			}
			if( listHi != nullptr )
			{
				for each( RCMrcError ^err in listHi )
				{
					this->gridError->Rows->Add( err->getAll( isJapanese ) );
				}
			}
		}
		void overloadGridWarn( 
			System::Collections::Generic::List<RCMrcError ^> ^listLo,
			System::Collections::Generic::List<RCMrcError ^> ^listHi, System::Boolean isJapanese )
		{
			// listLoの修正可能な情報をlistHiで上書きする(listHiは修正可能な情報のみであることが前提)
			this->gridWarn->Rows->Clear();
			if( listLo != nullptr )
			{
				for each( RCMrcError ^err in listLo )
				{
					if( !err->EnableModify )	// 修正可能な情報は表示しない
						this->gridWarn->Rows->Add( err->getAll( isJapanese ) );
				}
			}
			if( listHi != nullptr )
			{
				for each( RCMrcError ^err in listHi )
				{
					this->gridWarn->Rows->Add( err->getAll( isJapanese ) );
				}
			}
		}

		// --------------------------------------------------------
		// 言語対応
		// --------------------------------------------------------
	public:
		void changeJapanese(void)
		{
			this->labError->Text = gcnew System::String( "エラー(必ず修正してください。)" );
			this->colErrorName->HeaderText  = gcnew System::String( "項目名" );
			this->colErrorBegin->HeaderText = gcnew System::String( "開始" );
			this->colErrorEnd->HeaderText   = gcnew System::String( "終了" );
			this->colErrorCause->HeaderText = gcnew System::String( "要因" );

			this->labWarn->Text  = gcnew System::String( "警告(修正は必須ではありませんが情報に誤りがないかご確認ください。)" );
			this->colWarnName->HeaderText  = gcnew System::String( "項目名" );
			this->colWarnBegin->HeaderText = gcnew System::String( "開始" );
			this->colWarnEnd->HeaderText   = gcnew System::String( "終了" );
			this->colWarnCause->HeaderText = gcnew System::String( "要因" );

			this->butClose->Text = gcnew System::String( "閉じる" );
		}

	public:
		void changeEnglish(void)
		{
			this->labError->Text = gcnew System::String( "Error Information(Modification is necessary.)" );
			this->colErrorName->HeaderText  = gcnew System::String( "Name" );
			this->colErrorBegin->HeaderText = gcnew System::String( "Begin" );
			this->colErrorEnd->HeaderText   = gcnew System::String( "End" );
			this->colErrorCause->HeaderText = gcnew System::String( "Reason" );

			this->labWarn->Text  = gcnew System::String( "Warning(Modification is not necessary. Please check validity of these information.)" );
			this->colWarnName->HeaderText  = gcnew System::String( "Name" );
			this->colWarnBegin->HeaderText = gcnew System::String( "Begin" );
			this->colWarnEnd->HeaderText   = gcnew System::String( "End" );
			this->colWarnCause->HeaderText = gcnew System::String( "Reason" );

			this->butClose->Text = gcnew System::String( "Close window" );
		}

	private:
		System::Void butClose_Click(System::Object^  sender, System::EventArgs^  e)
		{
			this->Close();
		}
};
}
