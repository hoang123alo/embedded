//-----------------------------------------------------------------------------
// F50x_InfoBlock.c - CAN_BL - Target_BL_FW
//-----------------------------------------------------------------------------
// Copyright (C) 2009 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// Program Description:
//
// This file contains a set of bytes that will be placed as the last bytes of
// the App FW. The locating is done by using the linker command line.
//
// Locate starting at address (63*1024-512-7-4) = 0xF9F5
//
// Target:         C8051F500
// Tool chain:     Keil C51 8.0 / Keil EVAL C51
// Command Line:   None
//
//
// Release 1.0 / 18NOV2009 (PKC)
//    -Initial Revision
//
//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "compiler_defs.h"
#include "C8051F580_defs.h"

const U8 code OPTICINFOBLOCK[9] = 
{0x00,0x00,0x00,0x63,0x63,0x00,0x00,0x00,0x00};

//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------