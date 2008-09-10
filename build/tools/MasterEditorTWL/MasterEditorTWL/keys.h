#pragma once

// 署名用の鍵データの宣言

#include <twl/types.h>

namespace MasterEditorTWL
{
	// 開発機用ROMヘッダ署名鍵
	extern const u8 g_devPubKey_DER[ 0xa2 ];
	extern const u8 g_devPrivKey_DER[ 0x263 ];

} // end of namespace MasterEditorTWL
