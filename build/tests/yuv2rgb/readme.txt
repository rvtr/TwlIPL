���̃p�b�P�[�W�ɂ���
======================

ARM7�R���|�[�l���g���Aracoon����jackal�ɂ���ƁA�J�������쎞��
ARM9�̏��������������Ȃ邱�Ƃ𒲂ׂ邽�߂̃T���v���ł��B

�R���|�[�l���g�̐؂�ւ��́AMakefile��

ifdef TWL_IPL_RED_ROOT
#�R���|�[�l���g��؂�ւ��܂��B
#�R�����g�A�E�g�����racoon���A�L���������jackal���g���܂��B
#���̍s��ύX������make clean���Ă��������B
TWL_IPL_USE_RED_IPL			:= TRUE
endif

��L�̍s���R�����g�A�E�g���邩�L�������邩�ōs���Ă��������B
�ύX������make clean���Ă��������B

������������������ƁA�f�o�b�O�o�͂�

mConvBusy! 0, nextp empty
mConvBusy! 1, nextp empty
  :

�ƕ\������܂��B�����͉񐔂ł��B���̕p�x���Aracoon��jackal�ňق�
��܂��B

��ʏ㕔�� '*' �̓����̃X���[�Y����A�J�����Ɍ������Ď��U������
����ƁA�J�N�J�N���邱�Ƃł����������ɂ��C�Â�������Ǝv���܂��B

�ȏ�ł��B



�ȉ��́A���̃p�b�P�[�W�Ɋ܂܂��֐���SDK��DSP JPEG���C�u�����ɒ�
�����Ē������߂ɒ񋟂����Ƃ��ɍ쐬�����h�L�������g�ł��B

����������������������������������������������������������������������
======================================================================

���̃p�b�P�[�W�ɂ���
======================

UIG�����`���[�̃J�����B�e����(�J�����}�l�[�W��)�̎����̂��߁A����
����������邽�߂̃p�b�P�[�W�ł��B���ڃJ������JPEG�ɂ͊֌W���Ȃ�
�t�@�C�����܂܂�Ă��܂������������������B

src/menuCameraManager.cpp�́A��{�I�Ɍ����UIG�����`���[�Ɠ�����
�̂ł��B�����ł̎����̐��ʂ�UIG�����`���[�֔��f���Ă��܂��B

YUV422->YUV420�ϊ��֐��ȊO�́A�������̕���������A�G�ɍ���Ă���
����������܂��̂ŁA���J�Ȃ���Ȃ��悤�ɂ��肢���܂��B

SD�J�[�h�A�N�Z�X���́AUIG�����`���[�����d�l�ł͂���܂���B������
�����UIG�����`���[�ɂ����̂܂܎g���Ă��܂��B�V���b�^�[�����܂�
��܂���B

���s���@�́A���ʂ�make����main.tlf�����[�h���Ē����ƁA���ʂɃA
�E�g�J�����̉摜���o�܂��BA�{�^���ŃV���b�^�[��؂�܂��BSD�J�[�h
�������Ă����/menu/menu####.jpg��A�ԂŐ������܂��B(SDK�̃f����
���̂܂ܒ����Ă��܂�)


YUV422->YUV420�ϊ��֐��̎g�p���@
================================

menuCameraManager.cpp��Convert8bitYuv422To16bitYuv420()�ŁA
YUV422->YUV420�ϊ����s���܂��B���̊֐����Ăяo���Ă��铯�t�@�C��
��1074�s�ڂ���1102�s�ڂ�����

  frame_y  = mpTestEncodeWork->y;
  frame_cb = mpTestEncodeWork->cbcr;
  frame_cr = frame_cb + (mShotWdt * mShotHgt / 4);
  tick = OS_GetTick();
  Convert8bitYuv422To16bitYuv420(reinterpret_cast<const u32*>(pBuf),
                                 frame_y,
                                 mShotWdt,
                                 mShotHgt,
                                 mpDtcmWork->mYuvBuf);
  // store YUV420 and invalidate encode result
  // DC_StoreAll(), DC_FlushAll() is NG?
  DC_FlushAll();

  IPL_PRINT("procShot: yuv422to420 time %d usec\n", (u32)OS_TicksToMicroSeconds(OS_GetTick() - tick));
  tick = OS_GetTick();

  // �o�̓T�C�Y�͂����قǕK�v�H
  DSP_SetJpegEncoderBuffer(mpTestEncodeWork->result, sizeof(mpTestEncodeWork->result));
  // explicit initialization
  DSP_SetJpegEncoderThumbnailBuffer(NULL, NULL, NULL);
  DSP_SetJpegEncoderAllocator( dspAlloc_, dspFree_ );
  u32 size = DSP_StartJpegEncoder( static_cast< s16 > ( mShotWdt ) ,
                                   static_cast< s16 > ( mShotHgt ) ,
                                   DSP_JPEG_THUMBNAIL ,
                                   4 );

  // need here too ? sometimes picture breaks...
  // DC_StoreAll(), DC_FlushAll() is NG?
  DC_InvalidateRange(mpTestEncodeWork->result, size);

�ɂ��Đ������܂��B

- 1078�s�ڂ̕ϊ��֐��̈���pBuf�́A�J������VGA�AYUV422�ŃL���v�`��
�������ʂ��w���Ă��܂��B���̃o�b�t�@�́A���O��procConvert()��ʂ�
�Ă��Ă���A917�s�ڂ�
        DC_FlushAll();
�ŁA�f�[�^�L���b�V����Invalidate���ς�ł��܂��B
�ϊ����f�[�^�̓��[�h(4�o�C�g)���[�h����邽�߁A
reinterpret_cast<const u32*>���Ă��܂��B

- ����frame_y�́ASDK��JPEG�G���R�[�_�̃O���[�o���ϐ��ł��B����11
�s�ڂ��璼��
extern "C" {
// build/libraries/dsp/ARM9.TWL/src/dsp_jpeg_enc.c
extern u16 *frame_y; 
extern u16 *frame_cb; 
extern u16 *frame_cr;
} /* extern "C" */
�ŎQ�Ƃ��Ă��܂����ASDK�̃O���[�o���ϐ����Ƃ��Ă͓K�؂ł͂Ȃ����߁A
�ϐ����𐮂���Set�֐���ǉ����Ē����������ǂ��Ǝv���܂��B

- SDK��JPEG�G���R�[�_�ɂ́Aframe_cb��frame_cr���^����K�v�������
���Bframe_cb��frame_cr��^���Ă���̂�1074�s�ڂ����
        frame_y  = mpTestEncodeWork->y;
        frame_cb = mpTestEncodeWork->cbcr;
        frame_cr = frame_cb + (mShotWdt * mShotHgt / 4);
�ɂȂ�܂��B

�ϊ��֐��́Aframe_y�Ɍ��Ԗ���������frame_cb�Aframe_cr���o�͂��܂�
�̂ŁA�ϊ��֐��̈�����frame_y�����ł��B

- mShotWdt��640�AmShotHgt��480�ł��B

- mpDtcmWork->mYuvBuf�ŁA�����������p��DTCM���[�N��^���Ă��܂��B
menuCameraManager.h��43�s�ڂɂ���A12�_�u�����[�h(96�o�C�g)
    u64 mYuvBuf[12];
�̗̈悪�K�v�ł��Bstrd���߂��g�p���Ă���̂�8�o�C�g�A���C�������g
���Ă��������B(���̏��mShrinkBuf�́A�k���p�Ȃ̂ŕϊ��֐��ɂ͊֌W
����܂���)

DTCM���g�����Ƃɂ��ẮASDK�Ƃ��Č��J����ꍇ�Ɍ������K�v���Ǝv
���܂��B���C����������VRAM�����蓖�ĂĂ����삷��͂��ł����A����
�Ă��܂���Bstatic��DTCM���[�N�łȂ��Ă��A���C���X���b�h�̃X�^�b
�N�t���[���Ȃ�ʏ��DTCM�ɂȂ�͂��ł����A�����������Ƃ��ӎ�����
��ƕ~���������Ȃ�܂��B

- �ϊ����1085�s��
        DC_FlushAll();
�́AJPEG�G���R�[�h���ɉ摜�������Ǐ�ɔY�܂���A�O�̂��߃f�[�^
�L���b�V���̓��e�����������֓f���o���܂����B�摜������Ȃ��Ȃ�΁A
�s�v��������܂���B

- 1094�s�ڂ�
        DSP_SetJpegEncoderAllocator( dspAlloc_, dspFree_ );
�Ŏw�肵�Ă���A���P�[�^�ł����A���̂�mainy.cpp��1667�s�ڂɂ����
���B

dspAlloc_()�́A���͉������Ă��܂���B�������Ă��Ȃ��ǂ��납�A�Ă�
�ꂽ��A�T�[�g�Œ�~�����܂��B

dspFree_()�́ASDK����Ă΂�Ă��܂��̂ŁA��~�͂��܂��񂪃_�~�[��
���ɂ��Ă���܂��B

���̃A���P�[�^�̎g����(SDK��dsp_jpeg_enc.c)�ɂ��ẮA���낢��
�v���Ƃ���͂���̂ł����A�ȒP�Ɍ�����

a. �ő�ŉ��o�C�g�K�v�ɂȂ�̂��A�ł���΃R���p�C�����ɒm�肽����
���̕��@���񋟂���Ă��Ȃ�

b. pTmp2���ŏ��Ɋm�ۂ��Ă��邽�߁ApYbuf�ApCBbuf�ApCRbuf���m�ۂ���
���
    DSPFree(pTmp2);
����̂̓t���O�����g�𐶂���\��������

c. extra_bytes�Ɋւ��鏈����
    Yuv_image_size=(u32)(image_width+32)*(image_height+32);
��+32���Ӑ}�s��

�Ƃ����Ƃ���ŕs���ɂȂ����̂��A�����ϊ��֐������삵���������
���R�ł��B

- JPEG�G���R�[�h���1102�s��
        DC_InvalidateRange(mpTestEncodeWork->result, size);
���AJPEG�G���R�[�h���̉摜����΍�e�X�g�ł��B
���������āASD�֏�������FS���C�u�����ɖ�肪���邩���Ǝv���AFS��
�C�u�����p�Ƀf�[�^�L���b�V����Invalidate�������̂ł��B����Ȃ���
��΁A������s�v�ɂȂ�ł��傤�B


YUV422->YUV420�ϊ��֐��̕⑫����
================================

- 793�s�ڂ���821�s�ڂ܂ŁA
#if 0
    // even line U
  :
#else
    // even column ave. uv (V0 U0)
  :
#endif

�ł����A�����YUV422->YUV420�ɔ����Ď�����U�AV������2���C����
���ϒl�Ƃ��邩�AWikipedia���L�ۂ݂���

| YUV420 : 2�~2�s�N�Z���̂����A�̐F���M�����ŏ��̃��C��������A
| �Ԃ̐F���M�������̃��C������Ƃ�����B�P�x�M����1�s�N�Z�����Ƃɂ�
| ��B 

�̕����ɂ��邩�����������̂ł��B���ϒl�ɂ��������m�C�Y���ጸ����
�Ă���悤�Ɋ��������߁A����#if 0�Ƃ��Ă���܂��B#if 1�ɂ���ƃV
���[�v�Ȋ����ɂȂ�܂��B

�������A��������Y�����͂��̂܂܂�U�AV�������ω������炨�������G��
�Ȃ�Ȃ��̂��A�Ƃ͂�����̃��C����V�����̃��C�����玝���Ă���̂�
�������Șb�ł́H�Ƃ����v���͂���܂��B

JPEG�G���R�[�_�̓��̓f�[�^�d�l���ڂ������ׂĂ��Ȃ��̂ŁA�ǂ��炪
�ǂ��̂��������Ă��܂���B

- �ϊ��֐��̈����̐����ɂ��āA714�s�ڂ���̃R�����g��⑫���܂��B

// pYCbCrBuf���Œ�4�o�C�g�A���C�������g�ł����A�e�f�o�C�X�Ƃ̘A�g��
// ������32�o�C�g�A���C�������g���Ă��������B

������́A�ϊ����ʂ�DSP�̓��̓f�[�^�ɂȂ�̂ŁA32�o�C�g�A���C����
���g���Ă����΃L���b�V���܂��̐S�z�����邩��ł��B

