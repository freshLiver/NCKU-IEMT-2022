
#include <stdlib.h> /* for system */
#include <fcntl.h>  /* for open */
#include <unistd.h> /* for close */
#include <math.h>   /* for max */

#include "scsi_inquiry.h"
#include "scsi_read.h"
#include "scsi_write.h"

#define CHECK_DURATION_SECS 10

int parse_arg(int argc, char *argv[], req_info_p req)
{
    /* reset request */
    memset(req, 0, sizeof(req_info_t));

    /* parse args */
    for (int iArg = 1; iArg < argc; ++iArg)
    {
        char *arg = argv[iArg];
        if (!strcmp(arg, "--read"))
        {
            req->mode = REQ_INFO_MODE_READ16;
        }
        else if (!strcmp(arg, "--write"))
        {
            req->mode = REQ_INFO_MODE_WRITE16;
        }
        else if (!strcmp(arg, "--inquiry"))
        {
            req->mode = REQ_INFO_MODE_INQUIRY;
        }
        else if (!strcmp(arg, "--disk"))
        {
            req->disk_id = atoi(argv[++iArg]);
        }
        else if (!strcmp(arg, "--lba"))
        {
            req->address = atoi(argv[++iArg]);
        }
        else if (!strcmp(arg, "--sector_cnt"))
        {
            req->count = atoi(argv[++iArg]);
        }
        else if (!strcmp(arg, "--data"))
        {
            // reset current data
            req->data = 0;

            // update
            char *pattern = argv[++iArg];
            for (int iChar = 0, data = 0; iChar < 2 && pattern[iChar]; ++iChar)
            {
                if (('0' <= pattern[iChar]) && (pattern[iChar] <= '9'))
                {
                    req->data |= (pattern[iChar] - '0') << (iChar ? 0 : 4);
                }
                else if (('a' <= (pattern[iChar] | 0x20)) && ((pattern[iChar] | 0x20) <= 'f'))
                {
                    req->data |= ((pattern[iChar] | 0x20) - 'a' + 10) << (iChar ? 0 : 4);
                }
                else
                {
                    error("Unexpected data pattern: %c", pattern[iChar]);
                    return -1;
                }
            }
        }
        else
        {
            error("Unexpected argument: %s", arg);
            return -1;
        }
    }

    print("Request Summary:");
    print("\tDisk   : /dev/sg%u", req->disk_id);
    print("\tMode   : %s", REQ_MODE_TABLE[req->mode]);
    print("\tData   : 0x%.2x", req->data);
    print("\tAddress: %lu", req->address);
    print("\tLength : %u", req->count);
    return 0;
}

int exec_request(req_info_p req, sg_io_hdr_t *phdr)
{
    char dev_path[20] = "/dev/sg";
    snprintf(dev_path + strlen(dev_path), 10, "%u", req->disk_id);

    print("Check your device path (%s):\n", dev_path);
    system("ls -l /dev/sg*");
    print("\nStart after %d secs, check your device... (^C to exit)", CHECK_DURATION_SECS);
    sleep(CHECK_DURATION_SECS);

    // try to open target device
    int dev_fd = open(dev_path, O_RDWR);
    if (dev_fd <= 0)
    {
        error("Unable to open device %s", dev_path);
        return -1;
    }

    /* init buffers */
    __uint32_t data_buffer_size = req->count * SECTOR_SIZE_BYTES;
    if (data_buffer_size < SCSI_DATA_BUFFER_SIZE_MIN)
    {
        warning("Data buffer size not specified or too small, used default size.");
        data_buffer_size = SCSI_DATA_BUFFER_SIZE_MIN;
    }

    unsigned char data_buffer[data_buffer_size];
    memset(data_buffer, 0, data_buffer_size);
    SET_XFER_DATA(phdr, data_buffer, data_buffer_size);

    SET_SENSE_DATA(phdr, SCSI_SENSE_BUFFER, SCSI_SENSE_BUFFER_SIZE);

    /* prepare sg handler */
    unsigned char cdb[16] = {0};
    switch (req->mode)
    {
    case REQ_INFO_MODE_INQUIRY:
        if (prepare_inquiry(req, phdr, cdb, false, 0))
            return -1;
        break;
    case REQ_INFO_MODE_READ16:
        if (prepare_read16(req, phdr, cdb))
            return -1;
        break;
    case REQ_INFO_MODE_WRITE16:
        if (prepare_write16(req, phdr, cdb))
            return -1;
        break;
    default:
        error("Unsupported request mode: %x", req->mode);
        break;
    }

    /* send request */
    if (ioctl(dev_fd, SG_IO, phdr))
    {
        error("ioctl failed, status = %x", phdr->status);
        close(dev_fd);
        return -1;
    }

    print("ioctl succeed, SCSI status code: %x", phdr->status);
    if (phdr->status)
    {
        error("Returned status %x, sense buffer content:", phdr->status);
        SHOW_BUFFER_CONTENT(phdr->sbp, phdr->mx_sb_len);
    }
    else
        switch (req->mode)
        {
        case REQ_INFO_MODE_INQUIRY:
            show_inquiry_result(phdr);
            break;
        case REQ_INFO_MODE_READ16:
            show_read_result(req, phdr);
            break;
        case REQ_INFO_MODE_WRITE16:
            show_write_result(req, phdr);
            break;
        default:
            error("Unsupported request mode: %x", req->mode);
            break;
        }
    close(dev_fd);
    print("Done");
    return 0;
}

int main(int argc, char *argv[])
{
    req_info_t req;
    sg_io_hdr_t hdr;
    INIT_SG_HDR(&hdr);

    if (parse_arg(argc, argv, &req))
        return -1;
    if (exec_request(&req, &hdr))
        return -1;
    return 0;
}
