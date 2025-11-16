
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

![](image/Pasted%20image%2020251116133247.png)

NFI

![](image/Pasted%20image%2020251116133125.png)

MIPI is NC, GPIO74, GPIO75 use as KCOL3, KCOL4

MATV is NC

other EINT than EINT2 and EINT14 is NC, they use as HP_ACCDET and MSDC1 inserted

![](image/Pasted%20image%2020251116133514.png)

KPD

![](image/Pasted%20image%2020251116133657.png)

CAM is Parallel 8bit, for build error fix define for both camera reset and pdn, use gpio73 for flash

![](image/Pasted%20image%2020251116133823.png)

![](image/Pasted%20image%2020251116151910.png)
RF BPI is BUS0->BUS5 2G and BUS7->BUS15 3G
## WD410 EINT

![](image/Pasted%20image%2020251116133943.png)
## WD410 KPD

![](image/Pasted%20image%2020251116134311.png)