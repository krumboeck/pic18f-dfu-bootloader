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

#include "pic18fregs.h"
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
u32 address = 0;
u8 transfer[DATA_BUFFER_SIZE];
u16 transfer_length;

void* memcpy(void *dest, const void *src, u16 count) {
    char *dst8 = (u8 *)dest;
    char *src8 = (u8 *)src;

    while (count--) {
        *dst8++ = *src8++;
    }
    return dest;
}

void init_dfu(void) {
	dfu_status.bStatus = OK;
	dfu_status.bwPollTimeout0 = 0;
	dfu_status.bwPollTimeout1 = 0;
	dfu_status.bwPollTimeout2 = 0;
	dfu_status.bState = dfuIDLE;
	dfu_status.iString = 0;
	dfu_op_state = INIT;
	dfuBusy = 0;
	address = 0;
}

u8 process_dfu_request(StandardRequest *request) {
	u8 currentState = dfu_status.bState;
	dfu_status.bStatus = OK;

	// debug2(" rtyp: %d\n", request->bmRequestType);
	// debug2(" rqst: %d\n", request->bRequest);
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
				dfu_status.bState = dfuMANIFEST_SYNC;
			}
		} else if (request->bRequest == DFU_UPLOAD) {
			dfu_status.bState = dfuUPLOAD_IDLE;

			if (request->wIndex == 0) {
				debug("DFU Start Upload\r\n");
				if (request->wValue == 0) {
					dfuSubCommand = DFU_CMD_GET_CMD;
				} else {
					dfuSubCommand = DFU_CMD_UPLOAD;
					address = ENTRY;
				}
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
				if (dfuSubCommand == DFU_CMD_MASS_ERASE) {
					dfu_status.bwPollTimeout0 = 0xFF;
					dfu_status.bwPollTimeout1 = 0x04;
				} else {
					dfu_status.bwPollTimeout0 = 0x20;
					dfu_status.bwPollTimeout1 = 0x00;
				}
				dfu_status.bState = dfuDNBUSY;

			} else if (dfu_op_state == BEGIN) {
				dfu_status.bState = dfuDNLOAD_SYNC;

			} else if (dfu_op_state == MIDDLE) {
				dfu_status.bState = dfuDNLOAD_SYNC;

			} else if (dfu_op_state == END) {
				dfu_status.bwPollTimeout0 = 0x00;
				dfu_status.bwPollTimeout1 = 0x00;
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
				dfu_status.bState = dfuMANIFEST_SYNC;
			}
		} else if (request->bRequest == DFU_ABORT) {
			dfu_status.bState = dfuIDLE;
			address = 0;
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

		dfu_status.bState = dfuMANIFEST;
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
				if (request->wValue == 0) {
					dfuSubCommand = DFU_CMD_GET_CMD;
				} else {
					dfuSubCommand = DFU_CMD_UPLOAD;
					address = ENTRY;
				}
			} else {
				dfu_status.bState = dfuERROR;
				dfu_status.bStatus = errNOTDONE;
			}
		} else if (request->bRequest == DFU_ABORT) {
			dfu_status.bState = dfuIDLE;
			address = 0;
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
		if (address >= ENTRY && address <= FLASH_END) {
			eraseFlash(address);
		} else {
			dfu_status.bState = dfuERROR;
			dfu_status.bStatus = errADDRESS;
		}
	} else if (dfuSubCommand == DFU_CMD_MASS_ERASE) {
		u32 erase_address;
		debug("start mass-erase\n");
		for (erase_address = ENTRY; erase_address < FLASH_END; erase_address += ERASE_PAGE_SIZE) {
			eraseFlash(erase_address);
		}
		debug("stop mass-erase\n");
	} else if (dfuSubCommand == DFU_CMD_DOWNLOAD) {
		if (address >= ENTRY && address <= FLASH_END) {
			int counter;
			int written = 0;
			if ((FLASH_END - address + 1) < transfer_length) {
				transfer_length = FLASH_END - address + 1;
			}
			counter = transfer_length;
			debug2("writing address: %lx\n", address);
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
			dfu_status.bState = dfuERROR;
			dfu_status.bStatus = errADDRESS;
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

u8 dfuWaitReset() {
	return dfu_status.bState == dfuMANIFEST_WAIT_RESET ? 1 : 0;
}

void setManifest() {
	dfu_status.bState = dfuMANIFEST;
}

void jump_to_app() {
    RCON |= 0x93;     // reset all reset flag
	debug("Jump to app\n");
	/* TODO: make goto variable
	if (address >= ENTRY && address <= FLASH_END) {
		address += 4;
	} else {
		address = ENTRY;
	}
	*/
	__asm
		goto ENTRY
    __endasm;
}

void process_dfu_data(u8 *buffer, u16 length) {
	if (dfuSubCommand == DFU_WAIT_CMD) {
		u8 command = buffer[0];
		if (command == SET_ADDRESS_TOKEN) {
			dfuSubCommand = DFU_CMD_SET_ADDRESS;
			if (length == 5) {
				address = (u32) buffer[4] << 24 | (u32) buffer[3] << 16 | (u32) buffer[2] << 8 | (u32) buffer[1];
				debug2("Set address to %lx\n", address);
				if (address < ENTRY || address > FLASH_END) {
					dfu_status.bState = dfuERROR;
					dfu_status.bStatus = errADDRESS;
				}
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
			dfu_status.bState = dfuERROR;
			dfu_status.bStatus = errUNKNOWN;
		}
	}

}

u16 read_dfu_data(StandardRequest *request, u8 *buffer, u16 max_length) {
	u16 length = 0;
	debug("read dfu\n");
	if (request->bRequest == DFU_GETSTATUS) {
		length = 6;
		buffer[0] = dfu_status.bStatus;
		buffer[1] = dfu_status.bwPollTimeout0;
		buffer[2] = dfu_status.bwPollTimeout1;
		buffer[3] = dfu_status.bwPollTimeout2;
		buffer[4] = dfu_status.bState;
		buffer[5] = dfu_status.iString;
	} else if (request->bRequest == DFU_GETSTATE) {
		length = 1;
		buffer[0] = dfu_status.bState;
	} else if (dfuSubCommand == DFU_CMD_GET_CMD) {
		length = 3;
		buffer[0] = GET_COMMAND_TOKEN;
		buffer[1] = SET_ADDRESS_TOKEN;
		buffer[2] = ERASE_PAGE_TOKEN;
	} else if (dfuSubCommand == DFU_CMD_UPLOAD) {
		u32 read_address;
		debug("upload\n");
		if (address < ENTRY) {
			address = ENTRY;
		}
		read_address = (request->wValue - 2) * DATA_BUFFER_SIZE + address;
		debug2("address: %lx\n", read_address);
		if (read_address >= FLASH_END) {
			length = 0;
			dfu_status.bState = dfuIDLE;
		} else {
			int counter;
			int readed = 0;
			length = DATA_BUFFER_SIZE;
			if (length > max_length) {
				length = max_length;
			}
			if (length > request->wLength) {
				length = request->wLength;
			}
			if ((FLASH_END - read_address + 1) < length) {
				length = FLASH_END - read_address + 1;
			}

			counter = length;
			while (counter > 0) {
				if (counter >= FLASH_WRITE_SIZE) {
					readFlash(read_address + readed, (u8 __data *)&buffer[readed], FLASH_WRITE_SIZE);
				} else {
					readFlash(read_address + readed, (u8 __data *)&buffer[readed], counter);
				}
				counter = counter - FLASH_WRITE_SIZE;
				readed = readed + FLASH_WRITE_SIZE;
			}

			debug2("1: %x\n", buffer[0]);

		}
	}
	return length;
}

