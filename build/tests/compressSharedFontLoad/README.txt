# ------------------------------------------
# ���L�t�H���g���k���̃��[�h���x����
#
# �쐬�� : nishikawa_takeshi
# ------------------------------------------

�����͈ȉ���2�X�e�b�v����\������܂��B

1. ���L�t�H���g�̈��k�ƃC���|�[�g
2. ���[�h���x�̑���

�ȉ��A�e�X�e�b�v�ɂ��Đ������܂��B


# ------------------------------------------
# �X�e�b�v1  ���L�t�H���g�̈��k�ƃC���|�[�g
# ------------------------------------------

(1)���L�t�H���g�̈��k

compSharedFont �f�B���N�g���Ɉړ����� make ���Ă��������B
 % cd compSharedFont
 % make

(2)���k�����t�H���g�̃C���|�[�g

���k���ꂽ���L�t�H���g�� NandInitializerRed �ŃC���|�[�g����K�v������܂����A
NandInitializerRed �́ASD����ł͂Ȃ��AROM�A�[�J�C�u����C���|�[�g���܂��B
(�܂�A���g��SRL�����ɋ��L�t�H���g�������܂��B)
���̂��߁ANandInitializerRed ���ēx�A�r���h����K�v������܂��B
���k�t�H���g��(1)�̏����ɂ���āANandInitializerRed �̃f�B���N�g���Ɏ����I�ɃR�s�[����܂��̂ŁA
�R�s�[����K�v�͂���܂���B

 % cd $(TWL_IPL_RED_ROOT)/build/systemMenu_tools/NandIntializerRed
 % make clean; make

�ăr���h���ꂽ NandInitializerRed.srl (ARM9.TWL/bin/�ȉ��ɂ���܂�) ��SD�J�[�h�ɃR�s�[���āA
SD�J�[�h�u�[�g����ƁA���j���[����t�H���g���C���|�[�g���鍀�ڂ�I���ł��܂��B

# ------------------------------------------
# �X�e�b�v2  ���[�h���x�̑���
# ------------------------------------------

���[�h���x�̑���v���O�����́AtestLoadSpeed �f�B���N�g���ɂ���܂��B
�����ŁAOS ���C�u�����̋��L�t�H���g�֘A�̊֐���ύX���A
����ɁATWLSDK �� SYSMENU �p�b�P�[�W�Ƃ��ăr���h���Ȃ����K�v������܂��B
�����̕ύX�ƃe�X�g�v���O�����̃r���h�́Amake �ɃI�v�V����������Ǝ����I�Ɏ��s����܂��B
(���D�݂ŁATWL_WITHOUT_DEMOS=TRUE �� TWL_FINALROM=TRUE �I�v�V�������������������B)

  % cd testLoadSpeed
  % make TEST_TWLSDK_REBUILD=TRUE

��L�� TWLSDK �̕ύX�����Ȃ��Ƃ��ɂ́A�e�X�g�v���O�����̃r���h������ɂł��܂���B

�e�X�g�v���O������ύX����ꍇ�A���� TWLSDK ���r���h����͎̂��Ԃ�������܂��̂ŁA
�I�v�V������������ make ����ƁA�e�X�g�v���O�����̃r���h�̂ݎ��s���܂��B

  % make

�쐬���ꂽ�e�X�g�v���O���������s����΁A���k�������L�t�H���g�̃��[�h���Ԃ𑪒�ł��܂��B
