1.�r���h�ɕK�v�Ȃ���
�@NOR�t�@�[���ANAND�t�@�[��
	�E(TwlIPL_private)
	�ETwlSDK
�@�V�X�e�����j���[
�@�@�E(TwlIPL_private)
�@�@�ETwlSDK
�@�@�ENitroSystem 070314��(�{�c���[��add-ins����TwlSDK�Ή��p�b�`�𓖂Ă����́j

�@TwlIPL_private�͈ꕔ�̌����g�p����ꍇ�ɂ̂ݕK�v


2.���ϐ��̐ݒ�
�@TWL_IPL_RED_ROOT
	ex.) TWL_IPL_RED_ROOT=c:/TwlIPL
  TWL_IPL_RED_PRIVATE_ROOT�iTwlIPL_private �𗘗p����ꍇ�̂݁j
	ex.) TWL_IPL_RED_PRIVATE_ROOT=c:/TwlIPL_private


3.�J�����@�����̃r���h��

    �@ TwlIPL�`�F�b�N�A�E�g
       file://Aqua/svn/TwlIPL/trunk
       �r���h�m�F�Fr2897

    �A TwlSDK(UIG_plus)�`�F�b�N�A�E�g
       http://10.114.1.206:80/~twl-dev/svn/TwlSDK/branches/branch-5_0rcplus_UIG_plus/TwlSDK
       �r���h�m�F�Fr10990

    �B DEA-SUP���NitroSystem�擾
       �r���h�m�F�F
         NITRO-System-Library-071126.zip ���𓀂������̂��A
         NITRO-System-Library-071126-patch1.zip ���𓀂������̂ŏ㏑��

       �t�H���_�\��
       . �� TwlSDK
         �� NitroSystem
         �� TwlIPL

    �C �K���ATwlSDK���r���h����O��TwlIPL�̊��ϐ��ݒ�
       cd TwlIPL
       source ./setup
       �� TWL_IPL_RED_ROOT����`����Ă��Ȃ��ƁA
       �� lcfg�̏����̎Z�o�����s����Ȃ�TwlSDK�ɂȂ��Ă��܂��B

    �D TwlSDK���r���h�i����Cygwin��ő�����j
       �r���h�m�F�F
         CWFOLDER_TWL C:\Program Files\Freescale\CW for NINTENDO DS V2.0

       cd ../TwlSDK
       source ./setup
       make TWLSDK_PLATFORM=TWL

    �E ���C�u�����̃}�[�W
       TwlSDK\add-ins\es\es-sdk-20080822\twl\lib\ARM9-TS.LTD.thumb\Release �ȉ���
       �t�@�C���� TwlSDK\lib\ARM9-TS\Release �ɃR�s�[����

    �F NitroSystem�̃}�[�W
       TwlIPL\add-ins\NitroSystem\NitroSystem_forTWL-080408-jp.zip
       ���𓀂������̂ŁANitroSystem���㏑������B
       TwlIPL\add-ins\NitroSystem\NitroSystem_forTWL-080408-SDK5_0FCplus5-patch-jp.zip
       ���𓀂������̂ŁANitroSystem���㏑������B

    �G NitroSystem���r���h�i����Cygwin��ő�����j
       cd ../NitroSystem
       export NITROSYSTEM_ROOT="NitoroSystem�̃t�H���_"
       export TWLSYSTEM_ROOT="NitoroSystem�̃t�H���_"
       make TWLSDK_PLATFORM=TWL

    �H �w��F�؃��b�J�[����TwlIPL_private���ۑ����ꂽUSB�����������
       USB�������̃h���C�u�������킹�āA����Cygwin��ŁA
       export TWL_IPL_RED_PRIVATE_ROOT=F:/TwlIPL_private

    �I TwlIPL���r���h�i����Cygwin��ő�����j
       cd ../TwlIPL
       make -f Makefile.full TWLSDK_PLATFORM=TWL 

    ���l�F
    �E�ߋ��Ƀr���h���������g�p����ꍇ�́A�ŏ���make clean���K�v


�ȏ�
