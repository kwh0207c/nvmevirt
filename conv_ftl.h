// SPDX-License-Identifier: GPL-2.0-only

#ifndef _NVMEVIRT_CONV_FTL_H
#define _NVMEVIRT_CONV_FTL_H

#include <linux/ktime.h>
#include <linux/types.h>
#include "pqueue/pqueue.h"
#include "ssd_config.h"
#include "ssd.h"

/* GC Settings */
#define GC_MODE_GREEDY 1
#define GC_MODE_CB 2
#define GC_MODE_RANDOM 3
#define GC_MODE_FIFO 4

#define GC_MODE (GC_MODE_CB)

#define SLC_GC_THRESHOLD 5
#define SLC_GC_THRESHOLD_HIGH 10
#define GC_THRESHOLD 240
#define GC_THRESHOLD_HIGH 240


struct convparams {
	uint32_t gc_thres_lines;
	uint32_t gc_thres_lines_high;
	bool enable_gc_delay;

	double op_area_pcent;
	int pba_pcent; /* (physical space / logical space) * 100*/

	/* SLCB */
	uint32_t slc_gc_thres_lines;
	uint32_t slc_gc_thres_lines_high;
};

struct line {
	int id; /* line id, the same as corresponding block id */
	int ipc; /* invalid page count in this line */
	int vpc; /* valid page count in this line */
	struct list_head entry;
	/* position in the priority queue for victim lines */
	size_t pos;
	ktime_t last_update;
	bool is_slc;
};

/* wp: record next write addr */
struct write_pointer {
	struct line *curline;
	uint32_t ch;
	uint32_t lun;
	uint32_t pg;
	uint32_t blk;
	uint32_t pl;
};

struct line_mgmt {
    struct line *lines;

    /* free line list, we only need to maintain a list of blk numbers */
    /* SLC */
    struct list_head slc_free_line_list;
	uint32_t slc_free_line_cnt;

    /* TLC */
    struct list_head free_line_list;
    struct list_head full_line_list;
	uint32_t free_line_cnt;
    uint32_t full_line_cnt;

    uint32_t tt_lines;

	#if (GC_MODE == GC_MODE_GREEDY)
    pqueue_t *victim_line_pq;

	#else
	struct list_head slc_victim_line_list;
    struct list_head victim_line_list;
	
	#endif

	uint32_t slc_victim_line_cnt;
    uint32_t victim_line_cnt;
};

struct write_flow_control {
	uint32_t write_credits;
	uint32_t credits_to_refill;
};

struct conv_ftl {
	struct ssd *ssd;

	struct convparams cp;
	struct ppa *maptbl; /* page level mapping table */
	uint64_t *rmap; /* reverse mapptbl, assume it's stored in OOB */
	struct write_pointer wp;
	struct write_pointer gc_wp;
	struct line_mgmt lm;
	struct write_flow_control wfc;

	/* Debug */
	uint32_t svl_called_count;
};

void conv_init_namespace(struct nvmev_ns *ns, uint32_t id, uint64_t size, void *mapped_addr,
			 uint32_t cpu_nr_dispatcher);

void conv_remove_namespace(struct nvmev_ns *ns);

bool conv_proc_nvme_io_cmd(struct nvmev_ns *ns, struct nvmev_request *req,
			   struct nvmev_result *ret);

#endif