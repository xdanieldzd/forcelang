forcelang
=========

A taiHEN plugin for overriding an application's language, if queried by `sceAppUtilSystemParamGetInt` and `SCE_SYSTEM_PARAM_ID_LANG`. Possibly inefficient, possibly still buggy, it is literally the author's "My First Vita Homebrew".

Requirements
------------

* A way to use taiHEN plugins (developed using a PS Vita on 3.65 Enso)
* [ioplus](https://github.com/CelesteBlue-dev/PSVita-RE-tools/tree/master/ioPlus/ioPlus-0.1/release) (for config file access)

Installation
------------

Copy `forcelang.suprx` and `ioplus.skprx` to `ur0:/tai`, then add the plugin to the `config.txt` section of each application you want to use it with.

```
*KERNEL
ur0:tai/ioplus.skprx

# Example for Digimon Story: Cyber Sleuth (European PSN)
*PCSB00861
ur0:tai/forcelang.suprx
```

Configuration
-------------

When starting an application with the plugin enabled, the plugin will look for, and if necessary create, a configuration file at `ur0:/data/forcelang/[title ID].txt`, ex. `PCSB00861.txt`.

This file contains a single number, corresponding to the Vita system's `SceSystemParamLang` enum. Thus, valid values are:

* 0: Japanese
* 1: American English
* 2: French
* 3: Spanish
* 4: German
* 5: Italian
* 6: Dutch
* 7: Portugal Portuguese
* 8: Russian
* 9: Korean
* 10: Traditional Chinese
* 11: Simplified Chinese
* 12: Finnish
* 13: Swedish
* 14: Danish
* 15: Norwegian
* 16: Polish
* 17: Brazil Portuguese
* 18: British English
* 19: Turkish

Do note that the application might not support the language requested, or it might not use the function patched by this plugin to determine the language. If an application supports switching languages ex. via its options menu, use that instead.
