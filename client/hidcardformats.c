//-----------------------------------------------------------------------------
// Copyright (C) 2018 grauerfuchs
//
// This code is licensed to you under the terms of the GNU GPL, version 2 or,
// at your option, any later version. See the LICENSE.txt file for the text of
// the license.
//-----------------------------------------------------------------------------
// HID card format packing/unpacking routines
//-----------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include "cmddata.h"
#include "hidcardformats.h"
#include "hidcardformatutils.h"
#include "parity.h" // for parity
#include "ui.h"

bool Pack_H10301(/*in*/hidproxcard_t* card, /*out*/hidproxmessage_t* packed){
  memset(packed, 0, sizeof(hidproxmessage_t));
  if (card->FacilityCode > 0xFF) return false; // Can't encode FC.
  if (card->CardNumber > 0xFFFF) return false; // Can't encode CN.
  
  packed->Length = 26; // Set number of bits
  packed->bot |= (card->CardNumber & 0xFFFF) << 1;
  packed->bot |= (card->FacilityCode & 0xFF) << 17;
  if (card->ParitySupported){
    packed->bot |= oddparity32((packed->bot >> 1) & 0xFFF) & 1;
    packed->bot |= (evenparity32((packed->bot >> 13) & 0xFFF) & 1) << 25;
  }
  return add_HID_header(packed);
}
bool Unpack_H10301(/*in*/hidproxmessage_t* packed, /*out*/hidproxcard_t* card){
  memset(card, 0, sizeof(hidproxcard_t));
  if (packed->Length != 26) return false; // Wrong length? Stop here.
  card->CardNumber = (packed->bot >> 1) & 0xFFFF;
  card->FacilityCode = (packed->bot >> 17) & 0xFF;
  card->ParitySupported = true;
  card->ParityValid =
    (oddparity32((packed->bot >> 1) & 0xFFF) == (packed->bot & 1)) &&
    ((evenparity32((packed->bot >> 13) & 0xFFF) & 1) == ((packed->bot >> 25) & 1));
  return true;
}

bool Pack_Tecom27(/*in*/hidproxcard_t* card, /*out*/hidproxmessage_t* packed){
  memset(packed, 0, sizeof(hidproxmessage_t));
  if (card->FacilityCode > 0x7FF) return false; // Can't encode FC.
  if (card->CardNumber > 0xFFFF) return false; // Can't encode CN.
  packed->Length = 27;
  set_nonlinear_field(packed, card->FacilityCode, 10, (uint8_t[]){15, 19, 24, 23, 22, 18, 6, 10, 14, 3, 2});
  set_nonlinear_field(packed, card->CardNumber, 16, (uint8_t[]){0, 1, 13, 12, 9, 26, 20, 16, 17, 21, 25, 7, 8, 11, 4, 5});
  return add_HID_header(packed);
}
bool Unpack_Tecom27(/*in*/hidproxmessage_t* packed, /*out*/hidproxcard_t* card){
  memset(card, 0, sizeof(hidproxcard_t));
  if (packed->Length != 27) return false; // Wrong length? Stop here.
  card->CardNumber = get_nonlinear_field(packed, 16, (uint8_t[]){0, 1, 13, 12, 9, 26, 20, 16, 17, 21, 25, 7, 8, 11, 4, 5});
  card->FacilityCode = get_nonlinear_field(packed, 10, (uint8_t[]){15, 19, 24, 23, 22, 18, 6, 10, 14, 3, 2});
  card->ParitySupported = false;
  return true;
}

bool Pack_2804W(/*in*/hidproxcard_t* card, /*out*/hidproxmessage_t* packed){
  memset(packed, 0, sizeof(hidproxmessage_t));
  if (card->FacilityCode > 0x0FF) return false; // Can't encode FC.
  if (card->CardNumber > 0x7FFF) return false; // Can't encode CN.
  packed->Length = 28;
  set_linear_field(packed, card->FacilityCode, 4, 8);
  set_linear_field(packed, card->CardNumber, 12, 15);
  if (card->ParitySupported){
    set_bit_by_position(packed, 
      oddparity32(get_nonlinear_field(packed, 16, (uint8_t[]){4, 5, 7, 8, 10, 11, 13, 14, 16, 17, 19, 20, 22, 23, 25, 26}))
      , 2);
    set_bit_by_position(packed, 
      evenparity32(get_linear_field(packed, 1, 13))
      , 0);
    set_bit_by_position(packed, 
      oddparity32(get_linear_field(packed, 0, 27))
      , 27);
  }
  return add_HID_header(packed);
}
bool Unpack_2804W(/*in*/hidproxmessage_t* packed, /*out*/hidproxcard_t* card){
  memset(card, 0, sizeof(hidproxcard_t));
  if (packed->Length != 28) return false; // Wrong length? Stop here.
  card->FacilityCode = get_linear_field(packed, 4, 8);
  card->CardNumber = get_linear_field(packed, 12, 15);
  card->ParitySupported = true;
  card->ParityValid = 
    (get_bit_by_position(packed, 0) == evenparity32(get_linear_field(packed, 1, 13))) &&
    (get_bit_by_position(packed, 2) == oddparity32(get_nonlinear_field(packed, 16, (uint8_t[]){4, 5, 7, 8, 10, 11, 13, 14, 16, 17, 19, 20, 22, 23, 25, 26}))) &&
    (get_bit_by_position(packed, 27) == oddparity32(get_linear_field(packed, 0, 27)));
  return true;
}

bool Pack_ATSW30(/*in*/hidproxcard_t* card, /*out*/hidproxmessage_t* packed){
  memset(packed, 0, sizeof(hidproxmessage_t));
  if (card->FacilityCode > 0xFFF) return false; // Can't encode FC.
  if (card->CardNumber > 0xFFFF) return false; // Can't encode CN.
  packed->Length = 30;
  set_linear_field(packed, card->FacilityCode, 1, 12);
  set_linear_field(packed, card->CardNumber, 13, 16);
  if (card->ParitySupported){
    set_bit_by_position(packed, 
      evenparity32(get_linear_field(packed, 1, 12))
      , 0);
    set_bit_by_position(packed, 
      oddparity32(get_linear_field(packed, 13, 16))
      , 29);
  }
  return add_HID_header(packed);
}
bool Unpack_ATSW30(/*in*/hidproxmessage_t* packed, /*out*/hidproxcard_t* card){
  memset(card, 0, sizeof(hidproxcard_t));
  if (packed->Length != 30) return false; // Wrong length? Stop here.
  card->FacilityCode = get_linear_field(packed, 1, 12);
  card->CardNumber = get_linear_field(packed, 13, 16);
  card->ParitySupported = true;
  card->ParityValid = 
    (get_bit_by_position(packed, 0) == evenparity32(get_linear_field(packed, 1, 12))) &&
    (get_bit_by_position(packed, 29) == oddparity32(get_linear_field(packed, 13, 16)));
  return true;
}
bool Pack_ADT31(/*in*/hidproxcard_t* card, /*out*/hidproxmessage_t* packed){
  memset(packed, 0, sizeof(hidproxmessage_t));
  if (card->FacilityCode > 0x0F) return false; // Can't encode FC.
  if (card->CardNumber > 0x7FFFFF) return false; // Can't encode CN.
  packed->Length = 31;
  set_linear_field(packed, card->FacilityCode, 1, 4);
  set_linear_field(packed, card->CardNumber, 5, 23);
  // Parity not known, but 4 bits are unused.
  return add_HID_header(packed);
}
bool Unpack_ADT31(/*in*/hidproxmessage_t* packed, /*out*/hidproxcard_t* card){
  memset(card, 0, sizeof(hidproxcard_t));
  if (packed->Length != 31) return false; // Wrong length? Stop here.
  card->FacilityCode = get_linear_field(packed, 1, 4);
  card->CardNumber = get_linear_field(packed, 5, 23);
  card->ParitySupported = false;
  return true;
}

bool Pack_D10202(/*in*/hidproxcard_t* card, /*out*/hidproxmessage_t* packed){
  memset(packed, 0, sizeof(hidproxmessage_t));
  if (card->FacilityCode > 0x007F) return false; // Can't encode FC.
  if (card->CardNumber > 0x00FFFFFF) return false; // Can't encode CN.
  
  packed->Length = 33; // Set number of bits
  set_linear_field(packed, card->FacilityCode, 1, 7);
  set_linear_field(packed, card->CardNumber, 8, 24);
  
  if (card->ParitySupported){
    set_bit_by_position(packed, evenparity32(get_linear_field(packed, 1, 16)), 0);
    set_bit_by_position(packed, oddparity32(get_linear_field(packed, 16, 16)), 32);
  }
  return add_HID_header(packed);
}
bool Unpack_D10202(/*in*/hidproxmessage_t* packed, /*out*/hidproxcard_t* card){
  memset(card, 0, sizeof(hidproxcard_t));
  if (packed->Length != 33) return false; // Wrong length? Stop here.
  
  card->CardNumber = get_linear_field(packed, 8, 24);
  card->FacilityCode = get_linear_field(packed, 1, 7);
  card->ParitySupported = true;
  card->ParityValid =
    (get_bit_by_position(packed, 0) == evenparity32(get_linear_field(packed, 1, 16))) &&
    (get_bit_by_position(packed, 32) == oddparity32(get_linear_field(packed, 16, 16)));
  return true;
}


bool Pack_H10306(/*in*/hidproxcard_t* card, /*out*/hidproxmessage_t* packed){
  memset(packed, 0, sizeof(hidproxmessage_t));
  if (card->FacilityCode > 0xFFFF) return false; // Can't encode FC.
  if (card->CardNumber > 0xFFFF) return false; // Can't encode CN.
  
  packed->Length = 34; // Set number of bits
  packed->bot |= (card->CardNumber & 0xFFFF) << 1;
  packed->bot |= (card->FacilityCode & 0x7FFF) << 17;
  packed->mid |= (card->FacilityCode & 0x8000) >> 15;
  if (card->ParitySupported){
    packed->mid |= (evenparity32((packed->mid & 0x00000001) ^ (packed->bot & 0xFFFE0000)) & 1) << 1;
    packed->bot |= ( oddparity32(packed->bot & 0x0001FFFE) & 1);
  }
  return add_HID_header(packed);
}
bool Unpack_H10306(/*in*/hidproxmessage_t* packed, /*out*/hidproxcard_t* card){
  memset(card, 0, sizeof(hidproxcard_t));
  if (packed->Length != 34) return false; // Wrong length? Stop here.
  
  card->CardNumber = (packed->bot >> 1) & 0xFFFF;
  card->FacilityCode = ((packed->mid & 1) << 15) | ((packed->bot >> 17) & 0xFF);
  card->ParitySupported = true;
  card->ParityValid =
    ((evenparity32((packed->mid & 0x00000001) ^ (packed->bot & 0xFFFE0000)) & 1) == ((packed->mid >> 1) & 1)) &&
    ((oddparity32(packed->bot & 0x0001FFFE) & 1) == ((packed->bot & 1)));
  return true;
}
bool Pack_N1002(/*in*/hidproxcard_t* card, /*out*/hidproxmessage_t* packed){
  memset(packed, 0, sizeof(hidproxmessage_t));
  if (card->FacilityCode > 0xFF) return false; // Can't encode FC.
  if (card->CardNumber > 0xFFFF) return false; // Can't encode CN.
  
  packed->Length = 34; // Set number of bits
  set_linear_field(packed, card->FacilityCode, 9, 8);
  set_linear_field(packed, card->CardNumber, 17, 16);
  return add_HID_header(packed);
}
bool Unpack_N1002(/*in*/hidproxmessage_t* packed, /*out*/hidproxcard_t* card){
  memset(card, 0, sizeof(hidproxcard_t));
  if (packed->Length != 34) return false; // Wrong length? Stop here.
  
  card->CardNumber = get_linear_field(packed, 17, 16);
  card->FacilityCode = get_linear_field(packed, 9, 8);
  card->ParitySupported = false;
  return true;
}


bool Pack_C1k35s(/*in*/hidproxcard_t* card, /*out*/hidproxmessage_t* packed){
  memset(packed, 0, sizeof(hidproxmessage_t));
  if (card->FacilityCode > 0xFFF) return false; // Can't encode FC.
  if (card->CardNumber > 0xFFFFF) return false; // Can't encode CN.
  
  packed->Length = 35; // Set number of bits
  packed->bot |= (card->CardNumber & 0x000FFFFF) << 1;
  packed->bot |= (card->FacilityCode & 0x000007FF) << 21;
  packed->mid |= (card->FacilityCode & 0x00000800) >> 11;
  if (card->ParitySupported){
    packed->mid |= (evenparity32((packed->mid & 0x00000001) ^ (packed->bot & 0xB6DB6DB6)) & 1) << 1;
    packed->bot |= ( oddparity32((packed->mid & 0x00000003) ^ (packed->bot & 0x6DB6DB6C)) & 1);
    packed->mid |= ( oddparity32((packed->mid & 0x00000003) ^ (packed->bot & 0xFFFFFFFF)) & 1) << 2;
  }
  return add_HID_header(packed);
}
bool Unpack_C1k35s(/*in*/hidproxmessage_t* packed, /*out*/hidproxcard_t* card){
  memset(card, 0, sizeof(hidproxcard_t));
  if (packed->Length != 35) return false; // Wrong length? Stop here.
  
  card->CardNumber = (packed->bot >> 1) & 0x000FFFFF;
  card->FacilityCode = ((packed->mid & 1) << 11) | ((packed->bot >> 21));
  card->ParitySupported = true;
  card->ParityValid =
    (evenparity32((packed->mid & 0x00000001) ^ (packed->bot & 0xB6DB6DB6)) == ((packed->mid >> 1) & 1)) &&
    ( oddparity32((packed->mid & 0x00000003) ^ (packed->bot & 0x6DB6DB6C)) == ((packed->bot >> 0) & 1)) &&
    ( oddparity32((packed->mid & 0x00000003) ^ (packed->bot & 0xFFFFFFFF)) == ((packed->mid >> 2) & 1));
  return true;
}

bool Pack_H10320(/*in*/hidproxcard_t* card, /*out*/hidproxmessage_t* packed){
  memset(packed, 0, sizeof(hidproxmessage_t));
  if (card->FacilityCode > 0) return false; // Can't encode FC. (none in this format)
  if (card->CardNumber > 99999999) return false; // Can't encode CN.
  
  packed->Length = 36; // Set number of bits
  // This card is BCD-encoded rather than binary. Set the 4-bit groups independently.
  set_linear_field(packed, (card->CardNumber / 10000000) % 10, 0, 4);
  set_linear_field(packed, (card->CardNumber / 1000000) % 10, 4, 4);
  set_linear_field(packed, (card->CardNumber / 100000) % 10, 8, 4);
  set_linear_field(packed, (card->CardNumber / 10000) % 10, 12, 4);
  set_linear_field(packed, (card->CardNumber / 1000) % 10, 16, 4);
  set_linear_field(packed, (card->CardNumber / 100) % 10, 20, 4);
  set_linear_field(packed, (card->CardNumber / 10) % 10, 24, 4);
  set_linear_field(packed, (card->CardNumber / 1) % 10, 28, 4);
  if (card->ParitySupported){
    set_bit_by_position(packed, evenparity32(
      get_nonlinear_field(packed, 8, (uint8_t[]){0, 4, 8, 12, 16, 20, 24, 28})
    ), 32);
    set_bit_by_position(packed, oddparity32(
      get_nonlinear_field(packed, 8, (uint8_t[]){1, 5, 9, 13, 17, 21, 25, 29})
    ), 33);
    set_bit_by_position(packed, evenparity32(
      get_nonlinear_field(packed, 8, (uint8_t[]){2, 6, 10, 14, 18, 22, 28, 30})
    ), 34);
    set_bit_by_position(packed, evenparity32(
      get_nonlinear_field(packed, 8, (uint8_t[]){3, 7, 11, 15, 19, 23, 29, 31})
    ), 35);
  }
  return add_HID_header(packed);
}
bool Unpack_H10320(/*in*/hidproxmessage_t* packed, /*out*/hidproxcard_t* card){
  memset(card, 0, sizeof(hidproxcard_t));
  if (packed->Length != 36) return false; // Wrong length? Stop here.
  // This card is BCD-encoded rather than binary. Get the 4-bit groups independently.
  card->CardNumber += get_linear_field(packed, 0, 4) * 10000000;
  card->CardNumber += get_linear_field(packed, 4, 4) * 1000000;
  card->CardNumber += get_linear_field(packed, 8, 4) * 100000;
  card->CardNumber += get_linear_field(packed, 12, 4) * 10000;
  card->CardNumber += get_linear_field(packed, 16, 4) * 1000;
  card->CardNumber += get_linear_field(packed, 20, 4) * 100;
  card->CardNumber += get_linear_field(packed, 24, 4) * 10;
  card->CardNumber += get_linear_field(packed, 28, 4);
  card->ParitySupported = true;
  card->ParityValid =
    (get_bit_by_position(packed, 32) == evenparity32(get_nonlinear_field(packed, 8, (uint8_t[]){0, 4, 8, 12, 16, 20, 24, 28}))) &&
    (get_bit_by_position(packed, 33) ==  oddparity32(get_nonlinear_field(packed, 8, (uint8_t[]){1, 5, 9, 13, 17, 21, 25, 29}))) &&
    (get_bit_by_position(packed, 34) == evenparity32(get_nonlinear_field(packed, 8, (uint8_t[]){2, 6, 10, 14, 18, 22, 28, 30}))) &&
    (get_bit_by_position(packed, 35) == evenparity32(get_nonlinear_field(packed, 8, (uint8_t[]){3, 7, 11, 15, 19, 23, 29, 31})));
  return true;
}


bool Pack_H10302(/*in*/hidproxcard_t* card, /*out*/hidproxmessage_t* packed){
  memset(packed, 0, sizeof(hidproxmessage_t));
  if (card->FacilityCode > 0) return false; // Can't encode FC. (none in this format)
  if (card->CardNumber > 0x00000007FFFFFFFF) return false; // Can't encode CN.
  
  packed->Length = 37; // Set number of bits
  set_linear_field(packed, card->CardNumber, 1, 35);
  if (card->ParitySupported){
    set_bit_by_position(packed,
      evenparity32(get_linear_field(packed, 1, 18))
    , 0);
    set_bit_by_position(packed, 
      oddparity32(get_linear_field(packed, 18, 18))
    , 36);
  }
  return add_HID_header(packed);
}
bool Unpack_H10302(/*in*/hidproxmessage_t* packed, /*out*/hidproxcard_t* card){
  memset(card, 0, sizeof(hidproxcard_t));
  if (packed->Length != 37) return false; // Wrong length? Stop here.
  
  card->CardNumber = get_linear_field(packed, 1, 35);
  card->ParitySupported = true;
  card->ParityValid =
    (get_bit_by_position(packed, 0) == evenparity32(get_linear_field(packed, 1, 18))) &&
    (get_bit_by_position(packed, 36) == oddparity32(get_linear_field(packed, 18, 18)));
    
  return true;
}

bool Pack_H10304(/*in*/hidproxcard_t* card, /*out*/hidproxmessage_t* packed){
  memset(packed, 0, sizeof(hidproxmessage_t));
  if (card->FacilityCode > 0x0000FFFF) return false; // Can't encode FC.
  if (card->CardNumber > 0x0007FFFF) return false; // Can't encode CN.
  
  packed->Length = 37; // Set number of bits
  packed->bot |= (card->CardNumber & 0x0007FFFF) << 1;
  packed->bot |= (card->FacilityCode & 0x00000FFF) << 20;
  packed->mid |= (card->FacilityCode & 0x0000F000) >> 12;
  if (card->ParitySupported){
    packed->mid |= (evenparity32((packed->mid & 0x0000000F) ^ (packed->bot & 0xFFFC0000)) & 1) << 4;
    packed->bot |= ( oddparity32(packed->bot & 0x0007FFFE) & 1);
  }
  return add_HID_header(packed);
}
bool Unpack_H10304(/*in*/hidproxmessage_t* packed, /*out*/hidproxcard_t* card){
  memset(card, 0, sizeof(hidproxcard_t));
  if (packed->Length != 37) return false; // Wrong length? Stop here.
  
  card->CardNumber = (packed->bot >> 1) & 0x0007FFFF;
  card->FacilityCode = ((packed->mid & 0xF) << 12) | ((packed->bot >> 20));
  card->ParitySupported = true;
  card->ParityValid =
    (evenparity32((packed->mid & 0x0000000F) ^ (packed->bot & 0xFFFC0000)) == ((packed->mid >> 4) & 1)) &&
    (oddparity32( packed->bot & 0x0007FFFE) == (packed->bot & 1));
  return true;
}


bool Pack_P10001(/*in*/hidproxcard_t* card, /*out*/hidproxmessage_t* packed){
  memset(packed, 0, sizeof(hidproxmessage_t));
  if (card->FacilityCode > 0xFFF) return false; // Can't encode FC.
  if (card->CardNumber > 0xFFFF) return false; // Can't encode CN.
  
  packed->Length = 40; // Set number of bits
  set_linear_field(packed, 0xF, 0, 4);
  set_linear_field(packed, card->FacilityCode, 4, 12);
  set_linear_field(packed, card->CardNumber, 16, 16);
  
  if (card->ParitySupported){
    set_linear_field(packed, 
      get_linear_field(packed, 0, 8) ^
      get_linear_field(packed, 8, 8) ^
      get_linear_field(packed, 16, 8) ^
      get_linear_field(packed, 24, 8)
    , 32, 8);
  }
  return add_HID_header(packed);
}
bool Unpack_P10001(/*in*/hidproxmessage_t* packed, /*out*/hidproxcard_t* card){
  memset(card, 0, sizeof(hidproxcard_t));
  if (packed->Length != 40) return false; // Wrong length? Stop here.
  
  card->CardNumber = get_linear_field(packed, 16, 16);
  card->FacilityCode = get_linear_field(packed, 4, 12);
  card->ParitySupported = true;
  card->ParityValid = (
    get_linear_field(packed, 0, 8) ^
    get_linear_field(packed, 8, 8) ^
    get_linear_field(packed, 16, 8) ^
    get_linear_field(packed, 24, 8)
  ) == get_linear_field(packed, 32, 8);
  return true;
}


bool Pack_C1k48s(/*in*/hidproxcard_t* card, /*out*/hidproxmessage_t* packed){
  memset(packed, 0, sizeof(hidproxmessage_t));
  if (card->FacilityCode > 0x003FFFFF) return false; // Can't encode FC.
  if (card->CardNumber > 0x007FFFFF) return false; // Can't encode CN.
  
  packed->Length = 48; // Set number of bits
  packed->bot |= (card->CardNumber & 0x007FFFFF) << 1;
  packed->bot |= (card->FacilityCode & 0x000000FF) << 24;
  packed->mid |= (card->FacilityCode & 0x003FFF00) >> 8;
  if (card->ParitySupported){
    packed->mid |= (evenparity32((packed->mid & 0x00001B6D) ^ (packed->bot & 0xB6DB6DB6)) & 1) << 14;
    packed->bot |= ( oddparity32((packed->mid & 0x000036DB) ^ (packed->bot & 0x6DB6DB6C)) & 1);
    packed->mid |= ( oddparity32((packed->mid & 0x00007FFF) ^ (packed->bot & 0xFFFFFFFF)) & 1) << 15;
  }
  return add_HID_header(packed);
}
bool Unpack_C1k48s(/*in*/hidproxmessage_t* packed, /*out*/hidproxcard_t* card){
  memset(card, 0, sizeof(hidproxcard_t));
  if (packed->Length != 48) return false; // Wrong length? Stop here.
  
  card->CardNumber = (packed->bot >> 1) & 0x007FFFFF;
  card->FacilityCode = ((packed->mid & 0x00003FFF) << 8) | ((packed->bot >> 24));
  card->ParitySupported = true;
  card->ParityValid =
    (evenparity32((packed->mid & 0x00001B6D) ^ (packed->bot & 0xB6DB6DB6)) == ((packed->mid >> 14) & 1)) &&
    ( oddparity32((packed->mid & 0x000036DB) ^ (packed->bot & 0x6DB6DB6C)) == ((packed->bot >> 0) & 1)) &&
    ( oddparity32((packed->mid & 0x00007FFF) ^ (packed->bot & 0xFFFFFFFF)) == ((packed->mid >> 15) & 1));
  return true;
}

static const hidcardformat_t FormatTable[] = {
    {"H10301", Pack_H10301, Unpack_H10301, "HID H10301 26-bit"}, // imported from old pack/unpack
    {"Tecom27", Pack_Tecom27, Unpack_Tecom27, "Tecom 27-bit"}, // from cardinfo.barkweb.com.au
    {"2804W", Pack_2804W, Unpack_2804W, "2804 Wiegand"}, // from cardinfo.barkweb.com.au
    {"ATSW30", Pack_ATSW30, Unpack_ATSW30, "ATS Wiegand 30-bit"}, // from cardinfo.barkweb.com.au
    {"ADT31", Pack_ADT31, Unpack_ADT31, "HID ADT 31-bit"}, // from cardinfo.barkweb.com.au
    {"D10202", Pack_D10202, Unpack_D10202, "HID D10202 33-bit"}, // from cardinfo.barkweb.com.au
    {"H10306", Pack_H10306, Unpack_H10306, "HID H10306 34-bit"}, // imported from old pack/unpack
    {"N1002", Pack_N1002, Unpack_N1002, "HID N1002 34-bit"}, // from cardinfo.barkweb.com.au
    {"C1k35s", Pack_C1k35s, Unpack_C1k35s, "HID Corporate 1000 35-bit standard layout"}, // imported from old pack/unpack
    {"H10320", Pack_H10320, Unpack_H10320, "HID H10320 36-bit BCD, Card num only"}, // from Proxmark forums
    {"H10302", Pack_H10302, Unpack_H10302, "HID H10302 37-bit huge, Card num only"}, // from Proxmark forums
    {"H10304", Pack_H10304, Unpack_H10304, "HID H10304 37-bit"}, // imported from old pack/unpack
    {"P10001", Pack_P10001, Unpack_P10001, "HID P10001 Honeywell 40-bit"}, // from cardinfo.barkweb.com.au
    {"C1k48s", Pack_C1k48s, Unpack_C1k48s, "HID Corporate 1000 48-bit standard layout"}, // imported from old pack/unpack
    {NULL, NULL, NULL, NULL} // Must null terminate array
};

void HIDListFormats(){
  if (FormatTable[0].Name == NULL)
    return;
  int i = 0;
  PrintAndLog("%-10s %s", "Name", "Description");
  PrintAndLog("------------------------------------------------------------");
  while (FormatTable[i].Name)
  {
    PrintAndLog("%-10s %s", FormatTable[i].Name, FormatTable[i].Descrp);
    ++i;
  }
  PrintAndLog("");
  return;
}

hidcardformat_t HIDGetCardFormat(int idx){
  return FormatTable[idx];
}

int HIDFindCardFormat(const char *format)
{
  if (FormatTable[0].Name == NULL) 
    return -1;
  int i = 0;
  while (FormatTable[i].Name && strcmp(FormatTable[i].Name, format))
  {
    ++i;
  }

	if (FormatTable[i].Name) {
		return i;
	} else {
    return -1;
	}
}

bool HIDPack(/* in */int FormatIndex, /* in */hidproxcard_t* card, /* out */hidproxmessage_t* packed){
  memset(packed, 0, sizeof(hidproxmessage_t));
  if (FormatIndex < 0 || FormatIndex >= (sizeof(FormatTable) / sizeof(FormatTable[0]))) return false;
  return FormatTable[FormatIndex].Pack(card, packed);
}

bool HIDTryUnpack(/* in */hidproxmessage_t* packed){
  if (FormatTable[0].Name == NULL) 
    return false;
    
  bool result = false;
  int i = 0;
  hidproxcard_t card;
  memset(&card, 0, sizeof(hidproxcard_t));
  while (FormatTable[i].Name)
  {
    if (FormatTable[i].Unpack(packed, &card)){
      result = true;
      PrintAndLog("%-16s FC: %u, Card %"PRIu64", Parity %s", 
        FormatTable[i].Name, 
        card.FacilityCode, 
        card.CardNumber, 
        (card.ParitySupported) ? ((card.ParityValid) ? "valid" : "invalid") : "n/a"
      );
    }
    ++i;
  }
  return result;
}
