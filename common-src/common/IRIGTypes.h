#ifndef IRIGTypes_H
#define IRIGTypes_H

/**
 * Can't use bit fields for things that cross byte boundaries, so
 * we break things down into nibbles, where a nibble maps to a binary
 * coded decimal digit.<br>
 * Little-endian (_X86_) architectures swap nibbles, so they're 
 * conditionally mapped inverted here.
 * The general naming convention is 'field' suffixed by 'H' (hundreds)
 * 'T' (tens) or 'O' (ones).  
 * Nanoseconds have too many nibbles, so they're simply numbered from
 * 1 to 7 with the 1 being the left-most most significant nibble.
 * 
 * Stored in 8-bytes (DD DH HM MS SN NN NN NN)
 */
#pragma pack(push, 1)

static const double CLOCKWIDTH = 1e3 / 60; /* 60 MHz clock */

typedef struct
{
#if (CPU == PPC604)
   unsigned char dayH:4;
   unsigned char dayT:4;

   unsigned char dayO:4;
   unsigned char hourT:4;

   unsigned char hourO:4;
   unsigned char minT:4;

   unsigned char minO:4;
   unsigned char secT:4;

   unsigned char secO:4;
   unsigned char msecH:4;

   unsigned char msecT:4;
   unsigned char msecO:4;

   unsigned char usecH:4;
   unsigned char usecT:4;

   unsigned char usecO:4;
   unsigned char nsecH:4;
#else /* nibbles have to be swapped for X86 architectures */
   unsigned char dayT:4;
   unsigned char dayH:4;

   unsigned char hourT:4;
   unsigned char dayO:4;

   unsigned char minT:4;
   unsigned char hourO:4;

   unsigned char secT:4;
   unsigned char minO:4;

   unsigned char msecH:4;
   unsigned char secO:4;

   unsigned char msecO:4;
   unsigned char msecT:4;

   unsigned char usecT:4;
   unsigned char usecH:4;

   unsigned char nsecH:4;
   unsigned char usecO:4;
#endif
} UTCTimeType;
#pragma pack(pop)

#endif
