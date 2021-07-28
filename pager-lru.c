#include <stdio.h>
#include <stdlib.h>

#include "simulator.h"

void pageit(Pentry q[MAXPROCESSES]) {

    /* This file contains the stub for an LRU pager */
    /* You may need to add/remove/modify any part of this file */

    /* Static vars */
  static int initialized = 0;
  static int tick = 1; // artificial time
  static int timestamps[MAXPROCESSES][MAXPROCPAGES];

  /* Local vars */
  int proctmp;
  int pagetmp;
	int pc;
	int page;
	int lru_time;
	int swap_page;

  /* initialize static vars on first run */
  if(!initialized){
  	for(proctmp=0; proctmp < MAXPROCESSES; proctmp++) {
    	for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++) {
	    	timestamps[proctmp][pagetmp] = 0;
			}
		}
		initialized = 1;
	}
  for (int proc = 0; proc < MAXPROCESSES; proc++) {
  	if (q[proc].active) {
			pc = q[proc].pc;
      page = pc/PAGESIZE;
			timestamps[proc][page] = tick;
			// if (proc == 1) {
			// 	printf("for loop entered - page: %d \n",page);
			// }
			if(!q[proc].pages[page]) {
				// if (proc == 1){
				// 	printf("page not in - page: %d \n",page);
				// }
				if (!pagein(proc, page)) {
					// if (proc == 1){
					// 	printf("pages need to be swapped - page: %d \n",page);
					// }
					lru_time = tick;
					for (int lru_page = 0; lru_page < q[proc].npages; lru_page++) {
						if (lru_page != page && q[proc].pages[lru_page]) {
							if (timestamps[proc][lru_page] < lru_time) {
								lru_time = timestamps[proc][lru_page];
								swap_page = lru_page;
							}
						}
					}
					// if (proc == 1){
					// 	printf("swapping - process: %d ",proc);
					// 	printf("page in: %d ",page);
					// 	printf("page out: %d\n",swap_page);
					// }
					pageout(proc, swap_page);

				}
			}
  	}
	}
  /* TODO: Implement LRU Paging
  fprintf(stderr, "pager-lru not yet implemented. Exiting...\n");
  exit(EXIT_FAILURE);

  advance time for next pageit iteration */
  tick++;
}
