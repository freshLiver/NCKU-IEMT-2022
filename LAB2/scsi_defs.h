#ifndef __LAB2_SCSI_DEFS_H__
#define __LAB2_SCSI_DEFS_H__

#include <stdio.h>
#include <string.h> /* for memset */
#include <stdbool.h>

#include <scsi/sg.h>
#include <sys/ioctl.h>

#define print(fmt, ...)   printf(fmt "\n", ##__VA_ARGS__)
#define error(fmt, ...)   print("Error at %s:%d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#define warning(fmt, ...) print("Warning at %s:%d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

/* -------------------------------------------------------------------------- */
/*                                request utils                               */
/* -------------------------------------------------------------------------- */

typedef struct
{
    __uint8_t mode;
    __uint32_t disk_id;
    __uint8_t data;
    __uint64_t address;
    __uint32_t count;
} req_info_t, *req_info_p;

#define REQ_INFO_MODE_UNKNOWN 0x00
#define REQ_INFO_MODE_INQUIRY 0x12
#define REQ_INFO_MODE_READ16  0x88
#define REQ_INFO_MODE_WRITE16 0x8A

static const char *REQ_MODE_TABLE[256] = {
    [REQ_INFO_MODE_UNKNOWN] = "UNKNOWN",
    [REQ_INFO_MODE_INQUIRY] = "INQUIRY",
    [REQ_INFO_MODE_READ16]  = "READ16",
    [REQ_INFO_MODE_WRITE16] = "WRITE16",
};

/* -------------------------------------------------------------------------- */
/*                               sg_io_hdr utils                              */
/* -------------------------------------------------------------------------- */

#define SSD_WORD_SIZE             32
#define SECTOR_SIZE_BYTES         512
#define SECTOR_SIZE_WORDS         (SECTOR_SIZE_BYTES / SSD_WORD_SIZE)
#define SCSI_DATA_BUFFER_SIZE_MIN 64
#define SCSI_SENSE_BUFFER_SIZE    255
unsigned char SCSI_SENSE_BUFFER[SCSI_SENSE_BUFFER_SIZE];

#define RETURN_AT_INVALID_SG_HDR(phdr)                                                             \
    {                                                                                              \
        if (!(phdr))                                                                               \
        {                                                                                          \
            error("Invalid sg_io_hdr pointer!");                                                   \
            return -1;                                                                             \
        }                                                                                          \
    }

#define INIT_SG_HDR(phdr)                                                                          \
    {                                                                                              \
        RETURN_AT_INVALID_SG_HDR((phdr));                                                          \
        memset((phdr), 0, sizeof(sg_io_hdr_t));     /* reset handler */                            \
        (phdr)->interface_id = 'S';                 /* always be 'S' */                            \
        (phdr)->flags        = SG_FLAG_LUN_INHIBIT; /* TODO: put the LUN to 2nd byte of cdb */     \
        print("INIT_SG_HDR DONE!");                                                                \
    }

#define SET_XFER_DATA(phdr, data, len)                                                             \
    {                                                                                              \
        RETURN_AT_INVALID_SG_HDR((phdr));                                                          \
        (phdr)->dxferp    = data; /* pointer to data buffer */                                     \
        (phdr)->dxfer_len = len;  /* max data length */                                            \
        print("SET_XFER_DATA DONE!");                                                              \
    }

#define SET_SENSE_DATA(phdr, data, len)                                                            \
    {                                                                                              \
        RETURN_AT_INVALID_SG_HDR((phdr));                                                          \
        (phdr)->sbp       = data; /* pointer to sensor data buffer */                              \
        (phdr)->mx_sb_len = len;  /* max sensor data data length */                                \
        print("SET_SENSE_DATA DONE!");                                                             \
    }

#define SHOW_BUFFER_CONTENT(buf, len)                                                              \
    {                                                                                              \
        for (size_t i = 0; i < (len); ++i)                                                         \
            putchar((buf)[i]);                                                                     \
        putchar('\n');                                                                             \
    }

void show_hdr_outputs(sg_io_hdr_t *phdr)
{
    print("Show handler output:");
    print("\tstatus:%d", phdr->status);
    print("\tmasked_status:%d", phdr->masked_status);
    print("\tmsg_status:%d", phdr->msg_status);
    print("\tsb_len_wr:%d", phdr->sb_len_wr);
    print("\thost_status:%d", phdr->host_status);
    print("\tdriver_status:%d", phdr->driver_status);
    print("\tresid:%d", phdr->resid);
    print("\tduration:%d", phdr->duration);
    print("\tinfo:%d", phdr->info);
}

#endif /* __LAB2_SCSI_DEFS_H__ */