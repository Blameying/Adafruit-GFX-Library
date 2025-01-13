
#ifndef ARDUINO
#include <string.h>
#include <locale.h>
#include <time.h>
#include <ctype.h>
#include <ft2build.h>
#include <stdint.h>
#include <stdio.h>
#include FT_GLYPH_H
#include FT_MODULE_H
#include FT_TRUETYPE_DRIVER_H
#include "gfxfont.h" // Adafruit_GFX font structures
#include <assert.h>
#define DPI 141 // Approximate res. of Adafruit 2.8" TFT
FT_Library library;
FT_Face face;
FT_Glyph glyph;
FT_Bitmap* bitmap;
FT_BitmapGlyphRec* g;
GFXglyph* table;
uint8_t bit;
int i, j, i_1;
int err;
int size;
int first = ' ';
int last = '~';
int bitmapOffset = 0, x, y, byte;
char* fontName, c, * ptr;
wchar_t mb;
int cdat;
unsigned char s [2] = { '0' };
char str [1000];
wchar_t Chinese_unicode [1000];
int Chinese_code[1000] = { 0 };
void enbit(uint8_t value) {
  static uint8_t row = 0, sum = 0, bit = 0x80, firstCall = 1;
  if (value)
    sum |= bit;          // 如果需要，设置位
  if (!(bit >>= 1)) {    // 前进到下一位，是否到达字节末尾？
    if (!firstCall) {    // 很好地格式化输出表
      if (++row >= 12) { // 行上的最后一个条目?
        printf(",\n  "); //   Newline format output
        row = 0;         //   Reset row counter
      }
      else {           // Not end of line
        printf(", ");    //   Simple comma delim
      }
    }
    printf("0x%02X", sum); // 写入字节值
    sum = 0;               // Clear for next byte
    bit = 0x80;            // Reset bit counter
    firstCall = 0;         // Formatting flag
  }
}
//Unicode编码转utf-8，将中文Unicode编码以汉字输出
int enc_unicode_to_utf8_one(unsigned long unic, unsigned char* pOutput, int outSize) {
  assert(pOutput != NULL);
  assert(outSize >= 6);
  if (unic <= 0x0000007F) {
    // * U-00000000 - U-0000007F:  0xxxxxxx
    *pOutput = (unic & 0x7F);
    return 1;
  }
  else if (unic >= 0x00000080 && unic <= 0x000007FF) {
    // * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
    *(pOutput + 1) = (unic & 0x3F) | 0x80;
    *pOutput = ((unic >> 6) & 0x1F) | 0xC0;
    return 2;
  }
  else if (unic >= 0x00000800 && unic <= 0x0000FFFF) {
    // * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
    *(pOutput + 2) = (unic & 0x3F) | 0x80;
    *(pOutput + 1) = ((unic >> 6) & 0x3F) | 0x80;
    *pOutput = ((unic >> 12) & 0x0F) | 0xE0;
    return 3;
  }
  else if (unic >= 0x00010000 && unic <= 0x001FFFFF) {
    // * U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    *(pOutput + 3) = (unic & 0x3F) | 0x80;
    *(pOutput + 2) = ((unic >> 6) & 0x3F) | 0x80;
    *(pOutput + 1) = ((unic >> 12) & 0x3F) | 0x80;
    *pOutput = ((unic >> 18) & 0x07) | 0xF0;
    return 4;
  }
  else if (unic >= 0x00200000 && unic <= 0x03FFFFFF) {
    // * U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    *(pOutput + 4) = (unic & 0x3F) | 0x80;
    *(pOutput + 3) = ((unic >> 6) & 0x3F) | 0x80;
    *(pOutput + 2) = ((unic >> 12) & 0x3F) | 0x80;
    *(pOutput + 1) = ((unic >> 18) & 0x3F) | 0x80;
    *pOutput = ((unic >> 24) & 0x03) | 0xF8;
    return 5;
  }
  else if (unic >= 0x04000000 && unic <= 0x7FFFFFFF) {
    // * U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    *(pOutput + 5) = (unic & 0x3F) | 0x80;
    *(pOutput + 4) = ((unic >> 6) & 0x3F) | 0x80;
    *(pOutput + 3) = ((unic >> 12) & 0x3F) | 0x80;
    *(pOutput + 2) = ((unic >> 18) & 0x3F) | 0x80;
    *(pOutput + 1) = ((unic >> 24) & 0x3F) | 0x80;
    *pOutput = ((unic >> 30) & 0x01) | 0xFC;
    return 6;
  }
  return 0;
}

void English_convert_Bitmaps() {
  for (i = first, j = 0; i <= last; i++, j++) {

    // MONO渲染器通过位图结构提供具有完美裁剪（没有浪费像素）的干净图像。
    if ((err = FT_Load_Char(face, i, FT_LOAD_TARGET_MONO))) {
      fprintf(stderr, "Error %d loading char '%c'\n", err, i);
      continue;
    }

    if ((err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO))) {
      fprintf(stderr, "Error %d rendering char '%c'\n", err, i);
      continue;
    }

    if ((err = FT_Get_Glyph(face->glyph, &glyph))) {
      fprintf(stderr, "Error %d getting glyph '%c'\n", err, i);
      continue;
    }

    bitmap = &face->glyph->bitmap;
    g = (FT_BitmapGlyphRec*)glyph;

    //存储最小的字体和每个字形信息，以减少闪存空间需求。
    //Glyph位图是完全位压缩的；没有按扫描线填充，
    //不过在需要时可以将每个字符的末尾填充到下一个字节边界。
    //16位偏移量意味着位图的最大64K，代码当前不检查溢出。
    //（也不检查大小和偏移是否在范围内……请负责任地转换字体。）
    table [j].bitmapOffset = bitmapOffset;
    table [j].width = bitmap->width;
    table [j].height = bitmap->rows;
    table [j].xAdvance = face->glyph->advance.x >> 6;
    table [j].xOffset = g->left;
    table [j].yOffset = 1 - g->top;

    for (y = 0; y < bitmap->rows; y++) {
      for (x = 0; x < bitmap->width; x++) {
        byte = x / 8;
        bit = 0x80 >> (x & 7);
        enbit(bitmap->buffer [y * bitmap->pitch + byte] & bit);
      }
    }

    // 如果需要，将字符位图的末尾填充到下一个字节边界
    int n = (bitmap->width * bitmap->rows) & 7;
    if (n) {     // Pixel count not an even multiple of 8?
      n = 8 - n; // # bits to next multiple
      while (n--)
        enbit(0);
    }
    bitmapOffset += (bitmap->width * bitmap->rows + 7) / 8;

    FT_Done_Glyph(glyph);
  }
}

void English_convert_Glyphs() {
  for (i = first, j = 0; i <= last; i++, j++) {
    printf("  { %5d, %3d, %3d, %3d, %4d, %4d }", table [j].bitmapOffset,
      table [j].width, table [j].height, table [j].xAdvance, table [j].xOffset,
      table [j].yOffset);
    printf(",   // 0x%02X", i);
    printf(" '%c'", i);
    putchar('\n');
  }
}



void Chinese_convert_Bitmaps() {
  for (i = 0; i < i_1; i++, j++) {
    //printf("\n");
    //printf("开始第%d个", i);
    // MONO渲染器通过位图结构提供具有完美裁剪（没有浪费像素）的干净图像。
    if ((err = FT_Load_Char(face, Chinese_code [i], FT_LOAD_TARGET_MONO))) {
      fprintf(stderr, "Error %d loading char '%c'\n", err, Chinese_code [i]);
      continue;
    }

    if ((err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO))) {
      fprintf(stderr, "Error %d rendering char '%c'\n", err, Chinese_code [i]);
      continue;
    }

    if ((err = FT_Get_Glyph(face->glyph, &glyph))) {
      fprintf(stderr, "Error %d getting glyph '%c'\n", err, Chinese_code [i]);
      continue;
    }

    bitmap = &face->glyph->bitmap;
    g = (FT_BitmapGlyphRec*)glyph;
    table [j].bitmapOffset = bitmapOffset;
    table [j].width = bitmap->width;
    table [j].height = bitmap->rows;
    table [j].xAdvance = face->glyph->advance.x >> 6;
    table [j].xOffset = g->left;
    table [j].yOffset = 1 - g->top;
    for (y = 0; y < bitmap->rows; y++) {
      for (x = 0; x < bitmap->width; x++) {
        byte = x / 8;
        bit = 0x80 >> (x & 7);
        enbit(bitmap->buffer [y * bitmap->pitch + byte] & bit);
      }
    }

    // 如果需要，将字符位图的末尾填充到下一个字节边界
    int n = (bitmap->width * bitmap->rows) & 7;
    if (n) {     // 像素数不是8的偶数倍吗？
      n = 8 - n; // # 位到下一个倍数
      while (n--)
        enbit(0);
    }
    bitmapOffset += (bitmap->width * bitmap->rows + 7) / 8;

    FT_Done_Glyph(glyph);
  }
}


void Chinese_convert_Glyphs() {
  //int icode = Chinese_unicode [0];
  for (i = 0; i < i_1; i++, j++) {
    printf("  { %5d, %3d, %3d, %3d, %4d, %4d }", table [j].bitmapOffset,
      table [j].width, table [j].height, table [j].xAdvance, table [j].xOffset,
      table [j].yOffset);
    if (i < i_1 - 1) {
      printf(",   // 0x%02X", Chinese_code [i]);
      enc_unicode_to_utf8_one(Chinese_code [i], s, 10);
      printf(" '%s'", s);
      putchar('\n');
    }
    else {
      printf("    // 0x%02X", Chinese_code [i]);
      enc_unicode_to_utf8_one(Chinese_code [i], s, 10);
      printf(" '%s'", s);
      putchar('\n');
    }

  }

}



int main(int argc, char* argv[]) {
  size = atoi(argv [2]);
  if (argc == 4) {
    setlocale(LC_ALL, "");
    strcpy(str, argv [3]);
    size_t len = mbstowcs(Chinese_unicode, str, 1000);
    Chinese_code [0] = Chinese_unicode [0];
    for (i = 0;i < len;i++) {
      Chinese_code [i] = Chinese_unicode [i];
    }
    i_1 = len;
    int temp;
    for (i = 0;i < len;i++) {
      for (j = i + 1;j < len;j++) {
        if (Chinese_code [i] > Chinese_code [j]) {
          temp = Chinese_code [i];
          Chinese_code [i] = Chinese_code [j];
          Chinese_code [j] = temp;
        }
      }
    }
    //printf("0x%d \n", i_1);
    //printf("0x%x \n", Chinese_unicode [1]);
    //printf("0x%x \n", Chinese_unicode [2]);
    ptr = strrchr(argv [1], '/'); // 查找文件名中的最后一个斜线
    if (ptr)
      ptr++; // 文件名的第一个字符（去掉路径）
    else
      ptr = argv [1]; // 没有路径；本地目录中的字体。
    // 为字体名称和字形表分配空间
    if ((!(fontName = malloc(strlen(ptr) + 20))) ||
      (!(table = (GFXglyph*)malloc((last - first + 1 + i_1) * sizeof(GFXglyph))))) {
      fprintf(stderr, "Malloc错误\n");
      return 1;
    }
    strcpy(fontName, ptr);
    ptr = strrchr(fontName, '.'); // 查找最后一个句点（文件文本）
    if (!ptr)
      ptr = &fontName [strlen(fontName)]; // If none, append
    sprintf(ptr, "%dpt%db", size, (last > 127) ? 8 : 7);
    for (i = 0; (c = fontName [i]); i++) {
      if (isspace(c) || ispunct(c))
        fontName [i] = '_';
    }
    if ((err = FT_Init_FreeType(&library))) {
      fprintf(stderr, "FreeType init error: %d", err);
      return err;
    }
    FT_UInt interpreter_version = TT_INTERPRETER_VERSION_35;
    FT_Property_Set(library, "truetype", "interpreter-version",
      &interpreter_version);
    if ((err = FT_New_Face(library, argv [1], 0, &face))) {
      fprintf(stderr, "Font load error: %d", err);
      FT_Done_FreeType(library);
      return err;
    }
    // << 6 because '26dot6' fixed-point format
    FT_Set_Char_Size(face, size << 6, 0, DPI, 0);
    printf("const uint8_t %sBitmaps[] PROGMEM = {\n  ", fontName);
    English_convert_Bitmaps();//转换英文字母及符号
    Chinese_convert_Bitmaps();
    printf(" };\n\n"); // End bitmap array
    printf("const GFXglyph %sGlyphs[] PROGMEM = {\n", fontName);
    English_convert_Glyphs();
    Chinese_convert_Glyphs();
    printf(" }; ");
    printf("\n\n");
    //输出中文unicode码
    printf("uint16_t Chinese_code[] ={\n");
    for (i = 0;i < i_1 - 1;i++) {
      if (i > 0 && ((i + 1) % 5 == 0)) {
        printf("0x%02X,\n", Chinese_code [i]);
      }
      else {
        printf("0x%02X,", Chinese_code [i]);
      }
    }
    printf("0x%02X", Chinese_code [i_1 - 1]);
    //printf("0x%02X", Chinese_code [i + 1]);
    printf("}; \n");
    // 输出字体结构
    printf("const GFXfont %s PROGMEM = {\n", fontName);
    printf("  (uint8_t  *)%sBitmaps,\n", fontName);
    printf("  (GFXglyph *)%sGlyphs,\n", fontName);
    printf("  (uint16_t*)Chinese_code,\n");
    printf("  sizeof(Chinese_code)/sizeof(Chinese_code[0]),\n");
    if (face->size->metrics.height == 0) {
      // 没有面部高度信息，假设固定宽度并从字形中获取。
      printf("  0x%02X, 0x%02X, %d };\n\n", first, last + i_1, table [0].height);
    }
    else {
      printf("  0x%02X, 0x%02X, %ld };\n\n", first, last + i_1,
        face->size->metrics.height >> 6);
    }
    printf("// 总计： %d bytes\n", bitmapOffset + (last + i_1 - first + 1) * 7 + 7);
    // Size estimate is based on AVR struct and pointer sizes;
    // actual size may vary.

    FT_Done_FreeType(library);

    return 0;
  }
  else {
    printf("参数不够，请按格式输入 ./fontconvert **.ttf 12 ****** >> **.h\n");
    return 1;
  }
}


#endif /* !ARDUINO */
