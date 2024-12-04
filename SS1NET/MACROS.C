/****************************************************************************
macros.c
28/10/88

This is a set of macros that allow testing of high level blitter functions.
****************************************************************************/

/* Declare the global variables */
           
#include "stdio.h"
#include "testpat.def"
#define DEBUG	1

/****************************************************************************
BLITCMD

Starts the blitter up using the parameters in <pars>, at time posn.  The 
assertion of bus request is monitored, and bus acknowledge is granted, but
the calling code is responsible for these signals after the last command.

The length of this operation is 62 cycles.

The parameters are in a string of hex bytes separated by commas (or anything
else), e.g. "12,34,56,...,45"
****************************************************************************/

int blitcmd (pars, addr, posn)

char *pars, *addr;
int posn;

{
char arg[3];
int n, m, offset, tim;

offset = 0;
arg[2] = 0;

/* Set up the program address */
arg[0] = *(addr+3);
arg[1] = *(addr+4);
offset += qiowrite (arg, BLITPC0, posn+offset);
arg[0] = *(addr+1);
arg[1] = *(addr+2);
offset += qiowrite (arg, BLITPC1, posn+offset);
arg[0] = '0';
arg[1] = *(addr+0);
offset += qiowrite (arg, BLITPC2, posn+offset);

/* write the command */
arg[0] = *(pars+0);
arg[1] = *(pars+1);
checksig (hold, 'L', posn+offset-1, 1);
offset += qiowrite (arg, BLITCMD, posn+offset);
checksig (hold, 'H', posn+offset, 1);

offset += 1;
tim = posn+offset;		/* record the start of bus acknowledge */

/* fetch the program */
for (n=0 ; n<12 ; n++)
	{
	arg[0]= *(pars+3*n+3);
	arg[1]= *(pars+3*n+4);
	offset += blitbrd (arg, addr, posn+offset);
	for (m = 4; m >= 0; m--)
		{
		*(addr+m) += 1;
		if (*(addr+m) <= '9') break;
		if (*(addr+m) == '9'+1)
			{
			*(addr+m) = 'A';
			break;
			}
		if (*(addr+m) <= 'F') break;
		if (*(addr+m) == 'F'+1)
			*(addr+m) = '0';
		}
	}
setsig (hlda, '1', tim, posn+offset-tim);       
return offset;
}

/****************************************************************************
BLITBRD

Simulates a blitter byte read cycle at <addr> of <data>, at time <posn>.  This
is a function returning the time taken.  The top half of the data bus is
defined as zero to keep the high part of the data path defined.
****************************************************************************/

int blitbrd (data, addr, posn)

char *data, *addr;
int posn;

{
char dat[5];
int n;

if ((*addr >= '0') && (*addr <= '3'))		/* fast SRAM */
	{
	checkaddr (addr, posn, 2);
	sscanf (addr+4, "%x", &n);
	if (n & 1)
		{
		strcpy (dat, data);
		strcat (dat, "00");
		setwdata (dat, posn+1, 2);
		checksig (scel1, '0', posn, 2);
		checksig (scel1, '1', posn+2, 1);
		checksig (scel0, '1', posn, 2);
		}
	else
		{
		strcpy (dat, "00");
		strcat (dat, data);
		setwdata (dat, posn+1, 2);
		checksig (scel0, '0', posn, 2);
		checksig (scel0, '1', posn+2, 1);
		checksig (scel1, '1', posn, 2);
		}
	checksig (oel, '0', posn, 2);
	checksig (oel, '1', posn+2, 1);
	checksig (wel, '1', posn, 2);
	return 2;
	}
if ((*addr >= '4') && (*addr <= '7'))		/* internal memory */
	{
	printf ("WARNING - test pattern performing internal memory cycles\n");
	return 0;
	}
if (((*addr >= '8') && (*addr <= '9')) ||		/* slow ROM */
    ((*addr >= 'A') && (*addr <= 'F')))
	{
	checkaddr (addr, posn, 3);
	strcpy (dat, "00");
	strcat (dat, data);
	setwdata (dat, posn+2, 2);
	checksig (oel, '0', posn, 3);
	checksig (oel, '1', posn+3, 1);
	if ((*addr >= 'C') && (*addr <= 'F'))		/* which ROM? */
		{
		checksig (csl1, '0', posn, 3);
		checksig (csl1, '1', posn+3, 1);
		}
	else
		{
		checksig (csl0, '0', posn, 3);
		checksig (csl0, '1', posn+3, 1);
		}
	return 3;
	}
}

/****************************************************************************
BLITWRD

Simulates a blitter word read cycle at <addr> of <data>, at time <posn>.  This
is a function returning the time taken.
****************************************************************************/

int blitwrd (data, addr, posn)

char *data, *addr;
int posn;

{

if ((*addr >= '0') && (*addr <= '3'))		/* fast SRAM */
	{
	checkaddr (addr, posn, 2);
	setwdata (data, posn+1, 2);
	checksig (scel0, '0', posn, 2);
	checksig (scel0, '1', posn+2, 1);
	checksig (scel1, '0', posn, 2);
	checksig (scel1, '1', posn+2, 1);
	checksig (oel, '0', posn, 2);
	checksig (oel, '1', posn+2, 1);
	checksig (wel, '1', posn, 2);
	return 2;
	}
else printf ("ERROR - test pattern attempting word cycles in byte memory\n");
}

/****************************************************************************
BLITBWR

Simulates a blitter byte write cycle at <addr> of <data>, at time <posn>.  This
is a function returning the time taken.
****************************************************************************/

int blitbwr (data, addr, posn)

char *data, *addr;
int posn;

{
int n;

setsig (ai2, '0', posn, 1);			/* disable data in first tick */

if ((*addr >= '0') && (*addr <= '3'))		/* fast SRAM */
	{
	checkaddr (addr, posn, 2);
	sscanf (addr+4, "%x", &n);
	if (n & 1)
		{
		checkhdata (data, posn+1, 1);
		checksig (scel1, '0', posn, 2);
		checksig (scel1, '1', posn+2, 1);
		checksig (scel0, '1', posn, 2);
		}
	else
		{
		checkdata (data, posn+1, 1);
		checksig (scel0, '0', posn, 2);
		checksig (scel0, '1', posn+2, 1);
		checksig (scel1, '1', posn, 2);
		}
	checksig (wel, '0', posn, 1);
	checksig (wel, '1', posn+2, 1);
	checksig (oel, '1', posn, 2);
	return 2;
	}
if ((*addr >= '4') && (*addr <= '7'))		/* internal memory */
	{
	printf ("WARNING - test pattern performing internal memory cycles\n");
	return 0;
	}
if (((*addr >= '8') && (*addr <= '9')) ||		/* slow ROM */
    ((*addr >= 'A') && (*addr <= 'F')))
	{
	checkaddr (addr, posn, 3);
	checkdata (data, posn+2, 1);
	checksig (wel, '0', posn, 1);
	if ((*addr >= 'C') && (*addr <= 'F'))		/* which ROM? */
		{
		checksig (csl1, '0', posn, 3);
		checksig (csl1, '1', posn+3, 1);
		}
	else
		{
		checksig (csl0, '0', posn, 3);
		checksig (csl0, '1', posn+3, 1);
		}
	return 3;
	}
}

/****************************************************************************
BLITWWR

Simulates a blitter word write cycle at <addr> of <data>, at time <posn>.  This
is a function returning the time taken.
****************************************************************************/

int blitwwr (data, addr, posn)

char *data, *addr;
int posn;

{
setsig (ai2, '0', posn, 1);			/* disable data in first tick */

if ((*addr >= '0') && (*addr <= '3'))		/* fast SRAM */
	{
	checkaddr (addr, posn, 2);
	checkwdata (data, posn+1, 1);
	checksig (scel0, '0', posn, 2);
	checksig (scel0, '1', posn+2, 1);
	checksig (scel1, '0', posn, 2);
	checksig (scel1, '1', posn+2, 1);
	checksig (wel, '0', posn, 1);
	checksig (wel, '1', posn+2, 1);
	checksig (oel, '1', posn, 2);
	return 2;
	}
else printf ("ERROR - test pattern attempting word cycles in byte memory\n");
}

/****************************************************************************
INHIBWR

Simulates an inhibited blitter write cycle at time <posn>.  This
takes 2 cycles.
****************************************************************************/

int inhibwr (posn)

int posn;

{
checksig (scel0, '1', posn, 2);
checksig (scel1, '1', posn, 2);
checksig (csl0, '1', posn, 2);
checksig (csl1, '1', posn, 2);
checksig (wel, '1', posn, 2);
checksig (oel, '1', posn, 2);
return 2;
}

/****************************************************************************
QIOWRITE

Simulates a quick IO write cycle with <data> to <addr>, at time <posn>
This takes 4 cycles
****************************************************************************/

int qiowrite (data, addr, posn)

char *data, *addr;
int posn;

{
setaddr (addr, posn, 2);
setsig (ale,'1',posn,1);
setdata (data, posn+2, 2);
setsig (wrl,'0',posn+2,1);
setsig (iom,'1',posn,4);

return 4;
}

/****************************************************************************
QIOREAD

Simulates a quick IO read cycle with <data> to <addr>, at time <posn>
This takes 5 cycles
****************************************************************************/

int qioread (data, addr, posn)

char *data, *addr;
int posn;

{
setaddr (addr, posn, 2);
setsig (ale,'1',posn,1);
checkdata (data, posn+3, 1);
setsig (iom,'1',posn,4);
setsig (rdl,'0',posn+3,1);

return 5;
}

/****************************************************************************
QMWRITE
 
Simulates a quick memory write cycle with <data> to <addr>, at time <posn>
This takes 4 cycles
****************************************************************************/
 
int qmwrite (data, addr, posn)
 
char *data, *addr;
int posn;
 
{
char mid[3];
mid[0] = *(addr+1);
mid[1] = *(addr+2);
mid[2] = 0;
 
setaddr (addr, posn, 2);
setsig (ale,'1',posn,1);
setdata (data, posn+2, 2);
setmaddr (mid, posn+2, 2);
setsig (wrl,'0',posn+2,1);
setsig (iom,'0',posn,4);
 
return 4;
}
 
/****************************************************************************
DSPIWR

Dsp Intrude Write.
Write a word into DSP memory space. Assumes DSP in stop mode.
The length of this operation is 8 cycles.

****************************************************************************/

int dspiwr (addr, data, posn)

char *data, *addr;
int posn;

{
char address[6];
char hddress[6];
char lodata[3];
char hidata[3];

address[0]='4';
address[1]='1';
address[2]= *addr;
address[3]= *(addr+1);
address[4]= *(addr+2);
address[5]= '\0';

hddress[0]='4';
hddress[1]='1';
hddress[2]= *addr;
hddress[3]= *(addr+1);
hddress[4]= *(addr+2)+1;
hddress[5]= '\0';

hidata[0]= *data;
hidata[1]= *(data+1);
hidata[2]= '\0';
lodata[0]= *(data+2);
lodata[1]= *(data+3);
lodata[2]= '\0';
qmwrite (lodata, address, posn);
qmwrite (hidata, hddress, posn+4);
return (posn + 8);
}

