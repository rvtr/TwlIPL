#ifndef MENU_IRQ_HANDLER_H_
#define MENU_IRQ_HANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <twl/types.h>

typedef struct MenuIrqHandlerLogBuffer {
    union {
        u32 word0;
        struct {
            u32 mInVLineCount  :  9;   // d31-d23
            u32 mIntr          :  5;   // d22-d18
            u32 mInVBlankCount : 18;   // d17-d0
        };
    };
    union {
        u32 word1;
        struct {
            u32 mTicks       : 16;     // d31-d16
            u32 mVLineCounts : 16;     // d15-d0
        };
    };
} MenuIrqHandlerLogBuffer;

extern void MenuIrqHandlerStart( MenuIrqHandlerLogBuffer* pBuffer, u32 size );
extern void MenuIrqHandlerEnd( void );
extern BOOL MenuIrqHandlerIsUsed( void );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
