/****************************************************************************
lutils.c
28/9/88

Description

This is a set of utility programs used in building LSI Logic simulator test
patterns and the files used for checking the output.

****************************************************************************/

/* Declare the global variables */
	   
#include "stdio.h"
#define UTILS 1			/* used in testpat.def */
#include "testpat.def"

#define DEBUG	0

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
	for (m = 0; m < SIGNALS; m++)
		pat[m][n] = def_val[m];
	}
/* But set the first cycle to LSI Logic's requirement of X on inputs and Z
on tri-states */
for (n = FIRST_INPUT; n < FIRST_OUTPUT; n++) pat[n][0] = 'X';
for (n = FIRST_OUTPUT; n <= LAST_OUTPUT; n++) pat[n][0] = 'Z';
pat[xtal][0] = 'C';
pat[lpl][0] = 'D';
}

/****************************************************************************
SIM_TERM

Write out the test pattern at the end of pattern generation.
****************************************************************************/

sim_term (offset)
int offset;

{
int n,m;

/* Open the test pattern file for writing */
printf ("Writing out test pattern file\n");

if ((testfile=fopen("../SS.TPT1A","w"))==NULL)
	{
	printf ("ERROR - unable to open stimulus file for writing\n");
	exit (0);
	}

for (n = 0; n <= offset; n++)
	{
	for (m = FIRST_INPUT; m <= LAST_INPUT; m++)
		{
		if (pat[m][n] == '1') fputc ('1',testfile);
		if (pat[m][n] == '0') fputc ('0',testfile);
		if (pat[m][n] == 'Z') fputc ('Z',testfile);
		if (pat[m][n] == 'X') fputc ('X',testfile);
		if (pat[m][n] == 'H') fputc ('Z',testfile);
		if (pat[m][n] == 'L') fputc ('Z',testfile);
		if (pat[m][n] == 'C') 
			{
			if (n == 0) fputc ('X',testfile);
			else	fputc ('0',testfile);
			}
		if (pat[m][n] == 'D')
			{
			if (n == 0) fputc ('X',testfile);
			else	fputc ('1',testfile);
			}
		}
	fprintf (testfile," %d\n",n*CYCLE*10);
	for (m = FIRST_INPUT; m <= LAST_INPUT; m++)
		{
		if (pat[m][n] == 'D')	fputc ('0',testfile);
		else			fputc (' ',testfile);
		}
	fprintf (testfile," %d\n",n*CYCLE*10 + QCLKL_POS*10);
        for (m = FIRST_INPUT; m <= LAST_INPUT; m++)
                {
                if (pat[m][n] == 'D')   fputc ('1',testfile);
                else                    fputc (' ',testfile);
                }
        fprintf (testfile," %d\n",n*CYCLE*10 + QCLKH_POS*10);
	for (m = FIRST_INPUT; m <= LAST_INPUT; m++)
		{
		if (pat[m][n] == 'C')	fputc ('1',testfile);
		else			fputc (' ',testfile);
		}
	fprintf (testfile," %d\n",n*CYCLE*10 + CLKH_POS*10);
	for (m = FIRST_INPUT; m <= LAST_INPUT; m++)
		{
		if (pat[m][n] == 'C')	fputc ('0',testfile);
		else			fputc (' ',testfile);
		}
	fprintf (testfile," %d\n",n*CYCLE*10 + CLKL_POS*10);
	}
fclose (testfile);

/* Open the checker pattern file for writing */
printf ("Writing out checker file\n");

if ((testfile=fopen("../SS.CHKA","w"))==NULL)
	{
	printf ("ERROR - unable to open checker file for writing\n");
	exit (0);
	}

for (n = 0; n <= offset; n++)
	{
	for (m = FIRST_OUTPUT; m <= LAST_OUTPUT; m++)
		{
		if (pat[m][n] == '1') fputc (' ',testfile);
		if (pat[m][n] == '0') fputc (' ',testfile);
		if (pat[m][n] == 'Z') fputc (' ',testfile);
		if (pat[m][n] == 'X') fputc (' ',testfile);
		if (pat[m][n] == 'U') fputc (' ',testfile);
		if (pat[m][n] == 'H') fputc ('1',testfile);
		if (pat[m][n] == 'L') fputc ('0',testfile);
		if (pat[m][n] == 'C') fputc (' ',testfile);
		}
	fprintf (testfile,": %d\n",n);
	}
printf ("Test time %d 100ps units\n",offset*CYCLE*10);
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

 
/****************************************************************************
SETMADDR
 
Sets <value> on the address lines a[8..15] at <start>,
for <length>.  <value> is a hexadecimal string.
Where <value> is "ZZ" the bus is set to high impedance.
****************************************************************************/
 
setmaddr (value,start,length)
 
char *value;
int start,length;
 
{
int val,n;
char state;
 
if (strcmp(value,"ZZ")==0) return;
sscanf (value,"%x",&val);
for (n = 0; n < 8; n++)
  {
  state = '0';
  if (val & (1 << n)) state = '1';
  setsig (a8+n,state,start,length);
  }
}

