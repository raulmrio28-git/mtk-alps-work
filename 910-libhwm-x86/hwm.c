#define __LIBHWM_C__
/*---------------------------------------------------------------------------*/
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <dirent.h>
#include "libhwm.h"
#include "libnvram.h"
#include "CFG_file_lid.h"
#include "CFG_module_file.h"
#include "libfile_op.h"

/*---------------------------------------------------------------------------*/
#undef LOG_TAG
#define LOG_TAG "HWMLIB"
#include <log/log.h>
#include "linux/sensors_io.h"
/*---------------------------------------------------------------------------*/
#define HWMLOGD(fmt, arg...) \
    do {                     \
        printf(fmt, ##arg);  \
        ALOGD(fmt, ##arg);   \
    } while (0)
#define HWMLOGE(fmt, arg...)                 \
    do {                                     \
        printf("%s: " fmt, __func__, ##arg); \
        ALOGE("%s: " fmt, __func__, ##arg);  \
    } while (0)
/*----------------------------------------------------------------------------*/
#define DIVERSE_X 0x0008
#define DIVERSE_Y 0x0004
#define DIVERSE_Z 0x0002
#define DIVERSE_XYZ 0x0001
#define C_MAX_HWMSEN_EVENT_NUM 4

/*---------------------------------------------------------------------------*/

struct SENSOR_DATA {
	int x;
	int y;
	int z;
};

/*---------------------------------------------------------------------------*/
//
//      Accelerometer Self-Test
//
/*---------------------------------------------------------------------------*/
int gsensor_selftest(int enable) {
    int err, fd = open(GSENSOR_ATTR_SELFTEST, O_RDWR);
    char buf[] = {enable + '0'};

    if (fd == -1) {
        HWMLOGE("open gsensor err = %s\n", strerror(errno));
        return -errno;
    }

    do {
        err = write(fd, buf, sizeof(buf));
    } while (err < 0 && errno == EINTR);

    if (err != sizeof(buf)) {
        HWMLOGE("write fails = %s\n", strerror(errno));
        err = -errno;
    } else {
        err = 0; /*no error*/
    }

    if (close(fd) == -1) {
        HWMLOGE("close fails = %s\n", strerror(errno));
        err = (err) ? (err) : (-errno);
    }
    return err;
}
/*----------------------------------------------------------------------------*/
static int gsensor_judge_selftest_result(int prv[C_MAX_HWMSEN_EVENT_NUM],
                                         int nxt[C_MAX_HWMSEN_EVENT_NUM]) {
    struct criteria {
        int min;
        int max;
    };
    struct criteria self[4][3] = {
            {{50, 540}, {-540, -50}, {75, 875}},
            {{25, 270}, {-270, -25}, {38, 438}},
            {{12, 135}, {-135, -12}, {19, 219}},
            {{6, 67}, {-67, -6}, {10, 110}},
    };
    struct criteria(*ptr)[3] = &self[0]; /*choose full resolution*/
    int res = 0;

    if (((nxt[0] - prv[0]) > (*ptr)[0].max) || ((nxt[0] - prv[0]) < (*ptr)[0].min)) {
        HWMLOGE("X is over range\n");
        res = -EINVAL;
    }

    if (((nxt[1] - prv[1]) > (*ptr)[1].max) || ((nxt[1] - prv[1]) < (*ptr)[1].min)) {
        HWMLOGE("Y is over range\n");
        res = -EINVAL;
    }

    if (((nxt[2] - prv[2]) > (*ptr)[2].max) || ((nxt[2] - prv[2]) < (*ptr)[2].min)) {
        HWMLOGE("Z is over range\n");
        res = -EINVAL;
    }
    return res;
}

/*---------------------------------------------------------------------------*/
int gsensor_enable_selftest(int enable) {
    int err, fd = open(GSENSOR_ATTR_SELFTEST, O_RDWR);
    char buf[] = {enable + '0'};

    if (fd == -1) {
        HWMLOGD("open gsensor err = %s\n", strerror(errno));
        return -errno;
    }

    do {
        err = write(fd, buf, sizeof(buf));
    } while (err < 0 && errno == EINTR);

    if (err != sizeof(buf)) {
        HWMLOGD("write fails = %s\n", strerror(errno));
        err = -errno;
    } else {
        err = 0; /*no error*/
    }

    if (close(fd) == -1) {
        HWMLOGD("close fails = %s\n", strerror(errno));
        err = (err) ? (err) : (-errno);
    }
    return err;
}

/*---------------------------------------------------------------------------*/
int gsensor_self_test(int fd, int count, HwmPrivate* prv) {
    struct item {
        int raw[C_MAX_HWMSEN_EVENT_NUM];
    };
    int idx, res, x, y, z;
    struct item *pre = NULL, *nxt = NULL;
    char buf[60];
    int avg_prv[C_MAX_HWMSEN_EVENT_NUM] = {0, 0, 0};
    int avg_nxt[C_MAX_HWMSEN_EVENT_NUM] = {0, 0, 0};

    pre = malloc(sizeof(*pre) * count);
    nxt = malloc(sizeof(*nxt) * count);
    if (!pre || !nxt) {
        goto exit;
    }

    memset(pre, 0x00, sizeof(*pre) * count);
    memset(nxt, 0x00, sizeof(*nxt) * count);
    HWMLOGD("NORMAL:\n");
    for (idx = 0; idx < count; idx++) {
        res = ioctl(fd, GSENSOR_IOCTL_READ_RAW_DATA, &buf);

        if (res) {
            HWMLOGD("read data fail: %d\n", res);
            goto exit;
        }
        sscanf(buf, "%x %x %x", &x, &y, &z);
        nxt[idx].raw[0] = x;
        nxt[idx].raw[1] = y;
        nxt[idx].raw[2] = z;
        avg_nxt[0] += nxt[idx].raw[0];
        avg_nxt[1] += nxt[idx].raw[1];
        avg_nxt[2] += nxt[idx].raw[2];

        HWMLOGD("[%5d %5d %5d]\n", pre[idx].raw[0], pre[idx].raw[1], pre[idx].raw[2]);
    }
    avg_prv[0] /= count;
    avg_prv[1] /= count;
    avg_prv[2] /= count;

    /*initial setting for self test*/
    if (0 != (res = gsensor_enable_selftest(1))) {
        goto exit;
    }

    HWMLOGD("SELFTEST:\n");
    for (idx = 0; idx < count; idx++) {
        res = ioctl(fd, GSENSOR_IOCTL_READ_RAW_DATA, &buf);
        if (res) {
            HWMLOGD("read data fail: %d\n", res);
            goto exit;
        }
        sscanf(buf, "%x %x %x", &x, &y, &z);
        nxt[idx].raw[0] = x;
        nxt[idx].raw[1] = y;
        nxt[idx].raw[2] = z;
        avg_nxt[0] += nxt[idx].raw[0];
        avg_nxt[1] += nxt[idx].raw[1];
        avg_nxt[2] += nxt[idx].raw[2];
        HWMLOGD("[%5d %5d %5d]\n", nxt[idx].raw[0], nxt[idx].raw[1], nxt[idx].raw[2]);
    }
    avg_nxt[0] /= count;
    avg_nxt[1] /= count;
    avg_nxt[2] /= count;

    HWMLOGD("X: %5d - %5d = %5d \n", avg_nxt[0], avg_prv[0], avg_nxt[0] - avg_prv[0]);
    HWMLOGD("Y: %5d - %5d = %5d \n", avg_nxt[1], avg_prv[1], avg_nxt[1] - avg_prv[1]);
    HWMLOGD("Z: %5d - %5d = %5d \n", avg_nxt[2], avg_prv[2], avg_nxt[2] - avg_prv[2]);

    if (0 == gsensor_judge_selftest_result(avg_prv, avg_nxt)) {
        HWMLOGD("SELFTEST : PASS\n");
    } else {
        HWMLOGD("SELFTEST : FAIL\n");
    }
exit:
    /*restore the setting*/
    res = gsensor_enable_selftest(0);
    free(pre);
    free(nxt);
    if (prv && prv->ptr && (prv->len == 2 * sizeof(HwmData))) {
        HwmData* dat = (HwmData*)prv->ptr;
        dat[0].rx = avg_prv[0];
        dat[0].ry = avg_prv[1];
        dat[0].rz = avg_prv[1];
        dat[1].rx = avg_nxt[0];
        dat[1].ry = avg_nxt[1];
        dat[1].rz = avg_nxt[1];
    }
    return res;
}
/*---------------------------------------------------------------------------*/
//
//      Accelerometer Interface function
//
/*---------------------------------------------------------------------------*/
int gsensor_open(int* fd) {
    if (*fd <= 0) {
        *fd = open(GSENSOR_NAME, O_RDONLY);

        if (*fd < 0) {
            HWMLOGE("Couldn't find or open file sensor (%s)", strerror(errno));
            return -errno;
        }
    }
    return 0;
}
/*---------------------------------------------------------------------------*/
int gsensor_close(int fd) {
    if (fd > 0) {
        close(fd);
    }

    return 0;
}
/*---------------------------------------------------------------------------*/
int gsensor_init(int fd) {
    int err;
    unsigned long flag = 1;

    if (0 != (err = ioctl(fd, GSENSOR_IOCTL_INIT, &flag))) {
        HWMLOGE("Accelerometer init err: %d %d (%s)\n", fd, err, strerror(errno));
        return err;
    }
    return 0;
}

int gsensor_read(int fd, HwmData* dat) {
    int err;
    int x, y, z;
    char buf[64];
    if (fd < 0) {
        HWMLOGE("invalid file handle!\n");
        return -EINVAL;
    } else if (0 != (err = ioctl(fd, GSENSOR_IOCTL_READ_SENSORDATA, buf))) {
        HWMLOGE("read err: %d %d (%s)\n", fd, err, strerror(errno));
        return err;
    } else if (3 != sscanf(buf, "%x %x %x", &x, &y, &z)) {
        HWMLOGE("parsing error\n");
        return -EINVAL;
    } else {
        dat->x = (float)(x) / 1000;
        dat->y = (float)(y) / 1000;
        dat->z = (float)(z) / 1000;
        return 0;
    }
}
/*---------------------------------------------------------------------------*/
int gsensor_get_cali(int fd, HwmData* dat) {
    int err;
    struct SENSOR_DATA cali;

    if (fd < 0) {
        HWMLOGE("invalid file handle: %d\n", fd);
        return -EINVAL;
    } else if (0 != (err = ioctl(fd, GSENSOR_IOCTL_GET_CALI, &cali))) {
        HWMLOGE("get_cali err: %d\n", err);
        return err;
    } else {
        dat->x = (float)(cali.x) / 1000;
        dat->y = (float)(cali.y) / 1000;
        dat->z = (float)(cali.z) / 1000;
        HWMLOGD("[RD] %9.4f %9.4f %9.4f => %5d %5d %5d\n", dat->x, dat->y, dat->z, cali.x, cali.y,
                cali.z);
        return 0;
    }
}
/*---------------------------------------------------------------------------*/
int gsensor_set_cali(int fd, HwmData* dat) {
    int err;
    struct SENSOR_DATA cali;

    cali.x = round(dat->x * 1000);
    cali.y = round(dat->y * 1000);
    cali.z = round(dat->z * 1000);
    HWMLOGD("[WD] %9.4f %9.4f %9.4f => %5d %5d %5d\n", dat->x, dat->y, dat->z, cali.x, cali.y,
            cali.z);
    if (fd < 0) {
        HWMLOGE("invalid file handle: %d\n", fd);
        return -EINVAL;
    } else if (0 != (err = ioctl(fd, GSENSOR_IOCTL_SET_CALI, &cali))) {
        HWMLOGE("set_cali err: %d\n", err);
        return err;
    } else {
        return 0;
    }
}
/*---------------------------------------------------------------------------*/
int gsensor_rst_cali(int fd) {
    int err, flags = 0;
    if (fd < 0) {
        HWMLOGE("invalid file handle %d\n", fd);
        return -EINVAL;
    } else if (0 != (err = ioctl(fd, GSENSOR_IOCTL_CLR_CALI, &flags))) {
        HWMLOGE("rst_cali err: %d\n", err);
        return err;
    } else {
        return 0;
    }
}

/*---------------------------------------------------------------------------*/
int gsensor_read_nvram(HwmData* dat) {
#ifdef SUPPORT_SENSOR_ACCESS_NVRAM
    int file_lid = AP_CFG_RDCL_HWMON_ACC_LID;
    int rec_size;
    int rec_num;
    F_ID FileID = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
    int res, fd = FileID.iFileDesc;

    NVRAM_HWMON_ACC_STRUCT hwmonAcc = {
            {0, 0, 0},
    };

    if (fd < 0) {
        HWMLOGE("nvram open = %d\n", fd);
        return fd;
    }

    res = read(fd, &hwmonAcc, rec_size * rec_num);
    if (res < 0) {
        dat->x = dat->y = dat->z = 0.0;
        HWMLOGE("nvram read = %d(%s)\n", errno, strerror(errno));
    } else {
        dat->x = (hwmonAcc.offset[0] * LIBHWM_GRAVITY_EARTH) / LIBHWM_ACC_NVRAM_SENSITIVITY;
        dat->y = (hwmonAcc.offset[1] * LIBHWM_GRAVITY_EARTH) / LIBHWM_ACC_NVRAM_SENSITIVITY;
        dat->z = (hwmonAcc.offset[2] * LIBHWM_GRAVITY_EARTH) / LIBHWM_ACC_NVRAM_SENSITIVITY;
        HWMLOGD("[RN] %9.4f %9.4f %9.4f => %5d %5d %5d\n", dat->x, dat->y, dat->z,
                hwmonAcc.offset[0], hwmonAcc.offset[1], hwmonAcc.offset[2]);
    }

    NVM_CloseFileDesc(FileID);
#else
    dat->x = 0;
    dat->y = 0;
    dat->z = 0;
#endif
    return 0;
}
/*---------------------------------------------------------------------------*/
int gsensor_write_nvram(HwmData* dat) {
#ifdef SUPPORT_SENSOR_ACCESS_NVRAM
    int file_lid = AP_CFG_RDCL_HWMON_ACC_LID;
    int rec_size;
    int rec_num;
    bool bRet = false;
    F_ID FileID = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
    int res, fd = FileID.iFileDesc;
    NVRAM_HWMON_ACC_STRUCT hwmonAcc = {
            {0, 0, 0},
    };

    if (fd < 0) {
        HWMLOGE("nvram open = %d\n", fd);
        return fd;
    }

    hwmonAcc.offset[0] = round((dat->x * LIBHWM_ACC_NVRAM_SENSITIVITY) / LIBHWM_GRAVITY_EARTH);
    hwmonAcc.offset[1] = round((dat->y * LIBHWM_ACC_NVRAM_SENSITIVITY) / LIBHWM_GRAVITY_EARTH);
    hwmonAcc.offset[2] = round((dat->z * LIBHWM_ACC_NVRAM_SENSITIVITY) / LIBHWM_GRAVITY_EARTH);
    HWMLOGD("[WN] %9.4f %9.4f %9.4f => %5d %5d %5d\n", dat->x, dat->y, dat->z, hwmonAcc.offset[0],
            hwmonAcc.offset[1], hwmonAcc.offset[2]);
    res = write(fd, &hwmonAcc, rec_size * rec_num);
    if (res < 0) {
        HWMLOGE("nvram write = %d(%s)\n", errno, strerror(errno));
    }
    NVM_CloseFileDesc(FileID);

    // NVM_AddBackupFileNum(AP_CFG_RDCL_HWMON_ACC_LID); // risk of lost NVRAM BIN region data, when
    // lose power
    bRet = FileOp_BackupToBinRegion_All();
    sync();

    return !bRet;
#else
    HWMLOGD("gsensor [WN] %9.4f %9.4f %9.4f\n", dat->x, dat->y, dat->z);
#endif
    return 0;
}

/*---------------------------------------------------------------------------*/
int libhwm_wait_delay(int ms) {
    struct timespec req = {.tv_sec = 0, .tv_nsec = ms * 1000000};
    struct timespec rem;
    int ret = nanosleep(&req, &rem);

    while (ret) {
        if (errno == EINTR) {
            req.tv_sec = rem.tv_sec;
            req.tv_nsec = rem.tv_nsec;
            ret = nanosleep(&req, &rem);
        } else {
            perror("nanosleep");
            return errno;
        }
    }
    return 0;
}
/*---------------------------------------------------------------------------*/
long libhwm_current_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    // HWMLOGD("%ld:%ld\n", ts.tv_sec, ts.tv_nsec/1000000);
    return (long)(ts.tv_nsec / 1000000) + ts.tv_sec * 1000;
}

/*---------------------------------------------------------------------------*/
//
//      Accelerometer Implementation
//
/*---------------------------------------------------------------------------*/
typedef struct {
    long long time;
    HwmData dat;
} AccItem;

/*---------------------------------------------------------------------------*/
int gsensor_update_nvram(HwmData* dat) {
    int err;
    HwmData old, cur;

    if (0 != (err = gsensor_read_nvram(&old))) {
        return err;
    }

    cur.x = old.x + dat->x;
    cur.y = old.y + dat->y;
    cur.z = old.z + dat->z;

    if (0 != (err = gsensor_write_nvram(&cur))) {
        return err;
    }

    return 0;
}
/*---------------------------------------------------------------------------*/
int gsensor_reset_nvram() {
    int err;
    HwmData cur;

    cur.x = cur.y = cur.z = 0.0;
    if (0 != (err = gsensor_write_nvram(&cur))) {
        return err;
    }

    return 0;
}
/*----------------------------------------------------------------------------*/
int checkAccelerometerData(AccItem* list, int num, HwmData* avg, int tolerance) {
    float maxdiff = (LIBHWM_GRAVITY_EARTH * tolerance) / 100.0;
    HwmData min, max;
    char chkstr[1024];
    float diffx, diffy, diffz, varx = 0, vary = 0, varz = 0;
    int idx;
    int diverse = 0, curdiv;

    min.x = min.y = min.z = +100 * LIBHWM_GRAVITY_EARTH;
    max.x = max.y = max.z = -100 * LIBHWM_GRAVITY_EARTH;
    HWMLOGD("----------------------------------------------------------------\n");
    HWMLOGD("                         Calibration Data                       \n");
    HWMLOGD("----------------------------------------------------------------\n");
    HWMLOGD("maxdiff = %+9.4f\n", maxdiff);
    HWMLOGD("average = %+9.4f, %+9.4f %+9.4f\n", avg->x, avg->y, avg->z);
    HWMLOGD("----------------------------------------------------------------\n");
    for (idx = 0; idx < num; idx++) {
        if (max.x < list[idx].dat.x) {
            max.x = list[idx].dat.x;
        }

        if (max.y < list[idx].dat.y) {
            max.y = list[idx].dat.y;
        }

        if (max.z < list[idx].dat.z) {
            max.z = list[idx].dat.z;
        }

        if (min.x > list[idx].dat.x) {
            min.x = list[idx].dat.x;
        }

        if (min.y > list[idx].dat.y) {
            min.y = list[idx].dat.y;
        }

        if (min.z > list[idx].dat.z) {
            min.z = list[idx].dat.z;
        }

        diffx = list[idx].dat.x - avg->x;
        diffy = list[idx].dat.y - avg->y;
        diffz = list[idx].dat.z - avg->z;
        varx += diffx * diffx;
        vary += diffy * diffy;
        varz += diffz * diffz;

        curdiv = 0;
        if (ABS(diffx) > maxdiff) {
            curdiv |= DIVERSE_X;
        }

        if (ABS(diffy) > maxdiff) {
            curdiv |= DIVERSE_Y;
        }

        if (ABS(diffz) > maxdiff) {
            curdiv |= DIVERSE_Z;
        }

        if ((diffx * diffx + diffy * diffy + diffz * diffz) > maxdiff * maxdiff) {
            curdiv |= DIVERSE_XYZ;
        }
        diverse |= curdiv;

        if (curdiv) {
            snprintf(chkstr, sizeof(chkstr),
                     "=> UNSTABLE: 0x%04X, %+9.4f(%+5.2f), %+9.4f(%+5.2f), %+9.4f(%+5.2f), "
                     "%+9.4f(%+5.2f)",
                     curdiv, diffx, diffx / LIBHWM_GRAVITY_EARTH, diffy,
                     diffy / LIBHWM_GRAVITY_EARTH, diffz, diffz / LIBHWM_GRAVITY_EARTH,
                     sqrt(diffx * diffx + diffy * diffy + diffz * diffz),
                     sqrt(diffx * diffx + diffy * diffy + diffz * diffz) / LIBHWM_GRAVITY_EARTH);
        } else {
            snprintf(chkstr, sizeof(chkstr), " ");
        }

        HWMLOGD("[%8lld] (%+9.4f, %+9.4f, %+9.4f) %s\n", list[idx].time / 1000000, list[idx].dat.x,
                list[idx].dat.y, list[idx].dat.z, chkstr);
    }

    varx = sqrt(varx / num);
    vary = sqrt(vary / num);
    varz = sqrt(varz / num);

    HWMLOGD("----------------------------------------------------------------\n");
    HWMLOGD("X-Axis: min/avg/max = (%+9.4f, %+9.4f, %+9.4f), diverse = %+9.4f ~ %+9.4f, std = "
            "%9.4f\n",
            min.x, avg->x, max.x, (min.x - avg->x) / LIBHWM_GRAVITY_EARTH,
            (max.x - avg->x) / LIBHWM_GRAVITY_EARTH, varx);
    HWMLOGD("Y-Axis: min/avg/max = (%+9.4f, %+9.4f, %+9.4f), diverse = %+9.4f ~ %+9.4f, std = "
            "%9.4f\n",
            min.y, avg->y, max.y, (min.y - avg->y) / LIBHWM_GRAVITY_EARTH,
            (max.y - avg->y) / LIBHWM_GRAVITY_EARTH, vary);
    HWMLOGD("Z-Axis: min/avg/max = (%+9.4f, %+9.4f, %+9.4f), diverse = %+9.4f ~ %+9.4f, std = "
            "%9.4f\n",
            min.z, avg->z, max.z, (min.z - avg->z) / LIBHWM_GRAVITY_EARTH,
            (max.z - avg->z) / LIBHWM_GRAVITY_EARTH, varz);
    HWMLOGD("----------------------------------------------------------------\n");
    if (diverse) {
        return -EINVAL;
    }

    return 0;
}
/*---------------------------------------------------------------------------*/
// Parse event and return event type
/*---------------------------------------------------------------------------*/
int gsensor_poll_data(int fd, int period, int count) {
    int64_t nt;
    struct timespec time;
    int err, num = 0;
    AccItem* item = NULL;
    HwmData avg, dat;

    avg.x = avg.y = avg.z = 0.0;
    if (fd < 0) {
        return -EINVAL;
    }
    if (NULL == (item = calloc(count, sizeof(*item)))) {
        return -ENOMEM;
    }

    while (0 == (err = gsensor_read(fd, &dat))) {
        clock_gettime(CLOCK_MONOTONIC, &time);
        nt = time.tv_sec * 1000000000LL + time.tv_nsec;
        item[num].dat.x = dat.x;
        item[num].dat.y = dat.y;
        item[num].dat.z = dat.z;
        item[num].time = nt;
        avg.x += dat.x;
        avg.y += dat.y;
        avg.z += dat.z;

        if (++num >= count) {
            break;
        }
        libhwm_wait_delay(period);
    }

    avg.x /= count;
    avg.y /= count;
    avg.z /= count;
    checkAccelerometerData(item, count, &avg, 0.0);

    free(item);
    return err;
}

int getGravityStandard(float in[C_MAX_HWMSEN_EVENT_NUM], HwmData* out) {
    out->x = in[0] / LIBHWM_GRAVITY_EARTH;
    out->y = in[1] / LIBHWM_GRAVITY_EARTH;
    out->z = in[2] / LIBHWM_GRAVITY_EARTH;

    HWMLOGD("%9.4f %9.4f %9.4f => %9.4f %9.4f %9.4f\n", in[0], in[1], in[2], out->x, out->y,
            out->z);
    return 0;
}

/*---------------------------------------------------------------------------*/
int calculateStandardCalibration(HwmData* avg, HwmData* cali) {
    HwmData golden;
    golden.x = golden.y = 0.0;
    golden.z = LIBHWM_GRAVITY_EARTH;

    cali->x = golden.x - avg->x;
    cali->y = golden.y - avg->y;
    cali->z = golden.z - avg->z;
    HWMLOGD("%s (%9.4f, %9.4f, %9.4f)\n", __func__, cali->x, cali->y, cali->z);
    return 0;
}

/*---------------------------------------------------------------------------*/
int gsensor_calibration(int fd, int period, int count, int tolerance, int trace, HwmData *cali) {
    int err = 0, num = 0;
    AccItem* item = NULL;
    HwmData avg, dat;
    int64_t nt;
    struct timespec time;

    avg.x = avg.y = avg.z = 0.0;
    if (fd < 0) {
        return -EINVAL;
    }
    if (NULL == (item = calloc(count, sizeof(*item)))) {
        return -ENOMEM;
    }

    while (num < count) {
        /* read the next event */
        err = gsensor_read(fd, &dat);

        if (err) {
            HWMLOGE("read data fail: %d\n", err);
            goto exit;
        } else {
            clock_gettime(CLOCK_MONOTONIC, &time);
            nt = time.tv_sec * 1000000000LL + time.tv_nsec;
            item[num].dat.x = dat.x;
            item[num].dat.y = dat.y;
            item[num].dat.z = dat.z;
            item[num].time = nt;
            avg.x += item[num].dat.x;
            avg.y += item[num].dat.y;
            avg.z += item[num].dat.z;
        }

        num++;
        libhwm_wait_delay(period);
    }

    /*calculate average*/
    avg.x /= count;
    avg.y /= count;
    avg.z /= count;

    if (0 != (err = checkAccelerometerData(item, count, &avg, tolerance))) {
        HWMLOGE("check accelerometer fail\n");
    } else if (0 != (err = calculateStandardCalibration(&avg, cali))) {
        HWMLOGE("calculate standard calibration fail\n");
    }
exit:
    free(item);
    return err;
}

/* gyroscope calibration */
int gyroscope_open(int* fd) {
    if (*fd <= 0) {
        *fd = open(GYROSCOPE_NAME, O_RDONLY);

        if (*fd < 0) {
            HWMLOGE("Couldn't find or open file sensor (%s)", strerror(errno));
            return -errno;
        }
    }
    return 0;
}
/*---------------------------------------------------------------------------*/
int gyroscope_close(int fd) {
    if (fd >= 0) {
        close(fd);
    }

    return 0;
}

int gyroscope_init(int fd) {
    int err;
    unsigned long flag = 0;

    if (0 != (err = ioctl(fd, GYROSCOPE_IOCTL_INIT, &flag))) {
        HWMLOGE("Gyroscope init err: %d %d (%s)\n", fd, err, strerror(errno));
        return err;
    }
    return 0;
}
/*---------------------------------------------------------------------------*/
int gyroscope_read(int fd, HwmData* dat) {
    int err;
    int x, y, z;
    char buf[64];
    if (fd < 0) {
        HWMLOGE("invalid file handle!\n");
        return -EINVAL;
    } else if (0 != (err = ioctl(fd, GYROSCOPE_IOCTL_READ_SENSORDATA, buf))) {
        HWMLOGE("read err: %d %d (%s)\n", fd, err, strerror(errno));
        return err;
    } else if (3 != sscanf(buf, "%x %x %x", &x, &y, &z)) {
        HWMLOGE("parsing error\n");
        return -EINVAL;
    } else {
        dat->x = (float)(x);
        dat->y = (float)(y);
        dat->z = (float)(z);
        return 0;
    }
}
/*---------------------------------------------------------------------------*/
int gyroscope_get_cali(int fd, HwmData* dat) {
    int err;
    struct SENSOR_DATA cali;

    if (fd < 0) {
        HWMLOGE("invalid file handle: %d\n", fd);
        return -EINVAL;
    } else if (0 != (err = ioctl(fd, GYROSCOPE_IOCTL_GET_CALI, &cali))) {
        HWMLOGE("get_cali err: %d\n", err);
        return err;
    } else {
        dat->x = (float)(cali.x);
        dat->y = (float)(cali.y);
        dat->z = (float)(cali.z);
        HWMLOGD("[RD] %9.4f %9.4f %9.4f => %5d %5d %5d\n", dat->x, dat->y, dat->z, cali.x, cali.y,
                cali.z);
        return 0;
    }
}
/*---------------------------------------------------------------------------*/
int gyroscope_set_cali(int fd, HwmData* dat) {
    int err;
    struct SENSOR_DATA cali;

    cali.x = round(dat->x);
    cali.y = round(dat->y);
    cali.z = round(dat->z);
    HWMLOGD("[WD] %9.4f %9.4f %9.4f => %5d %5d %5d\n", dat->x, dat->y, dat->z, cali.x, cali.y,
            cali.z);
    if (fd < 0) {
        HWMLOGE("invalid file handle: %d\n", fd);
        return -EINVAL;
    } else if (0 != (err = ioctl(fd, GYROSCOPE_IOCTL_SET_CALI, &cali))) {
        HWMLOGE("set_cali err: %d\n", err);
        return err;
    } else {
        return 0;
    }
}
/*---------------------------------------------------------------------------*/
int gyroscope_rst_cali(int fd) {
    int err, flags = 0;
    if (fd < 0) {
        HWMLOGE("invalid file handle %d\n", fd);
        return -EINVAL;
    } else if (0 != (err = ioctl(fd, GYROSCOPE_IOCTL_CLR_CALI, &flags))) {
        HWMLOGE("rst_cali err: %d\n", err);
        return err;
    } else {
        return 0;
    }
}

/*---------------------------------------------------------------------------*/
int gyroscope_read_nvram(HwmData* dat) {
#ifdef SUPPORT_SENSOR_ACCESS_NVRAM
    int file_lid = AP_CFG_RDCL_HWMON_GYRO_LID;
    int rec_size;
    int rec_num;
    F_ID FileID = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
    int res, fd = FileID.iFileDesc;
    NVRAM_HWMON_GYRO_STRUCT hwmonGyro = {
            {0, 0, 0},
    };

    if (fd < 0) {
        HWMLOGE("nvram open = %d\n", fd);
        return fd;
    }

    res = read(fd, &hwmonGyro, rec_size * rec_num);
    if (res < 0) {
        dat->x = dat->y = dat->z = 0.0;
        HWMLOGE("nvram read = %d(%s)\n", errno, strerror(errno));
    } else {
        dat->x = hwmonGyro.offset[0] / LIBHWM_GYRO_NVRAM_SENSITIVITY;
        dat->y = hwmonGyro.offset[1] / LIBHWM_GYRO_NVRAM_SENSITIVITY;
        dat->z = hwmonGyro.offset[2] / LIBHWM_GYRO_NVRAM_SENSITIVITY;
        HWMLOGD("[RN] %9.4f %9.4f %9.4f => %5d %5d %5d\n", dat->x, dat->y, dat->z,
                hwmonGyro.offset[0], hwmonGyro.offset[1], hwmonGyro.offset[2]);
    }

    NVM_CloseFileDesc(FileID);
#else
    dat->x = 0;
    dat->y = 0;
    dat->z = 0;
#endif
    return 0;
}
/*---------------------------------------------------------------------------*/
int gyroscope_write_nvram(HwmData* dat) {
#ifdef SUPPORT_SENSOR_ACCESS_NVRAM
    int file_lid = AP_CFG_RDCL_HWMON_GYRO_LID;
    int rec_size;
    int rec_num;
    bool bRet = false;
    F_ID FileID = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
    int res, fd = FileID.iFileDesc;
    NVRAM_HWMON_GYRO_STRUCT hwmonGyro = {
            {0, 0, 0},
    };

    if (fd < 0) {
        HWMLOGE("nvram open = %d\n", fd);
        return fd;
    }

    hwmonGyro.offset[0] = round(dat->x * LIBHWM_GYRO_NVRAM_SENSITIVITY);
    hwmonGyro.offset[1] = round(dat->y * LIBHWM_GYRO_NVRAM_SENSITIVITY);
    hwmonGyro.offset[2] = round(dat->z * LIBHWM_GYRO_NVRAM_SENSITIVITY);
    HWMLOGD("[WN] %9.4f %9.4f %9.4f => %5d %5d %5d\n", dat->x, dat->y, dat->z, hwmonGyro.offset[0],
            hwmonGyro.offset[1], hwmonGyro.offset[2]);
    res = write(fd, &hwmonGyro, rec_size * rec_num);
    if (res < 0) {
        HWMLOGE("nvram write = %d(%s)\n", errno, strerror(errno));
    }
    NVM_CloseFileDesc(FileID);
    // NVM_AddBackupFileNum(AP_CFG_RDCL_HWMON_GYRO_LID);
    bRet = FileOp_BackupToBinRegion_All();
    sync();
    return !bRet;
#else
    HWMLOGD("gyro [WN] %9.4f %9.4f %9.4f\n", dat->x, dat->y, dat->z);
#endif
    return 0;
}

/*----------------------------------------------------------------------------*/
int checkGyroscopeData(AccItem* list, int num, HwmData* avg, int tolerance) {
    float maxdiff = tolerance;
    HwmData min, max;
    char chkstr[1024];
    float diffx, diffy, diffz, varx = 0, vary = 0, varz = 0;
    int idx;
    int diverse = 0, curdiv;

    min.x = min.y = min.z = +10 * tolerance;
    max.x = max.y = max.z = -10 * tolerance;
    HWMLOGD("----------------------------------------------------------------\n");
    HWMLOGD("                         Calibration Data                       \n");
    HWMLOGD("----------------------------------------------------------------\n");
    HWMLOGD("maxdiff = %+9.4f\n", maxdiff);
    HWMLOGD("average = %+9.4f, %+9.4f %+9.4f\n", avg->x, avg->y, avg->z);
    HWMLOGD("----------------------------------------------------------------\n");
    for (idx = 0; idx < num; idx++) {
        if (max.x < list[idx].dat.x) {
            max.x = list[idx].dat.x;
        }

        if (max.y < list[idx].dat.y) {
            max.y = list[idx].dat.y;
        }

        if (max.z < list[idx].dat.z) {
            max.z = list[idx].dat.z;
        }

        if (min.x > list[idx].dat.x) {
            min.x = list[idx].dat.x;
        }

        if (min.y > list[idx].dat.y) {
            min.y = list[idx].dat.y;
        }

        if (min.z > list[idx].dat.z) {
            min.z = list[idx].dat.z;
        }

        diffx = list[idx].dat.x - avg->x;
        diffy = list[idx].dat.y - avg->y;
        diffz = list[idx].dat.z - avg->z;
        varx += diffx * diffx;
        vary += diffy * diffy;
        varz += diffz * diffz;

        curdiv = 0;
        if (ABS(diffx) > maxdiff) {
            curdiv |= DIVERSE_X;
        }

        if (ABS(diffy) > maxdiff) {
            curdiv |= DIVERSE_Y;
        }

        if (ABS(diffz) > maxdiff) {
            curdiv |= DIVERSE_Z;
        }

        if ((diffx * diffx + diffy * diffy + diffz * diffz) > maxdiff * maxdiff) {
            curdiv |= DIVERSE_XYZ;
        }
        diverse |= curdiv;

        if (curdiv) {
            snprintf(chkstr, sizeof(chkstr),
                     "=> UNSTABLE: 0x%04X, %+9.4f(%+5.2f), %+9.4f(%+5.2f), %+9.4f(%+5.2f), "
                     "%+9.4f(%+5.2f)",
                     curdiv, diffx, diffx, diffy, diffy, diffz, diffz,
                     sqrt(diffx * diffx + diffy * diffy + diffz * diffz),
                     sqrt(diffx * diffx + diffy * diffy + diffz * diffz));
        } else {
            snprintf(chkstr, sizeof(chkstr), " ");
        }

        HWMLOGD("[%8lld] (%+9.4f, %+9.4f, %+9.4f) %s\n", list[idx].time / 1000000, list[idx].dat.x,
                list[idx].dat.y, list[idx].dat.z, chkstr);
    }

    varx = sqrt(varx / num);
    vary = sqrt(vary / num);
    varz = sqrt(varz / num);

    HWMLOGD("----------------------------------------------------------------\n");
    HWMLOGD("X-Axis: min/avg/max = (%+9.4f, %+9.4f, %+9.4f), diverse = %+9.4f ~ %+9.4f, std = "
            "%9.4f\n",
            min.x, avg->x, max.x, (min.x - avg->x), (max.x - avg->x), varx);
    HWMLOGD("Y-Axis: min/avg/max = (%+9.4f, %+9.4f, %+9.4f), diverse = %+9.4f ~ %+9.4f, std = "
            "%9.4f\n",
            min.y, avg->y, max.y, (min.y - avg->y), (max.y - avg->y), vary);
    HWMLOGD("Z-Axis: min/avg/max = (%+9.4f, %+9.4f, %+9.4f), diverse = %+9.4f ~ %+9.4f, std = "
            "%9.4f\n",
            min.z, avg->z, max.z, (min.z - avg->z), (max.z - avg->z), varz);
    HWMLOGD("----------------------------------------------------------------\n");
    if (diverse) {
        return -EINVAL;
    }

    return 0;
}
/*---------------------------------------------------------------------------*/
int do_gsensor_calibration(int tolerance) {
    int gs_fd = -1;
    int err = 0;
    HwmData cali;

    err = gsensor_open(&gs_fd);
    if (err < 0) {
        HWMLOGE("gsensor_open fail in do_gsensor_calibration");
        return 0;
    }
    //******************************
    if ((err = gsensor_calibration(gs_fd, 50, 20, tolerance * 10, 0, &cali)) != 0) {
        HWMLOGE("calibrate acc: %d\n", err);
        gsensor_close(gs_fd);
        return 0;
    } else if ((err = gsensor_set_cali(gs_fd, &cali)) != 0) {
        HWMLOGE("set calibration fail: (%s) %d\n", strerror(errno), err);
        gsensor_close(gs_fd);
        return 0;
    } else if ((err = gsensor_get_cali(gs_fd, &cali)) != 0) {
        HWMLOGE("get calibration fail: (%s) %d\n", strerror(errno), err);
        gsensor_close(gs_fd);
        return 0;
    } else if ((err = gsensor_write_nvram(&cali)) != 0) {
        HWMLOGE("write nvram fail: (%s) %d\n", strerror(errno), err);
        gsensor_close(gs_fd);
        return 0;
    }

    //******************************

    err = gsensor_close(gs_fd);
    if (err < 0) {
        HWMLOGE("gsensor_close fail in do_gsensor_calibration");
        return 0;
    }

    return 1;
}

int get_gsensor_calibration(float* x, float* y, float* z) {
    HwmData cali;
    int err = 0;
    err = gsensor_read_nvram(&cali);
    if (err < 0) {
        HWMLOGE("gsensor_read_nvram fail in get_gsensor_calibration");
        return 0;
    }

    *x = cali.x;
    *y = cali.y;
    *z = cali.z;

    return 1;
}

int clear_gsensor_calibration(void) {
    int err;
    int gs_fd = -1;
    HwmData cali_nvram;

    cali_nvram.x = 0;
    cali_nvram.y = 0;
    cali_nvram.z = 0;

    err = gsensor_open(&gs_fd);
    if (err < 0) {
        HWMLOGE("gsensor_open fail in clear_gsensor_calibration");
        return 0;
    }
    //******************************
    err = gsensor_rst_cali(gs_fd);
    if (err) {
        HWMLOGE("rst calibration: %d\n", err);
        gsensor_close(gs_fd);
        return 0;
    } else if ((err = gsensor_write_nvram(&cali_nvram)) != 0) {
        HWMLOGE("write nvram: %d\n", err);
        gsensor_close(gs_fd);
        return 0;
    }
    //******************************

    err = gsensor_close(gs_fd);
    if (err < 0) {
        HWMLOGE("gsensor_close fail in clear_gsensor_calibration");
        return 0;
    }
    return 1;
}
/*---------------------------------------------------------------------------*/
int do_gyroscope_calibration(int tolerance) {
    int gyro_fd = -1;
    int err = 0;
    HwmData cali;

    err = gyroscope_open(&gyro_fd);
    if (err < 0) {
        HWMLOGE("gsensor_open fail in do_gyroscope_calibration");
        return 0;
    }
    //******************************
    if ((err = gyroscope_calibration(gyro_fd, 50, 20, tolerance * 100, 0, &cali)) != 0) {
        HWMLOGE("calibrate acc: %d\n", err);
        gyroscope_close(gyro_fd);
        return 0;
    } else if ((err = gyroscope_set_cali(gyro_fd, &cali)) != 0) {
        HWMLOGE("set calibration fail: (%s) %d\n", strerror(errno), err);
        gyroscope_close(gyro_fd);
        return 0;
    } else if ((err = gyroscope_get_cali(gyro_fd, &cali)) != 0) {
        HWMLOGE("get calibration fail: (%s) %d\n", strerror(errno), err);
        gyroscope_close(gyro_fd);
        return 0;
    } else if ((err = gyroscope_write_nvram(&cali)) != 0) {
        HWMLOGE("write nvram fail: (%s) %d\n", strerror(errno), err);
        gyroscope_close(gyro_fd);
        return 0;
    }

    //******************************

    err = gyroscope_close(gyro_fd);
    if (err < 0) {
        HWMLOGE("gsensor_close fail in do_gyroscope_calibration");
        return 0;
    }

    return 1;
}
int get_gyroscope_calibration(float* x, float* y, float* z) {
    HwmData cali;
    int err = 0;
    err = gyroscope_read_nvram(&cali);
    if (err < 0) {
        HWMLOGE("gyroscope_read_nvram fail in get_gyroscope_calibration");
        return 0;
    }

    *x = cali.x;
    *y = cali.y;
    *z = cali.z;

    return 1;
}

int clear_gyroscope_calibration(void) {
    int err;
    int gyro_fd = -1;
    HwmData cali_nvram;

    cali_nvram.x = 0;
    cali_nvram.y = 0;
    cali_nvram.z = 0;

    err = gyroscope_open(&gyro_fd);
    if (err < 0) {
        HWMLOGE("gyroscope_open fail in clear_gyroscope_calibration");
        return 0;
    }
    //******************************
    err = gyroscope_rst_cali(gyro_fd);
    if (err) {
        HWMLOGE("rst calibration: %d\n", err);
        gyroscope_close(gyro_fd);
        return 0;
    } else if ((err = gyroscope_write_nvram(&cali_nvram)) != 0) {
        HWMLOGE("write nvram: %d\n", err);
        gyroscope_close(gyro_fd);
        return 0;
    }
    //******************************

    err = gyroscope_close(gyro_fd);
    if (err < 0) {
        HWMLOGE("gyroscope_close fail in clear_gyroscope_calibration");
        return 0;
    }
    return 1;
}