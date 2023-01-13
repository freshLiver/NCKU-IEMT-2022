#ifndef __LAB2_SCSI_WRITE_H__
#define __LAB2_SCSI_WRITE_H__

#include "scsi_defs.h"

#define _BSD_SOURCE
#include <endian.h>

void show_write_result(req_info_p req, sg_io_hdr_t *phdr)
{
    //
    print("Write 0x%.2x to first %u sectors of LBA %lu.", req->data, req->count, req->address);
}

int prepare_write16(req_info_p req, sg_io_hdr_t *phdr, unsigned char *cdb)
{
    /* setup CDB */
    cdb[0] = REQ_INFO_MODE_WRITE16;
    /**
     * CDB[1] === FLAGS
     *
     *  7              5        4        3             2             1         0
     *  | WRPROTECT(3) | DPO(1) | FUA(1) | Reserved(1) | Obsolete(1) | DLD2(1) |
     *
     * - WRPROTECT: write protection info before returning status
     * - DPO (Disable Page Out): 1 means don't retain target block in cache
     * - FUA (Force Unit Access): 1 to pass over volatile cache (force stable storage)
     * - DLD2 (Dration Limit Descriptor):
     */
    cdb[1] = (0x0 << 5) + (1 << 4) + (1 << 3) + (0 << 2) + (0 << 1) + 0;
    /* CDB[2] ~ CDB[9] === LBA (Big-Endian) */
    *(__uint64_t *)(&cdb[2]) = htobe64(req->address);
    /* CDB[10] ~ CDB[13] === length */
    *(__uint32_t *)(&cdb[10]) = htobe32(req->count);
    /* CDB[14] === DLD + GROUP NUMBER */
    cdb[14] = 0;
    /* CDB[15] === control */
    cdb[15] = 0;

    /* setup handler */
    phdr->dxfer_direction = SG_DXFER_TO_DEV;
    phdr->cmdp            = cdb;
    phdr->cmd_len         = 16;

    /* fill buffer */
    __uint8_t *buffer = (__uint8_t *)phdr->dxferp;
    for (size_t iByte = 0; iByte < (SECTOR_SIZE_BYTES * req->count); ++iByte)
        buffer[iByte] = req->data;
    return 0;
}

#undef _BSD_SOURCE
#endif /* __LAB2_SCSI_WRITE_H__ */