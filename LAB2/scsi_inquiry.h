#ifndef __LAB2_SCSI_INQUIRY_H__
#define __LAB2_SCSI_INQUIRY_H__

#include "scsi_defs.h"

void show_inquiry_result(sg_io_hdr_t *phdr)
{
    unsigned char *buffer = (unsigned char *)phdr->dxferp;
    print("%-20s: %.08s", "Vendor", &buffer[8]);
    print("%-20s: %.16s", "Product ID", &buffer[16]);
    print("%-20s: %.04s", "Product Version", &buffer[32]);
}

int prepare_inquiry(req_info_p req, sg_io_hdr_t *phdr, unsigned char *cdb, bool evpd, int page_code)
{
    /* set Command Description Block content */
    cdb[0] = REQ_INFO_MODE_INQUIRY;
    cdb[1] = evpd & 0x1;
    cdb[2] = page_code & 0xFF;
    cdb[3] = 0;
    cdb[4] = 0xFF;
    cdb[5] = 0;

    /* setup hdr */
    phdr->dxfer_direction = SG_DXFER_FROM_DEV; /* recv info from device */
    phdr->cmdp            = cdb;               /* CDB address */
    phdr->cmd_len         = 6;                 /* 6 bytes for inquiry */
    return 0;
}

#endif /* __LAB2_SCSI_INQUIRY_H__ */