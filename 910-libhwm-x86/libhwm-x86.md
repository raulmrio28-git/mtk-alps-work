
# libhwm-x86

This source files are for compile hwm backport to X86 to prevent compile error in banyan simulator

Compile steps:

```
gcc -shared -o libhwm.so -fPIC '/media/sf_mtk-alps-work/910-libhwm-x86/hwm.c'  -I'/media/sf_mtk-alps-work/910-libhwm-x86' -I'/home/assassinscreed/Desktop/ALPS_KK_Featurephone/ALPS.KK1.MP7.V1.7_HEXING72_CWET_KK/mediatek/external/nvram/libnvram' -I'/home/assassinscreed/Desktop/ALPS_KK_Featurephone/ALPS.KK1.MP7.V1.7_HEXING72_CWET_KK/system/core/include'  -I'/home/assassinscreed/Desktop/ALPS_KK_Featurephone/ALPS.KK1.MP7.V1.7_HEXING72_CWET_KK/out/target/product/banyan_addon_wd410_x86/obj/CUSTGEN/custom/cgen/inc' -I'/home/assassinscreed/Desktop/ALPS_KK_Featurephone/ALPS.KK1.MP7.V1.7_HEXING72_CWET_KK/mediatek/external/nvram/libfile_op' -I'/home/assassinscreed/Desktop/ALPS_KK_Featurephone/ALPS.KK1.MP7.V1.7_HEXING72_CWET_KK/mediatek/kernel/include' 
```

Must copy to vendor/mediatek/banyan_addon_wd410_x86/artifacts/out/target/product/banyan_addon_wd410_x86/obj/lib with make dummy libhwm_intermediates