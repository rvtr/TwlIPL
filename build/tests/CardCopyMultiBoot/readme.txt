�ycard_copy����菇�z

1. TWL �J���@�� NMenu �܂��� NandInitializer �� card_copy_*.tad ���C���|�[�g�i�C���|�[�g�ς݂Ȃ�s�v�j
2. TWL ���j���[����u�J�[�h�R�s�[�}���`�u�[�g�v NAND �A�v�����N��
3. NTR �֋z���o������ DS �J�[�h������
4. NTR �� DS �_�E�����[�h���j���[���N��
5. NTR �� DS �_�E�����[�h���j���[�� "CardCopyMultiBoot" ���X�g���o����I��
6. DS �J�[�h�������� SD �J�[�h�̓]�����J�n����
7.�uSent/Received ROM size=....�v�Ƃ����\�����o����ASD �J�[�h�� card_dump.sbin ��
   �Q�[���̈�̋z�o��������

�yTwlSDK�ύX�ӏ��z

build/libraries/card/common/src/card_common.c

CARDAccessLevel CARDi_GetAccessLevel(void)
{
        .
        .
    else if (!OS_IsRunOnTwl())
    {
        level = CARD_ACCESS_LEVEL_FULL; // CARD_ACCESS_LEVEL_BACKUP ����ύX
    }
        .
        .
    return level;
}
