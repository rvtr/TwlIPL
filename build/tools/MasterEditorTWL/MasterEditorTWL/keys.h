#pragma once

// 署名用の鍵データの宣言

#include <twl/types.h>
#include <apptype.h>

namespace MasterEditorTWL
{
	// 開発機用ROMヘッダ署名鍵

#ifdef METWL_VER_APPTYPE_USER
	extern const u8 g_devPubKey_DER[ 0xa2 ];
	extern const u8 g_devPrivKey_DER[ 0x263 ];
#endif
#ifdef METWL_VER_APPTYPE_SYSTEM
	extern const u8 g_devPubKey_DER_system[ 0xa2 ];
	extern const u8 g_devPrivKey_DER_system[ 0x263 ];
#endif
#ifdef METWL_VER_APPTYPE_SECURE
	extern const u8 g_devPubKey_DER_secure[ 0xa2 ];
	extern const u8 g_devPrivKey_DER_secure[ 0x263 ];
#endif
#ifdef METWL_VER_APPTYPE_LAUNCHER
	extern const u8 g_devPubKey_DER_launcher[ 0xa2 ];
	extern const u8 g_devPrivKey_DER_launcher[ 0x263 ];
#endif

} // end of namespace MasterEditorTWL
