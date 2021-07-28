#include <stdio.h>
#include <stdlib.h>
#include "simulator.h"
// return least recently used page to page out 
int get_lru_page(int timestamps[MAXPROCESSES][MAXPROCPAGES],
							Pentry q[MAXPROCESSES], int proc, int page, int tick) {
	int swap_page;
	int lru_time = tick;
	for (int lru_page = 0; lru_page < q[proc].npages; lru_page++) {
		if (lru_page != page && q[proc].pages[lru_page]) {
			if (timestamps[proc][lru_page] < lru_time) {
				lru_time = timestamps[proc][lru_page];
				swap_page = lru_page;
			}
		}
	}
	return swap_page;
}
// page in current page plus two subsequent pages
void pagein_subsequent(int timestamps[MAXPROCESSES][MAXPROCPAGES],
							Pentry q[MAXPROCESSES], int proc, int page, int tick) {
	for (int i = 0; i < 3; i++) {
		if(!q[proc].pages[page+i]) {
			if (!pagein(proc, page+i)) {
				int swap_page = get_lru_page(timestamps, q, proc, page+i, tick);
				if (swap_page < page || swap_page > page+2) {
					pageout(proc, swap_page);
				}
				else {
					for (int p=0; p<20;p++) {
						if ((p < page || p > page+2) && q[proc].pages[p]) {
							pageout(proc, p);
							break;
						}
					}
				}
			}
		}
	}
}
// page in predicted page and page succeeding predicted page
void pagein_predict(int timestamps[MAXPROCESSES][MAXPROCPAGES],
							Pentry q[MAXPROCESSES], int proc, int page, int next_page, int tick) {
	for (int i = 0; i < 2; i++) {
		if(!q[proc].pages[next_page+i]) {
			if (!pagein(proc, next_page+i)) {
				int swap_page = get_lru_page(timestamps, q, proc, next_page+i, tick);
				if (swap_page != next_page && swap_page != next_page+1 && swap_page != page) {
					pageout(proc, swap_page);
				}
				else {
					for (int p=0; p<20;p++) {
						if (p != next_page && p != next_page+1 && p != page) {
							if (q[proc].pages[p]) {
								pageout(proc, p);
								break;
							}
						}
					}
				}
			}
		}
	}
}
// page in current page only, lru to swap out
void pagein_curr_page(int timestamps[MAXPROCESSES][MAXPROCPAGES],
								Pentry q[MAXPROCESSES], int proc, int page, int next_page, int tick) {
	if (!pagein(proc, page)) {
		int swap_page = get_lru_page(timestamps, q, proc, page, tick);
		if (swap_page != page && swap_page != next_page+1  && swap_page != next_page) {
			pageout(proc, swap_page);
		}
		else {
			for (int p=0; p<20;p++) {
				if (p != page && p != next_page+1 && p != next_page) {
					if (q[proc].pages[p]) {
						pageout(proc, p);
						break;
					}
				}
			}
		}
	}
}
// increment value at index of current page
void pc_next(int pcnext[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES],
								int proc, int curr_page, int prev_page) {
	pcnext[proc][prev_page][curr_page]++;
}
// return page most likely needed next, index of max value in pcnext array
int max_pc_next(int pcnext[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES],
								int proc, int page) {
	int max = 0;
	int pge = -1;
	for (int i = 0; i < 20; i++) {
		if (max < pcnext[proc][page][i]) {
			max = pcnext[proc][page][i];
			pge = i;
		}
	}
	return pge;
}

void pageit(Pentry q[MAXPROCESSES]) {
	static int last_page[MAXPROCESSES];
  	static int initialized = 0;
  	static int tick = 1;
	static int timestamps[MAXPROCESSES][MAXPROCPAGES];
	static int pcnext[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES];

	int proctmp;
	int pagetmp;
	int page;
	int prev_page;
	// initialize timestamps, next page array, and previous page array
	if(!initialized){
		for(proctmp=0; proctmp < MAXPROCESSES; proctmp++) {
			last_page[proctmp] = 0;
			for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++) {
				timestamps[proctmp][pagetmp] = 0;
				for (int p=0;p<20;p++) {
					pcnext[proctmp][pagetmp][p] = 0;
				}
			}
		}
		initialized = 1;
	}
	for (int proc = 0; proc < MAXPROCESSES; proc++) {
		if (q[proc].active) {
			if (prev_page == -1) {
				continue;
			}
			//get current page and add timestamp
      		page = q[proc].pc/PAGESIZE;
			timestamps[proc][page] = tick;
			//save previous page, then update last page
			prev_page = last_page[proc]/PAGESIZE;
      		last_page[proc] = q[proc].pc;
        	// if prev page different from current page, update pc next array
			if (prev_page != page){
				pc_next(pcnext, proc, page, prev_page);
			}
			// next page predicted = max of pc next array for current proc/page
			int next_page = max_pc_next(pcnext, proc, page);
			// if next_page = -1, no entries in array yet for proc/page
			if (next_page != -1) {
				pagein_curr_page(timestamps, q, proc, page, next_page, tick);
				pagein_predict(timestamps, q, proc, page, next_page, tick);
			}
			// no entries yet, page in current page plus two subsequent
			else {
				pagein_subsequent(timestamps, q, proc, page, tick);
			}
		}
	}
  tick++;
}
