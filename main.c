/*
    forcelang - language override plugin
    Written 2018/2019 by xdaniel - github.com/xdanieldzd
*/

#include "globals.h"

const char *languageNames[] =
{
    "Japanese",
    "English (American)",
    "French",
    "Spanish",
    "German",
    "Italian",
    "Dutch",
    "Portuguese (European)",
    "Russian",
    "Korean",
    "Chinese (Traditional)",
    "Chinese (Simplified)",
    "Finnish",
    "Swedish",
    "Danish",
    "Norwegian",
    "Polish",
    "Portuguese (Brazilian)",
    "English (British)",
    "Turkish"
};

char *appTitleId, *appTitle, *configFilename, *buffer;

int sysLanguage = SCE_SYSTEM_PARAM_LANG_ENGLISH_US;
int gameLanguage = -1;
int newLanguage = -1;
int menuActive = 0;

SceCtrlButtons lastButtons;

#define HOOK_LOADMODULE     0
#define HOOK_UNLOADMODULE   1
#define HOOK_SETFRAMEBUFFER 2
#define HOOK_CTRLPEEK1      3
#define HOOK_CTRLPEEK2      4
#define HOOK_CTRLREAD1      5
#define HOOK_CTRLREAD2      6
#define HOOK_SYSPARAMGETINT 7
#define HOOK_NUM_HOOKS      8

const char *hookNames[] =
{
    "Load Module",
    "Unload Module",
    "Set Framebuffer",
    "Ctrl Peek 1",
    "Ctrl Peek 2",
    "Ctrl Read 1",
    "Ctrl Read 2",
    "Sysparam Get Int",
};

static tai_hook_ref_t hookRefs[HOOK_NUM_HOOKS];
static SceUID hookIds[HOOK_NUM_HOOKS] = { -1, -1, -1, -1, -1, -1, -1, -1 };

void _start() __attribute__ ((weak, alias ("module_start")));

int sceSysmoduleLoadModule_patched(SceSysmoduleModuleId id)
{
    int ret = TAI_CONTINUE(int, hookRefs[HOOK_LOADMODULE], id);

    if (ret >= 0)
    {
        switch (id)
        {
            case SCE_SYSMODULE_APPUTIL:
                // Hook sceAppUtilSystemParamGetInt
                if (hookIds[HOOK_SYSPARAMGETINT] < 0)
                {
                    hookIds[HOOK_SYSPARAMGETINT] = taiHookFunctionImport(&hookRefs[HOOK_SYSPARAMGETINT],
                        "SceAppUtil",
                        0x48E01D74,   // SceAppUtil
                        0x5DFB9CA0,   // sceAppUtilSystemParamGetInt
                        sceAppUtilSystemParamGetInt_patched);

                    debugCheckHook(HOOK_SYSPARAMGETINT, hookIds[HOOK_SYSPARAMGETINT]);
                }
                break;

            default:
                break;
        }
    }

    return ret;
}

int sceSysmoduleUnloadModule_patched(SceSysmoduleModuleId id)
{
    int ret = TAI_CONTINUE(int, hookRefs[HOOK_UNLOADMODULE], id);

    if (ret >= 0)
    {
        switch (id)
        {
            case SCE_SYSMODULE_APPUTIL:
                // Unhook sceAppUtilSystemParamGetInt
                if (hookIds[HOOK_SYSPARAMGETINT] >= 0)
                {
                    taiHookRelease(hookIds[HOOK_SYSPARAMGETINT], hookRefs[HOOK_SYSPARAMGETINT]);
                    hookIds[HOOK_SYSPARAMGETINT] = -1;
                }
                break;

            default:
                break;
        }
    }

    return ret;
}

int sceCtrlPeekBufferPositive_patched(int port, SceCtrlData *pad_data, int count)
{
    int ret = TAI_CONTINUE(int, hookRefs[HOOK_CTRLPEEK1], port, pad_data, count);
    handleInput(pad_data);
    return ret;
}

int sceCtrlPeekBufferPositive2_patched(int port, SceCtrlData *pad_data, int count)
{
    int ret = TAI_CONTINUE(int, hookRefs[HOOK_CTRLPEEK2], port, pad_data, count);
    handleInput(pad_data);
    return ret;
}

int sceCtrlReadBufferPositive_patched(int port, SceCtrlData *pad_data, int count)
{
    int ret = TAI_CONTINUE(int, hookRefs[HOOK_CTRLREAD1], port, pad_data, count);
    handleInput(pad_data);
    return ret;
}

int sceCtrlReadBufferPositive2_patched(int port, SceCtrlData *pad_data, int count)
{
    int ret = TAI_CONTINUE(int, hookRefs[HOOK_CTRLREAD2], port, pad_data, count);
    handleInput(pad_data);
    return ret;
}

int sceDisplaySetFrameBuf_patched(const SceDisplayFrameBuf *pParam, SceDisplaySetBufSync sync)
{
    drawUpdate(pParam);
    renderMenu();

    return TAI_CONTINUE(int, hookRefs[HOOK_SETFRAMEBUFFER], pParam, sync);
}

int sceAppUtilSystemParamGetInt_patched(unsigned int paramId, int *value)
{
    int ret = TAI_CONTINUE(int, hookRefs[HOOK_SYSPARAMGETINT], paramId, value);

    if (paramId == SCE_SYSTEM_PARAM_ID_LANG)
    {
        *value = gameLanguage;
    }

    return ret;
}

void renderMenu()
{
    if (menuActive)
    {
        drawString(20, 20, 1, colorWhite, colorBlackTransparent,
            PLUGIN_NAME " " PLUGIN_VERSION " - github.com/xdanieldzd\n"
            "%s (%s)\n"
            "System Language   %s [%02d]\n"
            "Game Language    <                                >",
            appTitle, appTitleId, languageNames[sysLanguage], sysLanguage);

        drawString(242, 144, 1, colorGreen, colorNone, "%s [%02d]", languageNames[newLanguage], newLanguage);
    }
}

void handleInput(SceCtrlData *pad_data)
{
    if (((pad_data->buttons & MENU_KEYCOMBO) == MENU_KEYCOMBO) && ((lastButtons & MENU_KEYCOMBO) != MENU_KEYCOMBO))
    {
        if (menuActive)
        {
            // Closing the menu, so set the game language and write config file
            gameLanguage = newLanguage;
            writeConfig();
        }
        else
        {
            // Opening the menu, so set the new language to game language
            newLanguage = gameLanguage;
        }

        menuActive = !menuActive;
    }

    SceCtrlButtons newButtons = (pad_data->buttons & ~lastButtons);
    lastButtons = pad_data->buttons;

    if (menuActive)
    {
        // Handle menu inputs
        if (newButtons & SCE_CTRL_LEFT)
        {
            newLanguage--;
            if (newLanguage < SCE_SYSTEM_PARAM_LANG_JAPANESE)
            {
                newLanguage = SCE_SYSTEM_PARAM_LANG_TURKISH;
            }
        }
        else if (newButtons & SCE_CTRL_RIGHT)
        {
            newLanguage++;
            if (newLanguage > SCE_SYSTEM_PARAM_LANG_TURKISH)
            {
                newLanguage = SCE_SYSTEM_PARAM_LANG_JAPANESE;
            }
        }

        pad_data->buttons = 0;
    }
}

void fetchInformation()
{
    // Initialize SceAppUtil & get system language
    SceAppUtilInitParam initParam;
    memset(&initParam, 0, sizeof(SceAppUtilInitParam));
    SceAppUtilBootParam bootParam;
    memset(&bootParam, 0, sizeof(SceAppUtilBootParam));
    sceAppUtilInit(&initParam, &bootParam);
    sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_LANG, &sysLanguage);
    sceAppUtilShutdown();

    // Default game language to system language
    gameLanguage = sysLanguage;

    // Get application's title ID & full title
    appTitleId = malloc(sizeof(char) * 12);
    sceAppMgrAppParamGetString(0, 12, appTitleId, 256);
    appTitle = malloc(sizeof(char) * 256);
    sceAppMgrAppParamGetString(0, 9, appTitle, 256);

    // Set config filename
    configFilename = malloc(sizeof(char) * 128);
    sprintf(configFilename, DATA_DIRECTORY "/%s.txt", appTitleId);

    printf("[forcelang] System language is %s (%d), current title is %s (%s).\n", languageNames[sysLanguage], sysLanguage, appTitle, appTitleId);
}

void readConfig()
{
    SceUID fd = sceIoOpen(configFilename, SCE_O_RDONLY, 0777);
    if (fd >= 0)
    {
        // Config file exists, so allocate buffer and read config value
        buffer = malloc(sizeof(char) * 128);
        memset(buffer, 0, 128);

        sceIoRead(fd, buffer, 128);
        sceIoClose(fd);

        sscanf(buffer, "%d", &gameLanguage);

        // Ensure read value is valid, otherwise force default
        if (gameLanguage < SCE_SYSTEM_PARAM_LANG_JAPANESE || gameLanguage > SCE_SYSTEM_PARAM_LANG_TURKISH)
        {
            printf("[forcelang] Read config, language override is invalid (%d), defaulting to system language %s (%d).\n", gameLanguage, languageNames[sysLanguage], sysLanguage);
            gameLanguage = sysLanguage;
        }
        else
        {
            printf("[forcelang] Read config, language override is %s (%d).\n", languageNames[gameLanguage], gameLanguage);
        }

        newLanguage = gameLanguage;
    }
    else
    {
        // Config file doesn't exist, so try to create it
        writeConfig();
    }
}

void writeConfig()
{
    SceUID fd = sceIoOpen(configFilename, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    if (fd >= 0)
    {
        // Config file created or open for writing, write game language
        sprintf(buffer, "%d", gameLanguage);

        sceIoWrite(fd, buffer, sizeof(char));
        sceIoClose(fd);

        printf("[forcelang] Created config, defaulting to system language %s (%d).\n", languageNames[sysLanguage], sysLanguage);
    }
    else
    {
        printf("[forcelang] Failed to create config file at %s!\n", configFilename);
    }
}

void debugCheckHook(int num, int id)
{
    if (id >= 0)
    {
        printf("[forcelang] Hook %d (%s) succeeded.\n", num, hookNames[num]);
    }
    else
    {
        printf("[forcelang] Hook %d (%s) failed, error %x!\n", num, hookNames[num], id);
    }
}

int module_start(SceSize argc, const void *args)
{
    // Hello!
    sceKernelDelayThread(1000 * 1000);
    printf("\n[forcelang] Initializing...\n");

    // Ensure our data directory exists
    sceIoMkdir(DATA_DIRECTORY, 0777);

    // Perform initialization...
    fetchInformation();
    readConfig();

    // Hook sceSysmoduleLoadModule & sceSysmoduleUnloadModule
    hookIds[HOOK_LOADMODULE] = taiHookFunctionImport(&hookRefs[HOOK_LOADMODULE],
        TAI_MAIN_MODULE,
        0x03FCF19D, // SceSysmodule
        0x79A0160A, // sceSysmoduleLoadModule
        sceSysmoduleLoadModule_patched);
    hookIds[HOOK_UNLOADMODULE] = taiHookFunctionImport(&hookRefs[HOOK_UNLOADMODULE],
        TAI_MAIN_MODULE,
        0x03FCF19D, // SceSysmodule
        0x31D87805, // sceSysmoduleUnloadModule
        sceSysmoduleUnloadModule_patched);

    // Hook sceDisplaySetFrameBuf
    hookIds[HOOK_SETFRAMEBUFFER] = taiHookFunctionImport(&hookRefs[HOOK_SETFRAMEBUFFER],
        TAI_MAIN_MODULE,
        TAI_ANY_LIBRARY,
        0x7A410B64, // sceDisplaySetFrameBuf
        sceDisplaySetFrameBuf_patched);

    // Hook sceCtrlPeekBufferPositive
    hookIds[HOOK_CTRLPEEK1] = taiHookFunctionImport(&hookRefs[HOOK_CTRLPEEK1],
        TAI_MAIN_MODULE,
        TAI_ANY_LIBRARY,
        0xA9C3CED6, // sceCtrlPeekBufferPositive
        sceCtrlPeekBufferPositive_patched);

    // Hook sceCtrlPeekBufferPositive2
    hookIds[HOOK_CTRLPEEK2] = taiHookFunctionImport(&hookRefs[HOOK_CTRLPEEK2],
        TAI_MAIN_MODULE,
        TAI_ANY_LIBRARY,
        0x15F81E8C, // sceCtrlPeekBufferPositive2
        sceCtrlPeekBufferPositive2_patched);

    // Hook sceCtrlReadBufferPositive
    hookIds[HOOK_CTRLREAD1] = taiHookFunctionImport(&hookRefs[HOOK_CTRLREAD1], 
        TAI_MAIN_MODULE,
        TAI_ANY_LIBRARY,
        0x67E7AB83, // sceCtrlReadBufferPositive
        sceCtrlReadBufferPositive_patched);

    // Hook sceCtrlReadBufferPositive2
    hookIds[HOOK_CTRLREAD2] = taiHookFunctionImport(&hookRefs[HOOK_CTRLREAD2], 
        TAI_MAIN_MODULE,
        TAI_ANY_LIBRARY,
        0xC4226A3E, // sceCtrlReadBufferPositive2
        sceCtrlReadBufferPositive2_patched);

    // Hook sceAppUtilSystemParamGetInt (in main module)
    hookIds[HOOK_SYSPARAMGETINT] = taiHookFunctionImport(&hookRefs[HOOK_SYSPARAMGETINT],
        TAI_MAIN_MODULE,
        0x48E01D74, // SceAppUtil
        0x5DFB9CA0, // sceAppUtilSystemParamGetInt
        sceAppUtilSystemParamGetInt_patched);

    // See if hooks succeeded
    for (int i = 0; i < HOOK_NUM_HOOKS; i++)
    {
        debugCheckHook(i, hookIds[i]);
    }

    printf("[forcelang] Initialization finished.\n\n");

    sceKernelDelayThread(1000 * 1000);

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args)
{
    // Release and free everything
    for (int i = 0; i < HOOK_NUM_HOOKS; i++)
    {
        if (hookIds[i] >= 0)
        {
            taiHookRelease(hookIds[i], hookRefs[i]);
            hookIds[i] = -1;
        }
    }

    free(appTitleId);
    free(appTitle);
    free(configFilename);
    free(buffer);

    return SCE_KERNEL_STOP_SUCCESS;
}
