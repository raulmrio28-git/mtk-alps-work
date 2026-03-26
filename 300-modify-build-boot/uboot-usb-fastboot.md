
ALPS.KK1.MP7.V1.7_HEXING72_CWET_KK/mediatek/custom/hexing72_cwet_lca_wd411_test/lk/include/target/cust_usb.h

```c
#ifndef __CUST_USB_H__
#define __CUST_USB_H__


#define CONFIG_USBD_LANG "0409"


#define USB_VENDORID (0x0BB4)
#define USB_PRODUCTID (0x0C01)
#define USB_VERSIONID (0x0100)

//jdt usb info changing 20260304
#ifdef JDT_LK_USB_DEVICE_CHANGE
#define USB_MANUFACTURER "HUIZHOU JINDA TECHNOLOGY CO.,LTD"
#define USB_PRODUCT_NAME "JDT PRODUCT"
#define FASTBOOT_DEVNAME "JDT FASTBOOT"
#else
#define USB_MANUFACTURER "MediaTek"
#define USB_PRODUCT_NAME "Android"
#define FASTBOOT_DEVNAME "mt6572v1_phone"
#endif
//jdt usb info changing 20260304

#define SN_BUF_LEN 19

#endif /* __CUST_USB_H__ */
```

serial (use barcode in PRO_INFO partition): ALPS.KK1.MP7.V1.7_HEXING72_CWET_KK/bootable/bootloader/lk/app/mt_boot/mt_boot.c

```c
#define SERIAL_NUM_FROM_BARCODE
```

serial number for not programmed PRO_INFO

```c
//jdt serial number changing 20260304
#ifdef JDT_LK_USB_DEVICE_CHANGE
#define DEFAULT_SERIAL_NUM "JDTFACTORY$00000000" //空缺编号以芯片序列号填充
#else
#define DEFAULT_SERIAL_NUM "0123456789ABCDEF"
#endif
//jdt serial number changing 20260304
```

```c
//jdt serial number changing 20260304
#ifdef JDT_LK_USB_DEVICE_CHANGE
char sn_buf[SN_BUF_LEN+1] = DEFAULT_SERIAL_NUM; //空缺编号以芯片序列号填充
char pn_buf[20] = "FASTBOOT_00000000"; //pn value
#else
#if defined(CONFIG_MTK_USB_UNIQUE_SERIAL) || (defined(MTK_SECURITY_SW_SUPPORT) && defined(MTK_SEC_FASTBOOT_UNLOCK_SUPPORT))
#define SERIALNO_LEN 38 /* from preloader */
char sn_buf[SN_BUF_LEN+1] = ""; /* will read from EFUSE_CTR_BASE */
#else
char sn_buf[SN_BUF_LEN+1] = FASTBOOT_DEVNAME;
#endif
#endif
//jdt serial number changing 20260304
```

```c
void mt_boot_init(const struct app_descriptor *app)
{
...

#ifdef SERIAL_NUM_FROM_BARCODE
	ser_len = read_product_info(tmp);
	if(ser_len == 0) {
		ser_len = strlen(DEFAULT_SERIAL_NUM);
		strncpy(tmp, DEFAULT_SERIAL_NUM, ser_len);
//jdt serial number changing 20260305
#ifdef JDT_LK_USB_DEVICE_CHANGE
        {
            uint32_t key;
            uint32_t idx = 11; //after FASTBOOT_ this will chip key
            static char udc_chr[32] = {"ABCDEFGHIJKLMNOPQRSTUVWSYZ456789"};
            key = get_devinfo_with_index(13);
            key |= get_devinfo_with_index(12);
            for (int i = 0; i < 8; i++) {
                tmp[idx + i] = udc_chr[key & 0x1f];
                key >>= 5;
            }
        }
#endif
//jdt serial number changing 20260305
	}
	memset( sn_buf, 0, sizeof(sn_buf));
	strncpy( sn_buf, tmp, ser_len);
#endif
	sn_buf[SN_BUF_LEN] = '\0';
	surf_udc_device.serialno = sn_buf;
//jdt serial number changing 20260305
#ifdef JDT_LK_USB_DEVICE_CHANGE
	if (g_boot_mode == FASTBOOT)
	{
        uint32_t key;
        uint32_t idx = 9; //after FASTBOOT_ this will chip key
        static char udc_chr[32] = {"ABCDEFGHIJKLMNOPQRSTUVWSYZ456789"};
        key = get_devinfo_with_index(13);
		key |= get_devinfo_with_index(12);
        for (int i = 0; i < 8; i++) {
            pn_buf[idx + i] = udc_chr[key & 0x1f];
            key >>= 5;
        }
        surf_udc_device.product = pn_buf;
    }
#endif
//jdt serial number changing 20260305
... 
	sz = target_get_max_flash_size();
	fastboot_init(target_get_scratch_address(), sz);
	udc_start();

}
```