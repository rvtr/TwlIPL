#include "misc.h"

const u16 *const s_pStrCountryName[] = {
	(const u16 *)L"UNDEFINED",
	(const u16 *)L"JAPAN",					// ���{
	
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	
	// USA���[�W����
	(const u16 *)L"Anguilla",        // �A���M��
	(const u16 *)L"ANTIGUA_AND_BARBUDA",    // �A���e�B�O�A�E�o�[�u�[�_
	(const u16 *)L"ARGENTINA",		// �A���[���`��
	(const u16 *)L"ARUBA",                  // �A���o
	(const u16 *)L"BAHAMAS",                // �o�n�}
	(const u16 *)L"BARBADOS",               // �o���o�h�X
	(const u16 *)L"BELIZE",                 // �x���[�Y
	(const u16 *)L"BOLIVIA",                // �{���r�A
	(const u16 *)L"BRAZIL",                 // �u���W��
	(const u16 *)L"BRITISH_VIRGIN_ISLANDS", // �p�̃��@�[�W������
	(const u16 *)L"CANADA",                 // �J�i�_
	(const u16 *)L"CAYMAN_ISLANDS",         // �P�C�}������
	(const u16 *)L"CHILE",       // �`��
	(const u16 *)L"COLOMBIA",               // �R�����r�A
	(const u16 *)L"COSTA_RICA",             // �R�X�^���J
	(const u16 *)L"DOMINICA",               // �h�~�j�J��
	(const u16 *)L"DOMINICAN_REPUBLIC",     // �h�~�j�J���a��
	(const u16 *)L"ECUADOR",                // �G�N�A�h��
	(const u16 *)L"EL_SALVADOR",            // �G���T���o�h��
	(const u16 *)L"FRENCH_GUIANA",          // �t�����X�̃M�A�i
	(const u16 *)L"GRENADA",                // �O���i�_
	(const u16 *)L"GUADELOUPE",             // �O�A�h���[�v
	(const u16 *)L"GUATEMALA",       // �O�A�e�}��
	(const u16 *)L"GUYANA",                 // �K�C�A�i
	(const u16 *)L"HAITI",                  // �n�C�`
	(const u16 *)L"HONDURAS",               // �z���W�����X
	(const u16 *)L"JAMAICA",                // �W���}�C�J
	(const u16 *)L"MARTINIQUE",             // �}���e�B�j�[�N
	(const u16 *)L"MEXICO",                 // ���L�V�R
	(const u16 *)L"MONTSERRAT",             // �����g�Z���g
	(const u16 *)L"NETHERLANDS_ANTILLES",   // �I�����_�̃A���e�B��
	(const u16 *)L"NICARAGUA",              // �j�J���O�A
	(const u16 *)L"PANAMA",       // �p�i�}
	(const u16 *)L"PARAGUAY",               // �p���O�A�C
	(const u16 *)L"PERU",                   // �y���[
	(const u16 *)L"ST_KITTS_AND_NEVIS",     // �Z���g�L�b�c�E�l�C�r�X
	(const u16 *)L"ST_LUCIA",               // �Z���g���V�A
	(const u16 *)L"ST_VINCENT(��)GRENADINES",  // �Z���g�r���Z���g�E�O���i�f�B�[��
	(const u16 *)L"SURINAME",               // �X���i��
	(const u16 *)L"TRINIDAD_AND_TOBAGO",    // �g���j�_�[�h�E�g�o�S
	(const u16 *)L"TURKS_AND_CAICOS_ISLANDS",   // �^�[�N�X�E�J�C�R�X����
	(const u16 *)L"UNITED_STATES",          // �A�����J
	(const u16 *)L"URUGUAY",       // �E���O�A�C
	(const u16 *)L"US_VIRGIN_ISLANDS",      // �ė̃o�[�W������
	(const u16 *)L"VENEZUELA",              // �x�l�Y�G��
	
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",

    // EUR", NAL ���[�W����
	(const u16 *)L"ALBANIA",       // �A���o�j�A
	(const u16 *)L"AUSTRALIA",              // �I�[�X�g�����A
	(const u16 *)L"AUSTRIA",                // �I�[�X�g���A
	(const u16 *)L"BELGIUM",                // �x���M�[
	(const u16 *)L"BOSNIA_AND_HERZEGOVINA", // �{�X�j�A�E�w���c�F�S�r�i
	(const u16 *)L"BOTSWANA",               // �{�c���i
	(const u16 *)L"BULGARIA",       // �u���K���A
	(const u16 *)L"CROATIA",                // �N���A�`�A
	(const u16 *)L"CYPRUS",                 // �L�v���X
	(const u16 *)L"CZECH_REPUBLIC",         // �`�F�R
	(const u16 *)L"DENMARK",                // �f���}�[�N
	(const u16 *)L"ESTONIA",                // �G�X�g�j�A
	(const u16 *)L"FINLAND",                // �t�B�������h
	(const u16 *)L"FRANCE",                 // �t�����X
	(const u16 *)L"GERMANY",                // �h�C�c
	(const u16 *)L"GREECE",                 // �M���V��
	(const u16 *)L"HUNGARY",       // �n���K���[
	(const u16 *)L"ICELAND",                // �A�C�X�����h
	(const u16 *)L"IRELAND",                // �A�C�������h
	(const u16 *)L"ITALY",                  // �C�^���A
	(const u16 *)L"LATVIA",                 // ���g�r�A
	(const u16 *)L"LESOTHO",                // ���\�g
	(const u16 *)L"LIECHTENSTEIN",          // ���q�e���V���^�C��
	(const u16 *)L"LITHUANIA",              // ���g�A�j�A
	(const u16 *)L"LUXEMBOURG",             // ���N�Z���u���N
	(const u16 *)L"MACEDONIA",              // �}�P�h�j�A
	(const u16 *)L"MALTA",       // �}���^
	(const u16 *)L"MONTENEGRO",             // �����e�l�O��
	(const u16 *)L"MOZAMBIQUE",             // ���U���r�[�N
	(const u16 *)L"NAMIBIA",                // �i�~�r�A
	(const u16 *)L"NETHERLANDS",            // �I�����_
	(const u16 *)L"NEW_ZEALAND",            // �j���[�W�[�����h
	(const u16 *)L"NORWAY",                 // �m���E�F�[
	(const u16 *)L"POLAND",                 // �|�[�����h
	(const u16 *)L"PORTUGAL",               // �|���g�K��
	(const u16 *)L"ROMANIA",                // ���[�}�j�A
	(const u16 *)L"RUSSIA",      // ���V�A
	(const u16 *)L"SERBIA",                 // �Z���r�A
	(const u16 *)L"SLOVAKIA",               // �X���o�L�A
	(const u16 *)L"SLOVENIA",               // �X���x�j�A
	(const u16 *)L"SOUTH_AFRICA",           // ��A�t���J
	(const u16 *)L"SPAIN",                  // �X�y�C��
	(const u16 *)L"SWAZILAND",              // �X���W�����h
	(const u16 *)L"SWEDEN",                 // �X�E�F�[�f��
	(const u16 *)L"SWITZERLAND",            // �X�C�X
	(const u16 *)L"TURKEY",                 // �g���R
	(const u16 *)L"UNITED_KINGDOM",   // �C�M���X
	(const u16 *)L"ZAMBIA",                 // �U���r�A
	(const u16 *)L"ZIMBABWE",               // �W���o�u�G

	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",

    // TWN���[�W����
    (const u16 *)L"TAIWAN",      // ��p
    
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
    
    // KOR���[�W����
    (const u16 *)L"SOUTH_KOREA",      // �؍�
    
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
    
    // HKG���[�W�����iWii�̍����X�g�ɑ��݁j
    (const u16 *)L"HONG_KONG",      // �z���R��
    (const u16 *)L"MACAU",                  // �}�J�I
    
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
    
    // ASI���[�W�����iWii�̍����X�g�ɑ��݁j
    (const u16 *)L"INDONESIA",      // �C���h�l�V�A
    
    // USA���[�W����
    (const u16 *)L"SINGAPORE",      // �V���K�|�[��
    
    // ASI���[�W�����i�Ăсj
    (const u16 *)L"THAILAND",      // �^�C
    (const u16 *)L"PHILIPPINES",            // �t�B���s��
    (const u16 *)L"MALAYSIA",               // �}���[�V�A

	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
    
    // ����`���[�W�����iIQue���[�W�����H�j
    (const u16 *)L"CHINA",      // ����
    
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
    
    // USA���[�W����
    (const u16 *)L"UAE",      // �A���u�񒷍��A�M
    
    // ����`���[�W����
    (const u16 *)L"INDIA",      // �C���h
    (const u16 *)L"EGYPT",      // �G�W�v�g
    (const u16 *)L"OMAN",                   // �I�}�[��
    (const u16 *)L"QATAR",                  // �J�^�[��
    (const u16 *)L"KUWAIT",                 // �N�E�F�[�g
    (const u16 *)L"SAUDI_ARABIA",           // �T�E�W�A���r�A
    (const u16 *)L"SYRIA",                  // �V���A
    (const u16 *)L"BAHRAIN",                // �o�[���[��
    (const u16 *)L"JORDAN",                 // �����_��
    
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",//180
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",//190
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",//200
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",//210
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",//220
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",//230
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",//240
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",//250
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",

    (const u16 *)L"OTHERS",
    (const u16 *)L"UNKNOWN"
};

const u32 region_country_mapping[LCFG_TWL_REGION_MAX] = 
{
	LCFG_TWL_COUNTRY_MAPPING_JAPAN,
	LCFG_TWL_COUNTRY_MAPPING_AMERICA,
	LCFG_TWL_COUNTRY_MAPPING_EUROPE,
	LCFG_TWL_COUNTRY_MAPPING_AUSTRALIA,
	LCFG_TWL_COUNTRY_MAPPING_CHINA,
	LCFG_TWL_COUNTRY_MAPPING_KOREA
};