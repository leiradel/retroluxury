#include <rl_bdffont.h>
#include <rl_utf8.h>

#include <string.h>
#include <ctype.h>

#include <physfs.h>

#include <rl_hash.inl>

#define SPACE " \f\r\t\v"
#define DIGIT "0123456789"
#define ALPHA "ABCDEFGHIJKLMNOPQRSTUVWXYZ_"
#define ALNUM ALPHA DIGIT
#define HEXDG DIGIT "abcdefABCDEF"

#define BBX             0x0b87d481U
#define SIZE            0x7c8bd560U
#define SWIDTH          0xcfcab8d8U
#define ENDFONT         0x7e1a9db3U
#define ENCODING        0x3fb5ea0cU
#define STARTFONT       0x8cf73faaU
#define METRICSSET      0xaf5b3888U
#define DWIDTH          0xaccd5769U
#define ENDPROPERTIES   0x58fbd469U
#define STARTPROPERTIES 0xc27dd3a0U
#define BITMAP          0xa72bdb22U
#define DWIDTH1         0x467844baU
#define FONT            0x7c84cc7cU
#define ENDCHAR         0x7e18d91aU
#define STARTCHAR       0x8cf57b11U
#define CHARS           0x0ce40496U
#define COMMENT         0xe721d578U
#define FONTBOUNDINGBOX 0x3344af7bU
#define CONTENTVERSION  0x56774aa6U
#define VVECTOR         0xad72744eU

#define XVAL( x ) ( isdigit( x ) ? ( x ) - '0' : toupper( x ) - 'A' + 10 )

/*
"Lines may be of unlimited length."

"In this version, the new maximum length of a value of the type string is 65535
characters, and hence lines may now be at least this long."

https://wwwimages2.adobe.com/content/dam/acom/en/devnet/font/pdfs/5005.BDF_Spec.pdf

We limit a line at MAXLEN characters at most. If a line is greater than MAXLEN,
it's likely that the font is too big for use with this engine anyway.
*/
#define MAXLEN 1024

static inline void skip_spaces( const char** source )
{
  *source += strspn( *source, SPACE );
}

static const char* next_line( char* line, int* len, PHYSFS_File* file )
{
  int end = 0;

  if ( *len != 0 )
  {
    memmove( line, line + *len + 1, MAXLEN - *len - 1 );
    end = MAXLEN - *len - 2;
  }

  PHYSFS_sint64 num_read = PHYSFS_readBytes( file, line + end, MAXLEN - end - 1 );

  if ( num_read == -1 )
  {
    num_read = 0;
  }

  line[ num_read + end ] = 0;
  char* newline = strchr( line, '\n' );

  if ( newline != NULL )
  {
    *newline = 0;
    *len = newline - line;

    if ( newline > line && newline[ -1 ] == '\r' )
    {
      newline[ -1 ] = 0;
    }
  }
  else
  {
    *len = MAXLEN - 1;
  }

  return line;
}

/* Parse an integer updating the pointer. */
static int readint( const char** source, int* x )
{
  const char* aux = *source;
  int val = 0, sig = 1;
  size_t length;

  skip_spaces( &aux );

  /* Adobe's spec doesn't say numbers can be preceeded by '+' but we never know. */
  if ( *aux == '-' )
  {
    sig = -1;
    aux++;
  }
  else if ( *aux == '+' )
  {
    aux++;
  }

  /* Skip spaces between signal and first digit. */
  skip_spaces( &aux );
  length = strspn( aux, DIGIT );

  if ( length == 0 )
  {
    *source = aux;
    return -1;
  }

  /* Now we start reading digits. */
  do
  {
    val = val * 10 + *aux++ - '0';
  }
  while ( --length != 0 );

  /* We're done, update pointer and return value. */
  if ( x != NULL )
  {
    *x = sig * val;
  }

  *source = aux;
  return 0;
}

static int readint2( const char** source, int* x, int* y )
{
  if ( readint( source, x ) != 0 )
  {
    return -1;
  }

  return readint( source, y );
}

static int readint3( const char** source, int* x, int* y, int* z )
{
  if ( readint2( source, x, y ) != 0 )
  {
    return -1;
  }

  return readint( source, z );
}

static int readint4( const char** source, int* x, int* y, int* z, int* w )
{
  if ( readint3( source, x, y, z ) != 0 )
  {
    return -1;
  }

  return readint( source, w );
}

/* Compare function to sort characters by their codes. */
static int compare( const void* e1, const void* e2 )
{
  const rl_bdffontchar_t* c1 = (const rl_bdffontchar_t*)e1;
  const rl_bdffontchar_t* c2 = (const rl_bdffontchar_t*)e2;

  if ( c1->code < c2->code )
  {
    return -1;
  }
  else if ( c1->code > c2->code )
  {
    return 1;
  }

  return 0;
}

static int pass_all( int encoding, int glyph_index, void* userdata ) {
  (void)encoding;
  (void)glyph_index;
  (void)userdata;
  /* TODO: validate this assumption. */
  return encoding != -1 ? encoding : glyph_index;
}

int rl_bdffont_create( rl_bdffont_t* bdffont, const char* path )
{
  return rl_bdffont_create_filter( bdffont, path, pass_all, NULL );
}

int rl_bdffont_create_filter( rl_bdffont_t* bdffont, const char* path, rl_bdffont_filter_t filter, void* userdata )
{
  rl_bdffontchar_t* chr = NULL;
  size_t length;
  uint32_t hash;
  uint8_t* bits;
  int num_chars = 0, add = 0;
  int dwx0 = 0, dwy0 = 0;
  /*int dwx1, dwy1;*/
  int bbw = 0, bbh = 0, bbxoff0x = 0, bbyoff0y = 0;
  int i, j, chr_bbh = 0;
  char line[ MAXLEN ];

  PHYSFS_File* file = PHYSFS_openRead( path );

  if ( file == NULL )
  {
    return -1;
  }

  PHYSFS_sint64 bytes = PHYSFS_fileLength( file );

  if ( bytes == -1 )
  {
    PHYSFS_close( file );
    return -1;
  }

  int len = 0;
  const char* source = next_line( line, &len, file );

  for ( ;; )
  {
    /* Find end of keyword. */
    if ( strspn( source, ALPHA ) == 0 ) goto error;

    length = strspn( source, ALNUM );
    hash = djb2_length( source, length );
    source += length;

    switch ( hash )
    {
    /* Starts a font. */
    case STARTFONT:
      if ( readint( &source, &i ) != 0 ) goto error;
      if ( *source++ != '.' ) goto error;
      if ( readint( &source, &j ) != 0 ) goto error;

      /* Issue an error on versions higher than 2.2. */
      if ( i > 2 || ( i == 2 && j > 2 ) ) goto error;

      source = next_line( line, &len, file );
      break;

    /* The FONTBOUNDINGBOX values seems to be defaults for BBX values. */
    case FONTBOUNDINGBOX:
      if ( readint4( &source, &bbw, &bbh, &bbxoff0x, &bbyoff0y ) != 0 ) goto error;

      source = next_line( line, &len, file );
      break;

    case METRICSSET:
      if ( readint( &source, &bdffont->metrics_set) != 0 ) goto error;

      /* We only handle horizontal writing by now. */
      if ( bdffont->metrics_set != 0 ) goto error;

      source = next_line( line, &len, file );
      break;

    /* This is the character's width in pixels. */
    case DWIDTH:
      if ( readint2( &source, &i, &j ) != 0 ) goto error;

      if ( chr != NULL )
      {
        if ( add )
        {
          chr->dwx0 = i; chr->dwy0 = j;
        }
      }
      else
      {
        dwx0 = i; dwy0 = j;
      }

      source = next_line( line, &len, file );
      break;

    case CHARS:
      /* Read the number of chars in this font and malloc the required memory. */
      if ( readint( &source, &bdffont->num_chars) != 0 ) goto error;

      bdffont->chars = (rl_bdffontchar_t*)calloc( bdffont->num_chars, sizeof( rl_bdffontchar_t ) );

      if ( bdffont->chars == NULL ) goto error;

      source = next_line( line, &len, file );
      break;

    case STARTCHAR:
      /* If chr is not NULL the last character was not properly ended. */
      if ( chr != NULL ) goto error;

      /* Bounds check. */
      if ( num_chars == bdffont->num_chars ) goto error;

      chr = bdffont->chars + num_chars;

      /* Copy default values. */
      chr->code = -1;
      chr->dwx0 = dwx0;
      chr->dwy0 = dwy0;
      /*chr->dwx1 = dwx1;*/
      /*chr->dwy1 = dwy1;*/
      chr->bbw = bbw;
      chr->bbh = bbh;
      chr->bbxoff0x = bbxoff0x;
      chr->bbyoff0y = bbyoff0y;

      source = next_line( line, &len, file );
      break;

    case ENCODING:
      /* If chr is NULL the character was not properly started. */
      if ( chr == NULL ) goto error;

      /* Read character's code, it can be -1. */
      if ( readint( &source, &i ) != 0 ) goto error;

      /* If the encoding is -1, try to read another integer. */
      if ( i == -1 )
      {
        if ( readint( &source, &j ) != 0 )
        {
          j = -1;
        }
      }
      else
      {
        j = -1;
      }

      add = filter( i, j, userdata );

      if ( add != -1 )
      {
        chr->code = add;
        num_chars++;
        add = 1;
      }
      else
      {
        add = 0;
      }

      source = next_line( line, &len, file );
      break;

    /* The bounding box around the character's black pixels. */
    case BBX:
      /* If chr is NULL the character was not properly started. */
      if ( chr == NULL ) goto error;

      /* Only process the character if it was not filtered out. */
      if ( add )
      {
        if ( readint4( &source, &chr->bbw, &chr->bbh, &chr->bbxoff0x, &chr->bbyoff0y ) != 0 ) goto error;
      }
      else
      {
        /* Save the character's bbh so we can skip the BITMAP section later. */
        if ( readint4( &source, &i, &chr_bbh, &i, &i ) != 0 ) goto error;
      }

      source = next_line( line, &len, file );
      break;

    /* BITMAP signals the start of the hex data. */
    case BITMAP:
      /* If chr is NULL the character was not properly started. */
      if ( chr == NULL ) goto error;

      source = next_line( line, &len, file );

      /* Only process the character if it was not filtered out. */
      if ( add )
      {
        /* wbytes is the width of the char in bytes. */
        chr->wbytes = (chr->bbw + 7) / 8;

        /* Malloc the memory for the pixels. */
        chr->bits = bits = (uint8_t*)malloc( chr->wbytes * chr->bbh );

        if ( bits == NULL ) goto error;

        /* Read all pixels from file. */
        for ( i = chr->bbh; i != 0; i-- )
        {
          length = strspn( source, HEXDG );

          if ( length != chr->wbytes * 2 ) goto error;

          while ( length != 0 )
          {
            *bits++ = XVAL( source[ 0 ] ) * 16 + XVAL( source[ 1 ] );
            source += 2;
            length -= 2;
          }

          source = next_line( line, &len, file );
        }
      }
      else
      {
        /* Skip the bitmap. */
        for ( i = chr_bbh; i != 0; i-- )
        {
          source = next_line( line, &len, file );
        }
      }

      break;

    case ENDCHAR:
      /* If chr is NULL the character was not properly started. */
      if ( chr == NULL ) goto error;

      chr = NULL;
      source = next_line( line, &len, file );
      break;

    case ENDFONT:
      /* If chr is not NULL the last character was not properly ended. */
      if ( chr != NULL ) goto error;

      if ( num_chars < bdffont->num_chars )
      {
        bdffont->chars = realloc( bdffont->chars, num_chars * sizeof( rl_bdffontchar_t ) );
        bdffont->num_chars = num_chars;
      }

      /* Sort font by character codes (TODO: should be an hash table). */
      qsort( bdffont->chars, bdffont->num_chars, sizeof( rl_bdffontchar_t ), compare );
      PHYSFS_close( file );
      return 0;
    
    default:
      /* Unknown section, skip. */
      source = next_line( line, &len, file );
      break;
    }
  }

  error:
  /* Free everything. */
  rl_bdffont_destroy( bdffont );
  PHYSFS_close( file );
  return -1;
}

void rl_bdffont_destroy( const rl_bdffont_t* bdffont )
{
  int  i;
  rl_bdffontchar_t* chr;

  /* Free everything. */
  if ( bdffont->chars != NULL )
  {
    for ( i = bdffont->num_chars, chr = bdffont->chars; i != 0; i--, chr++ )
    {
      free( (void*)chr->bits );
    }

    free( bdffont->chars );
  }
}

/* Finds a char in the font. */
static inline const rl_bdffontchar_t* find_char( const rl_bdffont_t* bdffont, int code )
{
  rl_bdffontchar_t key;
  key.code = code;

  return (rl_bdffontchar_t*)bsearch( &key, bdffont->chars, bdffont->num_chars, sizeof(rl_bdffontchar_t), compare );
}

void rl_bdffont_size( const rl_bdffont_t* bdffont, int* x0, int* y0, int* width, int* height, const char* text )
{
  const rl_bdffontchar_t* chr;
  int code, y, h, minh = 0, maxh = 0;

  *x0 = *y0 = -1;
  *width = *height = -1;
  y = 0;

  for ( ;; )
  {
    code = rl_utf8_decode( &text );

    if ( code == 0 )
    {
      goto end;
    }

    chr = find_char( bdffont, code );

    if ( chr != NULL )
    {
      *x0 = *width = -chr->bbxoff0x;

      h = y - ( chr->bbyoff0y + chr->bbh );
      minh = h;

      h += chr->bbh - 1;
      maxh = h;

      *y0 = chr->bbyoff0y;
      *width += chr->dwx0;
      y += chr->dwy0;
      break;
    }
  }

  for ( ;; )
  {
    code = rl_utf8_decode( &text );

    if ( code == 0 )
    {
      goto end;
    }

    chr = find_char( bdffont, code );

    if ( chr != NULL )
    {
      h = y - ( chr->bbyoff0y + chr->bbh );

      if ( h < minh )
      {
        minh = h;
      }

      h += chr->bbh - 1;

      if ( h > maxh )
      {
        maxh = h;
      }

      if ( chr->bbyoff0y < *y0 )
      {
        *y0 = chr->bbyoff0y;
      }

      *width += chr->dwx0;
      y += chr->dwy0;
    }
  }

end:
  *height = maxh - minh + 1;
  *y0 += *height;
}

static void draw_char( uint32_t* pixel, int pitch, const rl_bdffontchar_t* chr, uint32_t color )
{
  const uint8_t* bits = chr->bits;

  for ( const uint8_t* endfont = bits + chr->wbytes * chr->bbh; bits < endfont; )
  {
    uint32_t* save = pixel;

    for ( const uint8_t* endline = bits + chr->wbytes; bits < endline; pixel += 8, bits++ )
    {
      if ( *bits & 0x80 ) pixel[ 0 ] = color;
      if ( *bits & 0x40 ) pixel[ 1 ] = color;
      if ( *bits & 0x20 ) pixel[ 2 ] = color;
      if ( *bits & 0x10 ) pixel[ 3 ] = color;
      if ( *bits & 0x08 ) pixel[ 4 ] = color;
      if ( *bits & 0x04 ) pixel[ 5 ] = color;
      if ( *bits & 0x02 ) pixel[ 6 ] = color;
      if ( *bits & 0x01 ) pixel[ 7 ] = color;
    }

    pixel = save + pitch;
  }
}

int rl_bdffont_render( rl_pixelsrc_t* pixelsrc, const rl_bdffont_t* bdffont, int* x0, int* y0, const char* text, uint32_t bg_color, uint32_t fg_color )
{
  rl_bdffont_size( bdffont, x0, y0, &pixelsrc->width, &pixelsrc->height, text );
  pixelsrc->pitch = pixelsrc->width;
  pixelsrc->parent = NULL;

  uint32_t* pixel;
  size_t count = pixelsrc->width * pixelsrc->height;
  pixelsrc->abgr = pixel = (uint32_t*)malloc( count * sizeof( uint32_t ) );

  if ( pixel == NULL )
  {
    return -1;
  }

  for ( uint32_t* clear = pixel; count != 0; clear++, count-- )
  {
    *clear = bg_color;
  }

  for ( ;; )
  {
    int code = rl_utf8_decode( &text );

    if ( code == 0 )
    {
      return 0;
    }

    const rl_bdffontchar_t* chr = find_char( bdffont, code );

    if ( chr != NULL )
    {
      draw_char( pixel, pixelsrc->pitch, chr, fg_color );
      pixel += chr->dwx0 + chr->dwy0 * pixelsrc->pitch;
    }
  }
}
