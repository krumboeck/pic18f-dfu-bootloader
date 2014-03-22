/*
 * PIC18F DFU Bootloader
 *
 * Author: Bernd Krumböck <krumboeck@universalnet.at>
 *         Based on LeafLabs LLC.'s Maple Bootloader
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * DFU status
 */
typedef struct {
	u8 bStatus;
	u8 bwPollTimeout0;
	u8 bwPollTimeout1;
	u8 bwPollTimeout2;
	u8 bState;
	u8 iString;
} DFU_Status;

typedef enum {
	BEGIN, MIDDLE, END, INIT
} DFU_OP_State;

/*** DFU bRequest Values ******/
/* bmRequestType, wValue, wIndex, wLength, Data */
#define DFU_DETACH 0x00 /* 0x21, wTimeout, Interface, Zero, None */
#define DFU_DNLOAD 0x01 /* 0x21, wBlockNum, Interface, Length, Firmware */
#define DFU_UPLOAD 0x02 /* 0xA1, Zero, Interface, Length, Firmware */
#define DFU_GETSTATUS 0x03 /* 0xA1, Zero, Interface, 6, Status */
#define DFU_CLRSTATUS 0x04 /* 0x21, Zero, Interface, Zero, None */
#define DFU_GETSTATE 0x05 /* 0xA1, Zero, Interface, 1, State */
#define DFU_ABORT 0x06 /* 0x21, Zero, Interface, Zero, None */

/*
 * DFU status values
 */
#define OK              0x00 // No error condition is present.
#define errTARGET       0x01 // File is not targeted for use by this device.
#define errFILE         0x02 // File is for this device but fails some vendor-specific verification test.
#define errWRITE        0x03 // Device is unable to write memory.
#define errERASE        0x04 // Memory erase function failed.
#define errCHECK_ERASED 0x05 // Memory erase check failed.
#define errPROG         0x06 // Program memory function failed.
#define errVERIFY       0x07 // Programmed memory failed verification.
#define errADDRESS      0x08 // Cannot program memory due to received address that is out of range.
#define errNOTDONE      0x09 // Received DFU_DNLOAD with wLength = 0, but device does not think it has all of the data yet.
#define errFIRMWARE     0x0A // Device’s firmware is corrupt. It cannot return to run-time (non-DFU) operations.
#define errVENDOR       0x0B // iString indicates a vendor-specific error.
#define errUSBR         0x0C // Device detected unexpected USB reset signaling.
#define errPOR          0x0D // Device detected unexpected power on reset.
#define errUNKNOWN      0x0E // Something went wrong, but the device does not know what it was.
#define errSTALLEDPKT   0x0F // Device stalled an unexpected request.

/*
 * DFU state values
 */
#define appIDLE                0
#define appDETACH              1
#define dfuIDLE                2
#define dfuDNLOAD_SYNC         3
#define dfuDNBUSY              4
#define dfuDNLOAD_IDLE         5
#define dfuMANIFEST_SYNC       6
#define dfuMANIFEST            7
#define dfuMANIFEST_WAIT_RESET 8
#define dfuUPLOAD_IDLE         9
#define dfuERROR               10

extern DFU_Status dfu_status;

#define DFU_NO_CMD                0
#define DFU_WAIT_CMD              1
#define DFU_CMD_GET_CMD           2
#define DFU_CMD_UPLOAD            3
#define DFU_CMD_DOWNLOAD          4
#define DFU_CMD_SET_ADDRESS       5
#define DFU_CMD_ERASE_PAGE        6
#define DFU_CMD_MASS_ERASE        7
#define DFU_CMD_READ_UNPROTECTED  8
#define DFU_CMD_JUMP_APP          9

#define GET_COMMAND_TOKEN       0x00
#define SET_ADDRESS_TOKEN       0x21
#define ERASE_PAGE_TOKEN        0x41
#define READ_UNPROTECTED_TOKEN  0x92

void init_dfu(void);
u8 process_dfu_request(StandardRequest *request);
void process_dfu_data(u8 *buffer, u16 length);
u16 read_dfu_data(StandardRequest *request, u8 *buffer, u16 max_length);

u8 dfuOperationStarted(void);
void dfuFinishOperation(void);
u8 dfuWaitReset(void);
void setManifest(void);
void jump_to_app(void);

