#ifndef _EDDYSTONE_H_
#define _EDDYSTONE_H_

//// Definition of data for the different Eddystone frame types.
// Note that multi-byte Eddystone data is in Big-Endian format.
// Frame type ids
#define EDDYSTONE_FRAME_TYPE_UID 0x00
#define EDDYSTONE_FRAME_TYPE_URL 0x10
#define EDDYSTONE_FRAME_TYPE_TLM 0x20
#define EDDYSTONE_FRAME_TYPE_EID 0x30

// TLM version
#define EDDYSTONE_TLM_VERSION   0x00

#define EDDYSTONE_TXPWR -17


// Arduino IDE has problems with enums passed as function parameters.
// Interestingly, you cannot declare the enum type in the file
// with a function having a parameter of this enum type. The workaround
// is to out-source the enum declaration to a separate file (as this
// one here).  
enum url_schemes {http_www_dot, https_www_dot, http, https};



#endif
