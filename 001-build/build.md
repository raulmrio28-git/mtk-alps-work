# build

## Requirement

1. Ubuntu 16.04.7
2. Virtualbox
3. MTK ALPS project

## Setup Virtualbox

1. install
2. create VM -> 8192mb/8cpu -> 120gb vdi full

## Install Ubuntu


![](image/Pasted%20image%2020251113000552.png)


![](image/Pasted%20image%2020251113000718.png)

continue

![](image/Pasted%20image%2020251113000836.png)

kb layout unchange

![](image/Pasted%20image%2020251113000927.png)

ubuntu is installing

restart now -> restart

setup guestaddition

## Setup alps

get ALPS source

get jdk6

```
https://www.oracle.com/java/technologies/javase-java-archive-javase6-downloads.html
```

setup prerequiresites

```
sudo apt-get install git-core gnupg flex bison gperf build-essential zip curl zlib1g-dev libc6-dev-i386 libncurses5 lib32ncurses5-dev x11proto-core-dev libx11-dev lib32z-dev libgl1-mesa-dev libxml2-utils xsltproc unzip
```

make should downgrade

```
<make 3.82 download and unzip>
<cd make-3.82>
./configure
make
sudo make install
<copy make to bin>
```

perl should downgrade

```
perlbrew --notest install perl-5.10.1
perlbrew use perl-5.10.1
perlbrew switch perl-5.10.1
```

setup mingw
```
sudo apt-get install mingw32
```

setup jdk6

```
<set execute>
'/home/assassinscreed/Desktop/jdk-6u45-linux-x64.bin' 
```

must modify mbldenv.sh

```
#!/bin/bash
# ##########################################################
# ALPS(Android4.1 based) build environment profile setting
# ##########################################################
# Overwrite JAVA_HOME environment variable setting if already exists
JAVA_HOME=/home/assassinscreed/jdk1.6.0_45
export JAVA_HOME

# Overwrite ANDROID_JAVA_HOME environment variable setting if already exists
ANDROID_JAVA_HOME=/home/assassinscreed/jdk1.6.0_45
export ANDROID_JAVA_HOME

# Overwrite PATH environment setting for JDK & arm-eabi if already exists
PATH=/home/assassinscreed/jdk1.6.0_45/bin:$PWD/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.6/bin:$PATH
export PATH

# Add MediaTek developed Python libraries path into PYTHONPATH
if [ -z "$PYTHONPATH" ]; then
  PYTHONPATH=$PWD/mediatek/build/tools
else
  PYTHONPATH=$PWD/mediatek/build/tools:$PYTHONPATH
fi
export PYTHONPATH
```

## How to make

```
Usage: (makeMtk|mk) [options] project actions [modules]

Options:
  -t, -tee      : Print log information on the standard-out.
  -o, -opt=bypass_argument_to_make
                : Pass extra arguments to make.

  -h, -help     : Print this message and exit.

Projects:
  one of available projects.

Actions:
  listp, listproject
                : List all available projects.

  check-env     : Check if build environment is ready.
  check-dep     : Check feature dependency.
  n, new        : Clean and perform a full build.
  c, clean      : Clean the immediate files(such as, objects, libraries etc.).
  r, remake     : Rebuild(target will be updated if any dependency updats).
  mrproper      : Remove all generated files + config + various backup files in Kbuild process.
  bm_new        : "new" + GNU make's "-k"(keep going when encounter error) feature.
  bm_remake     : "remake" + GNU make's "-k"(keep going when encounter error) feature.
  mm            : Build module through Android native command "mm"
  mma           : Build module through Android native command "mma"

  emigen        : Generate EMI setting source code.
  nandgen       : Generate supported NAND flash device list.
  codegen       : Generate trace DB(for META/Cather etc. tools used).
  drvgen        : Generate driver customization source.
  custgen       : Generate customization source.
  javaoptgen    : Generate the global java options.
  ptgen         : Generate partition setting header & scatter file.
  bindergen     : Generate binder related information

  sign-image    : Sign all the image generated.
  encrypt-image : Encrypt all the image generated.
  update-api    : Android default build action
                  (be executed if system setting or anything removed from API).
  check-modem   : Check modem image consistency.
  upadte-modem  : Update modem image located in system.img.
  modem-info    : Show modem version
  gen-relkey    : Generate releasekey for application signing.
  check-appres  : Check unused application resource.

  sdk           : Build sdk package.
  win_sdk       : Build sdk package with a few Windows tools.
  banyan_addon  : Build MTK sdk addon.
  banyan_addon_x86 :Build MTK sdk x86 addon.
  cts           : Build cts package.
  bootimage     : Build boot image(boot.img).
  cacheimage    : Build cache image(cache.img).
  systemimage   : Build system image(system.img).
  snod          : Build system image without dependency.
                  (that is, ONLY pack the system image, NOT checking its dependencies.)
  recoveryimage : Build recovery image(recovery.img).
  secroimage    : Build secro image(secro.img).
  factoryimage  : Build factory image(factory.img).
  userdataimage : Build userdata image(userdata.img).
  userdataimage-nodeps
                : Build userdata image without dependency.
                  (that is, ONLY pack the userdata image, NOT checking its dependencies.)
  dump-products : Dump products related configuration(PRODUCT_PACKAGE,PRODUCT_NAME ect.)
  target-files-package
                : Build the target files package.
                  (A zip of the directories that map to the target filesystem.
                   This zip can be used to create an OTA package or filesystem image
                   as a post-build step.)
  updatepackage : Build the update package.
  dist          : Build distribution package.

Modules:
  pl, preloader : Specify to build preloader.
  lk            : Specify to build little kernel.
  md32          : Specify to build DSP md32.
  tz, trustzone : Specify to build trusted execution environment.
  k,  kernel    : Specify to build kernel.
  dr, android   : Specify to build android.
  NULL          : Specify to build all components/modules in default.
  k <module path>
                : Specify to build kernel component/module with the source path.
  dr <module name>
                : Specify to build android component/module with module name.

Other tools:
  prebuilts/misc/linux-x86/ccache/ccache -M 10G
                : Set CCACHE pool size to 10GB

Example:
  ./mk -t e1k emigen
                : Generate EMI setting source code.
  ./mk -o=TARGET_BUILD_VARIANT=user e1k n
                : Start a user mode full build.
  ./mk listp    : List all available projects.
  ./mk e1k bootimage
                : Build bootimage for e1k project.
  ./mk e1k bm_new k
                : Build kernel for e1k project.
  ./makeMtk e1k c,bm_remake pl k
                : Clean & Build preloader and kernel for e1k project. 
  ./makeMtk e1k n k kernel/xxx/xxx
                : Build(full build) kernel component/module 
                  under the path "kernel/xxx/xxx" for e1k project.
  ./makeMtk e1k r dr Gallery
                : Rebuild android module named Gallery for e1k project.
  ./makeMtk e1k mm packages/apps/Settings
    : Change Directory to packages/apps/Settings and execute "mm"
  ./makeMtk e1k mma packages/apps/Settings
    : Change Directory to packages/apps/Settings and execute "mma"
```

new building:

```
./mk project n
```

remake:

```
./mk project r <type>
```

e.g. updated the LCD driver:

```
./mk project r lk kernel
```

e.g. update DCT, NAND

```
./mk project r preloader  
./mk project r lk  
./mk project r kernel  
./mk project bootimage
```

e.g. update application

```
./mk project r android
```