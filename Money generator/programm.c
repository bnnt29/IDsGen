#include "hpdf.h"
#include <ctype.h>
#include <dirent.h>
#include <iconv.h>
#include <locale.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

jmp_buf env;

wchar_t whitespace = L'\u00A0';
wchar_t ue = L'\u00fc';
wchar_t oe = L'\u00f6';
wchar_t ae = L'\u00e4';
wchar_t UE = L'\u00dc';
wchar_t OE = L'\u00d6';
wchar_t AE = L'\u00c4';

const bool FILL_REMAINING_PAGE = true;
const int ids_per_page = 21;
const int min_fill_templates = 15;

void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no,
                   void *user_data) {
  printf("ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no,
         (HPDF_UINT)detail_no);
  longjmp(env, 1);
}

void draw_rect(HPDF_Page page, double posx, double posy, double width,
               double height) {
  HPDF_Page_Rectangle(page, posx, posy, width, height);
}

void draw_line_common(HPDF_Page page, float x, float y, const char *label,
                      int yOffset) {
  HPDF_Page_BeginText(page);
  HPDF_Page_MoveTextPos(page, x, y - 10 + yOffset);
  HPDF_Page_ShowText(page, label);
  HPDF_Page_EndText(page);

  HPDF_Page_MoveTo(page, x, y - 15 + yOffset);
  HPDF_Page_LineTo(page, x + 220, y - 15 + yOffset);
  HPDF_Page_Stroke(page);
}

void draw_line(HPDF_Page page, float x, float y, const char *label) {
  draw_line_common(page, x, y, label, 0);
}

void draw_line2(HPDF_Page page, float x, float y, const char *label) {
  draw_line_common(page, x, y, label, -25);
}

void draw_image(HPDF_Doc pdf, const HPDF_Image image, float x, float y,
                float scale) {

  HPDF_Page page = HPDF_GetCurrentPage(pdf);

  /* Draw image to the canvas. */
  HPDF_Page_DrawImage(page, image, x + 1, y + 1,
                      HPDF_Image_GetWidth(image) / scale,
                      HPDF_Image_GetHeight(image) / scale);
}

void drawjpg_image(HPDF_Doc pdf, const char *filename, float x, float y,
                   float scale) {
#ifdef __WIN32__
  const char *FILE_SEPARATOR = "\\";
#else
  const char *FILE_SEPARATOR = "./";
#endif
  char *filename1 = malloc((strlen(filename) + 4) * sizeof(char));

  HPDF_Image image;

  strcpy(filename1, "");
  strcat(filename1, FILE_SEPARATOR);
  strcat(filename1, filename);
  strcat(filename1, "\0");

  image = HPDF_LoadJpegImageFromFile(pdf, filename1);

  /* Draw image to the canvas. */
  draw_image(pdf, image, x, y, scale);

  free(filename1);
}

void drawpng_image(HPDF_Doc pdf, const char *filename, float x, float y,
                   float scale) {
#ifdef __WIN32__
  const char *FILE_SEPARATOR = "\\";
#else
  const char *FILE_SEPARATOR = "./";
#endif
  char *filename1 = malloc((strlen(filename) + 4) * sizeof(char));

  HPDF_Image image;

  strcpy(filename1, "");
  strcat(filename1, FILE_SEPARATOR);
  strcat(filename1, filename);
  strcat(filename1, "\0");

  image = HPDF_LoadPngImageFromFile(pdf, filename1);

  /* Draw image to the canvas. */
  draw_image(pdf, image, x, y, scale);

  free(filename1);
}

void text(HPDF_Page page, float posx, float posy, const char *label) {
  HPDF_Page_BeginText(page);
  HPDF_Page_MoveTextPos(page, posx, posy - 10);
  HPDF_Page_ShowText(page, label);
  HPDF_Page_EndText(page);
}

char *splitstr(int x, const char *txt, bool first) {
  int e = -1;
  for (int i = 0; i < x; i++) {
    if (txt[i] == (char)whitespace || txt[i] == (char)' ' ||
        txt[i] == (char)'\0' || txt[i] == (char)'\n') {
      e = i;
    }
  }
  if (e == -1) {
    if (first) {
      char *txt0 = (char *)malloc(13 * sizeof(char));
      strncpy(txt0, txt, 12);
      txt0[12] = '\0';
      return txt0;
      free(txt0);
    } else {
      for (int i = x - 1; i < (int)strlen(txt); i++) {
        if (txt[i] == (char)whitespace || txt[i] == (char)' ' ||
            txt[i] == (char)'\0' || txt[i] == (char)'\n') {
          e = i;
          break;
        }
      }
      if (e == -1) {
        char *txt0 = (char *)malloc(14 * sizeof(char));
        strcpy(txt0, txt + 13);
        txt0[12] = '\0';
        return txt0;
        free(txt0);
      } else {
        char *txt0 = (char *)malloc((strlen(txt) - e) * sizeof(char));
        strcpy(txt0, txt + e + 1);
        txt0[strlen(txt) - e - 1] = '\0';
        return txt0;
        free(txt0);
      }
    }
  }
  if (first) {
    char *txt0 = (char *)malloc((e + 1) * sizeof(char));
    strncpy(txt0, txt, e);
    txt0[e] = '\0';
    return txt0;
    free(txt0);
  } else {
    char *txt0 = (char *)malloc((strlen(txt) - e) * sizeof(char));
    strcpy(txt0, txt + e + 1);
    txt0[strlen(txt) - e - 1] = '\0';
    return txt0;
    free(txt0);
  }
}

void drawImages(HPDF_Doc pdf, int posx, int posy) {
  /* Draw Image */
  drawpng_image(pdf, "resources/Kröten.png", posx + 10, posy + 30, 2.5);
}

void drawmoney(int x, char *name, int value, HPDF_Page page, HPDF_Font font,
               HPDF_Font fontbd, HPDF_Doc pdf) {
  // bool aftername = strcmp(add_name, "") == 0;

  int pageHeight = HPDF_Page_GetHeight(page);
  int pageWidth = HPDF_Page_GetWidth(page);
  double xwidth = 8.5;
  double xtoy = 5.2 / xwidth;
  int idsperline = 3;
  int idsperrow = ids_per_page / idsperline;
  double width = 267 / 1.4;
  double lettersize = 6;
  double min_space = 8;
  double min_inner_space = 2;
  double height = width * xtoy;
  double outer_space_X = (pageWidth - (width * idsperline)) / (idsperline + 1);
  double inner_space_X = outer_space_X / 2;
  inner_space_X =
      (inner_space_X < min_inner_space) ? min_inner_space : inner_space_X;
  outer_space_X =
      ((pageWidth - (width * idsperline)) - (idsperline - 1) * inner_space_X) /
      2;
  outer_space_X = (outer_space_X < min_space) ? min_space : outer_space_X;
  double outer_space_Y = (pageHeight - (height * idsperrow)) / (idsperrow + 1);
  double inner_space_Y = outer_space_Y / 2;
  inner_space_Y =
      (inner_space_Y < min_inner_space) ? min_inner_space : inner_space_Y;
  outer_space_Y =
      ((pageHeight - (height * idsperrow)) - (idsperrow - 1) * inner_space_Y) /
      2;
  outer_space_Y = (outer_space_Y < min_space) ? min_space : outer_space_Y;
  int posy = x % 21 / 3;
  posy =
      pageHeight - (outer_space_Y + (posy + 1) * height + posy * inner_space_Y);
  int posx = width * (x % 3) + outer_space_X + inner_space_X * (x % 3);

  HPDF_Page_SetLineWidth(page, 0);
  /* Draw Rectangle*/
  HPDF_Page_SetLineWidth(page, 0);
  HPDF_Page_SetRGBStroke(page, 0, 0, 0);
  HPDF_Page_SetRGBFill(page, 0, 0, 0);

  draw_rect(page, posx, posy, width, height);
  HPDF_Page_Stroke(page);

  drawImages(pdf, posx, posy);

  /*Text*/
  HPDF_Page_SetFontAndSize(page, fontbd, 20);
  HPDF_Page_SetRGBFill(page, 0.0, 0.0, 0.0);

  char *value_str = (char *)malloc(10 * sizeof(char));
  sprintf(value_str, "%d %s", value, name);

  text(page, posx + width / 2 - lettersize * sizeof(value_str) / sizeof(char),
       posy + lettersize * 2, value_str);
}

int main(int argc, char const *argv[]) {
  // const char *page_title = "Ausweise";

  HPDF_Doc pdf;
  const char *font_name_bold;
  const char *font_name;
  HPDF_Font font;
  HPDF_Font font_bold;
  HPDF_Page page;
  if (argc < 4) {
    printf("Usage: %s <output> <Name> <Times> <Amount> <Name> <Times> <Amount> "
           "...\n",
           argv[0]);
    return 1;
  }
  char *fname = (char *)malloc((strlen(argv[1]) + 1 + 5) * sizeof(char));
  char *filepath = (char *)malloc((200 + 1) * sizeof(char));
  strcpy(filepath, "");
  strcat(filepath, "./");

  strcpy(fname, argv[1]);
  strcat(fname, ".pdf");
  printf("output file : %s\n", fname);

  pdf = HPDF_New(error_handler, NULL);
  page = HPDF_AddPage(pdf);
  if (!pdf) {
    printf("error: cannot create PdfDoc object\n");
    return 1;
  }

  if (setjmp(env)) {
    HPDF_Free(pdf);
    return 1;
  }
  HPDF_UseUTFEncodings(pdf);
  setlocale(LC_ALL, "");
  HPDF_SetCompressionMode(pdf, HPDF_COMP_ALL);

  // Load fonts
  font_name = HPDF_LoadTTFontFromFile(pdf, "resources/arial.ttf", HPDF_TRUE);
  font = HPDF_GetFont(pdf, font_name, "UTF-8");

  font_name_bold =
      HPDF_LoadTTFontFromFile(pdf, "resources/arialbd.ttf", HPDF_TRUE);
  font_bold = HPDF_GetFont(pdf, font_name_bold, "UTF-8");

  char *lastname = malloc(100 * sizeof(char));
  int x;
  int lastvalue = 0;
  for (int i = 2; i < argc; i += 3) {
    strcpy(lastname, argv[i]);
    for (int j = 0; j < atoi(argv[i + 1]); j++) {
      lastvalue = atoi(argv[i + 2]);
      drawmoney(x++, lastname, lastvalue, page, font, font_bold, pdf);
      if (x % ids_per_page == 0 && j < atoi(argv[i]) - 1) {
        page = HPDF_AddPage(pdf);
      }
    }
  }

  if (FILL_REMAINING_PAGE) {
    for (x = x; x % ids_per_page != 0; x++) {
      // printf("Name: %s", line);
      drawmoney(x, lastname, lastvalue, page, font, font_bold, pdf);
    }
  }
  // Close the file when done with it

  HPDF_SaveToFile(pdf, fname);
  free(fname);
  free(filepath);
  HPDF_Free(pdf);
  return 0;
}
