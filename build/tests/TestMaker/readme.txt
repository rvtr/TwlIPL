#----------------------------------------------------------------------------
# [CLS �e�X�g�p] ROM �쐬
#
# -- CLS �e�X�g �p ROM ���쐬���܂��B
#    ���̃h�L�������g�͂قڊo���ł��B
#
#----------------------------------------------------------------------------

* ����
- Python 2.5 �̎��s�����K�{�ł��B�����܂���B
- PyYAML �̃C���X�g�[�����K�{�ł��B�����܂���B
- pyExcelerator �̃C���X�g�[�����ꕔ�K�v�ł��B�����܂���B
-- http://pyyaml.org/ ���Q�Ƃ��Ă��������B

* ���p���@

- CLS �e�X�g ROM �쐬�ɂ� make clstest -> make ���Ă��������B
- FONT �e�X�g ROM �쐬�ɂ� make fonttest -> make ���Ă��������B
- PARENTAL �e�X�g ROM �쐬�ɂ� make parentaltest -> make ���Ă��������B
- EULA �e�X�g ROM �쐬�ɂ� make eulatest -> make ���Ă��������B
- ��L make ��ƌ�ɁAmake forcls �ƑłƁA~
  CLS ����p�f�B���N�g�� (forCLS) ����������܂��B

* ROM �����p�����[�^
- romparam.yaml �Ƀp�����[�^���w�肳��Ă��܂��B
- CLS �e�X�g�p�p�����[�^�Ɋւ��ẮA
  ./docs/twl_cls_checksheet_20080508.xls ���Q�Ƃ��Ă��������B
- FONT �e�X�g�p�p�����[�^�Ɋւ��ẮA
  ./docs/20080617_DS_fontcodeList_NOE_FIANL.xls ���Q�Ƃ��Ă��������B
- PARENTAL �e�X�g�p�p�����[�^�Ɋւ��ẮA
  ./docs/twl_parentalcontrol_spec_20080704.xls ���Q�Ƃ��Ă��������B
- EULA �e�X�g�p�p�����[�^�Ɋւ��ẮA
  ���㌟�ؓ��e�̏ڍׂ��߂�K�v������܂��B

* CLS �e�X�g������
  HZ0A : System/NAND/�Z�L���A      --> ShopApp / Menu
  HZ4A : System/NAND/�f�[�^        --> �ʐ^��
  HZ5A : System/��\��/NAND/�f�[�^ --> ���� Firm
  KZ2A : User/NAND                 --> Shop �̔� App