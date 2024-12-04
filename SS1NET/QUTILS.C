/****************************************************************************
qutils.c
17/10/88

Description

This is a set of utility programs used in building the Qudos simulator test
patterns.

****************************************************************************/

/* Declare the global variables */
           
#include "stdio.h"
#define UTILS 1                 /* used in testpat.def */
#include "testpat.def"

#define DEBUG  0

/* Data Structures */

/* This array is used to build the simulation results in. */
char pat[SIGNALS][STEPS];

FILE *testfile;

/****************************************************************************
SIM_INIT

Initialises the simulation array at the start of the test pattern generation.
****************************************************************************/

sim_init ()

{
int n,m;

for (n = 0; n < STEPS; n++)
        {
        for (m = FIRST_INPUT; m <= LAST_OUTPUT; m++)
                pat[m][n] = def_val[m];
        }
}
/****************************************************************************
SIM_TERM

Write out the test pattern at the end of pattern generation.
****************************************************************************/

sim_term (offset)
int offset;

{
int n,m;
char dir[SIGNALS];

/* Open the test pattern file for writing */
printf ("Writing out stimulus language file\n");

if ((testfile=fopen("ss.stm","w"))==NULL)
        {
        printf ("ERROR - unable to open stimulus file for writing\n");
        exit (0);
        }

/* write the stm header */

fprintf (testfile,"%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
"STIMULUS ss (xad[0..7],xa[8..15],xd[8..15],xrdL,xwrL,",
"       xintr,xvsyncL,xhsyncL,xjoyL[0..2],",
"       xas[16..19],xresetL,xiom,xale,xinta,xhlda,xxtal,",
"       xai[0..3],xlpL,xdsp_in,",
"       xa[0..7],xa[16..17],xpclk,xhold,xsceL[0..1],",
"       xweL,xcsL[0..1],xr[0..3],xg[0..3],xb[0..3],",
"       xchroma,xleftl,xlefth,xrightl,xrighth,",
"       xinc,xmo,xoeL,xcasL,xdsp_out);");

/* Setup the test strobe */

fprintf (testfile,"%s\n%s\n%s\n%s\n%s\n%s\n",
"CLOCK tstrobe STROBES (xad[0..7],xa[8..15],xd[8..15],xrdL,xwrL,",
"       xintr,xvsyncL,xhsyncL,",
"       xa[0..7],xa[16..17],xpclk,xhold,xsceL[0..1],",
"       xweL,xcsL[0..1],xr[0..3],xg[0..3],xb[0..3],",
"       xchroma,xleftl,xlefth,xrightl,xrighth,xjoyL[0..2],",
"       xoeL,xcasL);");

fprintf (testfile,"BEGIN;\n", offset);
fprintf (testfile,"CYCLE FOR %d AT 0;\n", offset);
fprintf (testfile,"tstrobe 1 %d;\n",STRH_POS);
fprintf (testfile,"tstrobe 0 %d;\n",STRL_POS);
fprintf (testfile,"REPEAT AFTER %d;\n", CYCLE);

/* initialise from first cycle of test pattern */
n = 0;
for (m = FIRST_INPUT; m <= LAST_OUTPUT; m++)
  {
  if (pat[m][n] == 'C')
    {
    fprintf (testfile, "%s 0 0;\n%s 1 %d;\n%s 0 %d;\n",
      name[m],
      name[m], n*CYCLE + CLKH_POS, 
      name[m], n*CYCLE + CLKL_POS);
    dir[m] = 0;
    }
  else
    {
    if ((pat[m][n] == 'H') || 
        (pat[m][n] == 'L') ||
        (pat[m][n] == 'U'))
      {
      fprintf (testfile, "ASSERT %s %c %d;\n",
        name[m], pat[m][n], n*CYCLE);
      dir[m] = 1;
      }
    else
      {
      if ((m <= LAST_INPUT) || (pat[m][n] != 'X'))
        {
        fprintf (testfile, "%s %c %d;\n",
          name[m], pat[m][n], n*CYCLE);
        }
      dir[m] = 0;
      }
    }
  }
for (n = 1; n <= offset; n++)
  {
  for (m = FIRST_INPUT; m <= LAST_OUTPUT; m++)
    {
    if (pat[m][n] == 'C')
      fprintf (testfile, "%s 1 %d;\n%s 0 %d;\n",
        name[m], n*CYCLE + CLKH_POS, 
        name[m], n*CYCLE + CLKL_POS);
    else
      {
      if (pat[m][n] != pat[m][n-1])
        {
  /* is it an output check? */
        if ((pat[m][n] == 'H') || (pat[m][n] == 'L')) 
          {
          fprintf (testfile, "ASSERT %s %c %d;\n", name[m], pat[m][n], n*CYCLE);
          /* IO pads going from driven to checked must be set to Z */
          if ((dir[m] != 1) && (m <= LAST_INPUT) && (m >= FIRST_OUTPUT))
            fprintf (testfile, "%s Z %d;\n", name[m], n*CYCLE);
          dir[m] = 1;
          }
        else
          {
          if ((m <= LAST_INPUT) || (pat[m][n] != 'X'))
            fprintf (testfile, "%s %c %d;\n", name[m], pat[m][n], n*CYCLE);
    /* has it changed direction ? */
          if (dir[m] != 0)
            {
            fprintf (testfile,"ASSERT %s U %d;\n", name[m], n*CYCLE);
            dir[m] = 0;
          } }
        }
      }
    }
  }
fprintf (testfile, "END ss.\n");
fclose (testfile);
}

/****************************************************************************
SETSIG

Sets signal <signame> to <value>, starting at time <start> cycles, and 
running for <length> cycles.  <value> is a character, as specified by the
output code.
****************************************************************************/

setsig (signame,value,start,length)

int signame,start,length;
char value;

{
int n;
for (n = start; n < start+length; n++) pat[signame][n] = value;
}
  
/****************************************************************************
CLOCK

Clocks <ticks> cycles starting at <start>
****************************************************************************/

clock (start,ticks)

int start,ticks;

{
int n;
for (n = start; n < start+ticks; n++) pat[xtal][n] = 'C';
}

  
/****************************************************************************
SETDATA

Sets <value> on the data bus, at <start>, for <length>.  <value> is a 
hexadecimal string, of which only the bottom eight bits worth are used.
Special cases are where <value> is "UU"and "ZZ", in which case the bus is 
set to undefined or high impedance.
****************************************************************************/

setdata (value,start,length)

char *value;
int start,length;

{    
int n,val;
char state;
if (strcmp(value,"UU")==0)
        {
  for (n = ad0; n <= ad7; n++) setsig (n,'X',start,length);
  return;
  }
if (strcmp(value,"ZZ")==0)
        {
  for (n = ad0; n <= ad7; n++) setsig (n,'Z',start,length);
  return;
  }
sscanf (value,"%x",&val);
for (n = 0; n < 8; n++)
  {
  state = '0';
  if (val & (1 << n)) state++;
  setsig (ad0+n,state,start,length);
  }
}

/****************************************************************************
SETWDATA

Sets <value> on the word data bus, at <start>, for <length>.  <value> is a 
hexadecimal string, of which only the bottom sixteen bits worth are used.
****************************************************************************/

setwdata (value,start,length)

char *value;
int start,length;

{    
int n,val;
char state;
if (strcmp(value,"UUUU")==0)
        {
  for (n = ad0; n <= ad7; n++) setsig (n,'X',start,length);
  for (n = d8; n <= d15; n++) setsig (n,'X',start,length);
  return;
  }
if (strcmp(value,"ZZZZ")==0)
        {
  for (n = ad0; n <= ad7; n++) setsig (n,'Z',start,length);
  for (n = d8; n <= d15; n++) setsig (n,'X',start,length);
  return;
  }
sscanf (value,"%x",&val);
for (n = 0; n < 8; n++)
  {
  state = '0';
  if (val & (1 << n)) state++;
  setsig (ad0+n,state,start,length);
  }
for (n = 8; n < 16; n++)
  {
  state = '0';
  if (val & (1 << n)) state++;
  setsig (d8+n-8,state,start,length);
  }
}

/****************************************************************************
SETADDR

Sets <value> on the address bus, at <start>, for <length>.  <value> is a 
hexadecimal string, of which only the bottom twenty bits worth are used.

****************************************************************************/

setaddr (value,start,length)

char *value;
int start,length;

{                 
int n;
long int ival;
char state;

switch (value[0])
  {
  case ('U'):
    for (n = ad0; n <= ad7; n++) setsig (n,'U',start,length);
    for (n = a8; n <= a15; n++) setsig (n,'U',start,length);
    for (n = as16; n <= as19; n++) setsig (n,'U',start,length);
    break;
  case ('Z'):
    for (n = ad0; n <= ad7; n++) setsig (n,'Z',start,length);
    for (n = a8; n <= a15; n++) setsig (n,'Z',start,length);
    for (n = as16; n <= as19; n++) setsig (n,'Z',start,length);
    break;
  default:
    sscanf (value,"%lx",&ival);
    for (n = 0; n < 8; n++)
      {
      if (ival & (1 << n)) state = '1'; else state = '0';
      setsig (ad0+n,state,start,length);
      }
    for (n = 8; n < 16; n++)
      {
      if (ival & (1 << n)) state = '1'; else state = '0';
      setsig (a8+n-8,state,start,length);
      }
    for (n = 16; n < 20; n++)
      {
      if (ival & (1 << n)) state = '1'; else state = '0';
      setsig (as16+n-16,state,start,length);
      }
  }
}

/****************************************************************************
CHECKSIG

Checks signal <signame> is <value>, starting at time <start> cycles, and 
running for <length> cycles.  <value> is a character, as specified by the
output code, but 1 and 0 may be used in place of H and L.
****************************************************************************/

checksig (signame,value,start,length)

int signame,start,length;
char value;

{
int n;
if (value == '1') value = 'H';
if (value == '0') value = 'L';
for (n = start; n < start+length; n++) pat[signame][n] = value;
}
  
/****************************************************************************
CHECKDATA

Checks for <value> on the data bus, at <start>, for <length>.  <value> is a 
hexadecimal string, of which only the bottom eight bits worth are used.
"ZZ" and "XX" are allowed here, but are meaningless in terms of checking.
****************************************************************************/

checkdata (value,start,length)

char *value;
int start,length;

{                 
int n,val;
char state;
if (strcmp(value,"UU")==0)
        {
/*  for (n = ad0; n <= ad7; n++) checksig (n,'X',start,length); */
  return;
  }
if (strcmp(value,"ZZ")==0)
        {
/*  for (n = ad0; n <= ad7; n++) checksig (n,'Z',start,length); */
  return;
  }
sscanf (value,"%x",&val);
for (n = 0; n < 8; n++)
  {
  state = 'L';
  if (val & (1 << n)) state = 'H';
  checksig (ad0+n,state,start,length);
  }
}

/* same for high data byte */

checkhdata (value,start,length)

char *value;
int start,length;

{                 
int n,val;
char state;
if (strcmp(value,"UU")==0)
  return;
if (strcmp(value,"ZZ")==0)
  return;
sscanf (value,"%x",&val);
for (n = 0; n < 8; n++)
  {
  state = 'L';
  if (val & (1 << n)) state = 'H';
  checksig (d8+n,state,start,length);
  }
}

/****************************************************************************
CHECKWDATA

Checks for <value> on the word data bus, at <start>, for <length>.  
****************************************************************************/

checkwdata (value,start,length)

char *value;
int start,length;

{                 
int n,val;
char state;
if (strcmp(value,"UUUU")==0) return;
if (strcmp(value,"ZZZZ")==0) return;
sscanf (value,"%x",&val);
for (n = 0; n < 8; n++)
  {
  state = 'L';
  if (val & (1 << n)) state = 'H';
  checksig (ad0+n,state,start,length);
  }
for (n = 8; n < 16; n++)
  {
  state = 'L';
  if (val & (1 << n)) state = 'H';
  checksig (d8+n-8,state,start,length);
  }
}

/****************************************************************************
CHECKADDR

Checks for <value> on the address bus, at <start>, for <length>.  <value> is 
a hexadecimal string, of which only the bottom twenty bits worth are used.
Special cases as per checkdata
****************************************************************************/

checkaddr (value,start,length)

char *value;
int start,length;

{                 
int n;
long val;
char state;

if (*value == 'U')
        {
  for (n = a0; n <= a17; n++) checksig (n,'X',start,length);
  return;
  }
if (*value == 'Z')
        {
  for (n = a0; n <= a17; n++) checksig (n,'Z',start,length);
  return;
  }
sscanf (value,"%lx",&val);
for (n = 0; n < 8; n++)
  {
  state = 'L';
  if (val & (1 << n)) state = 'H';
  checksig (a0+n,state,start,length);
  }
for (n = 8; n < 16; n++)
  {
  state = 'L';
  if (val & (1 << n)) state = 'H';
  checksig (a8+n-8,state,start,length);
  }
for (n = 16; n < 18; n++)
  {
  state = 'L';
  if (val & (1 << n)) state = 'H';
  checksig (a16+n-16,state,start,length);
  }
}

