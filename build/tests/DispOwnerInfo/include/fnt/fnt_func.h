#ifndef FNT_FUNC_H_
#define FNT_FUNC_H_

// fnt_utils.c
extern int fnt_GetLeftFromOrigin( tFntPosition posx, int width  );
extern int fnt_GetTopFromOrigin ( tFntPosition posy, int height );

// fnt_font.c
extern void fnt_CreateFont       ( tFntEntry* font, const void* fontRsrc, const void* cmapRsrc );
extern void fnt_CreateDrawContext( tFntDrawContext* dc, u16* canvas, u16 width, u16 height, int type, tFntEntry* font );

extern void fnt_DrawLetter     ( tFntEntry* font, tFntDrawContext* dc, s32 x, s32 y, u16 letter, int palette );
extern void fnt_DrawLetterGlyph( tFntEntry* font, tFntDrawContext* dc, s32 x, s32 y, u16 glyph, int palette );
extern void fnt_DrawString     ( tFntEntry* font, tFntDrawContext* dc, tFntPosition posx, tFntPosition posy, int step_x, int step_y, const u16* str, int palette );
extern int  fnt_GetLetterWidth ( tFntEntry* font, u16 letter );
extern int  fnt_GetLetterOffset( tFntEntry* font, u16 letter );
extern void fnt_DrawLetterGlyphFast( tFntEntry* font, tFntDrawContext* dc, s32 x, s32 y, u16 glyph, int palette );
extern BOOL fnt_SetDefaultLetter( tFntEntry* font, u16 letter );

extern const u16* fnt_GetLineWidth   ( tFntEntry* font, const u16* str, int* pwidth, int step_x );
extern int        fnt_GetStringWidth ( tFntEntry* font, const u16* str, int step_x );
extern int        fnt_GetStringHeight( tFntEntry* font, const u16* str, int step_y );

extern void fnt_ClearRect( tFntDrawContext* dc, u16 x, u16 y, u16 height, u16 width );


// fnt_layout.c
extern void         fnt_DrawMessage     ( tFntDrawContext* dc, tFntMessage* message, const u16* mes_data );
extern void         fnt_DrawLetterLayout( tFntDrawContext* dc, const void* layout );
extern tFntMessage* fnt_GetFntMessage   ( const void* layout, int index );
extern const u16*   fnt_GetMessage      ( const void* layout, int index );
extern int          fnt_GetMessageNum   ( const void* layout );
extern int          fnt_LoadCelltoOam   ( const void* celldata, GXOamAttr* poam, tFntPosition posx, tFntPosition posy, int cell_index );
extern int          fnt_DrawCellLayout  ( const void* celldata, const void* layout, GXOamAttr* poam );

// fnt_celldata.c
extern int               fnt_GetCellOamNum        ( const void* celldata, int index );
extern tFntCellCharInfo* fnt_GetCellCharInfo      ( const void* celldata, int index );
extern tFntCellCharOam*  fnt_GetCellOam           ( const void* celldata, int index );
extern const void*       fnt_GetCellObjChar       ( const void* celldata );
extern u32               fnt_GetCellObjCharSizeAll( const void* celldata );
extern int               fnt_GetCellCharVramMode  ( const void* celldata );
extern int               fnt_GetCellCharNameShift ( const void* celldata );

// fnt_touch.c
extern int         fnt_GetFntRegionNum     ( const void* base );
extern tFntRegion* fnt_GetFntRegion        ( const void* base, int index );
extern int         fnt_GetHitFntRegionIndex( const void* base, int x, int y );

#endif // FNT_FUNC_H_
