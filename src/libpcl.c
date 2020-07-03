#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//------------------------------------------------------------//
//--------------------Library functions-----------------------//
//------------------------------------------------------------//

//--------------------Available functions-----------//
//--------------------------------------------------//
// Output:                  | Math functions:       //
// -writeInteger            | -abs                  //
// -writeBoolean            | -fabs                 //
// -writeChar               | -sqrt                 //
// -writeReal               | -sin                  //
// -writeString             | -cos                  //
//                          | -tan                  //
//                          | -arctan               //
// Input:                   | -exp                  //
// -readInteger             | -ln                   //
// -readBoolean             | -pi                   //
// -readChar                | Conversion functions: //
// -readReal                | -trunc                //
// -readString              | -round                //
//--------------------------------------------------//

// If a function is not implemented here then the C variant is used
// and we link with that implementation using the -lm flag

void writeInteger(int32_t n) {
  printf("%d", n);
}

void writeBoolean(int8_t b) {
  if (b)
    printf("true");
  else
    printf("false");
}

void writeChar(int8_t c) {
  printf("%c", c);
}

void writeReal(double r) {
  printf("%lf", r);
}

void writeString(char* s) {
  printf("%s", s);
}

int32_t readInteger() {
  int32_t n;
  scanf("%d", &n);
  return n;
}

int8_t readBoolean() {
  int32_t b;
  scanf("%d", &b);
  return (int8_t)b;
}

int8_t readChar() {
  int8_t c;
  c = getchar();
  return c;
}

double readReal() {
  double r;
  scanf("%lf", &r);
  return r;
}

void readString(int32_t size, char* s) {
  if(!fgets(s, size, stdin))
    exit(1);
}

double arctan(double r) {
  return atan(r);
}

double ln(double r) {
  return log(r);
}

double pi() {
  return M_PI;
}

int32_t ord(int8_t c) {
  return (int32_t) c;
}

int8_t chr(int32_t n) {
  return (int8_t) n;
}
