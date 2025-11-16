# drvgen

## Requirement

1. Windows XP,vista,7,8.X,10,11
2. SP DCT tool

## WD410 DCT

```
/home/assassinscreed/Desktop/ALPS_KK_Featurephone/ALPS.KK1.MP7.V1.7_HEXING72_CWET_KK/mediatek/custom/hexing72_cwet_lca_wd410/kernel/dct/dct/codegen.dws
```

## WD410 GPIO

LCM

![LCM CFG WD410](/image/Pasted%20image%2020251116133247.png)

NFI

![NFI CFG WD410](/image/Pasted%20image%2020251116133125.png)

MIPI is NC, GPIO74, GPIO75 use as KCOL3, KCOL4

MATV is NC

other EINT than EINT2 and EINT14 is NC, they use as HP_ACCDET and MSDC1 inserted

![EINT CFG WD410](/image/Pasted%20image%2020251116133514.png)

KPD

![KPD CFG WD410](/image/Pasted%20image%2020251116133657.png)

CAM is Parallel 8bit, for build error fix define for both camera reset and pdn, use gpio73 for flash

![CAM CFG WD410](/image/Pasted%20image%2020251116133823.png)

![FLASH CFG WD410](/image/Pasted%20image%2020251116151910.png)
RF BPI is BUS0->BUS5 2G and BUS7->BUS15 3G
## WD410 EINT

![EINT SETUP WD410](/image/Pasted%20image%2020251116133943.png)
## WD410 KPD

![KPD SETUP WD410](/image/Pasted%20image%2020251116134311.png)