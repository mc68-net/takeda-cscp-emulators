/*
	NEC PC-6001 Emulator 'yaPC-6001'

	Author : tanam
	Date   : 2013.07.15-

	[ keyboard ]
*/

#include "keyboard.h"
#include "../i8255.h"

/** Keyboard bindings ****************************************/
/* normal (small alphabet) */
byte Keys1[256][2] =
{
  /*  0        1        2         3        4        5       6         7 */
/* 00 */
  /* */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /* BS      TAB                                ENTER                  */
  {0,0x7f},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x0d},{0,0x00},{0,0x00},

/* 10 */
  /* SHIFT   CTRL      ALT     PAUSE    CAPS     KANA                  */
  {0,0x00},{0,0x00},{0,0x00},{1,0xfa},{1,0xfb},{0,0x00},{0,0x00},{0,0x00},
  /*                           ESC                                     */
  {0,0x00},{0,0x00},{0,0x00},{0,0x1b},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 20 */
  /* SPC    PAGE UP PAGE DOWN    END    HOME      LEFT    UP      RIGHT */
  {0,0x20},{0,0xfe},{0,0xfe},{0,0xfe},{0,0x0c},{0,0x1d},{0,0x1e},{0,0x1c},
  /* DOWN                              PRN SCR   INSERT   DELETE  HELP  */
  {0,0x1f},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x12},{0,0x08},{0,0x00},

/* 30 */
  /* 0        1         2        3        4       5          6       7  */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /* 8        9                                                         */
  {0,0x38},{0,0x39},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 40 */
  /*          A         B        C        D       E         F       G   */
  {0,0x00},{0,0x61},{0,0x62},{0,0x63},{0,0x64},{0,0x65},{0,0x66},{0,0x67},
  /*   H      I         J        K        L       M         N       O   */
  {0,0x68},{0,0x69},{0,0x6a},{0,0x6b},{0,0x6c},{0,0x6d},{0,0x6e},{0,0x6f},

/* 50 */
  /*  P       Q         R        S        T       U         V       W   */
  {0,0x70},{0,0x71},{0,0x72},{0,0x73},{0,0x74},{0,0x75},{0,0x76},{0,0x77},
  /*  X       Y         Z        */
  {0,0x78},{0,0x79},{0,0x7a},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 60   10key*/
  /*  0       1         2        3        4       5         6       7   */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /*  8       9         *        +                -         .       /   */
  {0,0x38},{0,0x39},{0,0x2a},{0,0x2b},{0,0x00},{0,0x2d},{0,0x2e},{0,0x2f},

/* 70 */
  /* F1       F2       F3        F4      F5       F6       F7       F8  */
  {1,0xf0},{1,0xf1},{1,0xf2},{1,0xf3},{1,0xf4},{1,0x00},{0,0x00},{1,0xfd},
  /* F9       F10      F11       F12     F13      F14      F15      F16 */
  {0,0x03},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 80 */
  /* F17      F18      F19       F20     F21      F22      F23      F24 */
  {0,0x20},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 90 */
  /* NUM     SCROLL                                                     */
  {0,0x00},{1,0xfe},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* A0 */
  /*                       SCROLL LOCK                                  */
  {0,0x00},{0,0x00},{0,0x00},{1,0xfe},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*                    *       +        ,         -        .        /  */
  {0,0x00},{0,0x00},{0,0x2a},{0,0x2b},{0,0x2c},{0,0x2d},{0,0x2e},{0,0x2f},

/* B0 */
  /*                                                                    */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*                    :*       ;+       ,<       -        .>      /?  */
  {0,0x00},{0,0x00},{0,0x3A},{0,0x3b},{0,0x2c},{0,0x2d},{0,0x2e},{0,0x2f},

/* C0 */
  /*  @                                                                 */
  {0,0x40},{1,0x00},{1,0x00},{0,0x00},{0,0x00},{0,0x00},{1,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* D0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*                            [{        \|        ]}      ^           */
  {0,0x00},{0,0x00},{0,0x00},{0,0x5b},{0,0x5c},{0,0x5d},{0,0x5e},{0,0x00},

/* E0 */
  /*                     _                                              */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{1,0xfe},{1,0xfb},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
/* F0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0xba},{0,0xbb},
  {0,0xbc},{0,0xbd},{0,0xbe},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
};

/* normal (small alphabet) + shift */
byte Keys2[256][2] =
{
/* 00 */
  {0,0x35},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /* BS      TAB                                ENTER                  */
  {0,0x7f},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x0d},{0,0x00},{0,0x00},

/* 10 */
  /* SHIFT   CTRL      ALT     PAUSE    CAPS     KANA                  */
  {0,0x00},{0,0x00},{0,0x00},{1,0xfa},{1,0xfb},{0,0x00},{0,0x00},{0,0x00},
  /*                           ESC                                     */
  {0,0x00},{0,0x00},{0,0x00},{0,0x1b},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 20 */
  /* SPC    PAGE UP PAGE DOWN    END    HOME      LEFT    UP      RIGHT */
  {0,0x20},{1,0xfc},{1,0xfc},{0,0xfc},{0,0x0b},{0,0x1d},{0,0x1e},{0,0x1c},
  /* DOWN                              PRN SCR   INSERT   DELETE  HELP  */
  {0,0x1f},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x12},{0,0x08},{0,0x00},

/* 30 */
  /* 0        1         2        3        4       5          6       7  */
  {0,0x00},{0,0x21},{0,0x22},{0,0x23},{0,0x24},{0,0x25},{0,0x26},{0,0x27},
  /* 8        9                                                         */
  {0,0x28},{0,0x29},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 40 */
  /*          A         B        C        D       E         F       G   */
  {0,0x40},{0,0x41},{0,0x42},{0,0x43},{0,0x44},{0,0x45},{0,0x46},{0,0x47},
  /*   H      I         J        K        L       M         N       O   */
  {0,0x48},{0,0x49},{0,0x4a},{0,0x4b},{0,0x4c},{0,0x4d},{0,0x4e},{0,0x4f},

/* 50 */
  /*  P       Q         R        S        T       U         V       W   */
  {0,0x50},{0,0x51},{0,0x52},{0,0x53},{0,0x54},{0,0x55},{0,0x56},{0,0x57},
  /*  X       Y         Z        */
  {0,0x58},{0,0x59},{0,0x5a},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 60  10 key */
  /*  0       1         2        3        4       5         6       7   */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /*  8       9         *        +                -         .       /   */
  {0,0x38},{0,0x39},{0,0x2a},{0,0x2b},{0,0x00},{0,0x2d},{0,0x2e},{0,0x2f},

/* 70 */
  /* F1       F2       F3        F4      F5       F6       F7       F8  */
  {1,0xf5},{1,0xf6},{1,0xf7},{1,0xf8},{1,0xf9},{1,0x00},{0,0x00},{1,0xfd},
  {0,0x03},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 80 */
  /* F17      F18      F19       F20     F21      F22      F23      F24 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x0d},{0,0x00},{0,0x00},

/* 90 */
  /* NUM     SCROLL                                                     */
  {0,0x00},{1,0xfe},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* A0 */
  /*                       SCROLL LOCK                                  */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x2a},{0,0x2b},{0,0x2c},{0,0x2d},{0,0x2e},{0,0x2f},

/* B0 */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /*                    :*       ;+       ,<       -        .>      /?  */
  {0,0x38},{0,0x39},{0,0x2A},{0,0x2b},{0,0x3c},{0,0x3d},{0,0x3e},{0,0x3f},

/* C0 */
  {0,0x00},{1,0x00},{1,0x00},{0,0x00},{0,0x00},{0,0x00},{1,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* D0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*                            [{        \|        ]}      ^           */
  {0,0x00},{0,0x00},{0,0x00},{0,0x7B},{0,0x7C},{0,0x7D},{0,0x00},{0,0x00},

/* E0 */
  /*                     _                                              */
  {0,0x00},{0,0x00},{0,0x5f},{0,0x00},{1,0xfe},{1,0xfb},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
/* F0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0xba},{0,0xbb},
  {0,0xbc},{0,0xbd},{0,0xbe},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
};

/* hiragana */
byte Keys3[256][2] =
{
/* 00 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /* BS      TAB                                ENTER                  */
  {0,0x7f},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x0d},{0,0x00},{0,0x00},

/* 10 */
  /* SHIFT   CTRL      ALT     PAUSE    CAPS     KANA                  */
  {0,0x00},{0,0x00},{0,0x00},{1,0xfa},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*                           ESC                                     */
  {0,0x00},{0,0x00},{0,0x00},{0,0x1b},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 20 */
  /* SPC    PAGE UP PAGE DOWN    END    HOME      LEFT    UP      RIGHT */
  {0,0x20},{0,0xfe},{0,0xfe},{0,0x00},{0,0x0c},{0,0x1d},{0,0x1e},{0,0x1c},
  /* DOWN                              PRN SCR   INSERT   DELETE  HELP  */
  {0,0x1f},{0,0x00},{0,0x00},{0,0x00},{0,0xe8},{0,0x12},{0,0x08},{0,0xf2},

/* 30 */
  /* 0        1         2        3        4       5          6       7  */
  {0,0xfc},{0,0xe7},{0,0xec},{0,0x91},{0,0x93},{0,0x94},{0,0x95},{0,0xf4},
  /* 8        9                                                         */
  {0,0xf5},{0,0xf6},{0,0x99},{0,0xfa},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 40 */
  /*          A         B        C        D       E         F       G   */
  {0,0x00},{0,0xe1},{0,0x9a},{0,0x9f},{0,0x9c},{0,0x92},{0,0xea},{0,0x97},
  /*   H      I         J        K        L       M         N       O   */
  {0,0x98},{0,0xe6},{0,0xef},{0,0xe9},{0,0xf8},{0,0xf3},{0,0xf0},{0,0xf7},

/* 50 */
  /*  P       Q         R        S        T       U         V       W   */
  {0,0x9e},{0,0xe0},{0,0x9d},{0,0xe4},{0,0x96},{0,0xe5},{0,0xeb},{0,0xe3},
  /*  X       Y         Z        */
  {0,0x9b},{0,0xfd},{0,0xe2},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 60  10 key */
  /*  0       1         2        3        4       5         6       7   */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /*  8       9         *        +                -         .       /   */
  {0,0x38},{0,0x39},{0,0x2a},{0,0x2b},{0,0x00},{0,0x2d},{0,0x2e},{0,0x2f},

/* 70 */
  /* F1       F2       F3        F4      F5       F6       F7       F8  */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x03},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 80 */
  {0,0x20},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x0d},{0,0x00},{0,0x00},


/* 90 */
  /* NUM     SCROLL                                                     */
  {0,0x00},{1,0xfe},{0,0x00},{0,0x00},{0,0x00},{0,0x37},{0,0x34},{0,0x38},
  {0,0x36},{0,0x32},{0,0x39},{0,0x33},{0,0x31},{0,0x35},{0,0x30},{0,0x2e},

/* A0 */
  /*                       SCROLL LOCK                                  */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x99},{0,0xfa},{0,0xe8},{0,0xee},{0,0xf9},{0,0xf2},

/* B0 */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /*                    :*       ;+       ,<       -        .>      /?  */
  {0,0x38},{0,0x39},{0,0x99},{0,0xfa},{0,0xe8},{0,0xee},{0,0xf9},{0,0xf2},

/* C0 */
  {0,0xde},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{1,0xfd},{0,0x00},
  {0,0x00},{0,0x00},{0,0x12},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* D0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*                            [{        \|        ]}      ^           */
  {0,0x00},{0,0x00},{0,0x00},{0,0xdf},{0,0xb0},{0,0xf1},{0,0xed},{0,0x00},

/* E0 */
  /*                    _              scroll?    caps? */
  {0,0x00},{0,0x00},{0,0xfb},{0,0x00},{1,0xfe},{1,0xfb},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
/* F0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x7f},
};

/* hiragana + shift */
byte Keys4[256][2] =
{
/* 00 */
  {0,0x35},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /* BS      TAB                                ENTER                  */
  {0,0x7f},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x0d},{0,0x00},{0,0x00},

/* 10 */
  /* SHIFT   CTRL      ALT     PAUSE    CAPS     KANA                  */
  {0,0x00},{0,0x00},{0,0x00},{1,0xfa},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*                           ESC                                     */
  {0,0x00},{0,0x00},{0,0x00},{0,0x1b},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 20 */
  /* SPC    PAGE UP PAGE DOWN    END    HOME      LEFT    UP      RIGHT */
  {0,0x20},{1,0xfc},{1,0xfc},{0,0x00},{0,0x0b},{0,0x1d},{0,0x1e},{0,0x1c},
  /* DOWN                              PRN SCR   INSERT   DELETE  HELP  */
  {0,0x1f},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0xa1},{0,0xa5},

/* 30 */
  /* 0        1         2        3        4       5          6       7  */
  {0,0x86},{0,0x00},{0,0x00},{0,0x87},{0,0x89},{0,0x8a},{0,0x8b},{0,0x8c},
  /* 8        9                                                         */
  {0,0x8d},{0,0x8e},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 40 */
  /*          A         B        C        D       E         F       G   */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x88},{0,0x00},{0,0x00},
  /*   H      I         J        K        L       M         N       O   */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 50 */
  /*  P       Q         R        S        T       U         V       W   */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*  X       Y         Z        */
  {0,0x00},{0,0x00},{0,0x8f},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 60  10 key*/
  /*  0       1         2        3        4       5         6       7   */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /*  8       9         *        +                -         .       /   */
  {0,0x38},{0,0x39},{0,0x2a},{0,0x2b},{0,0x00},{0,0x2d},{0,0x2e},{0,0x2f},

/* 70 */
  /* F1       F2       F3        F4      F5       F6       F7       F8  */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x03},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 80 */
  {0,0x20},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x0d},{0,0x00},{0,0x00},

/* 90 */
  /* NUM     SCROLL                                                     */
  {0,0x00},{1,0xfe},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* A0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x2a},{0,0x2b},{0,0x2c},{0,0x2d},{0,0x2e},{0,0x2f},

/* B0 */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /*                    :*       ;+       ,<       -        .>      /?  */
  {0,0x38},{0,0x39},{0,0x00},{0,0x00},{0,0xa4},{0,0x00},{0,0xa1},{0,0x00},

/* C0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{1,0xfd},{0,0x00},
  {0,0x00},{0,0x00},{0,0x12},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
/* D0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*                            [{        \|        ]}      ^           */
  {0,0x00},{0,0x00},{0,0x00},{0,0xa2},{0,0x00},{0,0xa3},{0,0x00},{0,0x00},
/* E0 */
  /*                     _                                              */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{1,0xfe},{1,0xfb},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
/* F0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x7f},
};

/* katakana */
byte Keys5[256][2] =
{
/* 00 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x7f},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x0d},{0,0x00},{0,0x00},

/* 10 */
  /* SHIFT   CTRL      ALT     PAUSE    CAPS     KANA                  */
  {0,0x00},{0,0x00},{0,0x00},{1,0xfa},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*                           ESC                                     */
  {0,0x00},{0,0x00},{0,0x00},{0,0x1b},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 20 */
  /* SPC    PAGE UP PAGE DOWN    END    HOME      LEFT    UP      RIGHT */
  {0,0x20},{0,0xfe},{0,0xfe},{0,0x00},{0,0x0c},{0,0x1d},{0,0x1e},{0,0x1c},
  /* DOWN                              PRN SCR   INSERT   DELETE  HELP  */
  {0,0x1f},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x12},{0,0x08},{0,0x00},

/* 30 */
  /* 0        1         2        3        4       5          6       7  */
  {0,0xdc},{0,0xc7},{0,0xcc},{0,0xb1},{0,0xb3},{0,0xb4},{0,0xb5},{0,0xd4},
  /* 8        9                                                         */
  {0,0xd5},{0,0xd6},{0,0xb9},{0,0xda},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 40 */
  /*          A         B        C        D       E         F       G   */
  {0,0x00},{0,0xc1},{0,0xba},{0,0xbf},{0,0xbc},{0,0xb2},{0,0xca},{0,0xb7},
  /*   H      I         J        K        L       M         N       O   */
  {0,0xb8},{0,0xc6},{0,0xcf},{0,0xc9},{0,0xd8},{0,0xd3},{0,0xd0},{0,0xd7},

/* 50 */
  /*  P       Q         R        S        T       U         V       W   */
  {0,0xbe},{0,0xc0},{0,0xbd},{0,0xc4},{0,0xb6},{0,0xc5},{0,0xcb},{0,0xc3},
  /*  X       Y         Z        */
  {0,0xbb},{0,0xdd},{0,0xc2},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 60  10 key*/
  /*  0       1         2        3        4       5         6       7   */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /*  8       9         *        +                -         .       /   */
  {0,0x38},{0,0x39},{0,0x2a},{0,0x2b},{0,0x00},{0,0x2d},{0,0x2e},{0,0x2f},

/* 70 */
  /* F1       F2       F3        F4      F5       F6       F7       F8  */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x03},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 80 */
  {0,0x20},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x0d},{0,0x00},{0,0x00},

/* 90 */
  /* NUM     SCROLL                                                     */
  {0,0x00},{1,0xfe},{0,0x00},{0,0x00},{0,0x00},{0,0x37},{0,0x34},{0,0x38},
  {0,0x36},{0,0x32},{0,0x39},{0,0x33},{0,0x31},{0,0x35},{0,0x30},{0,0x2e},

/* A0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0xb9},{0,0xda},{0,0xc8},{0,0xce},{0,0xd9},{0,0xd2},

/* B0 */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /*                    :*       ;+       ,<       -        .>      /?  */
  {0,0x38},{0,0x39},{0,0xb9},{0,0xda},{0,0xc8},{0,0xce},{0,0xd9},{0,0xd2},

/* C0 */
  {0,0xde},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x12},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* D0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*                            [{        \|        ]}      ^           */
  {0,0x00},{0,0x00},{0,0x00},{0,0xdf},{0,0xb0},{0,0xd1},{0,0xcd},{0,0x00},

/* E0 */
  /*                     _                                              */
  {0,0x00},{0,0x00},{0,0xdb},{0,0x00},{1,0xfe},{1,0xfb},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
/* F0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x7f},
};

/* katakana + shift */
byte Keys6[256][2] =
{
/* 00 */
  {0,0x35},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x7f},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x0d},{0,0x00},{0,0x00},
/* 10 */
  {0,0x00},{0,0x00},{0,0x00},{1,0xfa},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x1b},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
/* 20 */
  /* SPC    PAGE UP PAGE DOWN    END    HOME      LEFT    UP      RIGHT */
  {0,0x20},{1,0xfc},{1,0xfc},{0,0x00},{0,0x0b},{0,0x1d},{0,0x1e},{0,0x1c},
  /* DOWN                              PRN SCR   INSERT   DELETE  HELP  */
  {0,0x1f},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0xa1},{0,0xa5},

/* 30 */
  /* 0        1         2        3        4       5          6       7  */
  {0,0xa6},{0,0x00},{0,0x00},{0,0xa7},{0,0xa9},{0,0xaa},{0,0xab},{0,0xac},
  /* 8        9                                                         */
  {0,0xad},{0,0xae},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 40 */
  /*          A         B        C        D       E         F       G   */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0xa8},{0,0x00},{0,0x00},
  /*   H      I         J        K        L       M         N       O   */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 50 */
  /*  P       Q         R        S        T       U         V       W   */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*  X       Y         Z        */
  {0,0x00},{0,0x00},{0,0xaf},{0,0xa2},{0,0xb0},{0,0xa3},{0,0x00},{0,0x00},

/* 60 */
  /*  0       1         2        3        4       5         6       7   */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /*  8       9         *        +                -         .       /   */
  {0,0x38},{0,0x39},{0,0x2a},{0,0x2b},{0,0x00},{0,0x2d},{0,0x2e},{0,0x2f},

/* 70 */
  /* F1       F2       F3        F4      F5       F6       F7       F8  */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x03},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 80 */
  {0,0x20},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x0d},{0,0x00},{0,0x00},

/* 90 */
  {0,0x00},{1,0xfe},{0,0x00},{0,0x00},{0,0x00},{0,0x37},{0,0x34},{0,0x38},
  {0,0x36},{0,0x32},{0,0x39},{0,0x33},{0,0x31},{0,0x35},{0,0x30},{0,0x2e},

/* A0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x2a},{0,0x2b},{0,0x2c},{0,0x2d},{0,0x2e},{0,0x2f},

/* B0 */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /*                    :*       ;+       ,<       -        .>      /?  */
  {0,0x38},{0,0x39},{0,0x00},{0,0x00},{0,0xa4},{0,0x00},{0,0xa1},{1,0x00},

/* C0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{1,0xfd},{0,0x00},
  {0,0x00},{0,0x00},{0,0x12},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* D0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*                            [{        \|        ]}      ^           */
  {0,0x00},{0,0x00},{0,0x00},{0,0xa2},{0,0x00},{0,0xa3},{0,0x00},{0,0x00},

/* E0 */
  /*                     _                                              */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{1,0xfe},{1,0xfb},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* F0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x7f},
};

/* with graph key */
byte Keys7[256][2] =
{
/* 00 */
  {0,0x35},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x7f},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 10 */
  {0,0x00},{0,0x00},{0,0x00},{1,0xfa},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 20 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{1,0x1f},{1,0x17},{1,0x1d},{1,0x80},

/* 30 */
  {1,0x0f},{1,0x07},{1,0x01},{1,0x02},{1,0x03},{1,0x04},{1,0x05},{1,0x06},
  {1,0x0d},{1,0x0e},{1,0x81},{1,0x82},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 40 */
  /*          A         B        C        D       E         F       G   */
  {1,0x83},{0,0x00},{1,0x1b},{1,0x1a},{1,0x14},{1,0x18},{1,0x15},{1,0x13},
  /*   H      I         J        K        L       M         N       O   */
  {1,0x0a},{1,0x16},{0,0x00},{0,0x00},{1,0x1e},{1,0x0b},{0,0x00},{0,0x00},

/* 50 */
  {1,0x10},{0,0x00},{1,0x12},{1,0x0c},{1,0x19},{0,0x00},{1,0x11},{0,0x00},
  {1,0x1c},{1,0x08},{0,0x00},{1,0x84},{1,0x09},{1,0x85},{0,0x00},{0,0x00},

/* 60 */
  {0,0x00},{0,0x00},{1,0x1b},{1,0x1a},{1,0x14},{1,0x18},{1,0x15},{1,0x13},
  {1,0x0a},{1,0x16},{0,0x00},{0,0x00},{1,0x1e},{1,0x0b},{0,0x00},{0,0x00},

/* 70 */
  /* F1       F2       F3        F4      F5       F6       F7       F8  */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x03},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* 80 */
  {0,0x20},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x09},{0,0x00},{0,0x00},{0,0x00},{0,0x0d},{0,0x00},{0,0x00},

/* 90 */
  /* NUM     SCROLL                                                     */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x37},{0,0x34},{0,0x38},
  {0,0x36},{0,0x32},{0,0x39},{0,0x33},{0,0x31},{0,0x35},{0,0x30},{0,0x2e},

/* A0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x2a},{0,0x2b},{0,0x2c},{1,0x17},{0,0x2e},{0,0x2f},

/* B0 */
  {0,0x30},{0,0x31},{0,0x32},{0,0x33},{0,0x34},{0,0x35},{0,0x36},{0,0x37},
  /*                    :*       ;+       ,<       -        .>      /?  */
  {0,0x38},{0,0x39},{0,0x81},{0,0x82},{1,0x1f},{1,0x17},{1,0x1d},{0,0x80},

/* C0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x12},{0,0x00},{0,0x00},{1,0x17},{0,0x00},{0,0x00},

/* D0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  /*                            [{        \|        ]}      ^           */
  {0,0x00},{0,0x00},{0,0x00},{0,0x84},{1,0x09},{0,0x85},{0,0x00},{0,0x00},

/* E0 */
  /*                     _                                              */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{1,0xfe},{1,0xfb},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},

/* F0 */
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},
  {0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x00},{0,0x7f},
};

// ****************************************************************************
//           key buffer (ring buffer)
// ****************************************************************************
struct _buffer2 {
  int   read_idx;
  int   write_idx;
  int   length;
  int   wParam[1024];
  int   lParam[1024];
  char  chr   [1024];
} keybuffer;

void init_keybuffer(void)
{
  keybuffer.read_idx=0;
  keybuffer.write_idx=0;
  keybuffer.length=1024-1;
}

void write_keybuffer(int wParam , int lParam , char chr)
{
  keybuffer.wParam[ keybuffer.write_idx]= wParam;
  keybuffer.lParam[ keybuffer.write_idx]= lParam;
  keybuffer.chr   [ keybuffer.write_idx]= chr;
  if( ++keybuffer.write_idx >keybuffer.length)
	{
	 keybuffer.write_idx=0;
	}
}

int read_keybuffer(int *wParam , int *lParam , char *chr)
{
 int ret;
 ret=0;
  if( keybuffer.read_idx != keybuffer.write_idx)
  	{
	 if(wParam) *wParam = keybuffer.wParam[ keybuffer.read_idx];
	 if(lParam) *lParam = keybuffer.lParam[ keybuffer.read_idx];
	 if(chr   ) *chr    = keybuffer.chr   [ keybuffer.read_idx];
	 if( ++keybuffer.read_idx >keybuffer.length)
		{
		 keybuffer.read_idx=0;
		}
	 ret=1;
	}
 return(ret);
}

int check_keybuffer()
{
	if (keybuffer.read_idx != keybuffer.write_idx) return 1;
	return 0;
}

void KEYBOARD::initialize()
{
	key_stat = emu->key_buffer();
	kbFlagCtrl=0;
	kbFlagGraph=0;
	kbFlagFunc=0;
	kanaMode=0;
	katakana=0;
	stick0=0;
	init_keybuffer();
	// register event to update the key status
	register_frame_event(this);
}

void KEYBOARD::event_frame()
{
	if (key_stat[VK_CONTROL] & 0x80) kbFlagCtrl=1;
	else kbFlagCtrl=0;
	stick0 = 0;
	if (key_stat[VK_SPACE]) stick0 |= STICK0_SPACE;
	if (key_stat[VK_LEFT]) stick0 |= STICK0_LEFT;
	if (key_stat[VK_RIGHT]) stick0 |= STICK0_RIGHT;
	if (key_stat[VK_DOWN]) stick0 |= STICK0_DOWN;
	if (key_stat[VK_UP]) stick0 |= STICK0_UP;
	if (key_stat[VK_F9]) stick0 |= STICK0_STOP;
	if (key_stat[VK_SHIFT]) stick0 |= STICK0_SHIFT;
	d_mem->write_data8(0xfeca, stick0);
	update_keyboard();
	if (check_keybuffer()) {
		d_cpu->write_signal(SIG_CPU_IRQ, 0x16, 0xff);
	}
}

void KEYBOARD::update_keyboard()
{
	for (int code=0; code < 256; code++) {
		if (key_stat[code] & 0x80) {
			if (code == VK_SHIFT || code == VK_CONTROL) continue;
			key_stat[code]=0;
			if (code == VK_F6) {kanaMode = -1 * (kanaMode-1);continue;}
			if (code == VK_F7) {katakana = -1 * (katakana-1);continue;}
			if (code == VK_F8) {kbFlagGraph = -1 * (kbFlagGraph-1);continue;}
			write_keybuffer(code, 0x80, 0);
			byte *Keys;
			byte ascii=0;
			if (kbFlagGraph) {
				Keys = Keys7[code];
			} else if (kanaMode) {
				if (katakana) {
  					if (stick0 & STICK0_SHIFT) Keys = Keys6[code];
  					else Keys = Keys5[code];
				} else if (stick0 & STICK0_SHIFT) { 
					Keys = Keys4[code];
				} else {
               		Keys = Keys3[code];
				}
			} else if (stick0 & STICK0_SHIFT) { 
           		Keys = Keys2[code];
			} else { 
          		Keys = Keys1[code];
			}
			ascii = Keys[1];
			/* control key + alphabet key */
			if ((kbFlagCtrl == 1) && (code >= 0x41) && (code <= 0x5a)) ascii = code - 0x41 + 1;
			/* function key */
            if (!kanaMode && (ascii>0xef && ascii<0xfa)) kbFlagFunc=1;
			d_pio->write_signal(SIG_I8255_PORT_A, ascii, 0xff);
		}
	}
}

uint32 KEYBOARD::intr_ack() {
	int code, value;
	char dummy;
	if (read_keybuffer(&code, &value, &dummy)) {
		if (kbFlagGraph==0 && kbFlagFunc==0) return 0x02;
		kbFlagFunc=0;
		return 0x14;
	}
	return 0x06;
}
