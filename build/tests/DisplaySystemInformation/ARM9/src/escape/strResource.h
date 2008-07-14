static const u8 s_numMenuK[] = {
	OWNERMENU_KSIZE,
	PARENTALMENU_KSIZE,
	OTHERMENU_KSIZE,
	SCFGMENU_KSIZE,
	FUSEMENU_KSIZE,
	0,
	0,
	0,
	0,
	0,
	ROOTMENU_KSIZE
	// !!! ���ƂŎc��̕����ǉ������
};

static const u8 s_numMenuV[] = {
	OWNERMENU_VSIZE,
	PARENTALMENU_VSIZE,
	OTHERMENU_VSIZE,
	SCFGMENU_VSIZE,
	FUSEMENU_VSIZE,
	0,
	0,
	0,
	0,
	0,
	ROOTMENU_VSIZE
	// !!! ���ƂŎc��̕����ǉ������
};

static const u16 *s_strRootMenu[] = {
	L"Owner",
	L"Parental control",
	L"Other machine setting",
	L"SCFG",
	L"Fuse rom"
};

static const char *s_strMenuName[] = {
	"Owner",
	"Parental control",
	"Other machine setting",
	"SCFG",
	"Fuse rom"
};


static const u16 *s_strOwnerMenu[] = {
	L"Language",
	L"Favorite color",
	L"Birthday",
	L"Country",
	L"Nickname",
	L"Comment"
};

static const u16 *s_strParentalMenu[] = {
	L"Parental control",
	L"Organization",
	L"Age",
	L"Password",
	L"Quastion id",
	L"Answer"
};

static const u16 *s_strOtherMenu[] = {
	L"Wireless",
	L"Force Disable Wireless",
	L"",
	L"Agree EULA",
	L"Eula Version",
	L"Region",
	L"",
	L"",
	L"Unique ID",
	L"",
	L"",
	
	L"Serial No"
};

static const u16 *s_strSCFGMenu[] = {
	L"UNDER CONSTRUCTION"
};

static const u16 *s_strFuseMenu[] = {
	L"UNDER CONSTRUCTION"	
};

static const u16 **s_strMetaMenu[] = {
	s_strOwnerMenu,
	s_strParentalMenu,
	s_strOtherMenu,
	s_strSCFGMenu,
	s_strFuseMenu,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	s_strRootMenu
};

static char *s_strEnable[] = {
	"DISABLED",
	"ENABLED"
};

static char *s_strBool[] = {
	"TRUE",
	"FALSE"
};

static char *s_strRatingOrg[] = {
	"CERO",
	"ESRB",
	"BBFC",
	"USK",
	"PEGI general",
	"PEGI Finland",
	"PEGI Portugal",
	"PEGI and BBFC Great Briten",
	"OFLC",
	"GRB"
};

static  char *s_strRegion[] = {
	"JAPAN",
	"AMERICA",
	"EUROPA",
	"AUSTRALIA",
	"CHINA",
	"KOREA",
};

static  char *s_strUserColor[] = {
	"GRAY      ",
	"BROWN     ",
	"RED       ",
	"PINK      ",
	"ORANGE    ",
	"YELLOW    ",
	"LIME_GREEN",
	"GREEN     ",
	"DARK_GREEN",
	"SEA_GREEN ",
	"TURQUOISE ",
	"BLUE      ",
	"DARK_BLUE ",
	"PURPLE    ",
	"VIOLET    ",
	"MAGENTA   ",
};

static char *s_strLanguage[] = {
	"JAPANESE",
	"ENGLISH",
	"FRENCH",
	"GERMAN",
	"ITALIAN",
	"SPANISH",
	"CHINESE",
	"KOREAN",
};

static char *s_strCountry[] = {
    	"UNDEFINED  ",        // ���ݒ�
    	"JAPAN      ",        // ���{
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"Anguilla   ",        // �A���M��
    	"ANTIGUA_AND_BARBUDA",   // �A���e�B�O�A�E�o�[�u�[�_
    	"ARGENTINA   ",      // �A���[���`��
    	"ARUBA",                 // �A���o
    	"BAHAMAS",               // �o�n�}
    	"BARBADOS",              // �o���o�h�X
    	"BELIZE",                // �x���[�Y
    	"BOLIVIA",               // �{���r�A
    	"BRAZIL",                // �u���W��
    	"BRITISH_VIRGIN_ISLANDS",    // �p�̃��@�[�W������
    	"CANADA",                // �J�i�_
    	"CAYMAN_ISLANDS",        // �P�C�}������
    	"CHILE       ",      // �`��
    	"COLOMBIA",              // �R�����r�A
    	"COSTA_RICA",            // �R�X�^���J
    	"DOMINICA",              // �h�~�j�J��
    	"DOMINICAN_REPUBLIC",    // �h�~�j�J���a��
    	"ECUADOR",               // �G�N�A�h��
    	"EL_SALVADOR",           // �G���T���o�h��
    	"FRENCH_GUIANA",         // �t�����X�̃M�A�i
    	"GRENADA",               // �O���i�_
    	"GUADELOUPE",            // �O�A�h���[�v
    	"GUATEMALA   ",      // �O�A�e�}��
    	"GUYANA",                // �K�C�A�i
    	"HAITI",                 // �n�C�`
    	"HONDURAS",              // �z���W�����X
    	"JAMAICA",               // �W���}�C�J
    	"MARTINIQUE",            // �}���e�B�j�[�N
    	"MEXICO",                // ���L�V�R
    	"MONTSERRAT",            // �����g�Z���g
    	"NETHERLANDS_ANTILLES",  // �I�����_�̃A���e�B��
    	"NICARAGUA",             // �j�J���O�A
    	"PANAMA      ",      // �p�i�}
    	"PARAGUAY",              // �p���O�A�C
    	"PERU",                  // �y���[
    	"ST_KITTS_AND_NEVIS",    // �Z���g�L�b�c�E�l�C�r�X
    	"ST_LUCIA",              // �Z���g���V�A
    	"ST_VINCENT_AND_THE_GRENADINES", // �Z���g�r���Z���g�E�O���i�f�B�[��
    	"SURINAME",              // �X���i��
    	"TRINIDAD_AND_TOBAGO",   // �g���j�_�[�h�E�g�o�S
    	"TURKS_AND_CAICOS_ISLANDS",  // �^�[�N�X�E�J�C�R�X����
    	"UNITED_STATES",         // �A�����J
    	"URUGUAY     ",      // �E���O�A�C
    	"US_VIRGIN_ISLANDS",     // �ė̃o�[�W������
    	"VENEZUELA",             // �x�l�Y�G��
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"ALBANIA     ",      // �A���o�j�A
    	"AUSTRALIA",             // �I�[�X�g�����A
    	"AUSTRIA",               // �I�[�X�g���A
    	"BELGIUM",               // �x���M�[
    	"BOSNIA_AND_HERZEGOVINA",    // �{�X�j�A�E�w���c�F�S�r�i
    	"BOTSWANA",              // �{�c���i
    	"BULGARIA    ",      // �u���K���A
    	"CROATIA",               // �N���A�`�A
    	"CYPRUS",                // �L�v���X
    	"CZECH_REPUBLIC",        // �`�F�R
    	"DENMARK",               // �f���}�[�N
    	"ESTONIA",               // �G�X�g�j�A
    	"FINLAND",               // �t�B�������h
    	"FRANCE",                // �t�����X
    	"GERMANY",               // �h�C�c
    	"GREECE",                // �M���V��
    	"HUNGARY     ",      // �n���K���[
    	"ICELAND",               // �A�C�X�����h
    	"IRELAND",               // �A�C�������h
    	"ITALY",                 // �C�^���A
    	"LATVIA",                // ���g�r�A
    	"LESOTHO",               // ���\�g
    	"LIECHTENSTEIN",         // ���q�e���V���^�C��
    	"LITHUANIA",             // ���g�A�j�A
    	"LUXEMBOURG",            // ���N�Z���u���N
    	"MACEDONIA",             // �}�P�h�j�A
    	"MALTA       ",      // �}���^
    	"MONTENEGRO",            // �����e�l�O��
    	"MOZAMBIQUE",            // ���U���r�[�N
    	"NAMIBIA",               // �i�~�r�A
    	"NETHERLANDS",           // �I�����_
    	"NEW_ZEALAND",           // �j���[�W�[�����h
    	"NORWAY",                // �m���E�F�[
    	"POLAND",                // �|�[�����h
    	"PORTUGAL",              // �|���g�K��
    	"ROMANIA",               // ���[�}�j�A
    	"RUSSIA      ",     // ���V�A
    	"SERBIA",                // �Z���r�A
    	"SLOVAKIA",              // �X���o�L�A
    	"SLOVENIA",              // �X���x�j�A
    	"SOUTH_AFRICA",          // ��A�t���J
    	"SPAIN",                 // �X�y�C��
    	"SWAZILAND",             // �X���W�����h
    	"SWEDEN",                // �X�E�F�[�f��
    	"SWITZERLAND",           // �X�C�X
    	"TURKEY",                // �g���R
    	"UNITED_KINGDOM ",  // �C�M���X
    	"ZAMBIA",                // �U���r�A
    	"ZIMBABWE",              // �W���o�u�G
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"TAIWAN      ",     // ��p
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"SOUTH_KOREA ",     // �؍�
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"HONG_KONG   ",     // �z���R��
    	"MACAU",                 // �}�J�I
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"INDONESIA   ",     // �C���h�l�V�A
    	"SINGAPORE   ",     // �V���K�|�[��
    	"THAILAND    ",     // �^�C
    	"PHILIPPINES",           // �t�B���s��
    	"MALAYSIA",              // �}���[�V�A
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"CHINA       ",     // ����
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"UAE         ",     // �A���u�񒷍��A�M
    	"INDIA       ",     // �C���h
    	"EGYPT       ",     // �G�W�v�g
    	"OMAN",                  // �I�}�[��
    	"QATAR",                 // �J�^�[��
    	"KUWAIT",                // �N�E�F�[�g
    	"SAUDI_ARABIA",          // �T�E�W�A���r�A
    	"SYRIA",                 // �V���A
    	"BAHRAIN",               // �o�[���[��
    	"JORDAN",                // �����_��
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"OTHERS      ",
    	"UNKNOWN     ",
};

static char s_strNA[] = {
	""
};	