copy ALPS.KK1.MP7.V1.7_HEXING72_CWET_KK/mediatek/custom/common/lk/logo/qvga to ALPS.KK1.MP7.V1.7_HEXING72_CWET_KK/mediatek/custom/common/lk/logo/qvga_jdt

logo added

![](300-modify-build-boot/preview/battery_bkg.bmp)

![](300-modify-build-boot/preview/battery_charging_1.bmp)

![](300-modify-build-boot/preview/battery_charging_2.bmp)

![](300-modify-build-boot/preview/battery_charging_3.bmp)

![](300-modify-build-boot/preview/battery_charging_4.bmp)

![](300-modify-build-boot/preview/battery_charging_5.bmp)

![](300-modify-build-boot/preview/battery_error.bmp)

![](300-modify-build-boot/preview/battery_full.bmp)

![](300-modify-build-boot/preview/logo.bmp)

modificaiton:

ALPS.KK1.MP7.V1.7_HEXING72_CWET_KK/bootable/bootloader/lk/lib/libshowlogo/show_animation_common.c

ALPS.KK1.MP7.V1.7_HEXING72_CWET_KK/mediatek/external/libshowlogo/show_animation_common.c

1. No show number for charge screen

```c
void fill_animation_number(unsigned int index, unsigned int number_position, void *fill_addr,  void * logo_addr, LCM_SCREEN_T phical_screen)
{
// for jdt logo, no showing number 20260301
#ifndef JDT_LK_LOGO
    LOG_ANIM("[show_animation_common: %s %d]index= %d, number_position = %d\n",__FUNCTION__,__LINE__, index, number_position);

    LOGO_PARA_T logo_info;
    if(check_logo_index_valid(index, logo_addr, &logo_info) != CHECK_LOGO_BIN_OK)
        return;                 

    // draw default number rect,
    decompress_logo((void*)logo_info.inaddr, (void*)number_pic_addr, logo_info.logolen, number_pic_size);

    //static RECT_REGION_T number_location_rect = {NUMBER_LEFT,NUMBER_TOP,NUMBER_RIGHT,NUMBER_BOTTOM};    
    RECT_REGION_T battery_number_rect = {NUMBER_LEFT +./ALPS.KK1.MP7.V1.7_HEXING72_CWET_KK/mediatek/platform/mt6572/lk/mt_logo.c (NUMBER_RIGHT - NUMBER_LEFT)*number_position,
                            NUMBER_TOP,
                            NUMBER_RIGHT + (NUMBER_RIGHT - NUMBER_LEFT)*number_position,
                            NUMBER_BOTTOM};   
                                                     
    fill_rect_with_content(fill_addr, battery_number_rect, (unsigned short *)number_pic_addr,phical_screen);    
#endif        
// for jdt logo, no showing number 20260301
}
```

2. Forcing type ani to new

```c
void fill_animation_battery_by_ver(unsigned int capacity,void *fill_addr, void * dec_logo_addr, void * logo_addr,
                        LCM_SCREEN_T phical_screen, int version)
{
    LOG_ANIM("[show_animation_common: %s %d]version : %d\n",__FUNCTION__,__LINE__, version);
//jdt force charging 20260301
#if defined(JDT_LK_LOGO)
    fill_animation_battery_ver_1(capacity, fill_addr, dec_logo_addr, logo_addr, phical_screen);
#else
    switch (version)
    {
        case 0:
            fill_animation_battery_ver_0(capacity, fill_addr, dec_logo_addr, logo_addr, phical_screen);
            
            break;
        case 1:
            fill_animation_battery_ver_1(capacity, fill_addr, dec_logo_addr, logo_addr, phical_screen);
            
            break;
        case 2:
            fill_animation_battery_ver_2(capacity, fill_addr, dec_logo_addr, logo_addr, phical_screen);
            
            break;            
        default:
            fill_animation_battery_ver_0(capacity, fill_addr, dec_logo_addr, logo_addr, phical_screen);
            
            break;   
    }    
#endif      
//jdt force charging 20260301           
}                       
```

ALPS.KK1.MP7.V1.7_HEXING72_CWET_KK/mediatek/custom/common/lk/include/target/cust_display.h

1. Order of screen

```c
//for jdt logo 20260301
#if defined(JDT_LK_LOGO)

// LOGO number
#define ANIM_V0_LOGO_NUM   4            // version 0: show 4 recatangle growing animation without battery number
#define ANIM_V1_LOGO_NUM   4           // version 1: show wave animation with  battery number 
#define ANIM_V2_LOGO_NUM   4           // version 2: show wireless charging animation      

// Common LOGO index
#define BOOT_LOGO_INDEX   0 
#define KERNEL_LOGO_INDEX   0 

#define ANIM_V0_BACKGROUND_INDEX   1 
#define ANIM_V1_BACKGROUND_INDEX   1
 
 
#define LOW_BATTERY_INDEX   1 
#define CHARGER_OV_INDEX   2 
#define FULL_BATTERY_INDEX   8 

// version 1: show wave animation with  battery number 

// NUMBER LOGO INDEX
#define NUMBER_PIC_START_0   0
#define NUMBER_PIC_PERCENT   0 

// DYNAMIC ANIMATION LOGO INDEX
#define BAT_ANIM_START_0   3

// LOW BATTERY(0~10%) ANIMATION LOGO
#define LOW_BAT_ANIM_START_0    3 

#define ANIM_LINE_INDEX   0 


// version 2: show wireless charging animation logo index

#define V2_NUM_START_0_INDEX  3  
#define V2_NUM_PERCENT_INDEX  0
 
#define V2_BAT_0_10_START_INDEX     3  
#define V2_BAT_10_40_START_INDEX    4 
#define V2_BAT_40_80_START_INDEX    5 
#define V2_BAT_80_100_START_NDEX   6

#define V2_BAT_0_INDEX   3
#define V2_BAT_100_INDEX   7

// show fast charging animation logo index

#define FAST_CHARGING_BAT_100_INDEX   7
#define FAST_CHARGING_BAT_START_0_INDEX   3

#define FAST_CHARGING_NUM_START_0_INDEX   0
#define FAST_CHARGING_NUM_PERCENT_INDEX   0
#else

// LOGO number
#define ANIM_V0_LOGO_NUM   5            // version 0: show 4 recatangle growing animation without battery number
#define ANIM_V1_LOGO_NUM   39           // version 1: show wave animation with  battery number 
#define ANIM_V2_LOGO_NUM   86           // version 2: show wireless charging animation      

// Common LOGO index
#define BOOT_LOGO_INDEX   0 
#define KERNEL_LOGO_INDEX   38 

#define ANIM_V0_BACKGROUND_INDEX   1 
#define ANIM_V1_BACKGROUND_INDEX   35
 
 
#define LOW_BATTERY_INDEX   2 
#define CHARGER_OV_INDEX   3 
#define FULL_BATTERY_INDEX   37 

// version 1: show wave animation with  battery number 

// NUMBER LOGO INDEX
#define NUMBER_PIC_START_0   4 
#define NUMBER_PIC_PERCENT   14 

// DYNAMIC ANIMATION LOGO INDEX
#define BAT_ANIM_START_0   15 

// LOW BATTERY(0~10%) ANIMATION LOGO
#define LOW_BAT_ANIM_START_0    25 

#define ANIM_LINE_INDEX   36 


// version 2: show wireless charging animation logo index

#define V2_NUM_START_0_INDEX  57  
#define V2_NUM_PERCENT_INDEX  67 
 
#define V2_BAT_0_10_START_INDEX     68  
#define V2_BAT_10_40_START_INDEX    72 
#define V2_BAT_40_80_START_INDEX    76 
#define V2_BAT_80_100_START_NDEX   80

#define V2_BAT_0_INDEX   84
#define V2_BAT_100_INDEX   85

// show fast charging animation logo index

#define FAST_CHARGING_BAT_100_INDEX   39
#define FAST_CHARGING_BAT_START_0_INDEX   40

#define FAST_CHARGING_NUM_START_0_INDEX   46
#define FAST_CHARGING_NUM_PERCENT_INDEX   56
#endif
//for jdt logo 20260301
```

ALPS.KK1.MP7.V1.7_HEXING72_CWET_KK/mediatek/external/libshowlogo/charging_animation.cpp

1. Forcer logotype

```c
void anim_logo_init(void)
{
...
//jdt force logo version 20260301
#ifdef JDT_LK_LOGO
    show_animationm_ver = 1;
#else
    if (show_animationm_ver > 0)
    {
        unsigned int *pinfo = (unsigned int*)logo_addr;         
        LOG_ANIM("[charging_animation: %s %d]pinfo[0]=0x%08x, pinfo[1]=0x%08x, pinfo[2]=%d\n", __FUNCTION__,__LINE__,
                    pinfo[0], pinfo[1], pinfo[2]);
                    
        if ((show_animationm_ver == WIRELESS_CHARGING_ANIM_VER) && (pinfo[0] < ANIM_V2_LOGO_NUM))
        {
            set_anim_version(1);
        }
        if (pinfo[0] < ANIM_V1_LOGO_NUM)
        {
            kernel_logo_position = ANIM_V0_LOGO_NUM - 1;
            set_anim_version(0);
        }
    }
#endif
//jdt force logo version 20260301
...
}
```

ALPS.KK1.MP7.V1.7_HEXING72_CWET_KK/mediatek/platform/mt6572/lk/mt_logo.c

1. Forcer logotype
```c
void sync_anim_version()
{
    dprintf(INFO, "[lk logo: %s %d]\n",__FUNCTION__,__LINE__);
// for jdt logo, force version 20260301
#if (defined(ANIMATION_NEW) || defined(JDT_LK_LOGO))
    show_animationm_ver = 1 ;     
#else
    show_animationm_ver = 0 ;
    dprintf(INFO, "[lk logo %s %d]not define ANIMATION_NEW:show old animation \n",__FUNCTION__,__LINE__); 
#endif
// for jdt logo, force version 20260301
}
```

```c
void init_fb_screen()
{
...
// for jdt logo, force version 20260301
#if !defined(JDT_LK_LOGO)
    if (show_animationm_ver == 1)
    {
        unsigned int logonum;
        unsigned int *db_addr = logo_addr;
    
        unsigned int *pinfo = (unsigned int*)db_addr;
        
        logonum = pinfo[0];
        dprintf(INFO, "[lk logo: %s %d]pinfo[0]=0x%08x, pinfo[1]=0x%08x, pinfo[2]=%d\n", __FUNCTION__,__LINE__,
                    pinfo[0], pinfo[1], pinfo[2]);
    
        dprintf(INFO, "[lk logo: %s %d]define ANIMATION_NEW:show new animation with capacity num\n",__FUNCTION__,__LINE__); 
        dprintf(INFO, "[lk logo: %s %d]CAPACITY_LEFT =%d, CAPACITY_TOP =%d \n",__FUNCTION__,__LINE__,(CAPACITY_LEFT) ,(CAPACITY_TOP) ); 
        dprintf(INFO, "[lk logo: %s %d]LCM_HEIGHT=%d, LCM_WIDTH=%d \n",__FUNCTION__,__LINE__,(CAPACITY_RIGHT),(CAPACITY_BOTTOM)); 
        if(logonum < 6)
        {
            show_animationm_ver = 0 ;
        } else {
            show_animationm_ver = 1 ; 
        }
    }
#endif
// for jdt logo, force version 20260301
}
```