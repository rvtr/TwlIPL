========================
�z���C�g���X�g�̍쐬���@
========================

���T�v

������ .srl �����t�H���_�ȉ��Ɋi�[���Ă����A����ɑ΂��ăc�[�����g�����Ƃ�
�e�� .bin �t�@�C���𐶐����܂��B

�{�t�H���_�ł́A�����ς݂� .bin �t�@�C���ƁA�o�[�W�������ʗp�� revision.bin ��
���ׂĘA�������̂��A .tad �ɕϊ����Ă��邾���ł��B

.bin �t�@�C���̍X�V���ɁA���̃t�H���_�ɔz�u�A�R�~�b�g��ɑS�̂�(�㏑��)
�`�F�b�N�A�E�g�������Ă���r���h���Ă��������B

�����A�z���C�g���X�g�� trunk �݂̂ŊǗ����܂��B

�u�����`�Ńr���h�������ꍇ�́Atrunk���X�V�����̂��A�Y���u�����`�Ƀ}�[�W����
�����p���������B


���ϊ��c�[��

TwlIPL_private�ȉ��Ƀ\�[�X�R�[�h������܂��̂ŁA�r���h���Ă��g�����������B

�EMakeDSHashTable.exe [.srl�t�H���_]

  ���}�X�^�����O���̂��̂ł���A�w�b�_�����ARM9/ARM7�X�^�e�B�b�N�̈�A
  ����уI�[�o�[���C�̂�����x��ΏۂƂ����f�[�^�x�[�X���쐬���܂��B

  �w��t�H���_�ȉ����ċA�I�ɑ������ADSHashTable.bin ���쐬���܂��B
  ���}�X�^�����O���炳��Ă��Ȃ� .srl ���Ώۂł��B


�EMakeDSHashTableEx.exe [.srl�t�H���_]

  �V�}�X�^�����O�Œǉ����ꂽ�o�i�[�̈��ΏۂƂ����f�[�^�x�[�X���쐬���܂��B

  �w��t�H���_�ȉ����ċA�I�ɑ������ADSHashTableEx.bin ���쐬���܂��B
  �V�}�X�^�����O����Ă��Ȃ� .srl ���Ώۂł��B


�EMakeDSHashTableAdHoc.exe [.srl�t�H���_]

  �^�C�g���ʂɔ͈͂��w�肵���n�b�V���l���i�[�����f�[�^�x�[�X���쐬���܂��B

  ����t�H���_�ɑ��݂��Ă���A MakeDSHashTableAdHoc.ini �����ɁA
  �K�v�� .srl ���w��t�H���_����T���o���� DSHashTableAdHoc.bin ��
  �쐬���܂��B

  MakeDSHashTableAdHoc.ini �́A���݈ȉ��̏ꏊ�ŊǗ����Ă��܂��B
  (No.1��USB���Q�Ƃ��Ă�������)

    $TwlIPL_private/build/tools/MakeDSHashTableAdHoc

  �� �ʓ|�Ȃ�TwlIPL�Ɉڂ��Ă��\���܂���B


��MakeDSHashTableAdHoc.ini�̏���

Windows INI�t�@�C���̏����ŁA�ݒ�ł��鍀�ڂ͎��̒ʂ�ł��B
�Ȃ��A���l�́A�\�i���ł��\�Z�i���ł��\���܂���B(0x���t���Ə\�Z�i���Ɣ��f�����)

(��)
  ; �Z�~�R�����̌��̓R�����g�ł�
  [Tak The Greet Juju Challenge]; ���̂��̂ƃ_�u��Ȃ���Γ��{��ł����ł�OK�ł�
  game_code	= A3TE		; �C�j�V�����R�[�h�ł�
  rom_version	= 0		; ���}�X�^�[�o�[�W�����ł�

  ; ��s�ǉ������D���ɂǂ���

  offset0	= 0x0013a400	; �n�b�V���ΏۃI�t�Z�b�g (0x200�̔{��) No.0
  length0	= 0x200		; �n�b�V���ΏۃT�C�Y (0x200�̔{��) No.0

  ; �ȉ��͏ȗ��� (offset��length�͕K���Z�b�g�ŁA�ʂ��ԍ��͌��Ԗ����悤��)
  offset1	= 0x00141a00	; �n�b�V���ΏۃI�t�Z�b�g (0x200�̔{��) No.1
  length1	= 0x200		; �n�b�V���ΏۃT�C�Y (0x200�̔{��) No.1
  offset2	= 0x00145800	; �n�b�V���ΏۃI�t�Z�b�g (0x200�̔{��) No.2
  length2	= 0x200		; �n�b�V���ΏۃT�C�Y (0x200�̔{��) No.2
  offset3	= 0x01a5ac00	; �n�b�V���ΏۃI�t�Z�b�g (0x200�̔{��) No.3
  length3	= 0x200		; �n�b�V���ΏۃT�C�Y (0x200�̔{��) No.3
  offset4	= 0x01a5d400	; �n�b�V���ΏۃI�t�Z�b�g (0x200�̔{��) No.4
  length4	= 0x5600	; �n�b�V���ΏۃT�C�Y (0x200�̔{��) No.4
  offset5	= 0x0005c400	; �n�b�V���ΏۃI�t�Z�b�g (0x200�̔{��) No.5
  length5	= 0x200		; �n�b�V���ΏۃT�C�Y (0x200�̔{��) No.5
  offset6	= 0x00142000	; �n�b�V���ΏۃI�t�Z�b�g (0x200�̔{��) No.6
  length6	= 0x200		; �n�b�V���ΏۃT�C�Y (0x200�̔{��) No.6
  offset7	= 0x00126400	; �n�b�V���ΏۃI�t�Z�b�g (0x200�̔{��) No.7
  length7	= 0x5C00	; �n�b�V���ΏۃT�C�Y (0x200�̔{��) No.7
  ; �`7���ő�ł�

��TWL�̍X�V���̒���

commondefs.DSHashTable���� DS_HASH_TABLE_MAJOR_VERSION �܂���
DS_HASH_TABLE_MINOR_VERSION ���X�V����̂�Y��Ȃ��ł��������B


��CTR�Ή�

�쐬���ꂽ HNHA-XXXX-YYYY.bin ���ȉ��̃t�H���_�ɃR�s�[���Ă��������B

  $Horizon/resources/shareddata/twl/DSHashTable/

�t�@�C�������ύX���ꂽ�ꍇ�́A�ȉ��̃t�@�C���̓��e���C�����Ă��������B
(DSHashTableAdHoc.bin �݂̂̏C���ł͕ω����Ȃ��͂�)

  $Horizon/resources/shareddata/twl/OMakefile
  $Horizon/sources/firmware/CTR-Kernel/updater1st/UpdaterContents/Contents/ProgramPathes.om

�ȏ�
