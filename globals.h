/*
    forcelang - language override plugin
    Written 2018/2019 by xdaniel - github.com/xdanieldzd
*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <malloc.h>

#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/sysmodule.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/appmgr.h>
#include <psp2/system_param.h>
#include <psp2/display.h>
#include <psp2/ctrl.h>

#include <taihen.h>

#include "main.h"
#include "draw.h"

#define PLUGIN_NAME     "forcelang"
#define PLUGIN_VERSION  "v0.2"
#define DATA_DIRECTORY  "ux0:/data/forcelang"
#define MENU_KEYCOMBO	(SCE_CTRL_SQUARE | SCE_CTRL_TRIANGLE | SCE_CTRL_SELECT)
