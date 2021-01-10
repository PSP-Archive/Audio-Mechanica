#ifndef __PSPUTILITY_USBMODULES_H__
#define __PSPUTILITY_USBMODULES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <psptypes.h>

#define PSP_USB_MODULE_PSPCM 1
#define PSP_USB_MODULE_ACC	 2
#define PSP_USB_MODULE_MIC	 3
#define PSP_USB_MODULE_CAM	 4
#define PSP_USB_MODULE_GPS	 5

/**
 * Load an usb module (PRX) from user mode.
 * Available on firmware 2.70 and higher only.
 *
 * @param module - module number to load (PSP_USB_MODULE_xxx)
 * @return 0 on success, < 0 on error
 */
int sceUtilityLoadUsbModule(int module);

/**
 * Unload an usb module (PRX) from user mode.
 * Available on firmware 2.70 and higher only.
 *
 * @param module - module number be unloaded
 * @return 0 on success, < 0 on error
 */
int sceUtilityUnloadUsbModule(int module);

#ifdef __cplusplus
}
#endif

#endif
