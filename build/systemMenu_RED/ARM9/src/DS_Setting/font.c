/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     font.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <sysmenu.h>
#include "font.h"
#include "main.h"

// define data----------------------------------
#define SEARCH_ENTRY_MAX_NUM			128			// �������d���Ȃ�̂�h�����߂́A�G���g���̃T�[�`�񐔏��
#define NITRO_CHAR_SIZE					0x20		// NITRO�W���L�����N�^�T�C�Y(4bit�J���[�j
#define CHAR_CODE_OFFSET				(SJIS_CHAR_VRAM_OFFSET / NITRO_CHAR_SIZE)	// �L�����i���o�[���X�N���[���ɓ]������ۂ̃I�t�Z�b�g�iBG_CHAR_VRAM+0x4000����n�߂Ă���̂ŁA�I�t�Z�b�g��0x200�j
#define DRAW_CHAR_NUM_LIMIT				(SJIS_CHAR_VRAM_SIZE   / NITRO_CHAR_SIZE)	// �P�̕�����ŕ\���ł���L�����N�^���̌��E

#define HANDLE_INDEX					0x40000000

// �t�H���g�\���f�[�^
typedef struct FontComponent {
	u16					need_tate_dot;				// ������\������̂ɕK�v�ȃL�����P�ʂ̃h�b�g��
	u16					tate_dot;					// �����L�����̏c�h�b�g��
	u16					yoko_dot;					// �����L�����̉��h�b�g��
	u16					org_char_size;				// �����L������byte�T�C�Y
	u32 				org_row_size;				// �����L����
	u32					start_adr;					// �L�����N�^�o�C�i���̃X�^�[�g�A�h���X
	u32					end_adr;					// �L�����N�^�o�C�i���̃G���h�A�h���X
}FontComponent;


// VRAM�L�����f�[�^�̃������u���b�N���I�m�ۃ��X�g
typedef struct CharAllocListTag CharAllocList;
struct CharAllocListTag {
	CharAllocList		*next;						// ���̃������u���b�N�m�ۍ\���̂ւ̃|�C���^
	u16					memTop;						// �L�������蓖�ăe�[�u��charAllocTbl�̐�L�u���b�N�̐擪�|�C���g
	u16					memNum;						// �m�ۂ����������u���b�N�̐�
}CharAllocListTag;


// SJIS������\���̂��߂�VRAM�G���g���f�[�^
typedef struct StrEntry {
	const FontComponent	*font;						// �\������t�H���g�̍\���v�f
	u32					dataAddr;					// �f�[�^���̃A�h���X�iID�Ƃ��Ďg�p�j
	u32					*charMemp;					// ���I�Ɋm�ۂ���VRAM�������u���b�N�ւ̃|�C���^
	u16					charNum;					// �g�[�^���̃L������
	u16					drawPos;					// �\���ʒu�iBG�X�N���[���o�b�t�@�̐擪�����2byte�I�t�Z�b�g�j
}StrEntry;


// �t�H���g�̕����Ԋu�l�ߓW�J�p�f�[�^
typedef struct FontPropotion {
	u16					*leftp;						// ���݂̍����t�H���g�f�[�^�ւ̃|�C���^
	u16					*rightp;					// ���݂̉E���t�H���g�f�[�^�ւ̃|�C���^
	u16					h_offset;					// 16x16�h�b�g�P�ʂ̃L�����N�^�f�[�^���̌��݂̉��I�t�Z�b�g
	u8					pad[2];
	u32 				buff16x16[2][16/2];
}FontPropotion;

// �C���N�������g�^�C���N�������g�Ȃ��t���O�iSetScrData�Ŏg�p�j
typedef enum IncFlag {
	NO_INC,											// �C���N�������g�Ȃ�
	INC_1											// �P�C���N�������g
}IncFlag;

// extern data----------------------------
extern void _binary_f12han_dat(void);
extern void _binary_f12han_dat_end(void);


// function's prototype declaration-------------
static void		SetScrData(u16 code_top, StrEntry *sEntryp, IncFlag inc_flag);
static u16		MeasureStringCharWidth(const u8 *str);
static void 	SpreadStringCloser(u8 *str, StrEntry *sEntryp, u16 v_offset);
static void 	SetFirstFont(u8 **strpp, u16 v_offset,  FontPropotion *fontProp);
static u8 * 	CloseFontChar(u8 **strpp, u16 v_offset,  FontPropotion *fontProp);
static u32  	CalcCharCodeAddress(u8 **strpp, const FontComponent **font);
static void 	SpreadFontZen(u16 bit0_color, u16 bit1_color, u8 *fromAdr, u32 *toAdr);
static void 	InitStrEntryTable( StrEntry **sEntrypp );
static StrEntry *GetStrEntry( u16 handle );
static StrEntry **GetStrEntryAddress( u16 handle );
static int		InsertStrEntry(u16 *handle, u16 charNum);
static int		InsertStrEntry2(u16 handle, u16 charNum);
static u16  	DeleteStrEntry( u16 handle);
static BOOL		GetEmptyStrEntry(u32 dataAddr, u16 *handlep);
static u16		SearchStrEntry(u32 dataAddr);
static u8 *     HexToDecimalString(const void *hexp, u8 *strp, u16 figure, u16 size);
static u8 *     HexToString(const void *hexp, u8 *strp, u16 figure);
static void		InitAllocSystemSJIS( TargetScreen target );
static void		*AllocMemoryFromVram( u32 size );
static void		FreeMemoryToVram( void *p );

static u16		DrawStringSJIS_Core( int x, int y, u16 color, const void *strp, u32 handleIndex);
static u32		CalcHandleIndex(u32 dataAddr, u32 index);

// global variables-----------------------------


// static variables-----------------------------
static int					font_init		= 0;
static TargetScreen			targetScreen	= BOTTOM_SCREEN;
static const FontComponent	*nowFontHan;
static StrEntry 			*entryTblS[ STR_ENTRY_MAX_NUM ];
static StrEntry 			*entryTblM[ STR_ENTRY_MAX_NUM ];
static StrEntry				**entryTblp;
static u16					*bgBakp;

// const data-----------------------------------
const u16 tbl_ASCII_to_SJIS[0x100];
const FontComponent fontTypeTableHan[] = {
	{ 16, 12,  6, 12, 14*0xbc, (u32)&_binary_f12han_dat, (u32)&_binary_f12han_dat_end},
};


// functions description------------------------

//----------------------------------------------
// API�֐�
//----------------------------------------------

// �t�H���gAPI�̏�����
void InitFont( TargetScreen target)
{
	if( font_init & (0x01 << target ) ) {							// �^�[�Q�b�g���������݂Ȃ烊�^�[��
		return;
	}
	
	InitAllocSystemSJIS( target );									// �^�[�Q�b�g��VRAM�A���[�i������
	
	if( target == TOP_SCREEN ) {									// �^�[�Q�b�g�̃G���g����������
		InitStrEntryTable( &entryTblM[0] );
	}else {
		InitStrEntryTable( &entryTblS[0] );
	}
	
	if( font_init == 0 ) {											// �܂���������������Ă��Ȃ��Ȃ�A�X�N���[���������ݒ�ɁB
		SetTargetScreenSJIS( target );
	}
	
	SetFont( (FontType)0 );
	font_init |= 0x01 << target;
}


// �\���t�H���g�̐؂�ւ�
void SetFont(FontType font)
{
	if(font >= FONT_TYPE_MAX) {
		font = (FontType)0;
	}
	nowFontHan = &fontTypeTableHan[font];
}


// �\���^�[�Q�b�g�X�N���[���̐؂�ւ�
void SetTargetScreenSJIS( TargetScreen target )
{
	targetScreen	= target;
	if( target == TOP_SCREEN ) {
		entryTblp		= &entryTblM[0];
		bgBakp			= bgBakM;
	}else {
		entryTblp		= &entryTblS[0];
		bgBakp			= bgBakS;
	}
}


// �w�肳�ꂽSJIS�������\������B�i�n���h���w��Ȃ��Ńn���h���Z�o���Z�l�t���j
static u16 DrawStringSJIS_Core( int x, int y, u16 color, const void *strp, u32 handleIndex)
{
	StrEntry *sEntryp;
	int		 newEntry;
	u16		 handle;
	u16		 charNum;
	u16		 code;
	u16		 v_offset = 1;		// ��v_offset�͈����ɂ��邱�Ƃ��ł���B
	
	InitFont( targetScreen );
	
	// �󕶎���`�F�b�N
	if(*(u8 *)strp == 0x00) {
		return 0;
	}
	
	// ������\���ɕK�v�ȃL�����N�^���̑���
	charNum = MeasureStringCharWidth((const u8 *)strp);
	if(charNum > DRAW_CHAR_NUM_LIMIT) {								// �K�v�L�����N�^�����T�P�Q�ȓ��Ȃ烁�����m�ہB
		OS_Printf("SJIS-draw: str length max over.\n");
		return DSJIS_ERR_STR_LENGTH_TOO_LONG;
	}
	
	// �n���h���擾
	if( !GetEmptyStrEntry( handleIndex, &handle ) ) {
		OS_Printf("SJIS-draw: error StrEntry get failed.\n");
		return DSJIS_ERR_ENTRY_GET_FAILED;
	}
	
	// ������G���g���̓o�^
	newEntry = InsertStrEntry2( handle, charNum );
	if(newEntry < 0) {
		OS_Printf("SJIS-draw: error StrEntry alloc failed.\n");
		return DSJIS_ERR_ENTRY_ALLOC_FAILED;
	}
	sEntryp				= GetStrEntry(handle);						// �G���g���ւ̃|�C���^���i�[
	sEntryp->dataAddr	= handleIndex;
	
	// VRAM�L�����������̓��I�m��
	if(newEntry > 0) {
		sEntryp->charMemp = AllocMemoryFromVram( (u32)( charNum * NITRO_CHAR_SIZE * 2 ) );
																	// ���c���Q�L�������K�v�Ȃ̂ŁANITRO_CHAR_SIZE*2
//		OS_Printf("alloc charMemp = %x\n",sEntryp->charMemp);
		if( sEntryp->charMemp == NULL ) {							// �������m�ۂɎ��s������A�G���g���̃�������������ă��^�[��
			OS_Printf("VRAM char memory alloc failed.\n");
			OS_Free(sEntryp);
			return DSJIS_ERR_CHAR_ALLOC_FAILED;
		}
	}
	
	// ������G���g���̓o�^
	sEntryp->font    = nowFontHan;									// ���݂̃t�H���g�^�C�v���i�[
	sEntryp->drawPos = (u16)(x + y * 0x20);							// �\���ꏊ���i�[�B
	sEntryp->charNum = charNum;
	
	// �c�z�u�ʒu�w��v_offset�̋���
	if(v_offset) {
		if(((nowFontHan->tate_dot & 0x07)== 0)) {					// �c8dot or 16dot�̏ꍇ��v_offset��0
			v_offset = 0;
		}else {
			if( (16 - nowFontHan->tate_dot) < v_offset ) {			// ����ȊO�̎��́A�c16dot�Ɏ��܂�͈͂ɋ���
				v_offset = (u16)(16 - nowFontHan->tate_dot);
			}
		}
	}
	// �Ώە�����̃t�H���g�f�[�^���r�b�g�W�J���L�����f�[�^�l�߂������Ȃ��āA�m�ۂ���VRAM�L�����������ɓ]��
	SpreadStringCloser((u8 *)strp, sEntryp, v_offset);
	
	// �X�N���[���f�[�^�ɍ���̕�����̃L�����i���o�[����������
	code = (u16)( (((u32)sEntryp->charMemp - (u32)G2_GetBG1CharPtr()) / NITRO_CHAR_SIZE) | color );
	SetScrData(code, sEntryp, INC_1);
	
	return handle;
}

// �n���h���C���f�b�N�X�̎Z�o
static u32 CalcHandleIndex(u32 dataAddr, u32 index)
{
	if(index) {
		index += (u32)( HANDLE_INDEX + dataAddr );					// �C���f�b�N�X�����Z�����ꍇ�́A���̃f�[�^�̃A�h���X�Ɣ��Ȃ��悤�ANITRO�������}�b�v�O�ɂȂ�悤�I�t�Z�b�g�����Z�B
	}else {
		index  = dataAddr;
	}
	return index;
}


// �w�肳�ꂽSJIS�������\������B�i�n���h���w��Ȃ��j
u16 DrawStringSJIS( int x, int y, u16 color, const void *strp)
{
	return DrawStringSJIS_Core( x, y, color, strp, (u32)strp );			// index = strp ��Core���R�[��
}


// �w�肳�ꂽ�l��16�i���ŕ\������B�i�n���h���w��Ȃ��Łj
u16 DrawHexSJIS( int x, int y, u16 color, const void *hexp, u16 figure)
{
	u8 buff[16];
	u8 *strp = HexToString(hexp, buff, figure);
	return DrawStringSJIS_Core(x, y, color, strp, (u32)hexp );
}


// �w�肳�ꂽ�l��10�i���ŕ\������B�i�n���h���w��Ȃ��Łj
u16 DrawDecimalSJIS( int x, int y, u16 color, const void *hexp, u16 figure, u16 size)
{
	u8 buff[16];
	u8 *strp = HexToDecimalString(hexp, buff, figure, size);
	return DrawStringSJIS_Core(x, y, color, strp, (u32)hexp );
}


// �w�肳�ꂽSJIS�������\������B�i�n���h���w��Ȃ��Ńn���h���Z�o���Z�l�t���j
u16 DrawStringSJISEx( int x, int y, u16 color, const void *strp, u32 index)
{
	index = CalcHandleIndex( (u32)strp, index);
	return DrawStringSJIS_Core( x, y, color, strp, index );				// index �����Z����Core���R�[��
}


// �w�肳�ꂽ�l��16�i���ŕ\������B�i�n���h���w��Ȃ��Łj
u16 DrawHexSJISEx( int x, int y, u16 color, const void *hexp, u16 figure, u32 index)
{
	u8 buff[16];
	u8 *strp = HexToString(hexp, buff, figure);
	index = CalcHandleIndex( (u32)hexp, index);
	return DrawStringSJIS_Core( x, y, color, strp, index );
}


// �w�肳�ꂽ�l��10�i���ŕ\������B�i�n���h���w��Ȃ��Łj
u16 DrawDecimalSJISEx( int x, int y, u16 color, const void *hexp, u16 figure, u16 size, u32 index)
{
	u8 buff[16];
	u8 *strp = HexToDecimalString(hexp, buff, figure, size);
	index = CalcHandleIndex( (u32)hexp, index);
	return DrawStringSJIS_Core(x, y, color, strp, index);
}


// �w��R�[�h����n�܂�X�N���[���f�[�^�̃Z�b�g
static void SetScrData(u16 code_top, StrEntry *sEntryp, IncFlag inc_flag)
{
	u16 i;
	u16	lastPos;
	u16	pos_l;
	u16	*buffp_l;
	u16	*buffp_h;
	
	lastPos = 0x800 >> 1;											// �o�b�t�@�ŏI�ʒu == BG�X�N���[���T�C�Y(0x800)
	pos_l   = (u16)(sEntryp->drawPos + 0x0020);						// ��8dot�L�����N�^�\���ʒu
	buffp_h = bgBakp + sEntryp->drawPos;
	buffp_l = buffp_h + 0x0020;
	
	for(i = 0; i < sEntryp->charNum; i++) {
		*buffp_h++ = code_top;
		code_top  += inc_flag;
		if(nowFontHan->need_tate_dot == 16)	{ 						// �K�v�c�h�b�g��16�̏ꍇ�i8�h�b�g�����ȊO�S���j�A�����P�L�����Z�b�g
			*buffp_l++ = code_top;
		}
		code_top  += inc_flag;
		pos_l++;
		if(pos_l == lastPos) {
			break;
		}
	}
}


// �w�肳�ꂽ�n���h����SJIS��������N���A����B
void ClearStringSJIS_handle(u16 handle)
{
	StrEntry *sEntryp;
	
	InitFont( targetScreen );
	
	sEntryp = GetStrEntry(handle);
	if(sEntryp == NULL) {
		return;
	}
	
	// �X�N���[���f�[�^�̃N���A
	SetScrData(0x0000, sEntryp, NO_INC);
	
	// �L�����N�^�f�[�^�̃N���A
//	MI_CpuClearFast(sEntryp->charMemp, sEntryp->charNum * NITRO_CHAR_SIZE * 2);
																	// ���I�Ɋm�ۂ���VRAM�L�����f�[�^�̃N���A
	FreeMemoryToVram( sEntryp->charMemp );							// ���̃G���g�����m�ۂ��Ă����L�����N�^�������̉���B
	(void)DeleteStrEntry(handle);
}


// �w�肳�ꂽ�f�[�^�A�h���X�ŕ\�����Ă���SJIS��������N���A����B
void ClearStringSJIS(void *datap)
{
	u16 handle = SearchStrEntry( (u32)datap );
	
	if( handle > STR_ENTRY_MAX_NUM ) {
		return;
	}
	ClearStringSJIS_handle(handle);
}


// �w�肳�ꂽ�f�[�^�A�h���X�ŕ\�����Ă���SJIS��������N���A����BEx�n�ŕ`�悵�����̂������ꍇ�B
void ClearStringSJISEx(void *datap, u32 handleIndex)
{
	u16 handle;
	
	handleIndex = CalcHandleIndex( (u32)datap, handleIndex);
	
	handle = SearchStrEntry( (u32)handleIndex );
	
	if( handle > STR_ENTRY_MAX_NUM ) {
		return;
	}
	ClearStringSJIS_handle(handle);
}



// ���ݓo�^����Ă���S�Ă�SJIS��������N���A����i���I���������S�ĉ���j
void ClearAllStringSJIS(void)
{
	u16 handle;
	
	InitFont( targetScreen );
	
	for(handle = 0; handle < STR_ENTRY_MAX_NUM; handle++) {
		if( GetStrEntry( handle ) != NULL) {
			ClearStringSJIS_handle(handle);
		}
	}
}


// �w�肳�ꂽ�n���h���ŕ\������Ă��镶����̃J���[��ύX�B
u16 ChangeColorSJIS(u16 handle, u16 new_color)
{
	int      i;
	u16	     *buffp_l;
	u16	     *buffp_h;
	StrEntry *sEntryp = GetStrEntry( handle );						// �G���g���ւ̃|�C���^���i�[
	
	if((handle > STR_ENTRY_MAX_NUM) || (sEntryp == NULL)) {
		return 0;
	}
	
	buffp_h = bgBakp + sEntryp->drawPos;
	buffp_l = buffp_h + 0x20;
	for(i = 0; i < sEntryp->charNum; i++) {
		*buffp_h = (u16)((*buffp_h & 0x0fff) | new_color);
		*buffp_l = (u16)((*buffp_l & 0x0fff) | new_color);
		buffp_h++;
		buffp_l++;
	}
	return 1;
}

//----------------------------------------------
// ������G���g���e�[�u���̏���
//----------------------------------------------

// �G���g���e�[�u���̏�����
static void InitStrEntryTable( StrEntry **sEntrypp )
{
	u16 i;
	for(i = 0; i < STR_ENTRY_MAX_NUM; i++) {
		*sEntrypp++ = NULL;
	}
}


// �w�肳�ꂽ�n���h����StrEntry�|�C���^��Ԃ�
static StrEntry * GetStrEntry( u16 handle )
{
	if ( handle >= STR_ENTRY_MAX_NUM ) {
		return NULL;
	}else {
//		return entryTblS[ handle ];
		return entryTblp[ handle ];
	}
}


// �w�肳�ꂽ�n���h����StrEntry�|�C���^�ւ̃|�C���^��Ԃ�
static StrEntry ** GetStrEntryAddress( u16 handle )
{
	if ( handle >= STR_ENTRY_MAX_NUM ) {
		return NULL;
	}else {
//		return &entryTblS[ handle ];
		return &entryTblp[ handle ];
	}
}


// StrEntry�̓o�^
static int InsertStrEntry(u16 *handle, u16 charNum)
{
	StrEntry **sEntrypp = GetStrEntryAddress( *handle );
	
	if(*handle >= STR_ENTRY_MAX_NUM) {
		return -1;
	}else if(*handle > 0) {
		if( *sEntrypp != NULL) {									// �w��n���h���Ɋ��ɃG���g��������ꍇ�́A�m�ۃ������ʂ�V�K�������ʂƔ�ׂāA
			if( charNum > (*sEntrypp)->charNum ) {					// �Ċm�ۂ��K�v�ȏꍇ�́A������������Ċm�ۂ��s���B
//				OS_Printf("SJIS: same entry free to charMemory.\n");
//				OS_Printf("free charMemp = %x\n",(*sEntrypp)->charMemp);
				SetScrData( 0x0000, *sEntrypp, NO_INC );			// �X�N���[���f�[�^�̃N���A
				FreeMemoryToVram( (*sEntrypp)->charMemp );			// ���̃G���g�����m�ۂ��Ă����L�����N�^�������̉���B
				return 1;
			}else {
				if( charNum < (*sEntrypp)->charNum ) {
					SetScrData( 0x0000, *sEntrypp, NO_INC );		// �X�N���[���f�[�^�̃N���A
				}
				return 0;											// ���̃G���g���ł́A�L�����N�^���������p�����Ďg�p
			}
		}
	}
	
	// *handle == 0													// �w��n���h�����O�̎��͐V�����n���h�����m��
	*handle		= 0;
	sEntrypp	= GetStrEntryAddress( *handle );
	
	for( ; *handle < STR_ENTRY_MAX_NUM; (*handle)++ ) {
		if( *sEntrypp++ == NULL) {									// NULL�G���g����T��
			break;
		}
	}
	if(*handle == STR_ENTRY_MAX_NUM) {
		return -1;													// NULL�G���g����������Ȃ�������A�G���[�I���B
	}
	
	// StrEntry�̃������m��
	*sEntrypp = (StrEntry *)OS_Alloc( sizeof(StrEntry) );
	if( *sEntrypp == NULL) {
		OS_Printf("StrEntry allocate failed.\n");
		return -1;
	}else {
		return 1;
	}
}


// �G���g���}�������Q
static int InsertStrEntry2(u16 handle, u16 charNum)
{
	StrEntry **sEntrypp = GetStrEntryAddress( handle );
	
	if( *sEntrypp != NULL) {									// �w��n���h���Ɋ��ɃG���g��������ꍇ�́A�m�ۃ������ʂ�V�K�������ʂƔ�ׂāA
		if( charNum > (*sEntrypp)->charNum ) {					// �Ċm�ۂ��K�v�ȏꍇ�́A������������Ċm�ۂ��s���B
//			OS_Printf("SJIS: same entry free to charMemory.\n");
//			OS_Printf("free charMemp = %x\n",(*sEntrypp)->charMemp);
			SetScrData( 0x0000, *sEntrypp, NO_INC );			// �X�N���[���f�[�^�̃N���A
			FreeMemoryToVram( (*sEntrypp)->charMemp );			// ���̃G���g�����m�ۂ��Ă����L�����N�^�������̉���B
			return 1;
		}else {
			if( charNum < (*sEntrypp)->charNum ) {
				SetScrData( 0x0000, *sEntrypp, NO_INC );		// �X�N���[���f�[�^�̃N���A
			}
			return 0;											// ���̃G���g���ł́A�L�����N�^���������p�����Ďg�p
		}
	}
	
	// StrEntry�̃�������V�K�m��
	*sEntrypp = (StrEntry *)OS_Alloc( sizeof(StrEntry) );
	if( *sEntrypp == NULL) {
		OS_Printf("StrEntry allocate failed.\n");
		return -1;
	}else {
		return 1;
	}
}


// �w�肳�ꂽ�n���h����StrEntry�v�f���G���g���e�[�u������폜
static u16 DeleteStrEntry(u16 handle)
{
	StrEntry **sEntrypp = GetStrEntryAddress( handle );
	
	if( *sEntrypp == NULL) {
		return NULL;
	}
	
	OS_Free( *sEntrypp );
	*sEntrypp = NULL;
	
	return handle;
}


// �󂫃G���g����T���i����܌����悭�Ȃ��j
static BOOL GetEmptyStrEntry(u32 dataAddr, u16 *handlep)
{
	int i		= SEARCH_ENTRY_MAX_NUM;								// �������d���Ȃ�̂�h�����߁A�T�[�`�񐔂ɐ�����������B
	u16 handle	= (u16)( (dataAddr >> 2) & (STR_ENTRY_MAX_NUM - 1) );
	
	while(i--) {
		StrEntry *sEntryp = GetStrEntry( handle );
		if( sEntryp == NULL ) {
			*handlep = handle;
			return TRUE;											// �V�K�n���h���ʒu�m��
		}else {
			if( sEntryp->dataAddr == dataAddr ) {
				*handlep = handle;
				return TRUE;										// �����n���h���ʒu����
			}
			handle = (u16)( (handle + 1) & (STR_ENTRY_MAX_NUM - 1) );
		}
	}
	return FALSE;
}


// �w��f�[�^�̃n���h����T���i����܌����悭�Ȃ��j
static u16 SearchStrEntry(u32 dataAddr)
{
	int i		= SEARCH_ENTRY_MAX_NUM;								// �������d���Ȃ�̂�h�����߁A�T�[�`�񐔂ɐ�����������B
	u16 handle	= (u16)( (dataAddr >> 2) & (STR_ENTRY_MAX_NUM - 1) );
	
	while(i--) {
		StrEntry *sEntryp = GetStrEntry( handle );
		if(sEntryp != NULL) {
			if( sEntryp->dataAddr == dataAddr ) {
				return handle;										// �����n���h���ʒu����
			}
		}
		handle = (u16)( (handle + 1) & (STR_ENTRY_MAX_NUM - 1) );
	}
	return 0xffff;
}


//----------------------------------------------
// �L�����N�^VRAM�̓��I�m�ۏ���
//----------------------------------------------

// ������\���ɕK�v�ȃL�����N�^���̑���
static u16 MeasureStringCharWidth(const u8 *str)
{
	u16 totalWidth = 0;												// ������̃g�[�^���̉��h�b�g�T�C�Y
	u16 charNum    = 0;												// ������\���ɕK�v�ȃL�����N�^��
	
	// ������̃g�[�^���̉��h�b�g�T�C�Y���Z�o
	while(*str) {													// �e�����́@SJIS�@���@ASCII�@���H
		if( ((*str >= SJIS_HIGHER_CODE1_MIN) && (*str <= SJIS_HIGHER_CODE1_MAX))
		 || ((*str >= SJIS_HIGHER_CODE2_MIN) && (*str <= SJIS_HIGHER_CODE2_MAX)) ) {
			str += 2;												// SJIS����
			totalWidth += nowFontHan->yoko_dot;
		}else {
			str++;													// ASCII����
			totalWidth += nowFontHan->yoko_dot;
		}
	}
	// �g�[�^�����h�b�g������A������̕\���ɕK�v�ȃL�����N�^�����Z�o
	charNum = (u16)(totalWidth >> 3);
	if(totalWidth & 0x0007) {
		charNum = (u16)(charNum + 1);
	}
	
	return charNum;
}


//----------------------------------------------
//  �t�H���g�f�[�^��NITRO�L�����f�[�^�ւ̕ϊ��֐��Q
//----------------------------------------------

// �w�肳�ꂽ��������w�胁�����u���b�N�ɓW�J���Ȃ���]���i�g�p�t�H���g�ɂ���ĕ����l�߂��s���j
static void SpreadStringCloser(u8 *strp, StrEntry *sEntryp, u16 v_offset)
{
	u32 charBuff[(NITRO_CHAR_SIZE * 4) / 4];						// 16x16�L�����Q�����łS�L�������̃o�b�t�@�B�i�Q�����W�J���āA�����l�߂��s�����߁j
	u8  *charMemp;
	u8  *fromAdr;
	u16 i;
	u16 charNum;
	FontPropotion fontPro;
	
	// Spread target string char data to WRAM pool
	charNum          = sEntryp->charNum;
	charMemp         = (u8 *)sEntryp->charMemp;
	fontPro.h_offset = 0;
	fontPro.leftp    = (u16 *)0x0;
	fontPro.rightp   = (u16 *)0x0;
	
	while(1) {
		// �אڂ���2��1bit�t�H���g�f�[�^��16x16�h�b�g�L�����N�^�z��Ƀ��[�h
		if(fontPro.h_offset == 0) {
			SetFirstFont(&strp, v_offset, &fontPro);				// �\���������16x16�h�b�g�P�ʂɋ�؂������ɐ擪�ɂ���L�����N�^�����[�h
		}
		fromAdr = CloseFontChar(&strp, v_offset, &fontPro);			// ��L�L�����N�^�ɗאڂ���L�����N�^�����[�h
		
		// ���[�h����1bit�t�H���g�f�[�^��4bitNITRO�L�����f�[�^�ɓW�J
		SpreadFontZen(3, 1, fromAdr, (u32 *)charBuff);				// �L�����N�^�f�[�^��4bit�ɓW�J�B
		
		// �W�J�����L�����f�[�^��VRAM�ɃR�s�[
		fromAdr = (u8 *)charBuff;
		for(i = 0; i < 2; i++) {									// ��������16x16�h�b�g�i�S�L�����j���̃f�[�^��]��
			MI_CpuCopyFast(fromAdr, charMemp, (NITRO_CHAR_SIZE * 2));
			fromAdr += NITRO_CHAR_SIZE * 2;
			charMemp+= NITRO_CHAR_SIZE * 2;
			if(--charNum == 0) {
				return;
			}
		}
	}
}


// �ŏ��̃t�H���g�f�[�^���o�b�t�@�ɃZ�b�g
static void SetFirstFont(u8 **strpp, u16 v_offset, FontPropotion *fontProp)
{
	u8					tate;
	u8					*fontAdr;
	u16					*firstp;
	const FontComponent	*fontComp;
	
	if(**strpp == 0x00) {
		return;
	}
	
	fontAdr = (u8 *)CalcCharCodeAddress(strpp, &fontComp);			// ����̕����̃R�[�h����A�t�H���g�f�[�^�i�[�A�h���X���Z�o�B
	
	MI_CpuClearFast(fontProp->buff16x16[0], 64);					// 16x16�h�b�g�P�ʂ̈ꎞ�W�J�o�b�t�@�̃N���A
	
	firstp = (u16 *)fontProp->buff16x16[0] + v_offset;
	for(tate = 0; tate < fontComp->tate_dot - 1; tate++) {
		*firstp = (u16)(*fontAdr++ << 8);
		if(fontComp->yoko_dot > 8) {
			*firstp |= *fontAdr++;
		}
		firstp++;
	}
	fontProp->h_offset = fontComp->yoko_dot;
	fontProp->leftp    = (u16 *)fontProp->buff16x16[0];
	fontProp->rightp   = (u16 *)fontProp->buff16x16[1];
}


// �אڂ���t�H���g�f�[�^���o�b�t�@�ɃZ�b�g
static u8 * CloseFontChar(u8 **strpp, u16 v_offset,  FontPropotion *fontProp)
{
	u8 					tate;
	u8 					*fontAdr;
	u16					*temp;
	u16					*leftp_tmp;
	u16					*rightp_tmp;
	const FontComponent	*fontComp;
	
	while(fontProp->h_offset < 16) {
		if(**strpp == 0x00) {
			break;
		}
		
		fontAdr = (u8 *)CalcCharCodeAddress(strpp, &fontComp);		// ����̕����̃R�[�h����A�t�H���g�f�[�^�i�[�A�h���X���Z�o�B
		
		leftp_tmp  = fontProp->leftp  + v_offset;					// �t�H���g�f�[�^�����h�b�g�l�߂��Ȃ��烍�[�h
		rightp_tmp = fontProp->rightp + v_offset;
		for(tate=0; tate < fontComp->tate_dot - 1; tate++) {
			*rightp_tmp = (u16)(*fontAdr++ << 8);
			if(fontComp->yoko_dot > 8) {
				*rightp_tmp |= *fontAdr++;
			}
			
			*leftp_tmp   |= *rightp_tmp >> fontProp->h_offset;
			*rightp_tmp <<= 16 - fontProp->h_offset;
			leftp_tmp++;
			rightp_tmp++;
		}
		fontProp->h_offset += fontComp->yoko_dot;
	}
	fontProp->h_offset -= 16;
	temp                = fontProp->leftp;
	fontProp->leftp     = fontProp->rightp;
	fontProp->rightp    = temp;
	return (u8 *)temp;
}


// �����R�[�h�ɑΉ������t�H���g�f�[�^�i�[�A�h���X���Z�o����B
static u32 CalcCharCodeAddress(u8 **strpp, const FontComponent **font)
{
	u8  higher_code;
	u8  lower_code;
	u16 sjis_code;
	u32 adr;
	
	// �Ώە������@SJIS or ASCII�@���𔻒�
	higher_code = *(*strpp)++;
	if( ((higher_code >= SJIS_HIGHER_CODE1_MIN) && (higher_code <= SJIS_HIGHER_CODE1_MAX))
	  ||((higher_code >= SJIS_HIGHER_CODE2_MIN) && (higher_code <= SJIS_HIGHER_CODE2_MAX)) ) {	// SJIS���H
		lower_code  = *(*strpp)++;
		*font       = nowFontHan;
	}else {	// ASCII
		sjis_code   = tbl_ASCII_to_SJIS[higher_code];
		higher_code = (u8)(sjis_code >> 8);
		lower_code  = (u8)(sjis_code & 0x00ff);
		*font       = nowFontHan;
	}
	
	// �����R�[�h����t�H���g�f�[�^�A�h���X���Z�o
	if(higher_code <= SJIS_HIGHER_CODE1_MIN + 3) {
  		higher_code -= 0x81;
	}else if(higher_code <= SJIS_HIGHER_CODE1_MIN + 6) {
  		higher_code -= 0x83;
	}else if(higher_code <= SJIS_HIGHER_CODE1_MAX) {
  		higher_code -= 0x84;
	}else if(higher_code >= SJIS_HIGHER_CODE2_MIN) {
 		higher_code -= 0xc4;
	}
	
	if(lower_code < 0x80) {
		lower_code -= 0x40;
	}else {
		lower_code -= 0x41;
	}
	
	adr = (*font)->start_adr
		+((*font)->org_row_size)    * higher_code
		+((*font)->org_char_size+2) * lower_code + 2;
	
	return adr;
}


// �w��o�b�t�@��1bit�t�H���g�f�[�^��4bitNITRO�L�����f�[�^�ɓW�J�B
static void SpreadFontZen(u16 bit0_color, u16 bit1_color, u8 *fromAdr, u32 *toAdr)
{
	int i;
	u8  tate;
	u8  yoko;
	u8	rd_data;
	u32 wr_data;
	u32 b0c;
	u32 b1c;
	u32 *toAdr_left;
	u32 *toAdr_right;
	
	toAdr_left  = toAdr;
	toAdr_right = toAdr + 16;										// toAdr + (16 * 4byte)
	
	for(tate = 0; tate < nowFontHan->need_tate_dot; tate++) {		// �L�����N�^�̏c�h�b�g�������W�J
		// ���E���ꂼ���8dot�L������W�J
		for(yoko = 0; yoko < 2; yoko++) {
			wr_data     = 0x00000000;								// ���C�g�f�[�^������
			
			if(tate < nowFontHan->tate_dot) {
				rd_data     = *fromAdr++;							// ���E8dot�L�����̓ǂݏo��
				b0c  = (u32)bit0_color;								// bit0 �̃J���[
				b1c  = (u32)bit1_color;								// bit1 �̃J���[
				
				for(i = 0; i < 8; i++) {							// 8dot���̊e1bit�̔���
					if(rd_data & 0x80) {
						wr_data |= b1c;								// "1"���J���[�P��
					}else {
						wr_data |= b0c;								// "0"���J���[�O��
					}
					rd_data    <<= 1;
					b0c <<= 4;
					b1c <<= 4;
        		}
			}
			
			if(yoko==0) {
	 			*toAdr_right++ = wr_data;							// 32bit��������
			}else {
	 			*toAdr_left++  = wr_data;
			}
 		}
	}
}


//----------------------------------------------
//  MALLOC�V�X�e���̏�����
//----------------------------------------------

// VRAM��Ƀq�[�v���쐬�B�i�A���[�i��VRAM�͂Ȃ��̂ŁAWRAM_MAIN, WRAM_SUB�̃A���[�i�ő�p�j
static void InitAllocSystemSJIS( TargetScreen target )
{
	void*			tempLo;
	OSHeapHandle	hh;
	
	// VRAM��Ƀq�[�v���쐬
	
	if( target == TOP_SCREEN ) {
		// ���C��LCD
		OS_SetArenaLo( OS_ARENA_WRAM_MAIN, (void *)VRAM_M_ARENA_LO );
		OS_SetArenaHi( OS_ARENA_WRAM_MAIN, (void *)VRAM_M_ARENA_HI );
		tempLo	= OS_InitAlloc( OS_ARENA_WRAM_MAIN, OS_GetWramMainArenaLo(), OS_GetWramMainArenaHi(), 1 );
		OS_SetArenaLo( OS_ARENA_WRAM_MAIN, tempLo );
		
		hh = OS_CreateHeap( OS_ARENA_WRAM_MAIN, OS_GetWramMainArenaLo(), OS_GetWramMainArenaHi() );
		if(hh < 0) OS_Panic( "ARM9-MAIN LCD: Fail to create heap...\n" );
		OS_Printf( " arena lo = %x\n", OS_GetWramMainArenaLo() );
	    OS_Printf( "WRAM arena hi = %x\n", OS_GetWramMainArenaHi() );
		hh		= OS_SetCurrentHeap( OS_ARENA_WRAM_MAIN, hh );
	}else {		// BOTTOM_SCREEN
		// �T�uLCD
		OS_SetArenaLo( OS_ARENA_WRAM_SUB, (void *)VRAM_S_ARENA_LO );
		OS_SetArenaHi( OS_ARENA_WRAM_SUB, (void *)VRAM_S_ARENA_HI );
		tempLo	= OS_InitAlloc( OS_ARENA_WRAM_SUB, OS_GetWramSubArenaLo(), OS_GetWramSubArenaHi(), 1 );
		OS_SetArenaLo( OS_ARENA_WRAM_SUB, tempLo );
		
		hh = OS_CreateHeap( OS_ARENA_WRAM_SUB, OS_GetWramSubArenaLo(), OS_GetWramSubArenaHi() );
		if(hh < 0) OS_Panic( "ARM9-SUB  LCD: Fail to create heap...\n" );
		OS_Printf( "WRAM arena lo = %x\n", OS_GetWramSubArenaLo() );
	    OS_Printf( "WRAM arena hi = %x\n", OS_GetWramSubArenaHi() );
		hh		= OS_SetCurrentHeap( OS_ARENA_WRAM_SUB, hh );
	}
}


// VRAM�ւ̃L�����������m��
static void *AllocMemoryFromVram( u32 size )
{
	if ( targetScreen == TOP_SCREEN ) {
		return OS_AllocFromWramMain( size );						// ���c���Q�L�������K�v�Ȃ̂ŁANITRO_CHAR_SIZE*2
	}else {
		return OS_AllocFromWramSub ( size );						// ���c���Q�L�������K�v�Ȃ̂ŁANITRO_CHAR_SIZE*2
	}
}


// VRAM�ւ̃L�������������
static void FreeMemoryToVram( void *p )
{
	if ( targetScreen == TOP_SCREEN ) {
		OS_FreeToWramMain( p );
	}else {
		OS_FreeToWramSub( p );
	}
}


//----------------------------------------------
//  16�i�f�[�^�𕶎���ɕϊ�
//----------------------------------------------

// ���C������������ł����v�Ȍ`���ł�1byte�̃f�[�^�ǂݏo���B
__inline static u16 ReadByteHWBus(const void *srcp)
{
	if( (u32)srcp & 0x00000001 ) return (u16)(*(u16 *)( (u32)srcp ^ 0x00000001 ) >> 8 );
	else						 return (u16)(*(u16 *)( srcp ) & 0x00ff );
}

// 16�i�f�[�^��10�i������ɕϊ�
static u8 *HexToDecimalString(const void *hexp, u8 *strp, u16 figure, u16 size)
{
	u16 i;
	u32 target = 0;
	
	for(i = 0; i < size; i++) {
		target |= ReadByteHWBus( ((u8 *)hexp)++ ) << (8 * i);
	}
	
	strp   += figure;
	*strp-- = 0x00;
	for(i = 0; i < figure; i++) {
		CP_SetDiv32_32(target, 10);
		*strp--	 = (u8 )(CP_GetDivRemainder32() + 0x30);
		target   = (u32) CP_GetDivResult32();
	}
	return (strp + 1);
}


// 16�i�f�[�^��16�i������ɕϊ�
static u8 *HexToString(const void *hexp, u8 *strp, u16 figure)
{
	u16 i;
	u8  hex;
	
	strp   += figure;
	*strp-- = 0x00;
	
	for(i = 0; i < figure; i++){
		hex = (u8)ReadByteHWBus(hexp);
		if(i & 0x01){
			hex = (u8)( (hex >> 4) & 0x0f);
			((u8 *)hexp)++;
		}else{
			hex = (u8)( hex & 0x0f);
		}
		if(hex < 0x000a)	hex += 0x30;
		else				hex += 0x41 - 0x0a;
		*strp-- = hex;
	}
	return (strp + 1);
}


//----------------------------------------------
//  ASCII -> SJIS �ϊ��e�[�u��
//----------------------------------------------

const u16 tbl_ASCII_to_SJIS[0x100] = {
																	// 00
	0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,
	0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,
																	// 10
	0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,
	0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,
																	// 20
	0x8140,0x8149,0x8168,0x81f2,0x8190,0x8193,0x8195,0x8166,
	0x8169,0x816a,0x8196,0x817b,0x8143,0x817c,0x8144,0x815e,
																	// 30
	0x824f,0x8250,0x8251,0x8252,0x8253,0x8254,0x8255,0x8256,
	0x8257,0x8258,0x8146,0x8147,0x8183,0x8181,0x8184,0x8148,
																	// 40
	0x8197,0x8260,0x8261,0x8262,0x8263,0x8264,0x8265,0x8266,
	0x8267,0x8268,0x8269,0x826a,0x826b,0x826c,0x826d,0x826e,
																	// 50
	0x826f,0x8270,0x8271,0x8272,0x8273,0x8274,0x8275,0x8276,
	0x8277,0x8278,0x8279,0x816d,0x818f,0x816e,0x814f,0x8151,
																	// 60
	0x8165,0x8281,0x8282,0x8283,0x8284,0x8285,0x8286,0x8287,
	0x8288,0x8289,0x828a,0x828b,0x828c,0x828d,0x828e,0x828f,
																	// 70
	0x8290,0x8291,0x8292,0x8293,0x8294,0x8295,0x8296,0x8297,
	0x8298,0x8299,0x829a,0x816f,0x8162,0x8170,0x8160,0x81fc,

																	// 80
	0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,
	0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,
																	// 90
	0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,
	0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,
																	// a0
	0x81fc,0x8142,0x8175,0x8176,0x8141,0x8145,0x8192,0x8340,
	0x8342,0x8344,0x8346,0x8348,0x8383,0x8385,0x8387,0x8362,
																	// b0
	0x815b,0x8341,0x8343,0x8345,0x8347,0x8349,0x834a,0x834c,
	0x834e,0x8350,0x8352,0x8354,0x8356,0x8358,0x835a,0x835c,
																	// c0
	0x835e,0x8360,0x8363,0x8365,0x8367,0x8369,0x836a,0x836b,
	0x836c,0x836d,0x836e,0x8371,0x8374,0x8377,0x837a,0x837d,
																	// d0
	0x837e,0x837f,0x838a,0x8382,0x8384,0x8386,0x8388,0x8389,
	0x838a,0x838b,0x838c,0x838d,0x838f,0x8393,0x814a,0x814b,

																	// e0
	0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,
	0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,
																	// f0
	0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,
	0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,0x81fc,
};

