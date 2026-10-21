#pragma once

#define MAXSTACKSIZE 1024

extern double acc;
extern double stack[MAXSTACKSIZE];
extern int sp;

extern int store(const char *variable);
extern int recall(const char *variable, double *value);
extern int push(void);
extern int pop(double *value);