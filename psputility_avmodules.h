#ifndef __PSPUTILITY_AVMODULES_H__
#define __PSPUTILITY_AVMODULES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <psptypes.h>

#define PSP_AV_MODULE_AVCODEC	0
#define PSP_AV_MODULE_SASCORE	1	 
#define PSP_AV_MODULE_ATRAC3	2
#define PSP_AV_MODULE_MPEG		3

/**
 * Load an AV module (PRX) from user mode.
 * Available on firmware 2.70 and higher only.
 *
 * @param module - module number to load (PSP_AV_MODULE_xxx)
 * @return 0 on success, < 0 on error
 */
int sceUtilityLoadAvModule(int module);

/**
 * Unload an av module (PRX) from user mode.
 * Available on firmware 2.70 and higher only.
 *
 * @param module - module number be unloaded
 * @return 0 on success, < 0 on error
 */
int sceUtilityUnloadAvModule(int module);

#ifdef __cplusplus
}
#endif

#endif

