#include <ctype.h>
#include <dirent.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wchar.h>

typedef struct group {
  char *name;
  int color_r;
  int color_g;
  int color_b;
} group;

int main(int argc, char const *argv[]) {

  // printf("-4");
  if (argc < 2) {
    printf("/\n");
    return 0;
  }
  char *filepath_groups = (char *)malloc((strlen(argv[1]) + 3) * sizeof(char));
  strcpy(filepath_groups, "");
  strcat(filepath_groups, argv[1]);
  FILE *file = fopen(filepath_groups, "r");
  if (!file) {
    printf("/\n");
    return 0;
  }

  free(filepath_groups);
  char group_line[256];
  group *groups = (group *)malloc(sizeof(group) * 256);
  int group_count = 0;

  // printf("-3");
  while (fgets(group_line, sizeof(group_line) / sizeof(char), file)) {

    // printf("##%s", line);
    if (group_line[0] == '/') {
      break;
    }
    if (group_line[0] == '\n') {
      continue;
    }
    if (group_line[0] == '#') {
      group_line[strlen(group_line)] = '\0';
      group_line[strcspn(group_line, "\n")] = 0;
      char *name = group_line + 1;
      char *group_name = malloc(sizeof(char) * sizeof(group_line));
      strcpy(group_name, name); // No need to allocate or free
      groups[group_count].name = group_name;
      fgets(group_line, sizeof(group_line), file);
      groups[group_count].color_r = atoi(group_line);
      fgets(group_line, sizeof(group_line), file);
      groups[group_count].color_g = atoi(group_line);
      fgets(group_line, sizeof(group_line), file);
      groups[group_count].color_b = atoi(group_line);
      // printf("%s:%f,%f,%f\n", groups[group_count].name,
      // groups[group_count].color_r, groups[group_count].color_g,
      // groups[group_count].color_b);
      group_count++;
    }
  }

  fclose(file);
  if (argc < 6) {
    printf("/\n");
    return 0;
  }
  if (strcmp(argv[3], argv[4]) == 0 || strcmp(argv[4], argv[5]) == 0 ||
      strcmp(argv[3], argv[5]) == 0 || strcmp(argv[3], argv[6])== 0 || strcmp(argv[4], argv[6])== 0 || strcmp(argv[5], argv[6])== 0 || group_count == 0) {
    printf("/\n");
    return 0;
  }

  // printf("-2");
  char *filepath = (char *)malloc((strlen(argv[2]) + 3) * sizeof(char));
  strcpy(filepath, "");
  strcat(filepath, "./");
  strcat(filepath, argv[2]);
  FILE *file2 = fopen(filepath, "r");

  if (!file2) {
    printf("/\n");
    return 0;
  }
  free(filepath);
  char line[256 * 4];
  fgets(line, sizeof(line) / sizeof(char), file2);
  int key_index[4]; // 1. role, 2. prename, 3. aftername, 4. extra
  key_index[0] = 0;
  key_index[1] = 0;
  key_index[2] = 0;
  key_index[3] = 0;
  // printf("-1");
  for (int i = 3; i < 7; i++) {
    char *b = strstr(line, argv[i]);
    int e = 0;
    if (b != NULL)                     /* strstr returns NULL if item not found */
    {
      e = b - line;
    }else{
      printf("/\n");
      return 0;
    }
    if (e < 0 || e > (int)strlen(line)) {
      printf("/\n");
      return 0;
    }

    for (int j = 0; j < e; j++) {
      if (line[j] == ';' || line[j] == ',' || line[j] == '\"') {
        key_index[(i - 3)]++;
      }
    }
  }

  // printf("0\n");
  group last;
  last.name = "Role not found";
  last.color_r = 0;
  last.color_g = 0;
  last.color_b = 0;
  groups[group_count] = last;
  group_count++;
  while (fgets(line, sizeof(line) / sizeof(char), file2)) {
    char **args = (char **)malloc(256 * 8 * sizeof(char *));
    int argsc = 0;
    int argcount = 0;
    if (line[argsc] == ';') {
      args[argsc++] = "";
      argcount++;
    }
    if (line[argsc] == ';') {
      args[argsc++] = "";
      argcount++;
    }
    char *token = strtok(line, ";,");
    do {
      args[argsc++] = token;
      token = strtok(NULL, ";,");
      argcount++;
    } while (args[argsc - 1] != NULL);
    if (argsc < key_index[0] || argsc < key_index[1] || argsc < key_index[2] || argsc < key_index[3]) {
      printf("/\n");
      return 0;
    }
    argcount--;
    char *role = malloc(256 * sizeof(char));
    role = args[key_index[0]];
    role = strtok(role, " \n\r\";");
    bool found = false;
    if (strcmp(last.name, role) == 0) {
      found = true;
    }
    for (int i = 0; i < group_count && !found; i++) {
      if (strcmp(groups[i].name, role) == 0) {
        last = groups[i];
        printf("#%s\n", groups[i].name);
        printf("%d\n", groups[i].color_r);
        printf("%d\n", groups[i].color_g);
        printf("%d\n", groups[i].color_b);
        found = true;
      }
    }
    if (!found) {
      printf("#%s\n", groups[group_count - 1].name);
      printf("%d\n", groups[group_count - 1].color_r);
      printf("%d\n", groups[group_count - 1].color_g);
      printf("%d\n", groups[group_count - 1].color_b);
    }
    if (strcmp(args[key_index[1]], "") == 0) {
      printf("\n");
    } else {
      char *prename = malloc(256 * sizeof(char));
      prename = args[key_index[1]];
      prename = strtok(prename, " \n\r\";");
      printf("%s\n", prename);
    }
    if (strcmp(args[key_index[2]], "") == 0) {
      printf("\n");
    } else {
      char *aftername = malloc(256 * sizeof(char));
      aftername = args[key_index[2]];
      aftername = strtok(aftername, " \n\r\";");
      printf("%s\n", aftername);
    }
    if(argcount < 4){
      printf("\n");
      continue;
    }
    if (strcmp(args[key_index[3]], "") == 0) {
      printf("\n");
    } else {
      char *foto = malloc(256 * sizeof(char));
      foto = args[key_index[3]];
      foto = strtok(foto, " \n\r\";");
      printf("%s\n", foto);
    }
    free(args);
  }
  /*for (int i = 0; i < group_count; i++) {
    free(groups[i].name);
  }
  free(groups);*/
  printf("/\n");
  fclose(file2);
  return 0;
}
