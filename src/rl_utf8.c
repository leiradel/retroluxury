#include <rl_utf8.h>

#include <stdint.h>

/* Copied from PHYSFS. */
int rl_utf8_decode( const char** utf8 )
{
  const uint8_t* str = (const uint8_t*)*utf8;
  uint32_t retval = 0;
  uint32_t octet = *str;
  uint32_t octet2, octet3, octet4;

  if ( octet == 0 )
  {
    /* null terminator, end of string. */
    return 0;
  }
  else if ( octet < 128 )
  {
    /* one octet char: 0 to 127 */
    ( *utf8 )++;  /* skip to next possible start of codepoint. */
    return octet;
  }
  else if ( ( octet > 127 ) && ( octet < 192 ) )
  {
    /* bad (starts with 10xxxxxx). */

    /*
     * Apparently each of these is supposed to be flagged as a bogus
     *  char, instead of just resyncing to the next valid codepoint.
     */
    ( *utf8 )++; /* skip to next possible start of codepoint. */
    return RL_UNICODE_UNKNOWN;
  }
  else if ( octet < 224 )
  {
    /* two octets */
    ( *utf8 )++; /* advance at least one byte in case of an error */
    octet -= 128 + 64;
    octet2 = *++str;

    if ( ( octet2 & ( 128 + 64 ) ) != 128 )
    {
      /* Format isn't 10xxxxxx? */
      return RL_UNICODE_UNKNOWN;
    }

    ( *utf8 )++; /* skip to next possible start of codepoint. */
    retval = ( ( octet << 6 ) | ( octet2 - 128 ) );

    if ( ( retval >= 0x80 ) && ( retval <= 0x7FF ) )
    {
      return retval;
    }
  }
  else if ( octet < 240 )
  {
    /* three octets */
    ( *utf8 )++; /* advance at least one byte in case of an error */
    octet -= 128 + 64 + 32;
    octet2 = *++str;

    if ( ( octet2 & ( 128 + 64 ) ) != 128 )
    {
      /* Format isn't 10xxxxxx? */
      return RL_UNICODE_UNKNOWN;
    }

    octet3 = *++str;

    if ( ( octet3 & ( 128 + 64 ) ) != 128 )
    {
      /* Format isn't 10xxxxxx? */
      return RL_UNICODE_UNKNOWN;
    }

    *utf8 += 2; /* skip to next possible start of codepoint. */
    retval = ( ( ( octet << 12 ) ) | ( ( octet2 - 128 ) << 6 ) | ( ( octet3 - 128 ) ) );

    /* There are seven "UTF-16 surrogates" that are illegal in UTF-8. */
    switch ( retval )
    {
    case 0xd800:
    case 0xdb7f:
    case 0xdb80:
    case 0xdbff:
    case 0xdc00:
    case 0xdf80:
    case 0xdfff:
      return RL_UNICODE_UNKNOWN;
    }

    /* 0xFFFE and 0xFFFF are illegal, too, so we check them at the edge. */
    if ( ( retval >= 0x0800 ) && ( retval <= 0xfffd ) )
    {
      return retval;
    }
  }
  else if ( octet < 248 )
  {
    /* four octets */
    (* utf8 )++; /* advance at least one byte in case of an error */
    octet -= 128 + 64 + 32 + 16;
    octet2 = *++str;

    if ( ( octet2 & ( 128 + 64 ) ) != 128 )
    {
      /* Format isn't 10xxxxxx? */
      return RL_UNICODE_UNKNOWN;
    }

    octet3 = *++str;

    if ( ( octet3 & ( 128 + 64 ) ) != 128 )
    {
      /* Format isn't 10xxxxxx? */
      return RL_UNICODE_UNKNOWN;
    }

    octet4 = *++str;

    if ( ( octet4 & ( 128 + 64 ) ) != 128 )
    {
      /* Format isn't 10xxxxxx? */
      return RL_UNICODE_UNKNOWN;
    }

    *utf8 += 3; /* skip to next possible start of codepoint. */
    retval = ( ( ( octet << 18 ) ) | ( ( octet2 - 128 ) << 12 ) |
      ( ( octet3 - 128 ) << 6 ) | ( ( octet4 - 128 ) ) );
    
    if ( ( retval >= 0x10000 ) && ( retval <= 0x10ffff ) )
    {
      return retval;
    }
  }
  /*
   * Five and six octet sequences became illegal in rfc3629.
   *  We throw the codepoint away, but parse them to make sure we move
   *  ahead the right number of bytes and don't overflow the buffer.
   */
  else if ( octet < 252 )
  {
    /* five octets */
    ( *utf8 )++; /* advance at least one byte in case of an error */
    octet = *++str;

    if ( ( octet & ( 128 + 64 ) ) != 128 )
    {
      /* Format isn't 10xxxxxx? */
      return RL_UNICODE_UNKNOWN;
    }

    octet = *++str;

    if ( ( octet & ( 128 + 64 ) ) != 128 )
    {
      /* Format isn't 10xxxxxx? */
      return RL_UNICODE_UNKNOWN;
    }

    octet = *++str;

    if ( ( octet & ( 128 + 64 ) ) != 128 )
    {
      /* Format isn't 10xxxxxx? */
      return RL_UNICODE_UNKNOWN;
    }

    octet = *++str;

    if ( ( octet & ( 128 + 64 ) ) != 128 )
    {
      /* Format isn't 10xxxxxx? */
      return RL_UNICODE_UNKNOWN;
    }

    *utf8 += 4;  /* skip to next possible start of codepoint. */
    return RL_UNICODE_UNKNOWN;
  }
  else /* six octets */
  {
    ( *utf8 )++; /* advance at least one byte in case of an error */
    octet = *++str;

    if ( ( octet & ( 128 + 64 ) ) != 128 )
    {
      /* Format isn't 10xxxxxx? */
      return RL_UNICODE_UNKNOWN;
    }

    octet = *++str;

    if ( ( octet & ( 128 + 64 ) ) != 128 )
    {
      /* Format isn't 10xxxxxx? */
      return RL_UNICODE_UNKNOWN;
    }

    octet = *++str;

    if ( ( octet & ( 128 + 64 ) ) != 128 )
    {
      /* Format isn't 10xxxxxx? */
      return RL_UNICODE_UNKNOWN;
    }

    octet = *++str;

    if ( ( octet & ( 128 + 64 ) ) != 128 )
    {
      /* Format isn't 10xxxxxx? */
      return RL_UNICODE_UNKNOWN;
    }

    octet = *++str;

    if ( ( octet & ( 128 + 64 ) ) != 128 )
    {
      /* Format isn't 10xxxxxx? */
      return RL_UNICODE_UNKNOWN;
    }

    *utf8 += 6; /* skip to next possible start of codepoint. */
    return RL_UNICODE_UNKNOWN;
  }

  return RL_UNICODE_UNKNOWN;
}
