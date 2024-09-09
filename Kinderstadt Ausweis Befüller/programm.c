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
const int ids_per_page = 10;
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

void draw_image(HPDF_Doc pdf, const HPDF_Image image, float x, float y, float scale) {

  HPDF_Page page = HPDF_GetCurrentPage(pdf);

  /* Draw image to the canvas. */
  HPDF_Page_DrawImage(page, image, x + 1, y + 1,
                      HPDF_Image_GetWidth(image) / scale,
                      HPDF_Image_GetHeight(image) / scale);
}

void drawjpg_image(HPDF_Doc pdf, const char *filename, float x, float y) {
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
  draw_image(pdf, image, x, y, 2.8);

  free(filename1);
}

void drawpng_image(HPDF_Doc pdf, const char *filename, float x, float y) {
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
  draw_image(pdf, image, x, y, 18);

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

void drawIdentity(int x, char *name, char *add_name, const bool foto, const char *group,
                  HPDF_Page page, HPDF_Font font, HPDF_Font fontbd,
                  HPDF_Doc pdf, double *color) {
  //bool aftername = strcmp(add_name, "") == 0;

  int pageHeight = HPDF_Page_GetHeight(page);
  int pageWidth = HPDF_Page_GetWidth(page);
  double xwidth = 8.5;
  double xtoy = 5.2 / xwidth;
  int idsperline = 2;
  int idsperrow = ids_per_page / idsperline;
  double width = 267;
  double lettersize = 6;
  double min_space = 11;
  double min_inner_space = 2;
  double height = width * xtoy;
  double groupspace_xtoy = 1.2 / xwidth;
  double groupspace = width * groupspace_xtoy;
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
  int posy = x / 2;
  posy =
      pageHeight - (outer_space_Y + (posy + 1) * height + posy * inner_space_Y);
  int posx = x % 2 == 0 ? outer_space_X : width + outer_space_X + inner_space_X;

  HPDF_Page_SetLineWidth(page, 0);

  /* Draw Rectangle */
  HPDF_Page_SetLineWidth(page, 0);
  HPDF_Page_SetRGBStroke(page, 0, 0, 0);
  HPDF_Page_SetRGBFill(page, color[0], color[1], color[2]);

  draw_rect(page, posx, posy, width, height);
  HPDF_Page_Stroke(page);

  draw_rect(page, posx, posy + height - groupspace, width, groupspace);
  HPDF_Page_FillStroke(page);

  /* Draw Image */

  drawjpg_image(pdf, "resources/Bild1.jpg", posx, posy);
  drawjpg_image(pdf, "resources/Bild1.jpg", posx + width / 3, posy);
  drawjpg_image(pdf, "resources/Bild2.jpg", posx + width - 93, posy);

  drawpng_image(pdf, (foto)?"resources/FotoJa.png":"resources/FotoNein.png", posx+width-34, posy+height-34);

  /*Text*/
  if (strcmp(name, "\0") == 0) {
    HPDF_Page_SetFontAndSize(page, fontbd, 24);
    HPDF_Page_SetRGBFill(page, 1.0, 1.0, 1.0);
    text(page, posx + width / 2 - sizeof(group) * lettersize * 0.85,
         posy + height - 15, group);
    return;
  }
  HPDF_Page_SetFontAndSize(page, fontbd, 35);
  if (strlen(name) > 9) {
    HPDF_Page_SetFontAndSize(page, fontbd, 32);
  }
  if (strlen(name) > 12) {
    HPDF_Page_SetFontAndSize(page, fontbd, 28);
  }

  HPDF_Page_SetRGBFill(page, 0.0, 0.0, 0.0);

  //int linelength = 9;
  //int maxlineexpand = 4;
  int yoffset1line = (height - groupspace) / 2 + lettersize * 2;
  int yoffset2line = (height - groupspace) / 5;
  text(page, posx + lettersize * 2, posy + yoffset1line * 1.05, name);
  HPDF_Page_SetFontAndSize(page, fontbd, 18);
  if (strlen(add_name) > 20) {
    HPDF_Page_SetFontAndSize(page, fontbd, 16);
  }
  if (strlen(add_name) > 28) {
    HPDF_Page_SetFontAndSize(page, fontbd, 12);
  }
  text(page, posx + lettersize * 2, posy + yoffset1line - yoffset2line,
       add_name);
  HPDF_Page_SetFontAndSize(page, fontbd, 24);
  HPDF_Page_SetRGBFill(page, 1.0, 1.0, 1.0);
  text(page, posx + width / 2 - sizeof(group) * lettersize * 0.85,
       posy + height - 15, group);
  
}

int main(int argc, char const *argv[])  {
  //const char *page_title = "Ausweise";

  HPDF_Doc pdf;
  const char *font_name_bold;
  const char *font_name;
  HPDF_Font font;
  HPDF_Font font_bold;
  HPDF_Page page;
  if(argc < 2) {
    printf("Usage: %s <outputfile> <1 to use pipelining>\n", argv[0]);
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

  double *color = malloc(3 * sizeof(double));
  color[0] = 0.0;
  color[1] = 0.0;
  color[2] = 0.0;

  FILE *file = NULL;

  if (argc >= 2 && (strcmp(argv[2], "1") == 0 || strcmp(argv[2], "pipe") == 0 ||
                    strcmp(argv[2], "PIPE") == 0)) {
    // printf("Reading from stdin\n");
    file = stdin;
  } else {
    strcat(filepath, argv[2]);
    printf("Reading from file %s\n", filepath);
    file = fopen(filepath, "r");

    if (!file) {
      printf("\n Unable to open : %s ", filepath);
      return -1;
    }
  }
  int x = 0;
  char *group_name = malloc(256 * sizeof(char)); // Adjust size as needed
  char line[256];
  while (fgets(line, sizeof(line) / sizeof(char), file)) {
    // printf("##%s", line);
    if (line[0] == '/') {
      break;
    }
    if (line[0] == '#') {
      line[strlen(line) - 1] = '\0';
      strcpy(group_name, line + 1); // No need to allocate or free
      fgets(line, sizeof(line), file);
      color[0] = atoi(line)/255.0;
      fgets(line, sizeof(line), file);
      color[1] = atoi(line)/255.0;
      fgets(line, sizeof(line), file);
      color[2] = atoi(line)/255.0;
      printf("Group: %s with color: %f %f %f\n", group_name, color[0], color[1],
             color[2]);
      continue;
    }
    // printf("Name: %s", line);
    char aftername[256];
    fgets(aftername, sizeof(aftername), file);
    if (strstr(line, "\0") == NULL || strcmp(line, "") != 0) {
      line[strlen(line) - 1] = '\0';
    }
    if (strstr(aftername, "\0") == NULL || strcmp(aftername, "") != 0) {
      aftername[strlen(aftername) - 1] = '\0';
    }
    char foto[256];
    fgets(foto, sizeof(foto), file);
    if (strstr(foto, "\0") == NULL || strcmp(foto, "") != 0) {
      foto[strlen(foto) - 1] = '\0';
    }
    bool want_foto = false;
    if(foto[0] == '1'|| foto[0] == 'J' || foto[0] == 'j'|| foto[0] == 'Y' || foto[0] == 'y'|| foto[0] == 'T' || foto[0] == 't') {
      want_foto = true;
    }
    printf("%d. Name: %s %s aus %s\n", x, line, aftername, group_name);
    drawIdentity(x++ % ids_per_page, line, aftername, want_foto, group_name, page, font,
                 font_bold, pdf, color);
    if (x % ids_per_page == 0) {
      page = HPDF_AddPage(pdf);
    }
  }
  for (int z = 0; z < min_fill_templates; z++) {
    // printf("Name: %s", line);
    drawIdentity(x++ % ids_per_page, "\0", "\0", false, group_name, page, font,
                 font_bold, pdf, color);
    if (x % ids_per_page == 0 && z < min_fill_templates - 1) {
      page = HPDF_AddPage(pdf);
    }
  }
  if (FILL_REMAINING_PAGE) {
    if (x % ids_per_page == 0) {
      drawIdentity(x++ % ids_per_page, "\0", "\0", false, group_name, page, font,
                   font_bold, pdf, color);
    }
    for (x = x; x % ids_per_page != 0; x++) {
      // printf("Name: %s", line);
      drawIdentity(x % ids_per_page, "\0", "\0", false, group_name, page, font,
                   font_bold, pdf, color);
    }
  }

  // Close the file when done with it
  if (file != stdin) {
    fclose(file);
  }
  HPDF_SaveToFile(pdf, fname);
  free(fname);
  free(filepath);
  HPDF_Free(pdf);

  free(color);
  free(group_name);

  return 0;
}
