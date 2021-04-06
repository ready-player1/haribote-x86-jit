#include <stdio.h>
#include <stdlib.h>

void loadText(int argc, const char **argv, unsigned char *text, int size)
{
  if (argc < 2) {
    printf("usage: %s program-file\n", argv[0]);
    exit(1);
  }

  FILE *fp = fopen(argv[1], "rt");
  if (fp == NULL) {
    printf("failed to open %s\n", argv[1]);
    exit(1);
  }

  int nItems = fread(text, 1, size - 1, fp);
  fclose(fp);
  text[nItems] = 0;
}

int main(int argc, const char **argv)
{
  unsigned char text[10000]; // ソースコード
  int vars[256]; // 変数

  loadText(argc, argv, text, 10000);

  for (int i = 0; i < 10; ++i)
    vars['0' + i] = i;

  int pc;
  for (pc = 0; text[pc] != 0; ++pc) {
    if (text[pc] == '\n' || text[pc] == '\r' || text[pc] == ' ' || text[pc] == '\t' || text[pc] == ';')
      continue;

    if (text[pc + 1] == '=' && text[pc + 3] == ';')
      vars[text[pc]] = vars[text[pc + 2]];
    else if (text[pc + 1] == '=' && text[pc + 3] == '+' && text[pc + 5] == ';')
      vars[text[pc]] = vars[text[pc + 2]] + vars[text[pc + 4]];
    else if (text[pc + 1] == '=' && text[pc + 3] == '-' && text[pc + 5] == ';')
      vars[text[pc]] = vars[text[pc + 2]] - vars[text[pc + 4]];
    else if (text[pc] == 'p' && text[pc + 1] == 'r' && text[pc + 5] == ' ' && text[pc + 7] == ';')
      printf("%d\n", vars[text[pc + 6]]);
    else
      goto err;

    while (text[pc] != ';')
      ++pc;
  }
  exit(0);
err:
  printf("syntax error: %.10s\n", &text[pc]);
  exit(1);
}
