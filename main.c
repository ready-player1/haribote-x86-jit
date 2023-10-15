#include <acl.c>
#include <assert.h>

typedef unsigned char *String;

int loadText(String path, String text, int size)
{
  unsigned char buf[1000];

  int startPos = path[0] == '"'; // ダブルクォートがあれば外す
  int i = 0;
  while (path[startPos + i] != 0 && path[startPos + i] != '"') {
    buf[i] = path[startPos + i];
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
String tokenStrs[MAX_TOKEN_CODE + 1];
int    tokenLens[MAX_TOKEN_CODE + 1];

AInt vars[MAX_TOKEN_CODE + 1];

int getTokenCode(String str, int len)
{
  static unsigned char tokenBuf[(MAX_TOKEN_CODE + 1) * 10];
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

int lexer(String str, int *tc)
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
      printf("lexing error: %.10s\n", &str[pos]);
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
    tc[nTokens] = getTokenCode(&str[pos], len);
    pos += len;
    ++nTokens;
  }
}

int tc[10000]; // トークンコードを格納する

enum keyId {
  Wildcard = 0,
  Expr,
  Expr0,

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
  And,
  ShiftRight,
  AndAnd,

  Assign,

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

void initTc(String *defaultTokens, int len)
{
  assert(len == EndOfKeys);
  for (int i = 0; i < len; ++i)
    tc[i] = getTokenCode(defaultTokens[i], strlen(defaultTokens[i]));
}

#define PHRASE_LEN 31
#define WPC_LEN 9
int phraseTc[(PHRASE_LEN + 1) * 100]; // フレーズを字句解析して得たトークンコードを格納する
int nextPc, wpc[WPC_LEN]; // 一致したフレーズの次のトークンを指す, ワイルドカードのトークンの場所を指す
int wpcEnd[WPC_LEN]; // wpc[n]が式の場合、wpcEnd[n]はその式の直後のトークンを指す

int match(int phraseId, String phrase, int pc)
{
  int head = phraseId * (PHRASE_LEN + 1), nTokens;

  if (phraseTc[head + PHRASE_LEN] == 0) {
    nTokens = lexer(phrase, &phraseTc[head]);
    assert(nTokens <= PHRASE_LEN);
    phraseTc[head + PHRASE_LEN] = nTokens;
  }

  nTokens = phraseTc[head + PHRASE_LEN];
  for (int i = 0; i < nTokens; ++i) {
    int phraTc = phraseTc[head + i];
    if (phraTc == Wildcard || phraTc == Expr || phraTc == Expr0) {
      ++i;
      int num = phraseTc[head + i] - Zero;
      wpc[num] = pc; // トークンの位置（式の場合は式の開始位置）
      if (phraTc == Wildcard) {
        ++pc;
        continue;
      }
      int depth = 0; // 括弧の深さ
      for (;;) {
        if (tc[pc] == Semicolon)
          break;
        if (tc[pc] == Comma && depth == 0)
          break;

        if (tc[pc] == Lparen || tc[pc] == Lbracket)
          ++depth;
        if (tc[pc] == Rparen || tc[pc] == Rbracket)
          --depth;
        if (depth < 0)
          break;
        ++pc;
      }
      wpcEnd[num] = pc; // 式の終了位置
      if (phraTc == Expr && wpc[num] == pc)
        return 0;
      if (depth > 0)
        return 0;
      continue;
    }
    if (phraTc != tc[pc])
      return 0;
    ++pc;
  }
  nextPc = pc;
  return 1;
}

typedef AInt *IntPtr;

unsigned char *instructions;
unsigned char *ip; // instruction pointer

int jmps[10000]; // ジャンプ命令を書いた位置を格納する
int jp;

unsigned char *dumpBegin, *dumpEnd;

String opBins[] = { // 二項演算子のための機械語
  "%1L11; 3b_&<<3:%2m0; 0f_94_c0; 0f_b6_c0; 89_%0m0;",        // Equal
  "%1L11; 3b_&<<3:%2m0; 0f_95_c0; 0f_b6_c0; 89_%0m0;",        // NotEq
  "%1L11; 3b_&<<3:%2m0; 0f_9c_c0; 0f_b6_c0; 89_%0m0;",        // Les
  "%1L11; 3b_&<<3:%2m0; 0f_9d_c0; 0f_b6_c0; 89_%0m0;",        // GtrEq
  "%1L11; 3b_&<<3:%2m0; 0f_9e_c0; 0f_b6_c0; 89_%0m0;",        // LesEq
  "%1L11; 3b_&<<3:%2m0; 0f_9f_c0; 0f_b6_c0; 89_%0m0;",        // Gtr
  "%1L02; 03_&<<3:%2m0; %0S;",                                // Plus
  "%1L02; 2b_&<<3:%2m0; %0S;",                                // Mnus
  "%1L02; 0f_af_&<<3:%2m0; %0S;",                             // Multi
  "8b_%1m0; 99; f7_%2m7; 89_%0m0;",                           // Divi
  "8b_%1m0; 99; f7_%2m7; 89_%0m2;",                           // Mod
  "%1L02; 23_&<<3:%2m0; %0S;",                                // And
  "%1L02; 8b_%2m1; d3_&<<0:f8; %0S;",                         // ShiftRight
  "8b_%1m0; 23_%2m0; 83_f8_00; 0f_95_c0; 0f_b6_c0; 89_%0m0;", // AndAnd
};

inline static String getOpBin(int op)
{
  assert(Equal <= op && op < Assign);
  return opBins[op - Equal];
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

int varCounters[ MAX_TOKEN_CODE + 1 ];

void putModRM(unsigned reg, unsigned addVal, IntPtr var)
{
  int regVarNum = getRegVarNum(var);
  if (isRegVar(regVarNum)) {
    *ip = ( 0xc0 | (reg << 3) | regVarNum2regCode[regVarNum] ) + addVal; // mod=11, reg=???, r/m=???
    ++ip;
  }
  else {
    *ip = ( 0x05 | (reg << 3) ) + addVal; // mod=00, reg=???, r/m=101
    put32(ip + 1, (unsigned) var);
    ++varCounters[var - vars];
    ip += 5;
  }
}

unsigned char *instrBegin;     // 現在の命令の開始位置（直前のセミコロンの次の位置）
unsigned char *prevInstrBegin; // 1つ前の命令の開始位置（さらにその前のセミコロンの次の位置）
unsigned char *setccBegin;     // SETcc命令を見つけたら、その先頭の位置を記録する

// トークンコードを受け取り、定数か変数かを判定する
int isConst(int i)
{
  if ('0' <= tokenStrs[i][0] && tokenStrs[i][0] <= '9')
    return 1;
  if (tokenStrs[i][0] == '-' && '0' <= tokenStrs[i][1] && tokenStrs[i][1] <= '9')
    return 1;
  if (tokenStrs[i][0] == '"')
    return 1;

  return 0;
}

// %mの部分のポインタを受け取り、定数を指しているのかを判定する
int isConstM(unsigned char *p)
{
  if ((*p & 0xc7) != 0x05)
    return 0;

  return isConst(((AInt *) get32(p + 1)) - vars);
}

// 定数だった場合に%mの部分のポインタを受け取り、その定数値を返す
int getConstM(unsigned char *p)
{
  return *((AInt *) get32(p + 1));
}

// 受け取ったトークンコードが定数であれば計算結果のトークンを返す。そうでなければ0を返す
int calcConstForPrefixOp(int op, int a)
{
  if (isConst(a) == 0)
    return 0;

  int var = 0, varA = vars[a];
  switch (op) {
  case Minus: var = -varA; break;
  }

  char str[100];
  sprintf(str, "%d", var);
  return getTokenCode(str, strlen(str));
}

// 受け取ったトークンコードが定数であれば計算結果のトークンを返す。そうでなければ0を返す
int calcConstForInfixOp(int op, int a, int b)
{
  if (isConst(a) == 0 || isConst(b) == 0)
    return 0;

  int var = 0, varA = vars[a], varB = vars[b];
  switch (op) {
  case Equal:      var = varA == varB; break;
  case NotEq:      var = varA != varB; break;
  case Les:        var = varA <  varB; break;
  case GtrEq:      var = varA >= varB; break;
  case LesEq:      var = varA <= varB; break;
  case Gtr:        var = varA >  varB; break;
  case Plus:       var = varA +  varB; break;
  case Minus:      var = varA -  varB; break;
  case Multi:      var = varA *  varB; break;
  case Divi:       var = varA /  varB; break;
  case Mod:        var = varA %  varB; break;
  case And:        var = varA &  varB; break;
  case ShiftRight: var = varA >> varB; break;
  case AndAnd:     var = varA && varB; break;
  }

  char str[100];
  sprintf(str, "%d", var);
  return getTokenCode(str, strlen(str));
}

void putIcX86(String instructionStr, IntPtr p0, IntPtr p1, IntPtr p2, IntPtr p3);

// セミコロンが来たタイミングで呼ばれ、最適化を行う
void optimizeX86()
{
  if (instrBegin != ip) {
    if (instrBegin[0] == 0x0f && 0x90 <= instrBegin[1] && instrBegin[1] <= 0x9f) { // SETcc
      setccBegin = instrBegin;
    }
    if (instrBegin[0] == 0x8b && isConstM(&instrBegin[1])) { // mov r/m16/32,r16/32
      ip = instrBegin;
      --varCounters[ (AInt *) get32(&instrBegin[2]) - vars ];

      unsigned reg, rm;
      reg = rm = (instrBegin[1] >> 3) & 7;

      int i = getConstM(&instrBegin[1]);
      if (i == 0)
        putIcX86("31_%0c", (IntPtr) (0xc0 | (reg << 3) | rm), 0, 0, 0); // 同じレジスタ同士のxorを計算して、レジスタの値を0にする
      else
        putIcX86("%0c_%1i", (IntPtr) (0xb8 + reg), (IntPtr) i, 0, 0); // mov imm16/32,r16/32
    }
    if (instrBegin[0] <= 0x3f && (instrBegin[0] & 7) == 3 && isConstM(&instrBegin[1])) {
      /*
        03命令 add r/m16/32,r16/32
        0b命令 or r/m16/32,r16/32
        13命令 add r/m16/32,r16/32
        1b命令 sbb r/m16/32,r16/32
        23命令 and r/m16/32,r16/32
        2b命令 sub r/m16/32,r16/32
        33命令 xor r/m16/32,r16/32
        3b命令 cmp r/m16/32,r16/32
      */
      ip = instrBegin;
      --varCounters[ (AInt *) get32(&instrBegin[2]) - vars ];

      unsigned reg = instrBegin[0] & 0x38;
      unsigned rm  = (instrBegin[1] >> 3) & 7;

      int i = getConstM(&instrBegin[1]);
      if (-0x80 <= i && i <= 0x7f)
        putIcX86("83_%0c_%1c", (IntPtr) (0xc0 | reg | rm), (IntPtr) i, 0, 0); // 83 c0 ?? # opcode imm8,r/m16/32
      else
        putIcX86("81_%0c_%1i", (IntPtr) (0xc0 | reg | rm), (IntPtr) i, 0, 0); // 83 c0 ?? ?? ?? ?? # opcode imm16/32,r/m16/32
    }
    if (instrBegin[0] == 0x0f && instrBegin[1] & 0xaf && isConstM(&instrBegin[2])) { // imul r/m16/32,r16/32
      ip = instrBegin;
      --varCounters[ (AInt *) get32(&instrBegin[3]) - vars ];

      unsigned reg, rm;
      reg = rm = (instrBegin[2] >> 3) & 7;

      int i = getConstM(&instrBegin[2]);
      if (-0x80 <= i && i <= 0x7f)
        putIcX86("6b_%0c_%1c", (IntPtr) (0xc0 | (reg << 3) | rm), (IntPtr) i, 0, 0); // imul imm8,rm/16/32,r16/32
      else
        putIcX86("69_%0c_%1i", (IntPtr) (0xc0 | (reg << 3) | rm), (IntPtr) i, 0, 0); // imul imm16/32,r/m16/32,r16/32
    }
    if (instrBegin[0] == 0x83 && (instrBegin[1] & 0xf8) == 0xc0 && instrBegin[2] == 1) { // add 1
      ip = instrBegin;
      putIcX86("%0c", (IntPtr) (0x40 + (instrBegin[1] & 7)), 0, 0, 0); // inc r16/32
    }
    if (instrBegin[0] == 0x83 && (instrBegin[1] & 0xf8) == 0xe8 && instrBegin[2] == 1) { // sub 1
      ip = instrBegin;
      putIcX86("%0c", (IntPtr) (0x48 + (instrBegin[1] & 7)), 0, 0, 0); // dec r16/32
    }
    if (instrBegin[0] == 0x83 && (instrBegin[1] & 0xf8) == 0xf8 && instrBegin[2] == 0) { // cmp 0
      ip = instrBegin;
      unsigned reg, rm;
      reg = rm = (instrBegin[1] & 7);
      putIcX86("85_%0c", (IntPtr) (0xc0 | (reg << 3) | rm), 0, 0, 0); // test op1 op2 # op1とop2に同じレジスタを指定する。結果が0のときZFが1
    }
    if (instrBegin[0] == 0x8b && prevInstrBegin != NULL && prevInstrBegin[0] == 0x89 &&
        instrBegin[1] == 0x05 && prevInstrBegin[1] == 0x05 &&
        get32(instrBegin + 2) == get32(prevInstrBegin + 2)) {
      /*
        直前に書いたところをすぐに読み込む場合は、その読み込み命令(8b命令）を削除する。
        書き込み対象が一時変数の場合は、書き込み命令（89命令）も削除する。

        89 05 94 0a 42 00; // _t1 = eax; 無駄な書き込み
        8b 05 94 0a 42 00; // eax = _t1; 無駄な読み込み
      */
      ip = instrBegin; // 8b命令を削除
      int mem = (IntPtr) get32(prevInstrBegin + 2) - vars;
      --varCounters[mem];

      if (Tmp0 <= mem && mem <= Tmp9) {
        ip = prevInstrBegin; // 89命令を削除
        --varCounters[mem];
      }
    }
    prevInstrBegin = instrBegin;
    instrBegin = ip;
  }
  if (setccBegin + 14 == ip && memcmp(&setccBegin[2], "\xc0\x0f\xb6\xc0\x85\xc0\x0f", 7) == 0 &&
      0x84 <= setccBegin[9] && setccBegin[9] <= 0x85) {
    /*
      SETcc命令の後に最適化すべきパターンが来ていたら、命令列を加工する。

      3b 05 94 0a 42 00; // if (eax < _t1) { eax = 1; } (setl)
      0f 9c c0;          // 0f が*setccBegin、c0 が*(setccBegin + 2)
      0f b6 c0;
      85 c0;             // if (eax == 0) goto skip;
      0f 84 0c 00 00 00; // 0c が*(setccBegin + 10)、00 の次がsetccBegin + 14
    */
    memcpy(&setccBegin[2], &setccBegin[10], 4); // 2行目の「0f 9c c0;」->「0f 9c 0c 00 00 00;」
    setccBegin[1] -= 0x10; // SETcc「0f 9c 0c 00 00 00;」-> jcc「0f 8c 0c 00 00 00;」
    /*
      ここまでの加工で、命令列は次のようになる。

      3b 05 94 0a 42 00; // if (eax < _t1) { eax = 1; } (setl)
      0f 8c 0c 00 00 00; // 0f が*setccBegin
      85 c0;
      0f 84 0c 00 00 00; // 84 が*(setccBegin + 9)
    */
    if (setccBegin[9] == 0x84)
      setccBegin[1] ^= 0x01; // 条件反転
    /*
      ここまでの加工で、命令列は次のようになる。

      3b 05 94 0a 42 00; // if (eax < _t1) { eax = 1; } (setl)
      0f 8d 0c 00 00 00; // 0f が*setccBegin、*(setccBegin + 1)の8cと0x01のxorを取って8c -> 8d
      85 c0;
      0f 84 0c 00 00 00; // 84 が*(setccBegin + 9)
    */
    instrBegin = ip = setccBegin + 6; // 3行目の85の位置
    prevInstrBegin = setccBegin = NULL;
    jmps[jp - 1] = ip - 4 - instructions;
  }
}

void decodeX86(String str, IntPtr *operands)
{
  unsigned reg = 0; // ModR/Mバイトのregフィールドの値
  unsigned addVal = 0, regCode; // 命令に加算する値, addValの値を求める際に用いるレジスタ番号

  for (int pos = 0; str[pos] != 0;) {
    if (str[pos] == ' ' || str[pos] == '\t' || str[pos] == '_' || str[pos] == ':')
      ++pos;
    else if (str[pos] == ';') {
      ++pos;
      optimizeX86();
    }
    else if (getHex(str[pos]) >= 0 && getHex(str[pos + 1]) >= 0) { // 16進数2桁（opcode）
      *ip = ( ((unsigned) getHex(str[pos]) << 4) | getHex(str[pos + 1]) ) + addVal;
      ++ip;
      addVal = 0;
      pos += 2;
    }
    else if (str[pos] == '&') {
      /*
        「&<<i:」と「&i:」はプリフィックスで、続く命令に付加する。このプリフィックスは
        続く命令をinstructionsに格納する際に、命令にaddValの値を加算する。addValの値は
        regCodeの値（レジスタ番号）に次の演算を適用して求める。

        プリフィックスの書式が&<<i:の場合
        regCodeの値（レジスタ番号）をiだけ左シフトして求める。

        プリフィックス書式が&i:の場合
        regCodeの値（レジスタ番号）にiを掛けて求める。

        regCodeのデフォルト値は0（eax）なので、どちらのプリフィックスの書式でも
        regCodeの値が0のときはこのプリフィックスは無効になる。
      */
      if (str[pos + 1] == '<' && str[pos + 2] == '<') {
        addVal = (unsigned) regCode << (str[pos + 3] - '0');
        pos += 4;
      }
      else {
        addVal = (unsigned) regCode * (str[pos + 1] - '0');
        pos += 2;
      }
    }
    else if (str[pos] == '%') {
      int i = str[pos + 1] - '0'; // 参照する追加引数の番号

      switch (str[pos + 2]) {
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

          regフィールドは、レジスタ番号や追加オペコード情報を指定する。

          r/mフィールドは、modフィールドと組み合わせて24個のアドレッシングモードと8個のレジスタをコード化する。

          実効アドレスを得るために組み合わせるmodおよびr/mフィールドのコードと、regフィールドで指定する
          レジスタ番号や追加オペコード情報の値は以下の資料に示されている。

          『IA-32 インテル® アーキテクチャ・ソフトウェア・デベロッパーズ・マニュアル 中巻A：命令セット・リファレンスA-M』
          2.6.ModR/MおよびSIBバイトのアドレス指定モードのコード化 > 表2-2.  ModR/Mバイトによる32ビット・アドレス指定形式
          https://www.intel.co.jp/content/dam/www/public/ijkk/jp/ja/documents/developer/IA32_Arh_Dev_Man_Vol2A_i.pdf#G8.6121
        */
        reg = str[pos + 3] - '0';
        putModRM(reg, addVal, operands[i]);
        addVal = 0;
        pos += 4;
        continue;
      case 'L': // Load
        /*
          %拡張命令（L）は、メモリ／レジスタ変数からの読み込みについて
          「%iLjk」と書き、オペランドi, j, kの値に基づいて条件分岐を行い、
          レジスタ変数を使う場合とそうでない場合とで処理を分岐させる。
        */
        {
          int j = str[pos + 3] - '0'; // 参照する追加引数の番号
          int k = str[pos + 4] - '0'; // 参照する追加引数の番号

          int regVarNum = getRegVarNum(operands[j]);
          if (isRegVar(regVarNum) && operands[i] == operands[j]) {
            regCode = regVarNum2regCode[regVarNum];
          }
          else if (isRegVar(regVarNum) && operands[j] != operands[k]) {
            reg = regCode = regVarNum2regCode[regVarNum];
            *ip = 0x8b;
            ++ip;
            putModRM(reg, addVal, operands[i]);
            addVal = 0;
          }
          else {
            reg = regCode = 0;
            *ip = 0x8b;
            ++ip;
            putModRM(reg, addVal, operands[i]);
            addVal = 0;
          }
        }
        pos += 5;
        continue;
      case 'S': // Store
        /*
          %拡張命令（S）は、メモリ／レジスタ変数への書き込みについて
          「%0S」と書き、もしregCodeが0（eax）であれば「89_%0m0;」と同じ
          動作をするが、0以外であればinstructionsに何も書き込まない。
       */
        if (regCode == 0) {
          reg = 0;
          *ip = 0x89;
          ++ip;
          putModRM(reg, addVal, operands[i]);
          regCode = 0;
        }
        break;
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

void tmpFree(int i)
{
  if (Tmp0 <= i && i <= Tmp9)
    tmpFlags[i - Tmp0] = 0;
}

clock_t t0;

void printInteger(int i)  { printf("%d\n", i); }
void printString(char *s) { printf("%s\n", s); }
void printElapsedTime()   { printf("time: %.3f[sec]\n", (clock() - t0) / (double) CLOCKS_PER_SEC); }

int ff16sin(int x) { return (int) (sin(x * (2 * 3.14159265358979323 / 65536)) * 65536); }
int ff16cos(int x) { return (int) (cos(x * (2 * 3.14159265358979323 / 65536)) * 65536); }

int toExit;

// acl library functions
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
// end of acl library functions

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
int exprPutIcX86(int res, int len, void *fn, int *err);

#define N_PREFIX 2
#define N_INFIX 15

enum { Prefix = 0, Infix = N_PREFIX + 1 };

typedef struct precedence {
  int operator;
  int level;
} Precedence;

Precedence precedenceTable[ N_PREFIX + N_INFIX + 2 ] = {
  // Prefix
  {PlusPlus, 2},
  {Minus, 2},
  {.level = LOWEST_PRECEDENCE + 1},
  // Infix
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
  {And, 9},
  {AndAnd, 12},
  {Assign, 15},
  {.level = LOWEST_PRECEDENCE + 1}
};

int getPrecedenceLevel(int notationStyle, int operator)
{
  int n = notationStyle == Prefix ? N_PREFIX : N_INFIX;
  precedenceTable[notationStyle + n].operator = operator;

  int i = 0;
  while (precedenceTable[notationStyle + i].operator != operator)
    ++i;
  return precedenceTable[notationStyle + i].level;
}

int evalInfixExpression(int lhs, int precedenceLevel, int op)
{
  ++epc;
  int rhs = evalExpression(precedenceLevel);
  int res = calcConstForInfixOp(op, lhs, rhs);
  if (res == 0) {
    res = tmpAlloc();
    putIcX86(getOpBin(op), &vars[res], &vars[lhs], &vars[rhs], 0);
  }
  tmpFree(lhs);
  tmpFree(rhs);
  if (lhs < 0 || rhs < 0)
    return -1;
  return res;
}

int evalExpression(int precedenceLevel)
{
  int res = -1, e0 = 0, e1 = 0;
  nextPc = 0;

  if (match(99, "( !!**0 )", epc)) { // 括弧
    res = expression(0);
  }
  else if (tc[epc] == PlusPlus) { // 前置インクリメント
    ++epc;
    res = evalExpression(getPrecedenceLevel(Prefix, PlusPlus));
    putIcX86("8b_%0m0; 40; 89_%0m0;", &vars[res], 0, 0, 0);
  }
  else if (tc[epc] == Minus) { // 単項マイナス
    ++epc;
    e0 = evalExpression(getPrecedenceLevel(Prefix, Minus));
    res = calcConstForPrefixOp(Minus, e0);
    if (res == 0) {
      res = tmpAlloc();
      putIcX86("8b_%1m0; f7_d8; 89_%0m0;", &vars[res], &vars[e0], 0, 0);
    }
  }
  else if (match(72, "mul64shr(!!**0, !!**1, !!**2)", epc)) {
    e0 = expression(0);
    e1 = expression(1);
    int e2 = expression(2);
    res = tmpAlloc();
    putIcX86("8b_%2m1; 8b_%0m0; f7_%1m5; 0f_ad_d0; 89_%3m0;", &vars[e0], &vars[e1], &vars[e2], &vars[res]);
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
  else if (match(73, "aRgb8(!!**0, !!**1, !!**2)", epc)) {
    res = exprPutIcX86(res, 3, call_aRgb8, &e0);
  }
  else if (match(74, "aOpenWin(!!**0, !!**1, !!***2, !!***8)", epc)) {
    exprPutIcX86(0, 3, call_aOpenWin, &e0);
    putIcX86("85_c0; 0f_85_%0l;", &vars[toExit], 0, 0, 0); // test %eax,%eax; jz rel16/32;
    res = Zero;
  }
  else if (match(75, "aXorShift32()", epc)) {
    res = exprPutIcX86(res, 0, call_aXorShift32, &e0);
  }
  else if (match(76, "aGetPix(!!**8, !!**0, !!**1)", epc)) {
    res = exprPutIcX86(res, 2, call_aGetPix, &e0);
  }
  else if (match(77, "ff16sin(!!**0)", epc)) {
    res = exprPutIcX86(res, 1, ff16sin, &e0);
  }
  else if (match(78, "ff16cos(!!**0)", epc)) {
    res = exprPutIcX86(res, 1, ff16cos, &e0);
  }
  else if (match(79, "aInkey(!!***8, !!**0)", epc)) {
    res = exprPutIcX86(res, 1, call_aInkey, &e0);
  }
  else { // 変数もしくは定数
    res = tc[epc];
    ++epc;
  }
  if (nextPc > 0)
    epc = nextPc;
  for (;;) {
    tmpFree(e0);
    tmpFree(e1);
    if (res < 0 || e0 < 0 || e1 < 0) // ここまででエラーがあれば、処理を打ち切り
      return -1;
    if (epc >= epcEnd)
      break;

    int encountered; // ぶつかった演算子の優先順位を格納する
    e0 = e1 = 0;
    if (tc[epc] == PlusPlus) { // 後置インクリメント
      ++epc;
      e0 = res;
      res = tmpAlloc();
      putIcX86("8b_%1m0; 89_%0m0; 40; 89_%1m0;", &vars[res], &vars[e0], 0, 0);
    }
    else if (match(71, "[!!**0]=", epc)) {
      e1 = res;
      e0 = expression(0);
      epc = nextPc;
      res = evalExpression(getPrecedenceLevel(Infix, Assign));
      //                                               base       index
      putIcX86("8b_%2m0; 8b_%0m2; 8b_%1m1; 89_04_8a;", &vars[e1], &vars[e0], &vars[res], 0);
      /*
        8b_%2m0  -> mov r/m16/32,%eax
        8b_%0m2  -> mov r/m16/32,%edx
        8b_%1m1  -> mov r/m16/32,%ecx
        89_04_8a -> mov %eax,(%edx,%ecx,4)
      */
    }
    else if (match(70, "[!!**0]", epc)) {
      e1 = res;
      res = tmpAlloc();
      e0 = expression(0);
      //                                               base       index
      putIcX86("8b_%0m2; 8b_%1m1; 8b_04_8a; 89_%2m0;", &vars[e1], &vars[e0], &vars[res], 0);
      /*
        8b_%0m2  -> mov r/m16/32,%edx
        8b_%1m1  -> mov r/m16/32,%ecx
        8b_04_8a -> mov (%edx,%ecx,4),%eax
        89_%2m0  -> mov %eax,r/m16/32
      */
      epc = nextPc;
    }
    else if (precedenceLevel >= (encountered = getPrecedenceLevel(Infix, tc[epc]))) {
      /*
        「引数として渡された優先順位」が「ぶつかった演算子の優先順位」よりも
        低いか又は等しい(値が大きいか又は等しい)ときは、このブロックを実行して
        中置演算子を評価する。

        「引数として渡された優先順位」が「ぶつかった演算子の優先順位」よりも
        高い(値が小さい)ときは、このブロックを実行せずにこれまでに式を評価した
        結果を呼び出し元に返す。
      */
      switch (tc[epc]) {
      case Multi: case Divi: case Mod:
      case Plus: case Minus:
      case ShiftRight:
      case Les: case LesEq: case Gtr: case GtrEq:
      case Equal: case NotEq:
      case And:
      case AndAnd:
        res = evalInfixExpression(res, encountered - 1, tc[epc]);
        break;
      case Assign:
        ++epc;
        e0 = evalExpression(encountered);
        putIcX86("8b_%1m0; 89_%0m0;", &vars[res], &vars[e0], 0, 0);
        break;
      }
    }
    else
      break;
  }
  return res;
}

// 引数として渡したワイルドカード番号にマッチした式をコンパイルしてinstructions[]に書き込む
int expression(int num)
{
  if (wpc[num] == wpcEnd[num])
    return 0;

  int oldEpc = epc, oldEpcEnd = epcEnd;
  int buf[WPC_LEN * 2 + 1];
  for (int i = 0; i < WPC_LEN; ++i) {
    buf[i]           = wpc   [i];
    buf[i + WPC_LEN] = wpcEnd[i];
  }
  buf[WPC_LEN * 2] = nextPc;

  epc = wpc[num]; epcEnd = wpcEnd[num];
  int res = evalExpression(LOWEST_PRECEDENCE);
  if (epc < epcEnd)
    return -1;

  epc = oldEpc; epcEnd = oldEpcEnd;
  for (int i = 0; i < WPC_LEN; ++i) {
    wpc   [i] = buf[i];
    wpcEnd[i] = buf[i + WPC_LEN];
  }
  nextPc = buf[WPC_LEN * 2];
  return res;
}

enum { ConditionIsTrue = 0, ConditionIsFalse };

//                       je    jne   jl    jge   jle   jg
int conditionCodes[6] = {0x84, 0x85, 0x8c, 0x8d, 0x8e, 0x8f};

// 条件式wpc[i]を評価して、その結果に応じてlabel（トークンコード）に分岐する内部コードを生成する
void ifgoto(int i, int conditionType, int label) {
  int begin = wpc   [i];
  int end   = wpcEnd[i];

  int op = tc[begin + 1];
  if ((begin + 3 == end) && (Equal <= op && op <= Gtr)) {
    putIcX86("%2L22; 3b_&<<3:%3m0; 0f_%0c_%1l;",
        (IntPtr) conditionCodes[ (op - Equal) ^ conditionType ],
        &vars[label],
        &vars[tc[begin]],
        &vars[tc[begin + 2]]);
    /*
      ユーザがレジスタ変数を使わない場合は次の機械語を生成する。

      8b_%2m0    -> mov r/m16/32,%eax
      3b_%3m0    -> cmp r/m16/32,%eax
      0f_%0c_%1l -> jcc rel16/32
    */
  }
  else {
    i = expression(i);
    putIcX86("%2L22; 85_&9:c0; 0f_%0c_%1l;", (IntPtr) (0x85 - conditionType), &vars[label], &vars[i], 0);
    /*
      ユーザがレジスタ変数を使わない場合は次の機械語を生成する。

      8b_%2m0    -> mov r/m16/32,%eax
      85_c0      -> test %eax,%eax
      0f_%0c_%1l -> jcc rel16/32
    */
    tmpFree(i);
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

int align;

// ラベルに対応するipの位置を記録する
void defLabel(int label)
{
  // 命令が短くなりすぎて速度が出せないアドレスになりそうなときはNOP命令を入れる
  if (align > 0) {
    int len = (ip - instructions) & 15; // 0〜15
    if (0 < len && len <= 7) {
      putIcX86("66_0f_1f_84_00_00_00_00_00", 0, 0, 0, 0); // 9バイトのNOP
      len = (len + 9) & 15; // 8〜15
    }
    if (len > 0) {
      static char *nopTable[8] = {
        "0f_1f_84_00_00_00_00_00", // 8バイトのNOP
        "0f_1f_80_00_00_00_00",    // 7バイトのNOP
        "66_0f_1f_44_00_00",       // 6バイトのNOP
        "0f_1f_44_00_00",          // 5バイトのNOP
        "0f_1f_40_00",             // 4バイトのNOP
        "0f_1f_00",                // 3バイトのNOP
        "66_90",                   // 2バイトのNOP
        "90"                       // 1バイトのNOP
      };
      putIcX86(nopTable[len - 8], 0, 0, 0, 0);
    }
  }

  vars[label] = ip - instructions;
  prevInstrBegin = setccBegin = NULL;
}

#define BLOCK_INFO_UNIT_SIZE 10
int blockInfo[BLOCK_INFO_UNIT_SIZE * 100], blockDepth, loopDepth;

enum { BlockType, IfBlock, ForBlock, MainBlock };
enum { IfLabel0 = 1, IfLabel1 };
enum { ForBegin = 1, ForContinue, ForBreak, ForLoopDepth, ForWpc1, ForWpcEnd1, ForWpc2, ForWpcEnd2 };

// lenで渡した数の式を評価し、その結果を使ってputIcX86をする
int exprPutIcX86(int res, int len, void *fn, int *err)
{
  int e[9] = {0};
  for (int i = 0; i < len; ++i) {
    if ((e[i] = expression(i)) < 0)
      *err = -1;
    putIcX86("%0L00; 89_&<<3:44_24_%1c;", &vars[e[i]], (IntPtr) (i * 4), 0, 0); // 89 44 24 ?? -> mov %eax,0x??(%esp)
  }

  putIcX86("e8_%0r;", fn, 0, 0, 0); // call rel16/32 <fn>

  for (int i = 0; i < len; ++i)
    tmpFree(e[i]);

  if (res != 0) {
    res = tmpAlloc();
    putIcX86("89_%0m0;", &vars[res], 0, 0, 0);
  }
  return res;
}

int codedump;

int compile(String src)
{
  int nTokens = lexer(src, tc);
  tc[nTokens++] = Semicolon; // 末尾に「;」を付け忘れることが多いので、付けてあげる
  tc[nTokens] = tc[nTokens + 1] = tc[nTokens + 2] = tc[nTokens + 3] = Period; // エラー表示用

  instrBegin = ip = instructions;
  prevInstrBegin = setccBegin = NULL;
  jp = 0;
  putIcX86("60; 83_ec_7c;", 0, 0, 0, 0); // pusha; sub $0x7c,%esp;
  regVarSaveLoad(RvLoad); // 前回実行時のレジスタ変数の値を引き継ぐ
  dumpBegin = ip;

  for (int i = 0; i < N_TMPS; ++i)
    tmpFlags[i] = 0;
  tmpLabelNo = 0;
  for (int i = 0; i < MAX_TOKEN_CODE + 1; ++i)
    varCounters[i] = 0;
  toExit = tmpLabelAlloc();
  blockDepth = loopDepth = 0;

  int pc;
  for (pc = 0; pc < nTokens;) {
    int *curBlock = &blockInfo[blockDepth];
    int e0 = 0, e2 = 0;
    if (match(1, "!!*0 = !!*1;", pc)) {
      if (isRegVar(getRegVarNum(&vars[tc[wpc[0]]])))
        putIcX86("%0L00; 8b_&<<3:%1m0;", &vars[tc[wpc[0]]], &vars[tc[wpc[1]]], 0, 0);
      else
        putIcX86("%1L11; 89_&<<3:%0m0;", &vars[tc[wpc[0]]], &vars[tc[wpc[1]]], 0, 0);
    }
    else if (match(10, "!!*0 = !!*1 + 1; if (!!*2 < !!*3) goto !!*4;", pc) && tc[wpc[0]] == tc[wpc[1]] && tc[wpc[0]] == tc[wpc[2]]) {
      putIcX86("%1L11; &<<0:40; %1S; 3b_&<<3:%2m0; 0f_8c_%0l;", &vars[tc[wpc[4]]], &vars[tc[wpc[0]]], &vars[tc[wpc[3]]], 0);
      /*
        ユーザがレジスタ変数を使わない場合は次の機械語を生成する。

        8b_%1m0   -> mov r/m16/32,%eax
        40        -> inc %eax
        89_%1m0   -> mov %eax,r/m16/32
        3b_%2m0   -> cmp r/m16/32,%eax
        0f_8c_%0l -> jl rel16/32
      */
    }
    else if (match(9, "!!*0 = !!*1 + 1;", pc) && tc[wpc[0]] == tc[wpc[1]]) { // +1専用の命令
      putIcX86("%0L00; &<<0:40; %0S;", &vars[tc[wpc[0]]], 0, 0, 0);
    }
    else if (match(2, "!!*0 = !!*1 !!*2 !!*3;", pc) && Equal <= tc[wpc[2]] && tc[wpc[2]] < Assign) { // 加算、減算など
      int i = calcConstForInfixOp(tc[wpc[2]], tc[wpc[1]], tc[wpc[3]]);
      if (i == 0)
        putIcX86(getOpBin(tc[wpc[2]]), &vars[tc[wpc[0]]], &vars[tc[wpc[1]]], &vars[tc[wpc[3]]], 0);
      else {
        if (isRegVar(getRegVarNum(&vars[tc[wpc[0]]])))
          putIcX86("%0L00; 8b_&<<3:%1m0;", &vars[tc[wpc[0]]], &vars[i], 0, 0);
        else
          putIcX86("%1L11; 89_&<<3:%0m0;", &vars[tc[wpc[0]]], &vars[i], 0, 0);
      }
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
      ifgoto(0, ConditionIsTrue, tc[wpc[1]]);
    }
    else if (match(7, "time;", pc)) {
      exprPutIcX86(0, 0, printElapsedTime, &e0);
    }
    else if (match(11, "if (!!**0) {", pc)) { // if文
      blockDepth += BLOCK_INFO_UNIT_SIZE;
      curBlock = &blockInfo[blockDepth];
      curBlock[ BlockType ] = IfBlock;
      curBlock[ IfLabel0  ] = tmpLabelAlloc(); // 条件不成立のときの飛び先
      curBlock[ IfLabel1  ] = 0;
      ifgoto(0, ConditionIsFalse, curBlock[IfLabel0]);
    }
    else if (match(12, "} else {", pc) && curBlock[BlockType] == IfBlock) {
      curBlock[ IfLabel1 ] = tmpLabelAlloc(); // else節の終端
      putIcX86("e9_%0l;", &vars[curBlock[IfLabel1]], 0, 0, 0); // jmp rel16/32
      defLabel(curBlock[IfLabel0]);
    }
    else if (match(13, "}", pc) && curBlock[BlockType] == IfBlock) {
      if (curBlock[IfLabel1] == 0)
        defLabel(curBlock[IfLabel0]);
      else
        defLabel(curBlock[IfLabel1]);
      blockDepth -= BLOCK_INFO_UNIT_SIZE;
    }
    else if (match(14, "for (!!***0; !!***1; !!***2) {", pc)) { // for文
      blockDepth += BLOCK_INFO_UNIT_SIZE;
      curBlock = &blockInfo[blockDepth];
      curBlock[ BlockType    ] = ForBlock;
      curBlock[ ForBegin     ] = tmpLabelAlloc();
      curBlock[ ForContinue  ] = tmpLabelAlloc();
      curBlock[ ForBreak     ] = tmpLabelAlloc();
      curBlock[ ForLoopDepth ] = loopDepth;
      curBlock[ ForWpc1      ] = wpc   [1];
      curBlock[ ForWpcEnd1   ] = wpcEnd[1];
      curBlock[ ForWpc2      ] = wpc   [2];
      curBlock[ ForWpcEnd2   ] = wpcEnd[2];
      loopDepth = blockDepth;

      e0 = expression(0);
      if (wpc[1] < wpcEnd[1])
        ifgoto(1, ConditionIsFalse, curBlock[ForBreak]);
      defLabel(curBlock[ForBegin]);
    }
    else if (match(15, "}", pc) && curBlock[BlockType] == ForBlock) {
      defLabel(curBlock[ForContinue]);

      int wpc1 = curBlock[ForWpc1];
      int wpc2 = curBlock[ForWpc2];
      int isWpc1Les      = (wpc1 + 3 == curBlock[ForWpcEnd1]) && (tc[wpc1 + 1] == Les);
      int isWpc2PlusPlus = (wpc2 + 2 == curBlock[ForWpcEnd2]) &&
        ( (tc[wpc1] == tc[wpc2] && tc[wpc2 + 1] == PlusPlus) || (tc[wpc1] == tc[wpc2 + 1] && tc[wpc2] == PlusPlus) );

      if (isWpc1Les && isWpc2PlusPlus) {
        putIcX86("%1L11; &<<0:40; %1S; 3b_&<<3:%2m0; 0f_8c_%0l;", &vars[curBlock[ForBegin]], &vars[tc[wpc1]], &vars[tc[wpc1 + 2]], 0);
        /*
          ユーザがレジスタ変数を使わない場合は次の機械語を生成する。

          8b_%1m0   -> mov r/m16/32,%eax
          40        -> inc %eax
          89_%1m0   -> mov %eax,r/m16/32
          3b_%2m0   -> cmp r/m16/32,%eax
          0f_8c_%0l -> jl rel16/32
        */
      }
      else {
        wpc   [1] = curBlock[ ForWpc1    ];
        wpcEnd[1] = curBlock[ ForWpcEnd1 ];
        wpc   [2] = curBlock[ ForWpc2    ];
        wpcEnd[2] = curBlock[ ForWpcEnd2 ];

        e2 = expression(2);
        if (wpc[1] < wpcEnd[1]) // !!***1に何らかの式が書いてある
          ifgoto(1, ConditionIsTrue, curBlock[ForBegin]);
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
      ifgoto(0, ConditionIsTrue, loopBlock[ForContinue]);
    }
    else if (match(19, "if (!!**0) break;", pc) && loopDepth > 0) {
      int *loopBlock = &blockInfo[loopDepth];
      ifgoto(0, ConditionIsTrue, loopBlock[ForBreak]);
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
      curBlock[BlockType] = MainBlock; // ただ認識しているだけで何もしていない
    }
    else if (match(31, "}", pc) && curBlock[BlockType] == MainBlock) {
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
    else if (match(38, "!!*3 = mul64shr(!!*0, !!*1, !!*2);", pc) && strtol(tokenStrs[tc[tc[wpc[2]]]], 0, 0) > 0) {
      putIcX86("8b_%0m0; f7_%1m5; 0f_ac_d0_%2c; 89_%3m0;",
          &vars[tc[wpc[0]]], &vars[tc[wpc[1]]], (IntPtr) strtol(tokenStrs[tc[wpc[2]]], 0, 0), &vars[tc[wpc[3]]]);
      /*
        8b_%0m0      -> mov r/m16/32,%eax
        f7_%1m5      -> imul r/m16/32      # eaxに%mで指定した値を掛け算して、結果をedx:eaxに入れる
        0f_ac_d0_%2c -> shrd %cl,%edx,%eax # edx:eaxの64bitをecxだけ右シフトする。でもeaxしか更新されない（edxはそのまま）
        89_%3m0      -> mov %eax,r/m16/32
      */
    }
    else if (match(39, "align();", pc)) {
      align = 1;
    }
    else if (match(40, "align(!!*0);", pc)) {
      align = vars[tc[wpc[0]]];
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

  unsigned char *end = ip, *label, *dest;
  for (int i = 0; i < jp; ++i) { // ジャンプ命令の最適化
    label = jmps[i] + instructions;
    dest = *(IntPtr) get32(label) + instructions;
    while (*dest == 0xe9) { // 飛び先がjmp命令だったら、さらにその先を読む
      put32(label, get32(dest + 1));
      dest = *(IntPtr) get32(dest + 1) + instructions;
    }
  }
  for (int i = 0; i < jp; ++i) { // 飛び先を指定する（相対値にする）
    label = jmps[i] + instructions;
    dest = *(IntPtr) get32(label) + instructions;
    put32(label, dest - (label + 4));
  }
  return end - instructions;
err:
  printf("syntax error: %s %s %s %s\n", tokenStrs[tc[pc]], tokenStrs[tc[pc + 1]], tokenStrs[tc[pc + 2]], tokenStrs[tc[pc + 3]]);
  return -1;
}

AWindow *win;

int run(String src)
{
  if (compile(src) < 0)
    return 1;

  if (codedump == 0) {
    void (*exec)() = (void (*)()) instructions;
    t0 = clock();
    exec();
    if (win != NULL)
      aFlushAll(win);
  }
  else {
    // 機械語の命令列を表示する
    int nBytes = dumpEnd - dumpBegin;
    for (int i = 0; i < nBytes; ++i)
      printf("%02x ", *(dumpBegin + i));
    if (nBytes)
      printf("\n");
    printf("(len=%d)\n", nBytes);

    // 変数の出現頻度を表示する
    for (int i = 0; i < MAX_TOKEN_CODE + 1; ++i) {
      if (varCounters[i] != 0)
        printf("#%04d(%08x): %06d: %s\n", i, (int) &vars[i], varCounters[i], tokenStrs[i]);
    }
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
  initTc(defaultTokens, sizeof defaultTokens / sizeof defaultTokens[0]);
  if (aArgc >= 2) {
    if (loadText((String) aArgv[1], text, 10000) != 0)
      exit(1);
    run(text);
    exit(0);
  }

  for (int nLines = 1;; ++nLines) { // Read-Eval-Print Loop
    printf("[%d]> ", nLines);
    fgets(text, 10000, stdin);
    int inputLen = strlen(text);
    if (text[inputLen - 1] == '\n') // chomp
      text[inputLen - 1] = 0;

    int hasRemovedSemicolon = 0;
    char *semicolonPos;
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
