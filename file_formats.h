/* 
 * file_formats.h -- Which file formats are supported
 * 
 * Author:	Shawn Ostermann
 * 		Computer Science Department
 * 		Ohio University
 * Date:	Tue Nov  1, 1994
 *
 * Copyright (c) 1994 Shawn Ostermann
 */

/**************************************************************/
/**                                                          **/
/**  Input File Specific Stuff                               **/
/**                                                          **/
/**************************************************************/

struct supported_formats {
    int	(*(*test_func)())(void);/* pointer to the tester function	*/
    char	*format_name;	/* name of the file format		*/
    char	*format_descr;	/* description of the file format	*/
};

/* for each file type GLORP you want to support, provide a      	*/
/* function is_GLORP() that returns NULL if the stdin file is NOT	*/
/* of type GLORP, and returns a pointer to a packet reading routine	*/
/* if it is.  The packet reading routine is of the following type:	*/
/*	int pread_GLORP(						*/
/*	    struct timeval	*ptime,					*/
/*	    int		 	*plen,					*/
/*	    int		 	*ptlen,					*/
/*	    void		**pphys,				*/
/*	    int			*pphystype,				*/
/*	    struct ip		**ppip)					*/
/*   the reader function should return 0 at EOF and 1 otherwise		*/
/* This routine must return ONLY IP packets, but they need not all be	*/
/* TCP packets (if not, they're ignored).				*/


/* give the prototypes for the is_GLORP() routines supported */
#ifdef GROK_SNOOP
	int (*is_snoop(void))();
#endif GROK_SNOOP
#ifdef GROK_NETM
	int (*is_netm(void))();
#endif GROK_NETM
#ifdef GROK_TCPDUMP
	int (*is_tcpdump(void))();
#endif GROK_TCPDUMP
#ifdef GROK_ETHERPEEK
	int (*is_EP(void))();
#endif GROK_ETHERPEEK


/* install the is_GLORP() routines supported */
struct supported_formats file_formats[] = {
#ifdef GROK_TCPDUMP
	{is_tcpdump,	"tcpdump","tcpdump -- Public domain program from LBL"},
#endif GROK_TCPDUMP
#ifdef GROK_SNOOP
	{is_snoop,	"snoop","Sun Snoop -- Distributed with Solaris"},
#endif GROK_SNOOP
#ifdef GROK_ETHERPEEK
	{is_EP,		"etherpeek", "etherpeek -- Mac sniffer program"},
#endif GROK_ETHERPEEK
#ifdef GROK_NETM
	{is_netm,	"netmetrix","Net Metrix -- Commercial program from HP"},
#endif GROK_NETM
	{NULL,NULL},	/* You must NOT remove this entry */
};
