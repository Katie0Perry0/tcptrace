/* 
 * tcptrace.c - turn NETM protocol monitor traces into xplot
 * 
 * Author:	Shawn Ostermann
 * 		Dept. of Computer Sciences
 * 		Purdue University
 * Date:	Fri Sep  4 13:35:42 1992
 *
 * Copyright (c) 1992 Shawn Ostermann
 */

#include "tcptrace.h"
#include "gcache.h"
#include <math.h>



int
SynCount(ptp)
     tcp_pair *ptp;
{
	tcb *pab = &ptp->a2b;
	tcb *pba = &ptp->b2a;

	return(((pab->syn_count >= 1)?1:0) +
	       ((pba->syn_count >= 1)?1:0));
}



int
FinCount(ptp)
     tcp_pair *ptp;
{
	tcb *pab = &ptp->a2b;
	tcb *pba = &ptp->b2a;

	return(((pab->fin_count >= 1)?1:0) +
	       ((pba->fin_count >= 1)?1:0));
}


int
Complete(ptp)
     tcp_pair *ptp;
{
	return(SynCount(ptp) >= 2 && FinCount(ptp) >= 2);
}


static double
Average(
    double sum,
    int count
    )
{
    return((double) sum / ((double)count+.0001));
}



static double
Stdev(
    double sum,
    double sum2,
    int n
    )
{
    double term;
    double term1;
    double term2;

    if (n<=2)
	return(0.0);

    term1 = sum2;
    term2 = (sum * sum) / (double)n;
    term = term1-term2;
    term /= (double)(n-1);
    return(sqrt(term));
}




void
PrintBrief(ptp)
        tcp_pair *ptp;
{
	tcb *pab = &ptp->a2b;
	tcb *pba = &ptp->b2a;

	fprintf(stdout,"%s <==> %s", ptp->a_endpoint, ptp->b_endpoint);
	fprintf(stdout,"  %s2%s:%d",
		pab->host_letter,
		pba->host_letter,
		pab->packets);
	fprintf(stdout,"  %s2%s:%d",
		pba->host_letter,
		pab->host_letter,
		pba->packets);
	if (Complete(ptp))
	    fprintf(stdout,"  (complete)");
	fprintf(stdout,"\n");
}


void
PrintTrace(ptp)
     tcp_pair *ptp;
{
        unsigned long etime;
	float etime_float;
	tcb *pab = &ptp->a2b;
	tcb *pba = &ptp->b2a;
	char *host1 = pab->host_letter;
	char *host2 = pba->host_letter;

	fprintf(stdout,"\thost %s:        %s\n", host1, ptp->a_endpoint);
	fprintf(stdout,"\thost %s:        %s\n", host2, ptp->b_endpoint);
	fprintf(stdout,"\tcomplete conn: %s",
		Complete(ptp)?"yes":"no");
	if (Complete(ptp))
	    fprintf(stdout,"\n");
	else
	    fprintf(stdout,"\t(SYNs: %d)  (FINs: %d)\n",
		    SynCount(ptp), FinCount(ptp));

	fprintf(stdout,"\tfirst packet:  %s\n", ts(&ptp->first_time));
	fprintf(stdout,"\tlast packet:   %s\n", ts(&ptp->last_time));

	etime = elapsed(ptp->first_time,ptp->last_time);
	fprintf(stdout,"\telapsed time:  %d:%02d:%02d.%03u\n",
		(etime / 1000000) / (60 * 24),
		(etime / 1000000) % (60 * 24) / 60,
		((etime / 1000000) % (60 * 24)) % 60,
		(etime % 1000000) / 1000);
	fprintf(stdout,"\ttotal packets: %u\n", ptp->packets);
	

	fprintf(stdout,"    %s->%s:\t\t\t    %s->%s:\n",
		host1,host2,host2,host1);

	fprintf(stdout,
		"\tdata packets:  %8u\t\tdata packets:  %8u\n",
		pab->data_pkts,
		pba->data_pkts);
	fprintf(stdout,
		"\tdata bytes:    %8u\t\tdata bytes:    %8u\n",
		pab->data_bytes,
		pba->data_bytes);
	fprintf(stdout,
		"\trexmt packets: %8u\t\trexmt packets: %8u\n",
		pab->rexmit_pkts,
		pba->rexmit_pkts);
	fprintf(stdout,
		"\trexmt bytes:   %8u\t\trexmt bytes:   %8u\n",
		pab->rexmit_bytes,
		pba->rexmit_bytes);
	fprintf(stdout,
		"\tack pkts sent: %8u\t\tack pkts sent: %8u\n",
		pab->ack_pkts,
		pba->ack_pkts);
	fprintf(stdout,
		"\tmax window:    %8u\t\tmax window:    %8u\n",
		pab->win_max,
		pba->win_max);
	fprintf(stdout,
		"\tavg window:    %8u\t\tavg window:    %8u\n",
		pba->ack_pkts==0?0:pab->win_tot/pba->ack_pkts,
		pab->ack_pkts==0?0:pba->win_tot/pab->ack_pkts);
	fprintf(stdout,
		"\tzero windows:  %8u\t\tzero windows:  %8u\n",
		pab->win_zero_ct,
		pba->win_zero_ct);

	fprintf(stdout,
		"\ttotal packets: %8u\t\ttotal packets: %8u\n",
		pab->packets,
		pba->packets);
	etime_float = (float) etime / 1000000.0;
	fprintf(stdout,
		"\tthroughput:    %8.0f Bps\tthroughput:    %8.0f Bps\n",
		((float) pab->data_bytes / etime_float),
		((float) pba->data_bytes / etime_float));

	if (dortt) {
	    fprintf(stdout,"\n");
	    fprintf(stdout,
		    "\tRTT samples:   %8u\t\tRTT samples:   %8u\n",
		    pab->rtt_count, pba->rtt_count);
	    fprintf(stdout,
		    "\tRTT min:       %8.1f ms\tRTT min:       %8.1f ms\n",
		    (double)pab->rtt_min/1000.0, (double)pba->rtt_min/1000.0);
	    fprintf(stdout,
		    "\tRTT max:       %8.1f ms\tRTT max:       %8.1f ms\n",
		    (double)pab->rtt_max/1000.0, (double)pba->rtt_max/1000.0);
	    fprintf(stdout,
		    "\tRTT avg:       %8.1f ms\tRTT avg:       %8.1f ms\n",
		    Average(pab->rtt_sum, pab->rtt_count) / 1000.0,
		    Average(pba->rtt_sum, pba->rtt_count) / 1000.0);
	    fprintf(stdout,
		    "\tRTT stdev:     %8.1f ms\tRTT stdev:     %8.1f ms\n",
		    Stdev(pab->rtt_sum, pab->rtt_sum2, pab->rtt_count) / 1000.0,
		    Stdev(pba->rtt_sum, pba->rtt_sum2, pba->rtt_count) / 1000.0);

	    fprintf(stdout, "\
\t  For the following 5 RTT statistics, only ACKs for
\t  multiply-transmitted segments (ambiguous ACKs) were
\t  considered.  Times are taken from the last instance
\t  of a segment.
");
	    fprintf(stdout,
		    "\tambiguous acks: %7u\t\tambiguous acks: %7u\n",
		    pab->rtt_amback, pba->rtt_amback);
	    fprintf(stdout,
		    "\tRTT min (last): %7.1f ms\tRTT min (last): %7.1f ms\n",
		    (double)pab->rtt_min_last/1000.0,
		    (double)pba->rtt_min_last/1000.0);
	    fprintf(stdout,
		    "\tRTT max (last): %7.1f ms\tRTT max (last): %7.1f ms\n",
		    (double)pab->rtt_max_last/1000.0,
		    (double)pba->rtt_max_last/1000.0);
	    fprintf(stdout,
		    "\tRTT avg (last): %7.1f ms\tRTT avg (last): %7.1f ms\n",
		    Average(pab->rtt_sum_last, pab->rtt_count_last) / 1000.0,
		    Average(pba->rtt_sum_last, pba->rtt_count_last) / 1000.0);
	    fprintf(stdout,
		    "\tRTT sdv (last): %7.1f ms\tRTT sdv (last): %7.1f ms\n",
		    Stdev(pab->rtt_sum_last, pab->rtt_sum2_last, pab->rtt_count_last) / 1000.0,
		    Stdev(pba->rtt_sum_last, pba->rtt_sum2_last, pba->rtt_count_last) / 1000.0);

	    fprintf(stdout,
		    "\tsegs cum acked: %7u\t\tsegs cum acked: %7u\n",
		    pab->rtt_cumack, pba->rtt_cumack);
	    fprintf(stdout,
		    "\tredundant acks: %7u\t\tredundant acks: %7u\n",
		    pab->rtt_unkack, pba->rtt_unkack);
	    if (debug)
	    fprintf(stdout,
		    "\tunknown acks:  %8u\t\tunknown acks:  %8u\n",
		    pab->rtt_unkack, pba->rtt_unkack);
	    fprintf(stdout,
		    "\tmax # retrans: %8u\t\tmax # retrans: %8u\n",
		    pab->retr_max, pba->retr_max);
	    fprintf(stdout,
		    "\tmin retr time: %8.1f ms\tmin retr time: %8.1f ms\n",
		    (double)pab->retr_min_tm/1000.0,
		    (double)pba->retr_min_tm/1000.0);
	    fprintf(stdout,
		    "\tmax retr time: %8.1f ms\tmax retr time: %8.1f ms\n",
		    (double)pab->retr_max_tm/1000.0,
		    (double)pba->retr_max_tm/1000.0);
	    fprintf(stdout,
		    "\tavg retr time: %8.1f ms\tavg retr time: %8.1f ms\n",
		    Average(pab->retr_tm_sum, pab->retr_tm_count) / 1000.0,
		    Average(pba->retr_tm_sum, pba->retr_tm_count) / 1000.0);
	    fprintf(stdout,
		    "\tsdv retr time: %8.1f ms\tsdv retr time: %8.1f ms\n",
		    Stdev(pab->retr_tm_sum, pab->retr_tm_sum2, pab->retr_tm_count) / 1000.0,
		    Stdev(pba->retr_tm_sum, pba->retr_tm_sum2, pba->retr_tm_count) / 1000.0);
	}
}