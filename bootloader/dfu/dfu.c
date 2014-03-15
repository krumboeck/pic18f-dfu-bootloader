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

#include "typedef.h"
#include "debug.h"
#include "usb/usb_std_req.h"
#include "dfu/dfu.h"
#include "flash.h"
#include "config.h"

DFU_Status dfu_status;
DFU_OP_State dfu_op_state;
u8 dfuBusy;
u8 dfuSubCommand;
u32 address;
u8 transfer[DATA_BUFFER_SIZE];
u16 transfer_length;

void init_dfu(void) {
	dfu_status.bStatus = OK;
	dfu_status.bwPollTimeout0 = 0;
	dfu_status.bwPollTimeout1 = 0;
	dfu_status.bwPollTimeout2 = 0;
	dfu_status.bState = dfuIDLE;
	dfu_status.iString = 0;
	dfu_op_state = INIT;
	dfuBusy = 0;
}

u8 process_dfu_request(StandardRequest *request) {
	u8 currentState = dfu_status.bState;
	dfu_status.bStatus = OK;

	debug2(" rtyp: %d\n", request->bmRequestType);
	debug2(" rqst: %d\n", request->bRequest);
	// debug2("DFU Index  : %d\r\n", request->wIndex);

	dfuBusy = 1;

	if (currentState == dfuIDLE) {
		dfuBusy = 1;
		if (request->bRequest == DFU_DNLOAD) {

			if (request->wLength > 0) {
				// userFirmwareLen = 0;
				dfu_status.bState = dfuDNLOAD_SYNC;

				if (request->wIndex == 0) {
					if (request->wValue == 0) {
						debug("Subcommand\n");
						dfuSubCommand = DFU_WAIT_CMD;
					} else {
						debug("DFU Start Download\n");
						dfuSubCommand = DFU_CMD_DOWNLOAD;
					}
				} else {
					dfu_status.bState = dfuERROR;
					dfu_status.bStatus = errUNKNOWN;
				}

			} else {
				dfu_status.bState = dfuERROR;
				dfu_status.bStatus = errNOTDONE;
			}
		} else if (request->bRequest == DFU_UPLOAD) {
			dfu_status.bState = dfuUPLOAD_IDLE;

			/* record length of first block for calculating target
			 address from wValue in consecutive blocks */
			//uploadBlockLen = request->wLength;
			//thisBlockLen = uploadBlockLen; /* for this first block as well */
			/* calculate where the data should be copied from */
			//userFirmwareLen = uploadBlockLen * request->wValue;
			if (request->wIndex == 0) {
				debug("DFU Start Upload\r\n");
				/*
				 userAppAddr = USER_CODE_FLASH;
				 userAppEnd = FLASH_END;
				 */
			} else {
				dfu_status.bState = dfuERROR;
				dfu_status.bStatus = errUNKNOWN;
			}
		} else if (request->bRequest == DFU_ABORT) {
			dfu_status.bState = dfuIDLE;
			dfu_status.bStatus = OK; /* are we really ok? we were just aborted */
		} else if (request->bRequest == DFU_GETSTATUS) {
			debug("DFU get_status\r\n");
			dfu_status.bState = dfuIDLE;
		} else if (request->bRequest == DFU_GETSTATE) {
			dfu_status.bState = dfuIDLE;
		} else {
			dfu_status.bState = dfuERROR;
			dfu_status.bStatus = errSTALLEDPKT;
		}

	} else if (currentState == dfuDNLOAD_SYNC) {
		/* device received block, waiting for DFU_GETSTATUS request */

		if (request->bRequest == DFU_GETSTATUS) {
			/* todo, add routine to wait for last block write to finish */

			if (dfu_op_state == INIT) {
				dfu_op_state = BEGIN;
				dfu_status.bwPollTimeout0 = 0x20;
				dfu_status.bwPollTimeout1 = 0x00;
				dfu_status.bState = dfuDNBUSY;

			} else if (dfu_op_state == BEGIN) {
				dfu_status.bState = dfuDNLOAD_SYNC;

			} else if (dfu_op_state == MIDDLE) {
				dfu_status.bState = dfuDNLOAD_SYNC;

			} else if (dfu_op_state == END) {
				dfu_status.bwPollTimeout0 = 0x00;
				dfu_op_state = INIT;
				dfu_status.bState = dfuDNLOAD_IDLE;
			}

		} else if (request->bRequest == DFU_GETSTATE) {
			dfu_status.bState = dfuDNLOAD_SYNC;
		} else {
			dfu_status.bState = dfuERROR;
			dfu_status.bStatus = errSTALLEDPKT;
		}

	} else if (currentState == dfuDNBUSY) {
		/* if were actually done writing, goto sync, else stay busy */
		if (dfu_op_state == END) {
			dfu_status.bwPollTimeout0 = 0x00;
			dfu_op_state = INIT;
			dfu_status.bState = dfuDNLOAD_IDLE;
		} else {
			dfu_status.bState = dfuDNBUSY;
		}

	} else if (currentState == dfuDNLOAD_IDLE) {
		/* device is expecting dfu_dnload requests */
		if (request->bRequest == DFU_DNLOAD) {
			if (request->wLength > 0) {
				dfu_status.bState = dfuDNLOAD_SYNC;
				if (request->wValue == 0) {
					debug("Subcommand\n");
					dfuSubCommand = DFU_WAIT_CMD;
				} else {
					debug("DFU Start Download\n");
					dfuSubCommand = DFU_CMD_DOWNLOAD;
				}
			} else {
				/* todo, support "disagreement" if device expects more data than this */
				dfu_status.bState = dfuMANIFEST_SYNC;
			}
		} else if (request->bRequest == DFU_ABORT) {
			dfu_status.bState = dfuIDLE;
		} else if (request->bRequest == DFU_GETSTATUS) {
			dfu_status.bState = dfuIDLE;
		} else if (request->bRequest == DFU_GETSTATE) {
			dfu_status.bState = dfuIDLE;
		} else {
			dfu_status.bState = dfuERROR;
			dfu_status.bStatus = errSTALLEDPKT;
		}

	} else if (currentState == dfuMANIFEST_SYNC) {
		/* device has received last block, waiting DFU_GETSTATUS request */

		if (request->bRequest == DFU_GETSTATUS) {
			dfu_status.bState = dfuMANIFEST_WAIT_RESET;
			dfu_status.bStatus = OK;
		} else if (request->bRequest == DFU_GETSTATE) {
			dfu_status.bState = dfuMANIFEST_SYNC;
		} else {
			dfu_status.bState = dfuERROR;
			dfu_status.bStatus = errSTALLEDPKT;
		}

	} else if (currentState == dfuMANIFEST) {
		/* device is in manifestation phase */

		/* should never receive request while in manifest! */
		dfu_status.bState = dfuMANIFEST_WAIT_RESET;
		dfu_status.bStatus = OK;

	} else if (currentState == dfuMANIFEST_WAIT_RESET) {
		/* device has programmed new firmware but needs external
		 usb reset or power on reset to run the new code */

		/* consider timing out and self-resetting */
		dfu_status.bState = dfuMANIFEST_WAIT_RESET;

	} else if (currentState == dfuUPLOAD_IDLE) {
		/* device expecting further dfu_upload requests */

		if (request->bRequest == DFU_UPLOAD) {
			if (request->wLength > 0) {
				/* check that this is not the last possible block */
				/*
				 userFirmwareLen = uploadBlockLen * request->wValue;
				 if (userAppAddr + userFirmwareLen + uploadBlockLen <= userAppEnd) {
				 thisBlockLen = uploadBlockLen;
				 dfu_status.bState = dfuUPLOAD_IDLE;
				 } else {
				 */
				/* if above comparison was just equal, thisBlockLen becomes zero
				 next time when USBWValue has been increased by one */
//          thisBlockLen = userAppEnd - userAppAddr - userFirmwareLen;
				/* check for overflow due to USBwValue out of range */
				/*
				 if (thisBlockLen >= request->wLength) {
				 thisBlockLen = 0;
				 }
				 dfu_status.bState = dfuIDLE;
				 }
				 */
			} else {
				dfu_status.bState = dfuERROR;
				dfu_status.bStatus = errNOTDONE;
			}
		} else if (request->bRequest == DFU_ABORT) {
			dfu_status.bState = dfuIDLE;
		} else if (request->bRequest == DFU_GETSTATUS) {
			dfu_status.bState = dfuUPLOAD_IDLE;
		} else if (request->bRequest == DFU_GETSTATE) {
			dfu_status.bState = dfuUPLOAD_IDLE;
		} else {
			dfu_status.bState = dfuERROR;
			dfu_status.bStatus = errSTALLEDPKT;
		}

	} else if (currentState == dfuERROR) {
		/* status is in error, awaiting DFU_CLRSTATUS request */

		if (request->bRequest == DFU_GETSTATUS) {
			/* todo, add routine to wait for last block write to finish */
			dfu_status.bState = dfuERROR;
		} else if (request->bRequest == DFU_GETSTATE) {
			dfu_status.bState = dfuERROR;
		} else if (request->bRequest == DFU_CLRSTATUS) {
			/* todo handle any cleanup we need here */
			dfu_status.bState = dfuIDLE;
			dfu_status.bStatus = OK;
		} else {
			dfu_status.bState = dfuERROR;
			dfu_status.bStatus = errSTALLEDPKT;
		}

	} else {
		/* some kind of error... */
		dfu_status.bState = dfuERROR;
		dfu_status.bStatus = errSTALLEDPKT;
	}

	if (dfu_status.bStatus == OK) {
		return TRUE;
	} else {
		return FALSE;
	}

}

void dfuExecCommand() {
	if (dfuSubCommand == DFU_CMD_ERASE_PAGE) {
		debug("erasing ...\n");
		eraseFlash(address);
	} else if (dfuSubCommand == DFU_CMD_DOWNLOAD) {
		int counter = transfer_length;
		int written = 0;
		debug("writing ...\n");
		while (counter > 0) {
			if (counter >= FLASH_WRITE_SIZE) {
				writeFlash(address + written, (u8 __data *)&transfer[written], FLASH_WRITE_SIZE);
			} else {
				writeFlash(address + written, (u8 __data *)&transfer[written], counter);
			}
			counter = counter - FLASH_WRITE_SIZE;
			written = written + FLASH_WRITE_SIZE;
		}
	} else {
		debug("do nothing\n");
	}
}

u8 dfuOperationStarted() {
	return dfuBusy;
}

void dfuFinishOperation() {
	if (dfu_op_state == BEGIN) {
		dfu_op_state = MIDDLE;
		dfuExecCommand();
		dfu_op_state = END;
	}
}

void process_dfu_data(u8 *buffer, u16 length) {
	if (dfuSubCommand == DFU_WAIT_CMD) {
		u8 command = buffer[0];
		if (command == SET_ADDRESS_TOKEN) {
			dfuSubCommand = DFU_CMD_SET_ADDRESS;
			if (length == 5) {
				address = (u32) buffer[4] << 24 | (u32) buffer[3] << 16 | (u32) buffer[2] << 8 | (u32) buffer[1];
				debug2("Set address to %lx\n", address);
			} else {
				dfu_status.bState = dfuERROR;
				dfu_status.bStatus = errSTALLEDPKT;
			}
		} else if (command == ERASE_PAGE_TOKEN) {
			if (length == 1) {
				dfuSubCommand = DFU_CMD_MASS_ERASE;
				address = 0;
				debug("Mass Erase\n");
			} else if (length == 5) {
				dfuSubCommand = DFU_CMD_ERASE_PAGE;
				address = (u32) buffer[4] << 24 | (u32) buffer[3] << 16 | (u32) buffer[2] << 8 | (u32) buffer[1];
 				debug2("Erase page at %lx\n", address);
			} else {
				dfu_status.bState = dfuERROR;
				dfu_status.bStatus = errSTALLEDPKT;
			}
		} else if (command == READ_UNPROTECTED_TOKEN) {
			dfuSubCommand = DFU_CMD_READ_UNPROTECTED;
			debug("Read Unprotected\n");
		} else {
			dfu_status.bState = dfuERROR;
			dfu_status.bStatus = errSTALLEDPKT;
		}
	} else if (dfuSubCommand == DFU_CMD_DOWNLOAD) {
		int counter = 0;
		if (length <= DATA_BUFFER_SIZE) {
			transfer_length = length;
			for (counter = 0; counter < length; counter++) {
				transfer[counter] = buffer[counter];
			}
		} else {
			transfer_length = 0;
			dfuSubCommand = DFU_WAIT_CMD;
			// TODO: Error
		}
	}
}
