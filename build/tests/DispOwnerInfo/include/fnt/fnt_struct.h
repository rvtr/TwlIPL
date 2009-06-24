#ifndef FNT_STRUCT_H_
#define FNT_STRUCT_H_

/** --------------------------------------------------------------------
  �Z�����\����
  ----------------------------------------------------------------------*/
typedef struct tagFntCellInfo
{
    u16 width  : 5; // �ő�32
    u16 height : 5; // �ő�32
    u16 dummy  : 6;
} tFntCellInfo;

/** --------------------------------------------------------------------
  �����\����
  ----------------------------------------------------------------------*/
typedef struct tagFntWidthEntry
{
    u16 offset : 8;               // �I�t�Z�b�g�i�����͂��߂܂ł̃s�N�Z�����j
    u16 width  : 8;               // ��
} tFntWidthEntry;

/** --------------------------------------------------------------------
  �t�H���gCMAP�̘A���̈�p�\����
  ----------------------------------------------------------------------*/
typedef struct tagFntCtEntry
{
	u16			start;
	u16			end;
} tFntCtEntry;

/** --------------------------------------------------------------------
  �t�H���g�Ǘ��\����
  ----------------------------------------------------------------------*/
typedef struct tagFntEntry
{
    const u16*       font_image;              // �t�H���g�C���[�W�ւ̃|�C���^
    const u16*       cmap;                    // �R�[�h->glyph�ϊ��pcmap�ւ̃|�C���^
	tFntCtEntry*	 ctmap;					  // �R�[�h->glyph�ϊ��pcmap�ւ̃|�C���^
    tFntWidthEntry*  width_table;             // ���e�[�u��
    u16              glyph_num;               // glyph��
    u16              default_glyph;           // ������Ȃ������Ƃ��Ɏg���镶����glyph
    s16              cell_size;               // �ꕶ���̃Z���T�C�Y�i���[�h�P�ʁj
    tFntCellInfo     cell_info;               // �Z���̏��
	u16				 cmap_num;				  // glyph_comp == 1�̂Ƃ���cmap�̐�
	u16				 ctmap_num;				  // glyph_comp == 1�̂Ƃ���ctmap�̐�
	u32              glyph_sort : 1;          // �O���t���\�[�g����Ă��邩�ǂ���
	u32				 glyph_comp : 1;		  // �O���t�̘A������Ă��镔�������k����Ă��邩�ǂ���
} tFntEntry;

/** --------------------------------------------------------------------
  �`�悷���̃f�o�C�X�R���e�L�X�g�Ǘ��\����
  ----------------------------------------------------------------------*/
typedef struct tagFntDrawContext
{
    u16*         canvas;
    u16          width;
    u16          height;
    u16          canvas_type;
	u8				pad[2];
    tFntEntry*   font;
} tFntDrawContext;


/** --------------------------------------------------------------------
  ���C�A�E�g�\���p�̍\����
  ----------------------------------------------------------------------*/
/** --------------------------------------------------------------------
  ���C�A�E�g�w�b�_�[
  ----------------------------------------------------------------------*/
typedef struct tagFntLayoutHeader
{
	u8		name[4];
	u16		version;
	u16		table_num;
} tFntLayoutHeader;

/** --------------------------------------------------------------------
  CMAP�w�b�_�[
  ----------------------------------------------------------------------*/
typedef struct tagFntCmapHeader
{
	u32		ct_offset;
	u32		cm_offset;
} tFntCmapHeader;

/** --------------------------------------------------------------------
  ���C�A�E�g�\���̂��߂̃��b�Z�[�W�\����
  ----------------------------------------------------------------------*/
typedef struct tagFntPosition
{
	s16  pos        : 12;
	u16  pos_origin :  2;
	u16  text_align :  2;
} tFntPosition; // ���̐錾�ł��A2Bytes�ɂ�����Ǝ��܂�B�����n�ɂ���ẮAsizeof()�Ń`�F�b�N���邱�ƁB




/** ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  �������C�A�E�g��\�����邽�߂̍\���́B

  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/** --------------------------------------------------------------------
  ���b�Z�[�W�\����

  ���b�Z�[�W�f�[�^�́A
  tFntLayoutHeader header;
  tFntMessage      message[header.table_num];
  �ō\������܂��B
  ----------------------------------------------------------------------*/
typedef struct tagFntMessage
{
	// �ʒu
	tFntPosition	x;
	tFntPosition	y;

	// �����Ԋu
	s16  spacing_x : 8;
	s16  spacing_y : 8;

	// �F
	u16  color   : 8;
	u16  palette : 4;
	u16  font    : 4;

	// ���b�Z�[�W�ւ̃I�t�Z�b�g
	u32  offset;
} tFntMessage;




/** ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  OBJ�̃Z�����������߂̍\����

  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/** --------------------------------------------------------------------
  �Z�����\����

  �Z���f�[�^�́A
  tFntLayoutHeader header;
  tFntCell         cell[header.table_num];
  �ō\������܂��B
  ----------------------------------------------------------------------*/
typedef struct tagFntCell
{
	// �ʒu
	tFntPosition	x;
	tFntPosition	y;

	// �Z���L�����N�^�ւ̃C���f�b�N�X
	u32  cell_index;
} tFntCell;

/** --------------------------------------------------------------------
  �Z���L�����N�^���\����

  �Z���L�����N�^�f�[�^�́A
  tFntLayoutHeader   header;
  tFntCellCharHeader char_header;
  tFntCellCharInfo   cellchar_info[header.table_num];
  tFntCellCharOam    cellchar_oam[];
  u16                char_data[];
  �ō\������܂��B
  ----------------------------------------------------------------------*/
/** --------------------------------------------------------------------
  �Z���L�����N�^�w�b�_
  ----------------------------------------------------------------------*/
typedef struct tagFntCellCharHeader
{
	u32		cell_offset;	// tFntCellChar�z��ւ̃I�t�Z�b�g
	u32		oam_offset;		// tFntOamChar�z��ւ̃I�t�Z�b�g
	u32		char_offset;	// �L�����N�^�f�[�^�ւ̃I�t�Z�b�g
	u32		char_size;		// �L�����N�^�f�[�^�̃T�C�Y
	u32		char_mode : 3;	// OBJVRAMMODE_CHAR_1D��EXOBJ�t���O
} tFntCellCharHeader;

/** --------------------------------------------------------------------
  �Z���L�����N�^���
  ----------------------------------------------------------------------*/
typedef struct tagFntCellCharInfo
{
	u16		width   : 8; // �Z���̑傫�����B
	u16		height  : 8;
	u16		oam_index;	// oam_offset[oam_index] ����A
	u16		oam_num;	// oam_num��������oam���g�p���܂��B
} tFntCellCharInfo;

/** --------------------------------------------------------------------
  OAM���
  ----------------------------------------------------------------------*/
typedef struct tagFntCellCharOam
{
	GXOamAttr	attr;		// CharName�̕����́A�g�p���܂���B�i���[�h�ʒu�����[�U��`�ɂ��邽�߁B�j
	u16			char_index;	// �L�����N�^�f�[�^�̃C���f�b�N�X�B
	u16			char_num;	// �L�����N�^�f�[�^�̐��B�i�ő�A64���炢�j
} tFntCellCharOam;




/** ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  �^�b�`�̏ꏊ���擾���邽�߂̍\����

  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/** --------------------------------------------------------------------
  ���[�W�������
  ----------------------------------------------------------------------*/
typedef struct tagFntRegion
{
	// �ʒu
	tFntPosition	x;
	tFntPosition	y;

	// �T�C�Y
	u16		width   : 8; // �Z���̑傫�����B
	u16		height  : 8;
} tFntRegion;

#endif // FNT_STRUCT_H_
