/*
    forcelang - language override plugin
    Written 2018/2019 by xdaniel - github.com/xdanieldzd
*/

extern const char *languageNames[];

extern char *appTitleId, *appTitle, *configFilename, *buffer;

extern int sysLanguage;
extern int gameLanguage;
extern int menuActive;

extern const char *hookNames[];

int sceSysmoduleLoadModule_patched(SceSysmoduleModuleId id);
int sceSysmoduleUnloadModule_patched(SceSysmoduleModuleId id);
int sceCtrlPeekBufferPositive_patched(int port, SceCtrlData *pad_data, int count);
int sceCtrlPeekBufferPositive2_patched(int port, SceCtrlData *pad_data, int count);
int sceCtrlReadBufferPositive_patched(int port, SceCtrlData *pad_data, int count);
int sceCtrlReadBufferPositive2_patched(int port, SceCtrlData *pad_data, int count);
int sceDisplaySetFrameBuf_patched(const SceDisplayFrameBuf *pParam, SceDisplaySetBufSync sync);
int sceAppUtilSystemParamGetInt_patched(unsigned int paramId, int *value);

void renderMenu();
void handleInput();
void fetchInformation();
void readConfig();
void writeConfig();
void debugCheckHook(int num, int id);
