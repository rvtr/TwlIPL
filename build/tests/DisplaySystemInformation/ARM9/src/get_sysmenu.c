#include <twl.h>
#include <twl/os/common/sharedFont.h>
#include <twl/nam.h>
#include <twl/na.h>
#include <wchar.h>

#include "getInformation.h"
#include "viewSystemInfo.h"
#include "strResource.h"
#include "misc.h"
#include "util.h"


#define FILE_VERSION				"verdata:/version.bin"
#define FILE_EULA_URL				"verdata:/eula_url.bin"
#define FILE_NUP_HOSTNAME			"verdata:/nup_host.bin"
#define FILE_TIMESTAMP				"verdata:/time_stamp.bin"

#define FILE_SIGN_NUP_CERT			"verdata:/.twl-nup-cert.der"
#define FILE_SIGN_NUP_PRV			"verdata:/.twl-nup-prvkey.der"
#define FILE_SIGN_SHOP_CERT			"verdata:/.twl-shop-cert.der"
#define FILE_SIGN_SHOP_PRV			"verdata:/.twl-shop-prvkey.der"
#define FILE_SIGN_NINTENDO_CAG2		"verdata:/NintendoCA-G2.der"
#define NUM_FILE_SIGN				5


typedef struct SystemMenuVersion {
	u16		major;
	u16		minor;
	u16		str[ TWL_SYSMENU_VER_STR_LEN / sizeof(u16) ];
}SystemMenuVersion;

static char* s_strSignFilePath[] = {
	FILE_SIGN_NUP_CERT,
	FILE_SIGN_NUP_PRV,
	FILE_SIGN_SHOP_CERT,
	FILE_SIGN_SHOP_PRV,	
	FILE_SIGN_NINTENDO_CAG2,
};

static char* s_strSignHashDev[] = {
	"01e03e86fe11c5172ba742045c63e65c2f058e99",	
	"7497940e3a3591d592b46ff99d75ebe102c27550",
	"cf130c7674bae733f3b106109bb06cc0d6ac1a18",
	"ab38a52a384ab63ea8397de6eae8a96d6c108888",
	"c60b2a5cc90f0630cca33040df6b3378239f3bfa"
};

static char* s_strSignHashProd[] = {
	"0626f8ac62baaa0b70c543a33962e54507d451d6",
	"58c198c8099d579500cb5d9007bf81404a3c41fa",
	"72445f08ab30a41aff9e20a2e64ca7d2b263765e",
	"43a81069e6b6300dbe08d6fc3583d0c384a37996",
	"c60b2a5cc90f0630cca33040df6b3378239f3bfa"
};


void getSysmenuInfo( void )
{
	u8 *pBuffer = (u8*) Alloc (NA_VERSION_DATA_WORK_SIZE);
	
	// numLineやallinedの設定が必要なのであえてエラーチェックはしない
	NA_LoadVersionDataArchive( pBuffer, NA_VERSION_DATA_WORK_SIZE) ;
	
	
	// バージョンの読み出し
    {
        FSFile file;
        SystemMenuVersion bufVersion;
        s32 len;
		
        FS_InitFile(&file);
		
        if ( FS_OpenFileEx(&file, FILE_VERSION, FS_FILEMODE_R))
        {
	        len = FS_ReadFile(&file, &bufVersion, sizeof(SystemMenuVersion));
	        FS_CloseFile(&file);
			        
	        gAllInfo[MENU_SYSMENU][SYSMENU_VERSION_NUM].iValue = (int)( bufVersion.major << 16 | bufVersion.minor );
	        gAllInfo[MENU_SYSMENU][SYSMENU_VERSION_NUM].isNumData = TRUE;
	        
			wcsncpy( gAllInfo[MENU_SYSMENU][SYSMENU_VERSION_STR].str.utf, bufVersion.str, TWL_SYSMENU_VER_STR_LEN );
			gAllInfo[MENU_SYSMENU][SYSMENU_VERSION_STR].isSjis = FALSE;
		}
		else
		{
			// 成功しなかった場合はデータはN/A
			gAllInfo[MENU_SYSMENU][SYSMENU_VERSION_STR].str.sjis = s_strNA;
			gAllInfo[MENU_SYSMENU][SYSMENU_VERSION_STR].isSjis = TRUE;
		}
    }
	
	// EULA URLの読み出し
    {
        FSFile file;
        s32 len;
		
        FS_InitFile(&file);
		
        if ( FS_OpenFileEx(&file, FILE_EULA_URL, FS_FILEMODE_R)) {
	        len = FS_ReadFile(&file, gAllInfo[MENU_SYSMENU][SYSMENU_EULA_URL].str.sjis , TWL_EULA_URL_LEN) ;
	        FS_CloseFile(&file);
        }
		else
		{
			STD_StrLCpy( gAllInfo[MENU_SYSMENU][SYSMENU_EULA_URL].str.sjis, s_strNA, TWL_EULA_URL_LEN );
		}
    }
	
	// NUP HOST NAME の読み出し
    {
        FSFile file;
        s32 len;
		
        FS_InitFile(&file);
		
        if ( FS_OpenFileEx(&file, FILE_NUP_HOSTNAME, FS_FILEMODE_R)) {
			len = FS_ReadFile(&file, gAllInfo[MENU_SYSMENU][SYSMENU_NUP_HOST].str.sjis, TWL_NUP_HOSTNAME_LEN);
	        FS_CloseFile(&file);
		}
		else
		{
			STD_StrLCpy( gAllInfo[MENU_SYSMENU][SYSMENU_NUP_HOST].str.sjis, s_strNA , TWL_NUP_HOSTNAME_LEN );
		}
		
    }
	
	// タイムスタンプ の読み出し
    {
        FSFile file;
        s32 len;
		
        FS_InitFile(&file);
		
        if ( FS_OpenFileEx(&file, FILE_TIMESTAMP, FS_FILEMODE_R)) {
	        len = FS_ReadFile(&file, &gAllInfo[MENU_SYSMENU][SYSMENU_TIMESTAMP].iValue, sizeof(u32) );
       		gAllInfo[MENU_SYSMENU][SYSMENU_TIMESTAMP].isNumData = TRUE;
	        FS_CloseFile(&file);
        }
    }
    
    // 署名の照合
    {
		FSFile file[NUM_FILE_SIGN];
		u32 fileLen[NUM_FILE_SIGN], maxFileSize = 0;
		u8 i;
		u8 *srcBuf, *dstBuf, digestBuf[MATH_SHA1_DIGEST_SIZE],
			 cmpDigestDevBuf[MATH_SHA1_DIGEST_SIZE], cmpDigestProdBuf[MATH_SHA1_DIGEST_SIZE];

		for( i=0 ; i < NUM_FILE_SIGN; i++ )
		{
			// 最初にエントリの行数情報を設定しておく
			gAllInfo[MENU_SYSMENU][i + SYSMENU_HASH_IDX].numLines = 2;
			gAllInfo[MENU_SYSMENU][i + SYSMENU_HASH_IDX].isAligned = FALSE;
		}

		for( i=0 ; i < NUM_FILE_SIGN; i++ )
		{
			FS_InitFile( &file[i] );

			// 署名されたファイルをそれぞれオープン
			if( !FS_OpenFileEx( &file[i], s_strSignFilePath[i], FS_FILEMODE_R ) )
			{
				// 開けなかったらNANDアクセス禁止状態なので戻る
				OS_Printf("sysmenu info error: Openfile failed.\n" );
				return ;
			}
			
			// ファイル容量をそれぞれ取得
			fileLen[i] = FS_GetFileLength( &file[i] );
		}
		
		// バッファを大きくしたり縮めたりしたくないので
		// 一番大きいファイルと同じ大きさのバッファを確保してしまう
		for( i=0 ; i < NUM_FILE_SIGN; i++ )
		{
			if( maxFileSize < fileLen[i] )
			{
				maxFileSize = fileLen[i];
			}
		}
		
		srcBuf = (u8*) Alloc ( maxFileSize );
		dstBuf = (u8*) Alloc ( maxFileSize );
		
		// それぞれのファイルに対して署名検証を行う
		for( i=0 ; i < NUM_FILE_SIGN; i++ )
		{
			u8 idx = (u8)(SYSMENU_HASH_IDX + i) ;
			
			OS_TPrintf("Checking signature...%s\n", s_strMetaMenu[MENU_SYSMENU][idx] );
			
			// ファイル読み込み
			FS_ReadFile( &file[i], srcBuf, (s32)fileLen[i] );
			OS_TPrintf("FileSize: %d byte  HeadData: %02x%02x%02x\n", fileLen[i], srcBuf[0], srcBuf[1], srcBuf[2] );
						
			if( SYSMENU_NUP_CERT <= idx && idx < SYSMENU_NINTENDO_CAG2 )
			{
				s32 result;
				
				// SYSMENU_NUP_CERTからの4ファイルは暗号化されているので複合化する
				result = NA_DecodeVersionData( srcBuf, fileLen[i] , dstBuf, fileLen[i] );
				
				if( result < 0 )
				{
					OS_TPrintf( "NA_DecodeVersionData() Failed. errCode: %d\n", result );
					continue;
				}
				
				// 出コードするとファイルサイズが変わるっぽいので更新する
				fileLen[i] = (u32)result;
				
				OS_TPrintf("Dacode VersionData... HeadData: %02x%02x%02x\n", dstBuf[0], dstBuf[1], dstBuf[2] );

				// 複合化データからキャッシュを計算
				MATH_CalcSHA1( digestBuf, dstBuf, fileLen[i] );
			}
			else
			{
				// それ以外はそのままハッシュを求める
				MATH_CalcSHA1( digestBuf, srcBuf, fileLen[i] );
			}
			
			strToHexa( s_strSignHashDev[i] , cmpDigestDevBuf, MATH_SHA1_DIGEST_SIZE );
			strToHexa( s_strSignHashProd[i], cmpDigestProdBuf, MATH_SHA1_DIGEST_SIZE );

			gAllInfo[MENU_SYSMENU][idx].numLines = 2;
			gAllInfo[MENU_SYSMENU][idx].isAligned = FALSE;
			
			putBinary( cmpDigestDevBuf, MATH_SHA1_DIGEST_SIZE );
			putBinary( cmpDigestProdBuf, MATH_SHA1_DIGEST_SIZE );
			putBinary( digestBuf, MATH_SHA1_DIGEST_SIZE );

			// ハッシュ値が一致したらcorrect,一致しなかったらincorrect
			if( MI_CpuComp8( cmpDigestDevBuf, digestBuf, MATH_SHA1_DIGEST_SIZE ) == 0 )
			{
				gAllInfo[MENU_SYSMENU][idx].str.sjis = 
					idx == SYSMENU_NINTENDO_CAG2 ? s_strCorrect[1] : s_strSysMenuKey[1];
			}
			else if( MI_CpuComp8( cmpDigestProdBuf, digestBuf, MATH_SHA1_DIGEST_SIZE ) == 0 )
			{
				gAllInfo[MENU_SYSMENU][idx].str.sjis = 
					idx == SYSMENU_NINTENDO_CAG2 ? s_strCorrect[1] : s_strSysMenuKey[2];
			}
			else
			{
				gAllInfo[MENU_SYSMENU][idx].str.sjis = s_strSysMenuKey[0];
			}
			
			
		}
		
		// バッファの開放
		Free( srcBuf );
		Free( dstBuf );
	}
    
	
	// SystemMenuVersionのアンマウント
	if( !NA_UnloadVersionDataArchive() ) {
		return;
	}
	
	Free(pBuffer);
}