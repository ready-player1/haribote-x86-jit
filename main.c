#include <acl.c>
#include <limits.h>

typedef unsigned char *String;

int loadText(String path, String text, int size)
{
  unsigned char buf[1000];

  int startPos = path[0] == '"'; // ダブルクォートがあれば外す
  int i = 0;
  while (path[ startPos + i ] != 0 && path[ startPos + i ] != '"') {
    buf[i] = path[ startPos + i ];
    ++i;
  }
  buf[i] = 0;

  FILE *fp = fopen(buf, "rt");
  if (fp == NULL) {
    printf("failed to open %s\n", path);
    return 1;
  }

  int nItems = fread(text, 1, size - 1, fp);
  fclose(fp);
  text[nItems] = 0;
  return 0;
}

#define MAX_TOKEN_CODE 1000 // 格納できるトークンコードの最大値
String tokenStrs[ MAX_TOKEN_CODE + 1 ]; // 添字に指定したトークンコードに対応するトークン文字列のポインタを格納する
int    tokenLens[ MAX_TOKEN_CODE + 1 ]; // トークン文字列の長さを格納する
unsigned char tokenBuf[ (MAX_TOKEN_CODE + 1) * 10 ]; // トークン文字列の実体を格納する

AInt vars[ MAX_TOKEN_CODE + 1 ]; // 変数

int getTokenCode(String str, int len)
{
  static int nTokens = 0, unusedHead = 0; // 登録済みのトークンの数, 未使用領域へのポインタ

  int i;
  for (i = 0; i < nTokens; ++i) { // 登録済みのトークンコードの中から探す
    if (len == tokenLens[i] && strncmp(str, tokenStrs[i], len) == 0)
      break;
  }
  if (i == nTokens) {
    if (nTokens >= MAX_TOKEN_CODE) {
      printf("too many tokens\n");
      exit(1);
    }
    strncpy(&tokenBuf[ unusedHead ], str, len); // 見つからなかった場合は新規登録する
    tokenBuf[ unusedHead + len ] = 0;
    tokenStrs[i] = &tokenBuf[ unusedHead ];
    tokenLens[i] = len;
    unusedHead += len + 1;
    ++nTokens;
    vars[i] = strtol(tokenStrs[i], NULL, 0); // 定数だった場合に初期値を設定（定数ではないときは0になる）
    if (tokenStrs[i][0] == '"') {
      char *p = malloc(len - 1);
      if (p == NULL) {
        printf("failed to allocate memory\n");
        exit(1);
      }
      vars[i] = (AInt) p;
      memcpy(p, tokenStrs[i] + 1, len - 2); // 手抜き実装（エスケープシーケンスを処理していない）
      p[len - 2] = 0;
    }
  }
  return i;
}

inline static int isAlphabet(unsigned char ch)
{
  return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_';
}

inline static int isNumber(unsigned char ch)
{
  return '0' <= ch && ch <= '9';
}

int lexer(String str, int *tokenCodes)
{
  int pos = 0, nTokens = 0; // 現在読んでいる位置, これまでに変換したトークンの数
  int len;
  for (;;) {
    while (str[pos] == ' ' || str[pos] == '\t' || str[pos] == '\n' || str[pos] == '\r')
      ++pos;
    if (str[pos] == 0)
      return nTokens;

    len = 0;
    if (strchr("(){}[];,", str[pos]) != NULL)
      len = 1;
    else if (isAlphabet(str[pos]) || isNumber(str[pos])) {
      while (isAlphabet(str[pos + len]) || isNumber(str[pos + len]))
        ++len;
    }
    else if (strchr("=+-*/!%&~|<>?:.#", str[pos]) != NULL) {
      while (strchr("=+-*/!%&~|<>?:.#", str[pos + len]) != NULL && str[pos + len] != 0)
        ++len;
    }
    else if (str[pos] == '"') { // "文字列"
      len = 1;
      while (str[pos + len] != str[pos] && str[pos + len] >= ' ')
        ++len;
      if (str[pos + len] == str[pos])
        ++len;
    }
    else {
      printf("syntax error: %.10s\n", &str[pos]);
      exit(1);
    }
    if (strncmp(&str[pos], "//", 2) == 0) {
comment:
      while (str[pos] != 0 && str[pos] != '\n')
        ++pos;
      continue;
    }
    if (len == 7 && strncmp(&str[pos], "include", 7) == 0) // includeを無視する
      goto comment;
    tokenCodes[nTokens] = getTokenCode(&str[pos], len);
    pos += len;
    ++nTokens;
  }
}

int tokenCodes[10000]; // トークンコードを格納する

enum keyId {
  WildCard = 0,
  WildCardForExpr,
  WildCardForExpr0,

  Tmp0,
  Tmp1,
  Tmp2,
  Tmp3,
  Tmp4,
  Tmp5,
  Tmp6,
  Tmp7,
  Tmp8,
  Tmp9,

  PlusPlus,

  Equal,
  NotEq,
  Les,
  GtrEq,
  LesEq,
  Gtr,
  Plus,
  Minus,
  Multi,
  Divi,
  Mod,
  BitwiseAnd,
  ShiftRight,
  And,

  Assigne,

  Lparen,
  Rparen,
  Lbracket,
  Rbracket,
  Lbrace,
  Rbrace,
  Period,
  Comma,
  Semicolon,
  Colon,

  Zero,
  One,
  Two,
  Three,
  Four,
  Five,
  Six,
  Seven,
  Eight,
  Nine,

  Print,
  If,
  Goto,
  Time,
  Else,
  For,
  Continue,
  Break,
  Prints,

  EndOfKeys
};

String defaultTokens[] = {
  "!!*",
  "!!**",
  "!!***",

  "_t0",
  "_t1",
  "_t2",
  "_t3",
  "_t4",
  "_t5",
  "_t6",
  "_t7",
  "_t8",
  "_t9",

  "++",

  "==",
  "!=",
  "<",
  ">=",
  "<=",
  ">",
  "+",
  "-",
  "*",
  "/",
  "%",
  "&",
  ">>",
  "&&",

  "=",

  "(",
  ")",
  "[",
  "]",
  "{",
  "}",
  ".",
  ",",
  ";",
  ":",

  "0",
  "1",
  "2",
  "3",
  "4",
  "5",
  "6",
  "7",
  "8",
  "9",

  "print",
  "if",
  "goto",
  "time",
  "else",
  "for",
  "continue",
  "break",
  "prints",
};

void initTokenCodes(String *defaultTokens, int len)
{
  if (len != EndOfKeys) {
    printf("warong number of default tokens: expected %d, got %d\n", EndOfKeys, len);
    exit(1);
  }
  for (int i = 0; i < len; ++i)
    tokenCodes[i] = getTokenCode(defaultTokens[i], strlen(defaultTokens[i]));
}

#define MAX_PHRASE_LEN 31
#define WPC_LEN 9
int tokenCodesForPhrase[ (MAX_PHRASE_LEN + 1) * 100 ]; // フレーズを字句解析して得たトークンコードを格納する
int nextPc, wpc[WPC_LEN]; // 一致したフレーズの次のトークンを指す, ワイルドカードのトークンの場所を指す
int wpcEnd[WPC_LEN]; // wpc[n]が式の場合、wpcEnd[n]はその式の直後のトークンを指す

// tokenCodes[pc]からのトークンコード列が、phraseで指定されたトークン列と一致するかどうか調べる
int match(int phraseId, String phrase, int pc)
{
  int head = phraseId * (MAX_PHRASE_LEN + 1); // フレーズを字句解析した結果を格納する（している）場所をphraseIdで指定
  int nTokens;
  if (tokenCodesForPhrase[ head + MAX_PHRASE_LEN ] == 0) { // 含まれるトークンの個数が0のフレーズは字句解析する
    nTokens = lexer(phrase, &tokenCodesForPhrase[head]);
    if (nTokens > MAX_PHRASE_LEN) {
      printf("too long phrase\n");
      exit(1);
    }
    tokenCodesForPhrase[ head + MAX_PHRASE_LEN ] = nTokens;
  }
  nTokens = tokenCodesForPhrase[ head + MAX_PHRASE_LEN ]; // フレーズに含まれるトークンの個数を取得
  int tokenCode;
  int depth;
  for (int i = 0, num; i < nTokens; ++i) {
    tokenCode = tokenCodesForPhrase[head + i];
    if (tokenCode == WildCard || tokenCode == WildCardForExpr || tokenCode == WildCardForExpr0) {
      /*
        WildCard（!!*#）
        任意の1トークンにマッチする（#は0～8までの数字）。

        WildCardForExpr（!!**#）
        任意の式にマッチする（#は0～8までの数字）。
        ただし、式は1トークン以上の長さでなければいけない）。

        WildCardForExpr0（!!***#）
        任意の式にマッチする（#は0～8までの数字）。
        ただし、式は長さゼロでもよい。
      */
      ++i;
      num = tokenCodesForPhrase[head + i] - Zero; // 後続の番号を取得
      wpc[num] = pc; // トークン・式の開始位置
      if (tokenCode == WildCard) {
        ++pc;
        continue;
      }
      depth = 0; // 括弧の深さ
      for (;;) {
        if (tokenCodes[pc] == Semicolon)
          break;
        if (tokenCodes[pc] == Comma && depth == 0)
          break;

        if (tokenCodes[pc] == Lparen || tokenCodes[pc] == Lbracket) // 手抜きで ( と [ を区別せずに数えている
          ++depth;
        if (tokenCodes[pc] == Rparen || tokenCodes[pc] == Rbracket)
          --depth;
        if (depth < 0)
          break;
        ++pc;
      }
      wpcEnd[num] = pc; // 式の終了位置
      if (tokenCode == WildCardForExpr && wpc[num] == pc)
        return 0;
      if (depth > 0)
        return 0;
      continue;
    }
    if (tokenCode != tokenCodes[pc])
      return 0; // マッチせず
    ++pc;
  }
  nextPc = pc;
  return 1; // マッチした
}

typedef AInt *IntPtr;

unsigned char *instructions;
unsigned char *ip; // instruction pointer

int jmps[10000]; // ジャンプ命令を書いた位置を格納する
int jp;

unsigned char *dumpBegin, *dumpEnd;

String opBins[] = { // 二項演算子のための機械語
  "8b_%1m0; 3b_%2m0; 0f_94_c0; 0f_b6_c0; 89_%0m0;",           // Equal
  "8b_%1m0; 3b_%2m0; 0f_95_c0; 0f_b6_c0; 89_%0m0;",           // NotEq
  "8b_%1m0; 3b_%2m0; 0f_9c_c0; 0f_b6_c0; 89_%0m0;",           // Les
  "8b_%1m0; 3b_%2m0; 0f_9d_c0; 0f_b6_c0; 89_%0m0;",           // GtrEq
  "8b_%1m0; 3b_%2m0; 0f_9e_c0; 0f_b6_c0; 89_%0m0;",           // LesEq
  "8b_%1m0; 3b_%2m0; 0f_9f_c0; 0f_b6_c0; 89_%0m0;",           // Gtr
  "8b_%1m0; 03_%2m0; 89_%0m0;",                               // Plus
  "8b_%1m0; 2b_%2m0; 89_%0m0;",                               // Minus
  "8b_%1m0; 0f_af_%2m0; 89_%0m0;",                            // Multi
  "8b_%1m0; 99; f7_%2m7; 89_%0m0;",                           // Divi
  "8b_%1m0; 99; f7_%2m7; 89_%0m2;",                           // Mod
  "8b_%1m0; 23_%2m0; 89_%0m0;",                               // BitwiseAnd
  "8b_%1m0; 8b_%2m1; d3_f8; 89_%0m0;",                        // ShiftRight
  "8b_%1m0; 23_%2m0; 83_f8_00; 0f_95_c0; 0f_b6_c0; 89_%0m0;", // And
};

inline static String getOpBin(int tokenCode)
{
  return opBins[tokenCode - Equal];
}

// 渡された文字が16進数に使える文字なら、それを0-15の数に変換して返す
char getHex(char ch)
{
  return '0' <= ch && ch <= '9' ? ch - '0'
       : 'a' <= ch && ch <= 'f' ? ch - 'a' + 10
       : 'A' <= ch && ch <= 'F' ? ch - 'A' + 10
       :                          -1
       ;
}

unsigned get32(unsigned char *p)
{
  return p[0] + p[1] * 256 + p[2] * 65536 + p[3] * 16777216;
}

void put32(unsigned char *p, unsigned i)
{
  p[0] =  i        & 0xff; // 1バイト目に、ビット0～7の内容を書き込む
  p[1] = (i >>  8) & 0xff; // 2バイト目に、ビット8～15の内容を書き込む
  p[2] = (i >> 16) & 0xff; // 3バイト目に、ビット16～23の内容を書き込む
  p[3] = (i >> 24) & 0xff; // 4バイト目に、ビット24～31の内容を書き込む
}

#define N_REGVAR 4

IntPtr regVarTable[N_REGVAR]; // レジスタ変数の割り当て状況を記録する配列
/*
  インデックスはレジスタ変数番号

  要素の値は次のどちらか
  レジスタ変数を割り当てている場合   -> regVarSaveLoad関数で、レジスタ変数の値を書き戻す／読み込むメモリのアドレス
  レジスタ変数を割り当てていない場合 -> 0
*/

// もしvarがレジスタに割り当てられていれば、0〜3を返す。そうでなければ-1を返す
int getRegVarNum(IntPtr var)
{
  for (int i = 0; i < N_REGVAR; ++i) {
    if (regVarTable[i] == var)
      return i;
  }
  return -1;
}

static inline int isRegVar(int regVarNum)
{
  return regVarNum >= 0;
}

int regVarNum2regCode[N_REGVAR] = { // レジスタ変数番号から、レジスタ番号に変換する配列
  3, // ebx
  5, // ebp
  6, // esi
  7  // edi
};

void decodeX86(String str, IntPtr *operands)
{
  for (int pos = 0; str[pos] != 0;) {
    if (str[pos] == ' ' || str[pos] == '\t' || str[pos] == '_' || str[pos] == ':' || str[pos] == ';')
      ++pos;
    else if (getHex(str[pos]) >= 0 && getHex(str[pos + 1]) >= 0) { // 16進数2桁（opcode）
      *ip = ((unsigned) getHex(str[pos]) << 4) | getHex(str[pos + 1]);
      ++ip;
      pos += 2;
    }
    else if (str[pos] == '%') {
      int i = str[pos + 1] - '0'; // 参照する追加引数の番号

      switch (str[pos + 2]) {
      unsigned reg;
      case 'm': // ModR/Mバイト
        /*
          ModR/Mバイトは、オペランドを参照する多くの命令でオペコードの次に置くことになっている1バイトで、
          アドレッシングモードを指定するために使われる。ModR/Mバイトには、以下の3つの情報フィールドがある。

          MSB                             LSB
          mod (2bit) | reg (3bit) | r/m (3bit)

          modフィールドは、r/mフィールドと組み合わせて、24個のアドレッシングモードと8個のレジスタをコード化する。
          modフィールドは、次のようにr/mの用途を切り替える。

          mod=00: [レジスタ+レジスタ]
          mod=01: [レジスタ+disp8]
          mod=10: [レジスタ+disp16/32]
          mod=11: レジスタ

          disp8/16/32はレジスタを指定する場合の変位（displacement）のことで、ベースアドレスに加算して
          実効アドレスを生成する。

          regフィールドは、レジスタ番号や追加オペコード情報を指定する。

          r/mフィールドは、modフィールドと組み合わせて24個のアドレッシングモードと8個のレジスタをコード化する。

          実効アドレスを得るために組み合わせるmodおよびr/mフィールドのコードと、regフィールドで指定する
          レジスタ番号や追加オペコード情報の値は以下の資料に示されている。

          『IA-32 インテル® アーキテクチャ・ソフトウェア・デベロッパーズ・マニュアル 中巻A：命令セット・リファレンスA-M』
          2.6.ModR/MおよびSIBバイトのアドレス指定モードのコード化 > 表2-2.  ModR/Mバイトによる32ビット・アドレス指定形式
          https://www.intel.co.jp/content/dam/www/public/ijkk/jp/ja/documents/developer/IA32_Arh_Dev_Man_Vol2A_i.pdf#G8.6121
        */
        reg = str[pos + 3] - '0';
        int regVarNum = getRegVarNum(operands[i]);
        if (isRegVar(regVarNum)) {
          *ip = 0xc0 | (reg << 3) | regVarNum2regCode[regVarNum]; // mod=11, reg=???, r/m=???
          ++ip;
        }
        else {
          *ip = 0x05 | (reg << 3); // mod=00, reg=???, r/m=101
          put32(ip + 1, (unsigned) operands[i]);
          ip += 5;
        }
        pos += 4;
        continue;
      case 'i': // int
        put32(ip, (unsigned) operands[i]);
        ip += 4;
        break;
      case 'c': // char
        put32(ip, (unsigned) operands[i]);
        ++ip;
        break;
      case 'r': // relative -> 現在位置（次の命令の先頭位置）からの相対値を計算して4バイトを書く拡張命令
        put32(ip, (unsigned) operands[i] - (unsigned) (ip + 4));
        ip += 4;
        break;
      case 'l': // label
        put32(ip, (unsigned) operands[i]);
        jmps[jp] = ip - instructions; // ジャンプ命令のラベルを書いた位置を記録する
        ++jp;
        ip += 4;
        break;
      }
      pos += 3;
    }
    else {
      printf("decode error: '%s'\n", str);
      exit(1);
    }
  }
}

void putIcX86(String instructionStr, IntPtr p0, IntPtr p1, IntPtr p2, IntPtr p3)
{
  IntPtr operands[4];
  operands[0] = p0;
  operands[1] = p1;
  operands[2] = p2;
  operands[3] = p3;
  decodeX86(instructionStr, operands);
}

enum { RvSave = 0x89, RvLoad = 0x8b };

void regVarSaveLoad(int op)
{
  for (int regVarNum = 0; regVarNum < N_REGVAR; ++regVarNum) {
    if (regVarTable[regVarNum] != 0) {
      putIcX86("%0c_%1c_%2i;",
          (IntPtr) op, (IntPtr) ( 0x05 | ((unsigned) regVarNum2regCode[regVarNum] << 3) ), regVarTable[regVarNum], 0);
    }
  }
}

#define N_TMPS 10
char tmpFlags[N_TMPS];

// 未使用の一時変数を確保する
int tmpAlloc()
{
  int i = 0;
  while (tmpFlags[i] != 0) {
    if (i >= N_TMPS)
      return -1;
    ++i;
  }
  tmpFlags[i] = 1;
  return Tmp0 + i;
}

// 一時変数を未使用の状態に戻す（ただし、指定されたトークンコードが一時変数でないときは何もしない）
void tmpFree(int tokenCode)
{
  if (Tmp0 <= tokenCode && tokenCode <= Tmp9)
    tmpFlags[ tokenCode - Tmp0 ] = 0;
}

clock_t t0;

void printInteger(int i)  { printf("%d\n", i); }
void printString(char *s) { printf("%s\n", s); }
void printElapsedTime()   { printf("time: %.3f[sec]\n", (clock() - t0) / (double) CLOCKS_PER_SEC); }

int ff16sin(int x) { return (int) (sin(x * (2 * 3.14159265358979323 / 65536)) * 65536); }
int ff16cos(int x) { return (int) (cos(x * (2 * 3.14159265358979323 / 65536)) * 65536); }

int toExit;

// acl library functions call
AWindow *win;

int  call_aRgb8(int r, int g, int b)    { return aRgb8(r, g, b); }
int  call_aXorShift32()                 { return aXorShift32(); }
int  call_aGetPix(int x, int y)         { return aGetPix(win, x, y); }
int  call_aInkey(int opt)               { return aInkey(win, opt); }
void call_aSetPix0(int x, int y, int c) { aSetPix0(win, x, y, c); }
void call_aFillRct0(int xsiz, int ysiz, int x0, int y0, int c) { aFillRect0(win, xsiz, ysiz, x0, y0, c); }
void call_aDrawStr0(int x, int y, int col, int bcol, char *s)  { aDrawStr0(win, x, y, col, bcol, s); }

void gprintDec(int x, int y, int len, int col, int bcol, int i)
{
  char s[100];
  sprintf(s, "%*d", len, i);
  aDrawStr0(win, x, y, col, bcol, s);
}

int call_aOpenWin(int xsiz, int ysiz, char *s)
{
  if (win != NULL) {
    if (win->xsiz < xsiz || win->ysiz < ysiz) {
      printf("openWin error\n");
      return 1;
    }
  }
  else
    win = aOpenWin(xsiz, ysiz, s, 0);
  return 0;
}

int call_aWait(int msec)
{
  if (msec == -1) {
    if (win != NULL)
      aFlushAll(win);
    return 1;
  }
  aWait(msec);
  return 0;
}

int bitblit(int xsiz, int ysiz, int x0, int y0, int *ary)
{
  AInt32 *p32 = &win->buf[x0 + y0 * win->xsiz];
  int i, j;
  for (j = 0; j < ysiz; ++j) {
    for (i = 0; i < xsiz; ++i)
      p32[i] = ary[i];
    ary += xsiz;
    p32 += win->xsiz;
  }
}

// array

AInt *aryNew(int nElems)
{
  AInt *ary = malloc(nElems * sizeof(AInt));
  if (ary == NULL) {
    printf("failed to allocate memory\n");
    exit(1);
  }
  memset((char *) ary, 0, nElems * sizeof(AInt));
  return ary;
}

void aryInit(AInt *ary, AInt *ip, int nElems)
{
  memcpy((char *) ary, (char *) ip, nElems * sizeof(AInt));
}

#define LOWEST_PRECEDENCE 99
int epc, epcEnd; // exprのためのpc（式のどこを実行しているかを指す）, その式の直後のトークンを指す

int evalExpression(int precedenceLevel); // evalInfixExpression()が参照するので
int expression(int num);
int exprPutIcX86(int er, int len, void *fn, int *err);

enum notationStyle { Prefix = 0, Infix };

typedef struct precedence {
  int operator;
  int level;
} Precedence;

#define N_OPERATORS 15

Precedence precedenceTable[2][ N_OPERATORS + 1 ] = {
  { // Prefix
    {PlusPlus, 2},
    {Minus, 2},
    {.level = LOWEST_PRECEDENCE + 1}
  },
  { // Infix
    {Multi, 4},
    {Divi, 4},
    {Mod, 4},
    {Plus, 5},
    {Minus, 5},
    {ShiftRight, 6},
    {LesEq, 7},
    {GtrEq, 7},
    {Les, 7},
    {Gtr, 7},
    {Equal, 8},
    {NotEq, 8},
    {BitwiseAnd, 9},
    {And, 12},
    {Assigne, 15},
    {.level = LOWEST_PRECEDENCE + 1}
  }
};

int getPrecedenceLevel(int notationStyle, int operator)
{
  int i = 0;
  Precedence *precedence;
  while ((precedence = &precedenceTable[notationStyle][i])->level != LOWEST_PRECEDENCE + 1) {
    if (operator == precedence->operator)
      break;
    ++i;
  }
  return precedence->level;
}

int evalInfixExpression(int i, int precedenceLevel, int op)
{
  int j, k;
  ++epc;
  j = evalExpression(precedenceLevel);
  k = tmpAlloc();
  putIcX86(getOpBin(op), &vars[k], &vars[i], &vars[j], 0);
  tmpFree(i);
  tmpFree(j);
  if (i < 0 || j < 0)
    return -1;
  return k;
}

int evalExpression(int precedenceLevel)
{
  int er = -1; // ここまでの計算結果が入っている変数のトークンコード（vars[er]で計算結果にアクセスできる）
  int e0 = 0, e1 = 0;

  nextPc = 0;

  if (match(99, "( !!**0 )", epc)) { // 括弧
    er = expression(0);
  }
  else if (tokenCodes[epc] == PlusPlus) { // 前置インクリメント
    ++epc;
    er = evalExpression(getPrecedenceLevel(Prefix, PlusPlus));
    putIcX86("8b_%0m0; 40; 89_%0m0;", &vars[er], 0, 0, 0);
  }
  else if (tokenCodes[epc] == Minus) { // 単項マイナス
    ++epc;
    e0 = evalExpression(getPrecedenceLevel(Prefix, Minus));
    er = tmpAlloc();
    putIcX86("8b_%1m0; f7_d8; 89_%0m0;", &vars[er], &vars[e0], 0, 0);
  }
  else if (match(71, "mul64shr(!!**0, !!**1, !!**2)", epc)) {
    e0 = expression(0);
    e1 = expression(1);
    int e2 = expression(2);
    er = tmpAlloc();
    putIcX86("8b_%2m1; 8b_%0m0; f7_%1m5; 0f_ad_d0; 89_%3m0;", &vars[e0], &vars[e1], &vars[e2], &vars[er]);
    /*
      8b_%2m1  -> mov r/m16/32,%ecx
      8b_%0m0  -> mov r/m16/32,%eax
      f7_%1m5  -> imul r/m16/32      # eaxに%mで指定した値を掛け算して、結果をedx:eaxに入れる
      0f_ad_d0 -> shrd %cl,%edx,%eax # edx:eaxの64bitをecxだけ右シフトする。でもeaxしか更新されない（edxはそのまま）
      89_%3m0  -> mov %eax,r/m16/32
    */
    tmpFree(e2);
    if (e2 < 0)
      e0 = -1;
  }
  else if (match(72, "aRgb8(!!**0, !!**1, !!**2)", epc)) {
    er = exprPutIcX86(er, 3, call_aRgb8, &e0);
  }
  else if (match(73, "aOpenWin(!!**0, !!**1, !!***2, !!***8)", epc)) {
    exprPutIcX86(0, 3, call_aOpenWin, &e0);
    putIcX86("85_c0; 0f_85_%0l;", &vars[toExit], 0, 0, 0); // test %eax,%eax; jz rel16/32;
    er = Zero;
  }
  else if (match(74, "aXorShift32()", epc)) {
    er = exprPutIcX86(er, 0, call_aXorShift32, &e0);
  }
  else if (match(75, "aGetPix(!!**8, !!**0, !!**1)", epc)) {
    er = exprPutIcX86(er, 2, call_aGetPix, &e0);
  }
  else if (match(76, "ff16sin(!!**0)", epc)) {
    er = exprPutIcX86(er, 1, ff16sin, &e0);
  }
  else if (match(77, "ff16cos(!!**0)", epc)) {
    er = exprPutIcX86(er, 1, ff16cos, &e0);
  }
  else if (match(78, "aInkey(!!***8, !!**0)", epc)) {
    er = exprPutIcX86(er, 1, call_aInkey, &e0);
  }
  else { // 変数もしくは定数
    er = tokenCodes[epc];
    ++epc;
  }
  if (nextPc > 0)
    epc = nextPc;

  int encountered; // ぶつかった演算子の優先順位を格納する
  int tokenCode;
  for (;;) {
    tmpFree(e0);
    tmpFree(e1);
    if (er < 0 || e0 < 0 || e1 < 0) // ここまででエラーがあれば、処理を打ち切り
      return -1;
    if (epc >= epcEnd)
      break;

    e0 = e1 = 0;
    tokenCode = tokenCodes[epc];
    if (tokenCode == PlusPlus) { // 後置インクリメント
      ++epc;
      e0 = er;
      er = tmpAlloc();
      putIcX86("8b_%1m0; 89_%0m0; 40; 89_%1m0;", &vars[er], &vars[e0], 0, 0);
    }
    else if (match(70, "[!!**0]", epc)) { // 配列の添字演算子式
      e1 = er;
      e0 = expression(0);
      epc = nextPc;
      if (tokenCodes[epc] == Assigne && (precedenceLevel >= (encountered = getPrecedenceLevel(Infix, Assigne)))) {
        ++epc;
        er = evalExpression(encountered);
        //                                               base       index
        putIcX86("8b_%2m0; 8b_%0m2; 8b_%1m1; 89_04_8a;", &vars[e1], &vars[e0], &vars[er], 0);
        /*
          8b_%2m0  -> mov r/m16/32,%eax
          8b_%0m2  -> mov r/m16/32,%edx
          8b_%1m1  -> mov r/m16/32,%ecx
          89_04_8a -> mov %eax,(%edx,%ecx,4)
        */
      }
      else {
        er = tmpAlloc();
        //                                               base       index
        putIcX86("8b_%0m2; 8b_%1m1; 8b_04_8a; 89_%2m0;", &vars[e1], &vars[e0], &vars[er], 0);
        /*
          8b_%0m2  -> mov r/m16/32,%edx
          8b_%1m1  -> mov r/m16/32,%ecx
          8b_04_8a -> mov (%edx,%ecx,4),%eax
          89_%2m0  -> mov %eax,r/m16/32
        */
      }
    }
    else if (precedenceLevel >= (encountered = getPrecedenceLevel(Infix, tokenCode))) {
      /*
        「引数として渡された優先順位」が「ぶつかった演算子の優先順位」よりも低いか又は等しい
        (値が大きいか又は等しい)ときは、このブロックを実行して中置演算子を評価する。

        「引数として渡された優先順位」が「ぶつかった演算子の優先順位」よりも高い(値が小さい)
        ときは、このブロックを実行せずにこれまでに式を評価した結果を呼び出し元に返す。
      */
      switch (tokenCode) {
      // 左結合
      case Multi: case Divi: case Mod:
      case Plus: case Minus:
      case ShiftRight:
      case Les: case LesEq: case Gtr: case GtrEq:
      case Equal: case NotEq:
      case BitwiseAnd:
      case And:
        er = evalInfixExpression(er, encountered - 1, tokenCode);
        break;
      // 右結合
      case Assigne:
        ++epc;
        e0 = evalExpression(encountered);
        putIcX86("8b_%1m0; 89_%0m0;", &vars[er], &vars[e0], 0, 0);
        break;
      }
    }
    else
      break;
  }
  return er;
}

// 引数として渡したワイルドカード番号にマッチした式をコンパイルしてinstructions[]に書き込む
int expression(int num)
{
  int expressionBegin = wpc   [num];
  int expressionEnd   = wpcEnd[num];
  if (expressionBegin == expressionEnd)
    return 0;

  // evalExpression()の中でmatch()やexpression()を呼び出すこともあり得るので、変数を退避しておく
  int oldEpc    = epc;
  int oldEpcEnd = epcEnd;
  int buf[WPC_LEN * 2 + 1];
  for (int i = 0; i < WPC_LEN; ++i) {
    buf[i]           = wpc   [i];
    buf[i + WPC_LEN] = wpcEnd[i];
  }
  buf[WPC_LEN * 2] = nextPc;

  epc    = expressionBegin;
  epcEnd = expressionEnd;
  int er = evalExpression(LOWEST_PRECEDENCE);
  if (epc < epcEnd) // 式を最後まで解釈できなかったらエラー
    return -1;

  // 保存しておいた変数を復元する
  epc    = oldEpc;
  epcEnd = oldEpcEnd;
  for (int i = 0; i < WPC_LEN; ++i) {
    wpc   [i] = buf[i];
    wpcEnd[i] = buf[i + WPC_LEN];
  }
  nextPc = buf[WPC_LEN * 2];
  return er;
}

enum conditionType { WhenConditionIsTrue = 0, WhenConditionIsFalse };

//                       je    jne   jl    jge   jle   jg
int conditionCodes[6] = {0x84, 0x85, 0x8c, 0x8d, 0x8e, 0x8f};

// 条件式wpc[num]を評価して、その結果に応じてlabel（トークンコード）に分岐する内部コードを生成する
void ifgoto(int num, int conditionType, int label) {
  int conditionBegin = wpc   [num];
  int conditionEnd   = wpcEnd[num];

  int *tc = tokenCodes, operator = tc[conditionBegin + 1];
  if ((conditionBegin + 3 == conditionEnd) && (Equal <= operator && operator <= Gtr)) {
    // mov r/m16/32,%eax; cmp r/m16/32,%eax; jcc rel16/32;
    putIcX86("8b_%2m0; 3b_%3m0; 0f_%0c_%1l;",
        (IntPtr) conditionCodes[ (operator - Equal) ^ conditionType ],
        &vars[label],
        &vars[tc[conditionBegin]],
        &vars[tc[conditionBegin + 2]]);
  }
  else {
    num = expression(num);
    // mov %eax,r/m16/32; test %eax,%eax; jcc rel16/32;
    putIcX86("8b_%2m0; 85_c0; 0f_%0c_%1l;", (IntPtr) (0x85 - conditionType), &vars[label], &vars[num], 0);
    tmpFree(num);
  }
}

int tmpLabelNo;

int tmpLabelAlloc()
{
  char str[10];
  sprintf(str, "_l%d", tmpLabelNo);
  ++tmpLabelNo;
  return getTokenCode(str, strlen(str));
}

// ラベルに対応するipの位置を記録する
void defLabel(int tokenCode)
{
  vars[tokenCode] = ip - instructions;
}

#define BLOCK_INFO_UNIT_SIZE 10
int blockInfo[ BLOCK_INFO_UNIT_SIZE * 100 ], blockDepth, loopDepth;
/*
  blockInfo[ blockDepth ] -> 現ブロックのblockTypeを取得する
  blockInfo[ blockDepth - BLOCK_INFO_UNIT_SIZE ] -> 現ブロックの1つ外側のブロックのblockTypeを取得する
  blockInfo[ blockDepth - BLOCK_INFO_UNIT_SIZE * 2 ] -> 現ブロックの2つ外側のブロックのblockTypeを取得する
*/

#define BLOCK_TYPE 0
enum blockType { IfBlock = 1, ForBlock, MainFnBlock };
enum ifBlockInfo { IfLabel0 = 1, IfLabel1 };
enum forBlockInfo { ForBegin = 1, ForContinue, ForBreak, ForLoopDepth, ForWpc1, ForWpcEnd1, ForWpc2, ForWpcEnd2 };

// lenで渡した数の式を評価し、その結果を使ってputIcX86をする
int exprPutIcX86(int er, int len, void *fn, int *err)
{
  int e[9] = {0};
  for (int i = 0; i < len; ++i) {
    if ((e[i] = expression(i)) < 0)
      *err = -1;
    putIcX86("8b_%0m0; 89_44_24_%1c;", &vars[e[i]], (IntPtr) (i * 4), 0, 0); // 89 44 24 ?? -> mov %eax,0x??(%esp)
  }

  putIcX86("e8_%0r;", fn, 0, 0, 0); // call rel16/32 <fn>

  for (int i = 0; i < len; ++i)
    tmpFree(e[i]);

  if (er != 0) {
    er = tmpAlloc();
    putIcX86("89_%0m0;", &vars[er], 0, 0, 0);
  }
  return er;
}

int codedump;

int compile(String sourceCode)
{
  int nTokens = lexer(sourceCode, tokenCodes);

  String *ts = tokenStrs;  // 添字に指定したトークンコードに対応するトークン文字列のポインタを格納している配列
  int    *tc = tokenCodes; // トークンコードを格納している配列
  tc[nTokens++] = Semicolon; // 末尾に「;」を付け忘れることが多いので、付けてあげる
  tc[nTokens] = tc[nTokens + 1] = tc[nTokens + 2] = tc[nTokens + 3] = Period; // エラー表示用

  ip = instructions;
  jp = 0;
  putIcX86("60; 83_ec_7c;", 0, 0, 0, 0); // pusha; sub $0x7c,%esp;
  regVarSaveLoad(RvLoad); // 前回実行時のレジスタ変数の値を引き継ぐ
  dumpBegin = ip;

  for (int i = 0; i < N_TMPS; ++i)
    tmpFlags[i] = 0;
  tmpLabelNo = 0;
  toExit = tmpLabelAlloc();
  blockDepth = loopDepth = 0;

  int pc;
  for (pc = 0; pc < nTokens;) {
    int *curBlock = &blockInfo[blockDepth];
    int e0 = 0, e2 = 0;
    if (match(1, "!!*0 = !!*1;", pc)) {
      putIcX86("8b_%1m0; 89_%0m0;", &vars[tc[wpc[0]]], &vars[tc[wpc[1]]], 0, 0);
    }
    else if (match(10, "!!*0 = !!*1 + 1; if (!!*2 < !!*3) goto !!*4;", pc) && tc[wpc[0]] == tc[wpc[1]] && tc[wpc[0]] == tc[wpc[2]]) {
      // mov r/m16/32,%eax; inc %eax; mov %eax,r/m16/32; cmp r/m16/32,%eax; jl rel16/32;
      putIcX86("8b_%1m0; 40; 89_%1m0; 3b_%2m0; 0f_8c_%0l;", &vars[tc[wpc[4]]], &vars[tc[wpc[0]]], &vars[tc[wpc[3]]], 0);
    }
    else if (match(9, "!!*0 = !!*1 + 1;", pc) && tc[wpc[0]] == tc[wpc[1]]) { // +1専用の命令
      putIcX86("8b_%0m0; 40; 89_%0m0;", &vars[tc[wpc[0]]], 0, 0, 0);
    }
    else if (match(2, "!!*0 = !!*1 !!*2 !!*3;", pc) && Equal <= tc[wpc[2]] && tc[wpc[2]] < Assigne) { // 加算、減算など
      putIcX86(getOpBin(tc[wpc[2]]), &vars[tc[wpc[0]]], &vars[tc[wpc[1]]], &vars[tc[wpc[3]]], 0);
    }
    else if (match(4, "print !!**0;", pc)) {
      exprPutIcX86(0, 1, printInteger, &e0);
    }
    else if (match(0, "!!*0:", pc)) { // ラベル定義命令
      defLabel(tc[wpc[0]]);
    }
    else if (match(5, "goto !!*0;", pc)) {
      putIcX86("e9_%0l;", &vars[tc[wpc[0]]], 0, 0, 0); // jmp rel16/32
    }
    else if (match(6, "if (!!**0) goto !!*1;", pc)) {
      ifgoto(0, WhenConditionIsTrue, tc[wpc[1]]);
    }
    else if (match(7, "time;", pc)) {
      exprPutIcX86(0, 0, printElapsedTime, &e0);
    }
    else if (match(11, "if (!!**0) {", pc)) { // ブロックif文
      blockDepth += BLOCK_INFO_UNIT_SIZE;
      curBlock = &blockInfo[blockDepth];
      curBlock[ BLOCK_TYPE ] = IfBlock;
      curBlock[ IfLabel0   ] = tmpLabelAlloc(); // 条件不成立のときの飛び先
      curBlock[ IfLabel1   ] = 0;
      ifgoto(0, WhenConditionIsFalse, curBlock[IfLabel0]);
    }
    else if (match(12, "} else {", pc) && curBlock[BLOCK_TYPE] == IfBlock) {
      curBlock[ IfLabel1 ] = tmpLabelAlloc(); // else節の終端
      putIcX86("e9_%0l;", &vars[curBlock[IfLabel1]], 0, 0, 0); // jmp rel16/32
      defLabel(curBlock[IfLabel0]);
    }
    else if (match(13, "}", pc) && curBlock[BLOCK_TYPE] == IfBlock) {
      if (curBlock[IfLabel1] == 0)
        defLabel(curBlock[IfLabel0]);
      else
        defLabel(curBlock[IfLabel1]);
      blockDepth -= BLOCK_INFO_UNIT_SIZE;
    }
    else if (match(14, "for (!!***0; !!***1; !!***2) {", pc)) { // for文
      blockDepth += BLOCK_INFO_UNIT_SIZE;
      curBlock = &blockInfo[blockDepth];
      curBlock[ BLOCK_TYPE  ] = ForBlock;
      curBlock[ ForBegin    ] = tmpLabelAlloc();
      curBlock[ ForContinue ] = tmpLabelAlloc();
      curBlock[ ForBreak    ] = tmpLabelAlloc();

      // ループを開始する直前のloopDepthとあとで式を評価する際に必要になる値を保存しておく
      curBlock[ ForLoopDepth ] = loopDepth;
      curBlock[ ForWpc1      ] = wpc   [1];
      curBlock[ ForWpcEnd1   ] = wpcEnd[1];
      curBlock[ ForWpc2      ] = wpc   [2];
      curBlock[ ForWpcEnd2   ] = wpcEnd[2];

      loopDepth = blockDepth;

      e0 = expression(0);
      if (wpc[1] < wpcEnd[1]) // !!***1に何らかの式が書いてある
        ifgoto(1, WhenConditionIsFalse, curBlock[ForBreak]); // 最初から条件不成立の場合、ブロックを実行しない
      defLabel(curBlock[ForBegin]);
    }
    else if (match(15, "}", pc) && curBlock[BLOCK_TYPE] == ForBlock) {
      defLabel(curBlock[ForContinue]);

      int wpc1 = curBlock[ForWpc1];
      int wpc2 = curBlock[ForWpc2];
      int isWpc1Les      = (wpc1 + 3 == curBlock[ForWpcEnd1]) && (tc[wpc1 + 1] == Les);
      int isWpc2PlusPlus = (wpc2 + 2 == curBlock[ForWpcEnd2]) &&
        ( (tc[wpc1] == tc[wpc2] && tc[wpc2 + 1] == PlusPlus) || (tc[wpc1] == tc[wpc2 + 1] && tc[wpc2] == PlusPlus) );

      if (isWpc1Les && isWpc2PlusPlus) {
        // mov r/m16/32,%eax; inc %eax; mov %eax,r/m16/32; cmp r/m16/32,%eax; jl rel16/32;
        putIcX86("8b_%1m0; 40; 89_%1m0; 3b_%2m0; 0f_8c_%0l;", &vars[curBlock[ForBegin]], &vars[tc[wpc1]], &vars[tc[wpc1 + 2]], 0);
      }
      else {
        wpc   [1] = curBlock[ ForWpc1    ];
        wpcEnd[1] = curBlock[ ForWpcEnd1 ];
        wpc   [2] = curBlock[ ForWpc2    ];
        wpcEnd[2] = curBlock[ ForWpcEnd2 ];

        e2 = expression(2);
        if (wpc[1] < wpcEnd[1]) // !!***1に何らかの式が書いてある
          ifgoto(1, WhenConditionIsTrue, curBlock[ForBegin]);
        else
          putIcX86("e9_%0l;", &vars[curBlock[ForBegin]], 0, 0, 0); // jmp rel16/32
      }
      defLabel(curBlock[ForBreak]);
      loopDepth = curBlock[ForLoopDepth];
      blockDepth -= BLOCK_INFO_UNIT_SIZE;
    }
    else if (match(16, "continue;", pc) && loopDepth > 0) {
      int *loopBlock = &blockInfo[loopDepth];
      putIcX86("e9_%0l;", &vars[loopBlock[ForContinue]], 0, 0, 0); // jmp rel16/32
    }
    else if (match(17, "break;", pc) && loopDepth > 0) {
      int *loopBlock = &blockInfo[loopDepth];
      putIcX86("e9_%0l;", &vars[loopBlock[ForBreak]], 0, 0, 0); // jmp rel16/32
    }
    else if (match(18, "if (!!**0) continue;", pc) && loopDepth > 0) {
      int *loopBlock = &blockInfo[loopDepth];
      ifgoto(0, WhenConditionIsTrue, loopBlock[ForContinue]);
    }
    else if (match(19, "if (!!**0) break;", pc) && loopDepth > 0) {
      int *loopBlock = &blockInfo[loopDepth];
      ifgoto(0, WhenConditionIsTrue, loopBlock[ForBreak]);
    }
    else if (match(20, "prints !!**0;", pc)) {
      exprPutIcX86(0, 1, printString, &e0);
    }
    else if (match(21, "int !!*0[!!**2];", pc)) {
      e2 = expression(2);
      //                                                 base               index
      putIcX86("8b_%1m0; 89_44_24_00; e8_%2r; 89_%0m0;", &vars[tc[wpc[0]]], &vars[e2], (IntPtr) aryNew, 0);
      /*
        8b_%1m0     -> mov r/m16/32,%eax
        89_44_24_00 -> mov %eax,0x0(%esp)
        e8_%2r      -> call rel16/32 <aryNew>
        89_%0m0     -> mov %eax,r/m16/32
      */
    }
    else if (match(22, "int !!*0[!!**2] = {", pc)) {
      e2 = expression(2);
      putIcX86("8b_%1m0; 89_44_24_00; e8_%2r; 89_%0m0;", &vars[tc[wpc[0]]], &vars[e2], (IntPtr) aryNew, 0);

      int pc, nElems = 0;
      for (pc = nextPc; tc[pc] != Rbrace; ++pc) {
        if (pc >= nTokens)
          goto err;
        if (tc[pc] != Comma)
          ++nElems;
      }
      AInt *ary = malloc(nElems * sizeof(AInt));
      if (ary == NULL) {
        printf("failed to allocate memory\n");
        exit(1);
      }

      nElems = 0;
      for (pc = nextPc; tc[pc] != Rbrace; ++pc) {
        if (tc[pc] == Comma)
          continue;
        ary[nElems] = vars[tc[pc]];
        ++nElems;
      }
      putIcX86("8b_%0m0; 89_44_24_00; b8_%1i; 89_44_24_04; b8_%2i; 89_44_24_08; e8_%3r;",
          &vars[tc[wpc[0]]], (IntPtr) ary, (IntPtr) nElems, (IntPtr) aryInit);
      /*
        8b_%0m0     -> mov r/m16/32,%eax       # 引数3
        89_44_24_00 -> mov %eax,0x0(%esp)
        b8_%1i      -> mov imm16/32,%eax
        89_44_24_04 -> mov %eax,0x4(%esp)      # 引数2
        b8_%2i      -> mov imm16/32,%eax
        89_44_24_08 -> mov %eax,0x8(%esp)      # 引数1
        e8_%3r      -> call rel16/32 <aryInit>
      */
      nextPc = pc + 2; // } と ; の分
    }
    else if (match(23, "aSetPix0(!!***8, !!**0, !!**1, !!**2);", pc)) {
      exprPutIcX86(0, 3, call_aSetPix0, &e0);
    }
    else if (match(24, "aWait(!!**0);", pc)) {
      exprPutIcX86(0, 1, call_aWait, &e0);
      putIcX86("85_c0; 0f_85_%0l;", &vars[toExit], 0, 0, 0); // test %eax,%eax; jz rel16/32;
    }
    else if (match(25, "aFillRect0(!!***8, !!**0, !!**1, !!**2, !!**3, !!**4);", pc)) {
      exprPutIcX86(0, 5, call_aFillRct0, &e0);
    }
    else if (match(26, "aDrawStr0(!!***8, !!**0, !!**1, !!**2, !!**3, !!**4);", pc)) {
      exprPutIcX86(0, 5, call_aDrawStr0, &e0);
    }
    else if (match(27, "gprintDec(!!***8, !!**0, !!**1, !!**2, !!**3, !!**4, !!**5);", pc)) {
      exprPutIcX86(0, 6, gprintDec, &e0);
    }
    else if (match(28, "bitblt(!!***8, !!**0, !!**1, !!**2, !!**3, !!**4);", pc)) {
      exprPutIcX86(0, 5, bitblit, &e0);
    }
    else if (match(29, "printTime();", pc)) { // time;と同じ（C言語っぽく書けるようにした）
      exprPutIcX86(0, 0, printElapsedTime, &e0);
    }
    else if (match(30, "void aMain() {", pc)) {
      blockDepth += BLOCK_INFO_UNIT_SIZE;
      curBlock = &blockInfo[blockDepth];
      curBlock[BLOCK_TYPE] = MainFnBlock; // ただ認識しているだけで何もしていない
    }
    else if (match(31, "}", pc) && curBlock[BLOCK_TYPE] == MainFnBlock) {
      blockDepth -= BLOCK_INFO_UNIT_SIZE; // ただコードブロックを閉じているだけ
    }
    else if (match(32, "#", pc)) {
      ;
    }
    else if (match(33, "int", pc) || match(34, "AWindow", pc)) {
      while (tc[pc] != Semicolon)
        ++pc;
      nextPc = pc;
    }
    else if (match(35, "codedump !!*0", pc)) {
      codedump = vars[tc[wpc[0]]];
    }
    else if (match(36, "code", pc)) {
      for (++pc; tc[pc] != Semicolon; ++pc) {
        if (tc[pc] == Comma)
          continue;
        *ip = vars[tc[pc]];
        ++ip;
      }
      nextPc = pc + 1;
    }
    else if (match(37, "regVar(!!*0", pc)) {
      int regVarNum = tc[wpc[0]] - Zero;
      int firstNum = regVarNum;

      IntPtr tmp[N_REGVAR];
      int pc;
      for (pc = nextPc; tc[pc] != Rparen; ++pc) {
        if (pc >= nTokens)
          goto err;
        if (tc[pc] == Comma)
          continue;

        if (0 <= regVarNum && regVarNum < N_REGVAR)
          tmp[regVarNum] = tc[pc] == Zero ? 0 : &vars[tc[pc]];
        ++regVarNum;
      }

      regVarSaveLoad(RvSave);
      int lastNum = regVarNum;
      for (regVarNum = firstNum; regVarNum < lastNum; ++regVarNum)
        regVarTable[regVarNum] = tmp[regVarNum];
      regVarSaveLoad(RvLoad);

      nextPc = pc + 2; // ) と ; の分
    }
    else if (match(8, "!!***0;", pc)) {
      e0 = expression(0);
    }
    else {
      goto err;
    }
    tmpFree(e0);
    tmpFree(e2);
    if (e0 < 0 || e2 < 0)
      goto err;
    pc = nextPc;
  }
  if (blockDepth > 0) {
    printf("block nesting error: blockDepth=%d, loopDepth=%d, pc=%d, nTokens=%d\n", blockDepth, loopDepth, pc, nTokens);
    return -1;
  }
  defLabel(toExit);
  dumpEnd = ip;
  regVarSaveLoad(RvSave); // 次回実行時にレジスタ変数の値を引き継ぐ
  putIcX86("83_c4_7c; 61; c3;", 0, 0, 0, 0); // add $0x7c,%esp; popa; ret;
  unsigned char *end = ip, *src, *dest;
  for (int i = 0; i < jp; ++i) { // ジャンプ命令の最適化
    src  = jmps[i] + instructions;
    dest = *(IntPtr) get32(src) + instructions;
    while (*dest == 0xe9) { // 飛び先がjmp命令だったら、さらにその先を読む
      put32(src, get32(dest + 1));
      dest = *(IntPtr) get32(dest + 1) + instructions;
    }
  }
  for (int i = 0; i < jp; ++i) { // 飛び先を指定する（相対値にする）
    src  = jmps[i] + instructions;
    dest = *(IntPtr) get32(src) + instructions;
    put32(src, dest - (src + 4));
  }
  return end - instructions;
err:
  printf("syntax error: %s %s %s %s\n", ts[tc[pc]], ts[tc[pc + 1]], ts[tc[pc + 2]], ts[tc[pc + 3]]);
  return -1;
}

AWindow *win;

int run(String sourceCode)
{
  if (compile(sourceCode) < 0)
    return 1;
  if (codedump == 0) {
    void (*exec)() = (void (*)()) instructions;
    t0 = clock();
    exec();
    if (win != NULL)
      aFlushAll(win);
  }
  else {
    int nBytes = dumpEnd - dumpBegin;
    for (int i = 0; i < nBytes; ++i)
      printf("%02x ", *(dumpBegin + i));
    if (nBytes)
      printf("\n");
    printf("(len=%d)\n", nBytes);
  }
  return 0;
}

#include <unistd.h>
#include <sys/mman.h>

void *mallocRWX(int len)
{
  void *addr = mmap(0, len, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if (addr == MAP_FAILED) {
    printf("mmap failed\n");
    exit(1);
  }
  return addr;
}

void aMain()
{
  unsigned char text[10000]; // ソースコード

  instructions = mallocRWX(1024 * 1024);
  initTokenCodes(defaultTokens, sizeof defaultTokens / sizeof defaultTokens[0]);

  if (aArgc >= 2) {
    if (loadText((String) aArgv[1], text, 10000) != 0)
      exit(1);
    run(text);
    exit(0);
  }

  int inputLen;
  int hasRemovedSemicolon;
  char *semicolonPos;
  for (int i = 0;; ++i) { // Read-Eval-Print Loop
    printf("[%d]> ", i);
    fgets(text, 10000, stdin);
    inputLen = strlen(text);
    if (text[inputLen - 1] == '\n') // chomp
      text[inputLen - 1] = 0;

    hasRemovedSemicolon = 0;
    for (int i = strlen(text) - 1; i >= 0; --i) {
      if (text[i] == ';') {
        text[i] = 0;
        semicolonPos = &text[i];
        hasRemovedSemicolon = 1;
        break;
      }
    }

    if (strcmp(text, "exit") == 0)
      exit(0);
    if (strncmp(text, "run ", 4) == 0) {
      if (loadText(&text[4], text, 10000) == 0)
        run(text);
      continue;
    }
    if (hasRemovedSemicolon)
      *semicolonPos = ';';
    run(text);
  }
}
