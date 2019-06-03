/*
    forcelang - language override plugin
    Written in July 2018 by xdaniel - github.com/xdanieldzd
*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/appmgr.h>
#include <psp2/system_param.h>

#include <taihen.h>

#define DATA_DIRECTORY "ux0:/data/forcelang"

const char * languageNames[] =
{
    "Japanese",
    "American English",
    "French",
    "Spanish",
    "German",
    "Italian",
    "Dutch",
    "Portugal Portuguese",
    "Russian",
    "Korean",
    "Traditional Chinese",
    "Simplified Chinese",
    "Finnish",
    "Swedish",
    "Danish",
    "Norwegian",
    "Polish",
    "Brazil Portuguese",
    "British English",
    "Turkish"
};

static tai_hook_ref_t sysParamGet_hookRef;
static SceUID taiPatchRef;

char * appTitleId;
char * filename;
char * buffer;

int sysLanguage = SCE_SYSTEM_PARAM_LANG_ENGLISH_US;
int gameLanguage = -1;

int sceAppUtilSystemParamGetInt_patched(unsigned int paramId, int *value)
{
    if (paramId == SCE_SYSTEM_PARAM_ID_LANG)
    {
        *value = gameLanguage;
        return 0;
    }

    return TAI_CONTINUE(int, sysParamGet_hookRef, paramId, value);
}

void _start() __attribute__ ((weak, alias ("module_start")));

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

    // Get application's title ID
    appTitleId = malloc(sizeof(char) * 12);
    sceAppMgrAppParamGetString(0, 12, appTitleId, 256);

    printf("[forcelang] System language is %s (%d), current title ID is %s.\n", languageNames[sysLanguage], sysLanguage, appTitleId);
}

void readConfig()
{
    // Allocate buffer
    buffer = malloc(sizeof(char) * 128);
    memset(buffer, 0, 128);

    // Try to open config file, also create directory if necessary
    sceIoMkdir(DATA_DIRECTORY, 0777);

    filename = malloc(sizeof(char) * 128);
    sprintf(filename, DATA_DIRECTORY "/%s.txt", appTitleId);
    SceUID fd = sceIoOpen(filename, SCE_O_RDONLY, 0777);

    if (fd >= 0)
    {
        // Config file exists, read the value
        sceIoRead(fd, buffer, 128);
        sceIoClose(fd);

        sscanf(buffer, "%d", &gameLanguage);

        if (gameLanguage < SCE_SYSTEM_PARAM_LANG_JAPANESE || gameLanguage > SCE_SYSTEM_PARAM_LANG_TURKISH)
        {
            printf("[forcelang] Read config, language override is invalid (%d), defaulting to system language %s (%d).\n", gameLanguage, languageNames[sysLanguage], sysLanguage);
            gameLanguage = sysLanguage;
        }
        else
        {
            printf("[forcelang] Read config, language override is %s (%d).\n", languageNames[gameLanguage], gameLanguage);
        }
    }
    else
    {
        // Config file doesn't exist, try to create it
        fd = sceIoOpen(filename, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
        if (fd >= 0)
        {
            // Config file created, write the system language as default
            sprintf(buffer, "%d", sysLanguage);

            sceIoWrite(fd, buffer, sizeof(char));
            sceIoClose(fd);

            printf("[forcelang] Created config, defaulting to system language %s (%d).\n", languageNames[sysLanguage], sysLanguage);
        }
        else
        {
            printf("[forcelang] Failed to create config file at %s!\n", filename);
        }
    }
}

int module_start(SceSize argc, const void *args)
{
    // Hello!
    sceKernelDelayThread(1000 * 1000);
    printf("\n[forcelang] Initializing...\n");

    // Perform initialization...
    fetchInformation();
    readConfig();

    // Patch sceAppUtilSystemParamGetInt
    taiPatchRef = taiHookFunctionImport(&sysParamGet_hookRef,
                                         TAI_MAIN_MODULE,
                                         0x48E01D74,    // SceAppUtil
                                         0x5DFB9CA0,    // sceAppUtilSystemParamGetInt
                                         sceAppUtilSystemParamGetInt_patched);

    // See if hook succeeded
    if (taiPatchRef >= 0)
    {
        printf("[forcelang] Function hook succeeded.\n");
    }
    else
    {
        printf("[forcelang] Failed to hook function, error %x!\n", taiPatchRef);
    }

    printf("[forcelang] Initialization finished.\n\n");

    sceKernelDelayThread(1000 * 1000);

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args)
{
    // Release and free everything
    taiHookRelease(taiPatchRef, sysParamGet_hookRef);

    free(appTitleId);
    free(filename);
    free(buffer);

    return SCE_KERNEL_STOP_SUCCESS;
}
