�� SDLOG�Ή��Ń����`���[�ɂ���

���@���ł̂Ńf�o�b�O�p�ɁASD�J�[�h�Ƀf�o�b�O�v�����g���o�͂��郉���`���[�ł��B

TwlIPL/build/tests/Launcher_sdlog�Ɉړ����ăr���h����ƁA
�����f�B���N�g����HNAA.tad���쐬����܂��̂ŁANandInitializer����Import���ĉ������B

��ARM9�̃f�o�b�O�v�����g�݂̂��o��

$ make SDLOG=ARM9

��ARM9/ARM7�̃f�o�b�O�v�����g���o��

$ make SDLOG=ALL


�� SDLOG(������)���C�u�������K�����t�@�����X

// SDLOG�V�X�e���̊J�n�B�J�n�ȍ~�AOS_Printf�n��������ASD���O�o�b�t�@�ɒ~�ρ�SD�J�[�h�����o�����s���܂��B
//
// path:           �o�͂���t�@�C����u��SD�J�[�h��̃f�B���N�g��(��:sdmc:/log)
// filename:       �o�͂���t�@�C����
// buffer:         ���O�̈ꎞ������B��ʂ̃��O���c���ꍇ�� 1KB �ȏ�𐄏�
// partitionSize:  SD �J�[�h�ɏ������ރT�C�Y�B bufferSize �� 1/n �𐄏�
// writeType:      �㏑�� or �ǋL�̎w��B�t�@�C�������݂��Ȃ��ꍇ�͂ǂ�����V�K�쐬�ƂȂ�

BOOL SDLOG_InitEx(const char* path, const char* filename, char* buffer, u16 bufferSize, u16 partitionSize, SDLOGWriteType writeType);

// OS_Printf�n��������Ȃ���
// path:           �ۑ���B sdmc:/sample �Ǝw�肷��� sdmc:/sample/Log0.log ��
//                 ���O���ۑ������(�t�@�C�����͌Œ�ł��B)
BOOL SDLOG_Init(const char* path, char* buffer, u16 bufferSize, u16 partitionSize, SDLOGWriteType writeType);


// SDLOG�Ƀf�o�b�O���������o���B
void SDLOG_Printf(const char *fmt, ...);

// �o�b�t�@�ɒ��߂�ꂽ���O�������I�� SD �J�[�h�ɏ�������
// �����ł͊m���Ƀ��O�������o�������Ƃ����ꏊ�ɒǉ����邱�Ƃ𐄏��B
void SDLOG_Flush(void);

// �܂������o���Ă��Ȃ����O�� SD �J�[�h�ɏ�������Ńt�@�C�����N���[�Y
void SDLOG_FinishEx(void); // OS_Printf�������
void SDLOG_Finish(void);   // OS_Printf������Ȃ���

// ARM7�̃f�o�b�O���O���o��
void SDLOG_PrintServer(void);

