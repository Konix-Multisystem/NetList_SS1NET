
/****************************************************************************
testpat.c
27/10/88

Slipstream Blitter Test Pattern

****************************************************************************/

/* Declare the global variables */
           
#include "stdio.h"
#include "testpat.def"

#define	BMTEST	1		/* Test blitter in various modes */
#define	BCTEST	1		/* Test blitter counters */
#define	BDTEST	1		/* Test blitter with DSP going */
#define TESTZ	0		/* Tristate all outputs */
#define TESTS	0		/* Set scan test mode */
#define TESTL	0		/* Set ALU test mode */

main ()

{
int offset,m,n,test,temp,bustim, inttim, clocktim, dspclktim;
char ad[6];

printf ("Starting to compute Test Pattern\n");

/* Initialise the signals */

sim_init ();

offset=0;                

/****************************************************************************
Build the test pattern

****************************************************************************/

/****************************************************************************
Blitter test 
****************************************************************************/

offset += 1;					/* LSI Logic start-up cycle */

/* Reset the device */


setsig (rdl,'0',offset,1);
setsig (wrl,'0',offset,1);
setsig (reset,'1',offset,6);
setsig (joy0,'0',offset+5,2);			/* set up TV mode */
setsig (joy1,'0',offset+5,2);			/* set static ram */
setsig (joy2,'0',offset+5,2);			/* set up CPU speed */
setsig (a12, '0',offset+1, 7);			/* Try and keep DSP defined */
clock (offset, offset+5);			/* clock off for reset end */

offset += 7;

#if BMTEST || BCTEST || BDTEST

printf ("Initialising for blitter test\n");

clock (offset, offset+3);			/* clock to switch testen off */
offset += 4;

offset += qiowrite ("08", diag, offset);	/* set diagnostic clock */
offset +=1;
clocktim = offset;				/* clock on from here */
dspclktim = offset;				/* pervy DSP clock on */

setsig (inta, '1', offset, 2);
offset += 3;
offset += qiowrite ("00", ack, offset);		/* clear any interrupt */
offset += qiowrite ("04", endl, offset);	/* set end line to 4 */
offset += qiowrite ("00", vcnth, offset);	/* set line 4 for no refresh */
offset += qiowrite ("04", vcntl, offset);
offset += qiowrite ("00", hcnth, offset);	/* set to start of line */
offset += qiowrite ("00", hcntl, offset);
offset += qiowrite ("08", dis, offset);		/* disable AI_2 interrupt */
offset += qiowrite ("10", TESTREG, offset);	/* enable test disabling */

#endif

#if BMTEST

/* Blit some data into the palette RAM, then get it back */

offset += blitcmd ("21,54,32,01,00,00,04,20,C0,01,04,00,00","11111",offset);
 
bustim = offset;	/* store time to start driving bus ack */
offset += 1;		/* load inner count */

offset += blitbrd ("12","13254",offset);
checkaddr ("40000",offset,2);
setsig (ai2, '0', offset, 1);			/* disable data in first tick */
offset += 2;

offset += blitbrd ("ED","13255",offset);
checkaddr ("40001",offset,2);
setsig (ai2, '0', offset, 1);			/* disable data in first tick */
offset += 2;

offset += blitbrd ("55","13256",offset);
checkaddr ("40002",offset,2);
setsig (ai2, '0', offset, 1);			/* disable data in first tick */
offset += 2;

offset += blitbrd ("99","13257",offset);
checkaddr ("40003",offset,2);
setsig (ai2, '0', offset, 1);			/* disable data in first tick */
offset += 2;

offset += 1;
/* Now load up the program for the other direction */
offset += blitbrd ("21","1111D",offset);
offset += 1;		/* pass through stopped state */
offset += blitbrd ("00","1111E",offset);
offset += blitbrd ("00","1111F",offset);
offset += blitbrd ("04","11120",offset);
offset += blitbrd ("96","11121",offset);
offset += blitbrd ("53","11122",offset);
offset += blitbrd ("01","11123",offset);
offset += blitbrd ("20","11124",offset);
offset += blitbrd ("C0","11125",offset);
offset += blitbrd ("01","11126",offset);
offset += blitbrd ("04","11127",offset);
offset += blitbrd ("00","11128",offset);
offset += blitbrd ("00","11129",offset);
offset += 1;		/* load inner count */

checkaddr ("40000",offset,2);
offset += 2;
offset += blitbwr ("12","15396",offset);

checkaddr ("40001",offset,2);
offset += 2;
offset += blitbwr ("ED","15397",offset);

checkaddr ("40002",offset,2);
offset += 2;
offset += blitbwr ("55","15398",offset);

checkaddr ("40003",offset,2);
offset += 2;
offset += blitbwr ("99","15399",offset);

offset += 1;
offset += blitbrd ("00","1112A",offset);

/* Now watch it hand the bus back */
setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,5);
checksig (hold, 'L',offset,1);
offset += 2;

/* Check byte not equal operation, and collision stop and resume mechanism,
and throw in INT stop for good measure.
WARNING - can DSP access be tested too? */

offset += blitcmd ("23,11,11,1C,22,22,12,22,A2,33,FF,00,57","0EFFF",offset);
bustim = offset;	/* store time to start driving bus ack */
offset += 1;		/* load inner count */

offset += blitbrd ("57","C1111",offset);
offset += blitbwr ("57","22222",offset);

offset += blitbrd ("56","C1112",offset);
offset += 1;		/* allow the data to be loaded */

/* Now watch it hand the bus back */
setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,4);
checksig (hold, 'L',offset,1);

offset += 2;		/* let the bus release */

/* Read the address registers and counters */
offset += qioread ("23",BLITDST0, offset);
offset += qioread ("22",BLITDST1, offset);
offset += qioread ("02",BLITDST2, offset);
offset += qioread ("13",BLITSRC0, offset);
offset += qioread ("11",BLITSRC1, offset);
offset += qioread ("0C",BLITSRC2, offset);
offset += qioread ("FE",BLITICNT, offset);
offset += qioread ("32",BLITOCNT, offset);
offset += qioread ("06",BLITSTAT, offset);

/* Restart the blitter with resume */
offset += qiowrite ("02",BLITCON, offset);
offset += 2;		/* let resume trickle through */
bustim = offset;	/* store time to start driving bus ack */

offset += inhibwr (offset);

offset += blitbrd ("57","C1113",offset);
setsig (ai0, '0', offset-2,1);
/* it gives up the bus because of the interrupt */
setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,4);
checksig (hold, 'L',offset,1);
offset += 2;
offset += qioread ("05",BLITSTAT, offset);	/* Check status flag */
offset += qiowrite ("01",BLITCON, offset);	/* Mask interrupts */

/* Now clear the interrupt */
setsig (inta, '1', offset, 2);
offset += 3;
offset += qiowrite ("00", ack, offset);		/* clear any interrupt */
bustim = offset;	/* store time to start driving bus ack */

offset += blitbwr ("57","22224",offset);
setsig (ai1, '0', offset-2,1);			/* masked interrupt */
offset += blitbrd ("47","C1114",offset);

/* Now watch it hand the bus back */
setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,5);
checksig (hold, 'L',offset,1);
offset += 4;


setsig (inta, '1', offset, 2);			/* Now clear the interrupt */
offset += 3;
offset += qiowrite ("00", ack, offset);

offset += qioread ("06",BLITSTAT, offset);	/* check status */
offset += qiowrite ("04",BLITCON, offset);	/* Reset the blitter */
offset += qioread ("04",BLITSTAT, offset);	/* check status */

/* Very similiar to above, but with the reads on all the address bits 
inverted */

offset += blitcmd ("23,EB,EE,13,DC,DD,1D,22,A2,CE,FF,00,57","0EFFF",offset);
bustim = offset;	/* store time to start driving bus ack */
offset += 1;		/* load inner count */

offset += blitbrd ("77","3EEEB",offset);
offset += 1;		/* allow the data to be loaded */

/* Now watch it hand the bus back */
setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,4);
checksig (hold, 'L',offset,1);

offset += 2;		/* let the bus release */

/* Read the address registers */
offset += qioread ("DC",BLITDST0, offset);
offset += qioread ("DD",BLITDST1, offset);
offset += qioread ("0D",BLITDST2, offset);
offset += qioread ("EC",BLITSRC0, offset);
offset += qioread ("EE",BLITSRC1, offset);
offset += qioread ("03",BLITSRC2, offset);
offset += qioread ("CD",BLITOCNT, offset);
offset += qiowrite ("04",BLITCON, offset);	/* Reset the blitter */

/* This is a test of byte equal */

offset += blitcmd ("21,54,76,18,DE,BC,1A,20,C1,01,05,00,BB","FFFF8",offset);
bustim = offset;	/* store time to start driving bus ack */
offset += 1;		/* load inner count */

offset += blitbrd ("85","87654",offset);
offset += blitbwr ("85","ABCDE",offset);

offset += blitbrd ("BB","87655",offset);
offset += inhibwr (offset);

offset += blitbrd ("AB","87656",offset);
offset += blitbwr ("AB","ABCE0",offset);

offset += blitbrd ("BC","87657",offset);
offset += blitbwr ("BC","ABCE1",offset);

offset += blitbrd ("FB","87658",offset);
offset += blitbwr ("FB","ABCE2",offset);

offset += 1;
offset += blitbrd ("00","00004",offset);

/* Now watch it hand the bus back */
setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,5);
checksig (hold, 'L',offset,1);
offset += 2;
 
/* Test srccmp and dstcmp with a couple of byte equal commands */
 
offset += blitcmd ("61,59,26,03,89,73,12,20,C1,01,02,00,31","19322",offset);
bustim = offset;        /* store time to start driving bus ack */
offset += 1;            /* load inner count */
 
offset += blitbrd ("14","32659",offset);
offset += blitbrd ("15","27389",offset);
offset += blitbwr ("14","27389",offset);
offset += blitbrd ("92","3265A",offset);
offset += blitbrd ("31","2738A",offset);
offset += inhibwr (offset);
 
offset += 1;
offset += blitbrd ("00","1932E",offset);
 
/* Now watch it hand the bus back */
setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,5);
checksig (hold, 'L',offset,1);
offset += 2;
 
offset += blitcmd ("61,A6,D9,10,76,8C,01,20,52,01,02,00,CE","26CDD",offset);
bustim = offset;        /* store time to start driving bus ack */
offset += 1;            /* load inner count */
 
offset += blitbrd ("CE","0D9A6",offset);
offset += blitbrd ("15","18C76",offset);
offset += blitbwr ("EA","18C76",offset);
offset += blitbrd ("34","0D9A7",offset);
offset += blitbrd ("92","18C77",offset);
offset += inhibwr (offset);
 
offset += 1;
offset += blitbrd ("00","26CE9",offset);
 
/* Now watch it hand the bus back */
setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,5);
checksig (hold, 'L',offset,1);
offset += 2;
 
/* This is a lores line draw */

offset += blitcmd ("41,03,06,10,7E,22,12,18,C0,01,08,05,EE","11111",offset);
bustim = offset;	/* store time to start driving bus ack */
offset += 1;		/* load inner count */

offset += blitbrd ("11","2227E",offset);
offset += blitbwr ("1E","2227E",offset);
offset += blitbrd ("11","222FE",offset);
offset += blitbwr ("E1","222FE",offset);
offset += blitbrd ("11","2237F",offset);
offset += blitbwr ("1E","2237F",offset);
offset += blitbrd ("11","2237F",offset);
offset += blitbwr ("E1","2237F",offset);
offset += blitbrd ("11","22380",offset);
offset += blitbwr ("1E","22380",offset);
offset += blitbrd ("11","22400",offset);
offset += blitbwr ("E1","22400",offset);
offset += blitbrd ("11","22481",offset);
offset += blitbwr ("1E","22481",offset);
offset += blitbrd ("11","22501",offset);
offset += blitbwr ("E1","22501",offset);
                      
offset += 1;
offset += blitbrd ("00","1111D",offset);

/* Now watch it hand the bus back */
setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,5);
checksig (hold, 'L',offset,1);
offset += 2;

/* This is another lores line draw */

offset += blitcmd ("41,03,06,50,7E,22,52,18,C0,01,03,05,EE","11111",offset);
bustim = offset;	/* store time to start driving bus ack */
offset += 1;		/* load inner count */

offset += blitbrd ("11","2227E",offset);
offset += blitbwr ("1E","2227E",offset);
offset += blitbrd ("11","221FD",offset);
offset += blitbwr ("E1","221FD",offset);
offset += blitbrd ("11","2217D",offset);
offset += blitbwr ("1E","2217D",offset);
                      
offset += 1;
offset += blitbrd ("00","1111D",offset);

/* Now watch it hand the bus back */
setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,5);
checksig (hold, 'L',offset,1);
offset += 2;

/* These two exercise the outer loop */

offset += blitcmd ("2D,00,22,10,33,11,10,20,C0,02,01,73,EE","11111",offset);
bustim = offset;	/* store time to start driving bus ack */
offset += 1;		/* load inner count */

offset += blitbrd ("11","02200",offset);
offset += blitbwr ("11","01133",offset);
offset += 2;
offset += blitbrd ("02","1111D",offset);
offset += blitbrd ("33","1111E",offset);
offset += blitbrd ("00","1111F",offset);
offset += 1;
offset += blitbrd ("22","02274",offset);
offset += blitbwr ("22","01134",offset);
offset += blitbrd ("44","02275",offset);
offset += blitbwr ("44","01135",offset);
                      
offset += 1;
offset += blitbrd ("00","11120",offset);

/* Now watch it hand the bus back */
setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,5);
checksig (hold, 'L',offset,1);
offset += 2;

/* outer loop exercise, plus a collision stop from dest read */

offset += blitcmd ("67,00,22,10,33,11,10,20,C1,02,01,73,EE","11111",offset);
bustim = offset;	/* store time to start driving bus ack */
offset += 1;		/* load inner count */

offset += blitbrd ("11","02200",offset);
offset += blitbrd ("55","01133",offset);
offset += blitbwr ("11","01133",offset);
offset += 1;
offset += blitbrd ("02","1111D",offset);
offset += blitbrd ("33","1111E",offset);
offset += blitbrd ("00","1111F",offset);
offset += 1;
offset += blitbrd ("22","02201",offset);
offset += blitbrd ("22","01134",offset);
offset += 1;

/* Now watch it hand the bus back */
setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,4);
checksig (hold, 'L',offset,1);
offset += 2;		/* let the bus release */

offset += qiowrite ("04",BLITCON, offset);	/* Reset the blitter */
offset += 2;

/* This is a character paint in lores */

offset += blitcmd ("D1,00,00,10,FE,FF,32,84,C0,02,08,FC,33","3210F",offset);
bustim = offset;	/* store time to start driving bus ack */
offset += 1;		/* load inner count */

offset += blitbrd ("55","00000",offset);
offset += blitbrd ("CC","2FFFE",offset);
offset += blitbwr ("C3","2FFFE",offset);
offset += blitbrd ("CC","2FFFE",offset);
offset += inhibwr (offset);
offset += blitbrd ("CC","2FFFF",offset);
offset += blitbwr ("C3","2FFFF",offset);
offset += blitbrd ("CC","2FFFF",offset);
offset += inhibwr (offset);
offset += blitbrd ("CC","20000",offset);
offset += blitbwr ("C3","20000",offset);
offset += blitbrd ("CC","20000",offset);
offset += inhibwr (offset);
offset += blitbrd ("CC","20001",offset);
offset += blitbwr ("C3","20001",offset);
offset += blitbrd ("CC","20001",offset);
offset += inhibwr (offset);
offset += 3;		/* dec outer, update dest, load inner */
offset += blitbrd ("AA","00001",offset);
offset += blitbrd ("CC","200FE",offset);
offset += inhibwr (offset);
offset += blitbrd ("CC","200FE",offset);
offset += blitbwr ("3C","200FE",offset);
offset += blitbrd ("CC","200FF",offset);
offset += inhibwr (offset);
offset += blitbrd ("CC","200FF",offset);
offset += blitbwr ("3C","200FF",offset);
offset += blitbrd ("CC","20100",offset);
offset += inhibwr (offset);
offset += blitbrd ("CC","20100",offset);
offset += blitbwr ("3C","20100",offset);
offset += blitbrd ("CC","20101",offset);
offset += inhibwr (offset);
offset += blitbrd ("CC","20101",offset);
offset += blitbwr ("3C","20101",offset);

offset += 1;
offset += blitbrd ("00","3211B",offset);

/* Now watch it hand the bus back */
setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,5);
checksig (hold, 'L',offset,1);
offset += 2;

/* This is a hires line draw */

offset += blitcmd ("41,04,05,90,AA,AA,52,59,C0,01,0A,03,EE","AAAAA",offset);
bustim = offset;	/* store time to start driving bus ack */
offset += 1;		/* load inner count */

offset += blitbrd ("11","2AAAA",offset);
offset += blitbwr ("1E","2AAAA",offset);
offset += blitbrd ("11","2AAAA",offset);
offset += blitbwr ("E1","2AAAA",offset);
offset += blitbrd ("11","2A9AB",offset);
offset += blitbwr ("1E","2A9AB",offset);
offset += blitbrd ("11","2A8AB",offset);
offset += blitbwr ("E1","2A8AB",offset);
offset += blitbrd ("11","2A8AC",offset);
offset += blitbwr ("1E","2A8AC",offset);
offset += blitbrd ("11","2A7AC",offset);
offset += blitbwr ("E1","2A7AC",offset);
offset += blitbrd ("11","2A6AD",offset);
offset += blitbwr ("1E","2A6AD",offset);
offset += blitbrd ("11","2A6AD",offset);
offset += blitbwr ("E1","2A6AD",offset);
offset += blitbrd ("11","2A5AE",offset);
offset += blitbwr ("1E","2A5AE",offset);
offset += blitbrd ("11","2A4AE",offset);
offset += blitbwr ("E1","2A4AE",offset);
                      
offset += 1;
offset += blitbrd ("00","AAAB6",offset);

/* Now watch it hand the bus back */
setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,5);
checksig (hold, 'L',offset,1);
offset += 2;

/* This is a test of plane priority not equal in lores */

offset += blitcmd ("61,41,57,D1,7F,34,12,00,CA,01,04,FF,FF","11111",offset);
bustim = offset;	/* store time to start driving bus ack */
offset += 1;		/* load inner count */

offset += blitbrd ("85","15741",offset);	/* compare high pixel */
offset += blitbrd ("0C","2347F",offset);	/* with low pixel */
offset += blitbwr ("08","2347F",offset);
offset += blitbrd ("C0","15741",offset);
offset += blitbrd ("C0","2347F",offset);
offset += inhibwr (offset);
offset += blitbrd ("FF","15740",offset);
offset += blitbrd ("F0","23480",offset);
offset += inhibwr (offset);
offset += blitbrd ("4F","15740",offset);
offset += blitbrd ("80","23480",offset);
offset += blitbwr ("F0","23480",offset);

offset += 1;
offset += blitbrd ("00","1111D",offset);

/* Now watch it hand the bus back */
setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,5);
checksig (hold, 'L',offset,1);
offset += 2;

/* This is a test of plane priority greater than or equal in medres */

offset += blitcmd ("61,CC,99,12,11,99,13,20,3D,01,05,FF,FF","22222",offset);
bustim = offset;	/* store time to start driving bus ack */
offset += 1;		/* load inner count */

offset += blitbrd ("40","299CC",offset);
offset += blitbrd ("00","39911",offset);
offset += blitbwr ("BF","39911",offset);
offset += blitbrd ("BF","299CD",offset);
offset += blitbrd ("45","39912",offset);
offset += blitbwr ("40","39912",offset);
offset += blitbrd ("FF","299CE",offset);
offset += blitbrd ("C0","39913",offset);
offset += inhibwr (offset);
offset += blitbrd ("43","299CF",offset);
offset += blitbrd ("80","39914",offset);
offset += inhibwr (offset);
offset += blitbrd ("81","299D0",offset);
offset += blitbrd ("80","39915",offset);
offset += inhibwr (offset);

offset += 1;
offset += blitbrd ("00","2222E",offset);

/* Now watch it hand the bus back */
setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,5);
checksig (hold, 'L',offset,1);
offset += 2;

/* This is a test of pixel equal in hires*/

offset += blitcmd ("61,CC,DD,1E,11,22,13,40,61,01,04,FF,FF","14444",offset);
bustim = offset;	/* store time to start driving bus ack */
offset += 1;		/* load inner count */

offset += blitbrd ("03","EDDCC",offset);	/* with low pixel */
offset += blitbrd ("C1","32211",offset);	/* compare low pixel */
offset += blitbwr ("C2","32211",offset);	/* and write XORed data */

offset += blitbrd ("FF","EDDCC",offset);
offset += blitbrd ("F0","32211",offset);
offset += inhibwr (offset);

offset += blitbrd ("40","EDDCD",offset);
offset += blitbrd ("80","32212",offset);
offset += inhibwr (offset);

offset += blitbrd ("CB","EDDCD",offset);
offset += blitbrd ("E0","32212",offset);
offset += blitbwr ("20","32212",offset);

offset += 1;
offset += blitbrd ("00","14450",offset);

/* Now watch it hand the bus back */
setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,5);
checksig (hold, 'L',offset,1);
offset += 2;

/* This is a medres line draw */

offset += blitcmd ("01,03,04,10,55,55,10,28,80,01,04,04,77","28888",offset);
bustim = offset;	/* store time to start driving bus ack */
offset += 1;		/* load inner count */

offset += blitbwr ("77","05555",offset);
offset += blitbwr ("77","05656",offset);
offset += blitbwr ("77","05757",offset);
offset += blitbwr ("77","05858",offset);
offset += 1;
offset += blitbrd ("00","28894", offset);

/* Now watch it hand the bus back */
setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,5);
checksig (hold, 'L',offset,1);
offset += 2;

/* This is a test of the parameter read mechanism */

offset += blitcmd ("15,FF,FF,1F,AA,AA,1A,20,80,02,03,80,66","1FFFC",offset);
bustim = offset;	/* store time to start driving bus ack */
offset += 1;		/* load inner count */

offset += blitbwr ("66","AAAAA",offset);
offset += blitbwr ("66","AAAAB",offset);
offset += blitbwr ("66","AAAAC",offset);

offset += 2;		/* dec outer and dest update */
offset += blitbrd ("04","20008",offset);
offset += blitbrd ("00","20009",offset);
offset += blitbrd ("BB","2000A",offset);
offset += 1;

offset += blitbwr ("BB","AAB2D",offset);
offset += blitbwr ("BB","AAB2E",offset);
offset += blitbwr ("BB","AAB2F",offset);
offset += blitbwr ("BB","AAB30",offset);

offset += 1;
offset += blitbrd ("00","2000B",offset);

/* Now watch it hand the bus back */
setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,5);
checksig (hold, 'L',offset,1);
offset += 2;

/* This is a small character paint in medres */

offset += blitcmd ("91,FF,FF,3F,22,22,12,24,A0,03,08,F8,FF","00000",offset);
bustim = offset;	/* store time to start driving bus ack */
offset += 1;		/* load inner count */

offset += blitbrd ("A5","FFFFF",offset);
offset += blitbwr ("FF","22222",offset);
offset += inhibwr (offset);
offset += blitbwr ("FF","22224",offset);
offset += inhibwr (offset);
offset += inhibwr (offset);
offset += blitbwr ("FF","22227",offset);
offset += inhibwr (offset);
offset += blitbwr ("FF","22229",offset);
offset += 3;		/* dec outer, update dest, load inner */
offset += blitbrd ("37","F0000",offset);
offset += blitbwr ("FF","22322",offset);
offset += blitbwr ("FF","22323",offset);
offset += blitbwr ("FF","22324",offset);
offset += inhibwr (offset);
offset += blitbwr ("FF","22326",offset);
offset += blitbwr ("FF","22327",offset);
offset += inhibwr (offset);
offset += inhibwr (offset);
offset += 3;		/* dec outer, update dest, load inner */
offset += blitbrd ("C8","F0001",offset);
offset += inhibwr (offset);
offset += inhibwr (offset);
offset += inhibwr (offset);
offset += blitbwr ("FF","22425",offset);
offset += inhibwr (offset);
offset += inhibwr (offset);
offset += blitbwr ("FF","22428",offset);
offset += blitbwr ("FF","22429",offset);
offset += 1;
offset += blitbrd ("00","0000C",offset);

/* Now watch it hand the bus back */
setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,5);
checksig (hold, 'L',offset,1);
offset += 2;

/* This is a simple move of five bytes from a source to a destination on each
of two lines */

offset += blitcmd ("39,45,23,11,FD,FF,1F,20,C0,02,05,FB,00","ABCDE",offset);
 
bustim = offset;	/* store time to start driving bus ack */
offset += 1;		/* load inner count */

offset += blitbrd ("12","12345",offset);
offset += blitbwr ("12","FFFFD",offset);
offset += blitbrd ("78","12346",offset);
offset += blitbwr ("78","FFFFE",offset);
offset += blitbrd ("55","12347",offset);
offset += blitbwr ("55","FFFFF",offset);
offset += blitbrd ("AA","12348",offset);
offset += blitbwr ("AA","00000",offset);
offset += blitbrd ("CC","12349",offset);
offset += blitbwr ("CC","00001",offset);

offset += 4;		/* Dec outer, Source & dest updates, load inner */

offset += blitbrd ("12","12445",offset);
offset += blitbwr ("12","000FD",offset);
offset += blitbrd ("78","12446",offset);
offset += blitbwr ("78","000FE",offset);
offset += blitbrd ("55","12447",offset);
offset += blitbwr ("55","000FF",offset);
offset += blitbrd ("AA","12448",offset);
offset += blitbwr ("AA","00100",offset);
offset += blitbrd ("CC","12449",offset);
offset += blitbwr ("CC","00101",offset);

offset += 1;
offset += blitbrd ("00","ABCEA",offset);

/* Now watch it hand the bus back */
setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,5);
checksig (hold, 'L',offset,1);
offset += 2;

/* This is a test of priority equal in hires with an inner count of 1 and
using the outer counter, and the step register */

offset += blitcmd ("71,00,00,D3,7F,34,12,41,C9,04,01,FF,FF","30105",offset);
bustim = offset;	/* store time to start driving bus ack */
offset += 1;		/* load inner count */

offset += blitbrd ("85","30000",offset);	/* high nybble active - swap */
offset += blitbrd ("C0","2347F",offset);	/* low nybble active */
offset += blitbwr ("C8","2347F",offset);
offset += 3;		/* Dec outer, dest update, load inner */
offset += blitbrd ("C0","30000",offset);
offset += blitbrd ("38","2357F",offset);
offset += blitbwr ("30","2357F",offset);
offset += 3;		/* Dec outer, dest update, load inner */
offset += blitbrd ("0B","2FFFF",offset);
offset += blitbrd ("00","2367F",offset);
offset += inhibwr (offset);
offset += 3;		/* Dec outer, dest update, load inner */
offset += blitbrd ("49","2FFFF",offset);
offset += blitbrd ("8F","2377F",offset);
offset += inhibwr (offset);
offset += 1;
offset += blitbrd ("00","30111",offset);

/* Now watch it hand the bus back */
setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,5);
checksig (hold, 'L',offset,1);
offset += 2;

/* This is a word block move followed by a word write to test the word mode
and the ability to continue onto another command */

offset += blitcmd ("21,56,9B,52,7E,34,12,60,C0,01,03,00,00","31216",offset);
bustim = offset;	/* store time to start driving bus ack */
offset += 1;		/* load inner count */

offset += blitwrd ("1234","29B56",offset);
offset += blitwwr ("1234","2347E",offset);
offset += blitwrd ("AA55","29B54",offset);
offset += blitwwr ("AA55","23480",offset);
offset += blitwrd ("EDDC","29B52",offset);
offset += blitwwr ("EDDC","23482",offset);
offset += 1;            /* dec outer count */
offset += blitbrd ("01","31222",offset);
offset += 1;            /* pass through stopped state */
offset += blitbrd ("00","31223",offset);
offset += blitbrd ("00","31224",offset);
offset += blitbrd ("00","31225",offset);
offset += blitbrd ("88","31226",offset);
offset += blitbrd ("17","31227",offset);
offset += blitbrd ("41","31228",offset);
offset += blitbrd ("60","31229",offset);
offset += blitbrd ("C0","3122A",offset);
offset += blitbrd ("01","3122B",offset);
offset += blitbrd ("08","3122C",offset);
offset += blitbrd ("00","3122D",offset);
offset += blitbrd ("C3","3122E",offset);
offset += 1;		/* load inner count */
offset += blitwwr ("C3C3","11788",offset);
offset += blitwwr ("C3C3","11786",offset);
offset += blitwwr ("C3C3","11784",offset);
offset += blitwwr ("C3C3","11782",offset);
offset += blitwwr ("C3C3","11780",offset);
offset += blitwwr ("C3C3","1177E",offset);
offset += blitwwr ("C3C3","1177C",offset);
offset += blitwwr ("C3C3","1177A",offset);

offset += 1;
offset += blitbrd ("00","3122F",offset);

/* Now watch it hand the bus back */
setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,5);
checksig (hold, 'L',offset,1);
offset += 2;

#endif

/****************************************************************************
Test the blitter in conjunction with the DSP.

****************************************************************************/
#if BDTEST

/* Blit the program into the DSP program RAM */

	{
	char dspaddr[6]; 
	static char *dspprog[] = {
			"00","F1",	/* NOP			*/
			"00","F1",	/* NOP			*/
			"00","F1",	/* NOP			*/
			"00","F1",	/* NOP			*/
			"00","F1",	/* NOP			*/
			"00","F1",	/* NOP			*/
			"00","F1",	/* NOP			*/
			"00","F1",	/* NOP			*/
			"00","F1",	/* NOP			*/
			"00","F1",	/* NOP			*/
			"81","31",	/* MOV DMA1, (181)	*/
			"00","F1",	/* NOP			*/
			"00","F1",	/* NOP			*/
			"00","F1",	/* NOP			*/
			"80","29",	/* MOV DMA0, (180)	*/
			"00","F1",	/* NOP			*/
			"00","F1",	/* NOP			*/
			"00","31",	/* MOV DMA1, (100)	*/
			"05","E9",	/* MOV PC,(105)		*/
			"00","F1"};	/* NOP			*/
	/* this inner count must match the size above ....VV */
	offset += blitcmd ("21,00,00,00,00,14,04,20,C0,01,28,00,00",
		"10000",offset);
 
	bustim = offset;	/* store time to start driving bus ack */
	offset += 1;		/* load inner count */
	/* this for loop must also match the size */
	for (n = 0; n < 20 * 2; n++)
		{
		sprintf (dspaddr, "%05x", 0x00000 + n);
		offset += blitbrd (dspprog[n],dspaddr,offset);
		sprintf (dspaddr, "%05x", 0x41400 + n);
		checkaddr (dspaddr,offset,2);
		setsig (ai2, '0', offset, 1); /* disable data in first tick */
		offset += 2;
		}

	offset += 1;
	offset += blitbrd ("00","1000C",offset);

	/* Now watch it hand the bus back */
	setsig (hlda, '1', bustim, offset - bustim + 1);
	checksig (hold, 'H',offset-5,5);
	checksig (hold, 'L',offset,1);
	offset += 2;
	}

/* Now set up some data */

/* Define the DMA mode bits with loc'n 181 */
/*  15   14   13   12   11   10   9    8    7   6   5   4   3   2   1   0   */
/*   x    x    x    x  hold read byte lohi  x   x   x   x  a19 a18 a17 a16  */
/*   0    0    0    0    1    1   0    0    0   0   0   0   0   0   1   0   */
offset = dspiwr ("302","0C02",offset);

/* Set up the low address word with loc'n 180 */
offset = dspiwr ("300","5555",offset);

/* Set up the blitter address */
offset += qiowrite ("00", BLITPC0, offset);
offset += qiowrite ("00", BLITPC1, offset);
offset += qiowrite ("00", BLITPC2, offset);

/* Force the program counter to zero */
offset += qmwrite ("00","41400",offset);

/* and set DSP, then blitter run */

offset += qmwrite ("10","41600",offset);
checksig (hold, 'L', offset-1, 1);
offset += qiowrite ("01", BLITCMD, offset);
checksig (hold, 'H', offset, 1);
offset += 1;
bustim = offset;              /* record the start of bus acknowledge */
 
offset += blitbrd ("00","00000",offset);
offset += blitbrd ("00","00001",offset);
offset += blitbrd ("00","00002",offset);
offset += blitbrd ("00","00003",offset);

/* in comes the DSP */

offset += 3;
offset += blitwrd ("1234","25555",offset);
offset += 1;

/* and back to blitting */

offset += blitbrd ("00","00004",offset);
offset += blitbrd ("00","00005",offset);
offset += blitbrd ("20","00006",offset);
offset += blitbrd ("C0","00007",offset);
offset += blitbrd ("01","00008",offset);
offset += blitbrd ("01","00009",offset);
offset += blitbrd ("00","0000A",offset);
offset += blitbrd ("00","0000B",offset);
offset += 1;		/* load inner count */

offset += blitbwr ("00","00000",offset);

offset += 1;
offset += blitbrd ("00","0000C",offset);

/* Now watch it hand the bus back */
setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,5);
checksig (hold, 'L',offset,1);
offset += 2;

/* stop the DSP */
offset += qmwrite ("00","41600",offset);

/* run the pervy DSP clock */
setsig (lpl, 'D', dspclktim, offset - dspclktim);

#endif
#if BCTEST

/* Test blitter inner counter */

offset += blitcmd ("01,00,00,00,00,00,00,A2,C0,01,00,00,CC","00000",offset);
bustim = offset;	/* store time to start driving bus ack */
offset += 1;		/* load inner count */

for (n = 0; n < 256; n++)
	{
	sprintf (ad,"%05x",n);
	offset += blitbwr ("CC",ad,offset);
	}
offset += 1;
offset += blitbrd ("00","0000C",offset);

setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,5);
checksig (hold, 'L',offset,1);
offset += 2;

/* Test blitter outer counter */

offset += blitcmd ("01,00,00,00,00,00,00,A0,C0,80,01,00,CC","00000",offset);
bustim = offset;	/* store time to start driving bus ack */
offset += 1;		/* load inner count */

for (n = 0; n < 128; n++)
	{
	sprintf (ad,"%05x",n);
	offset += blitbwr ("CC",ad,offset);
	offset += 2;	/* dec outer, load inner */
	}
offset -= 1;	/* adjust for 2 above to be dec outer only */
offset += blitbrd ("00","0000C",offset);

setsig (hlda, '1', bustim, offset - bustim + 1);
checksig (hold, 'H',offset-5,5);
checksig (hold, 'L',offset,1);
offset += 2;

#endif
#if TESTZ
offset += qiowrite("06",TESTREG,offset);
#endif
#if TESTS
offset += qiowrite("01",TESTREG,offset);
#endif
#if TESTL
offset += qiowrite("08",TESTREG,offset);
#endif
clock (clocktim, offset - clocktim);

sim_term (offset);

printf ("Test Pattern generated, run for %d\n",offset*CYCLE);
}
