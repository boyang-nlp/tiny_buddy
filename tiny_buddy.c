/*Tiny Buddy System
 *Author : dingboyang
 *Date   : 2016.1.29
 */
#include <pmm.h>
#include "tiny_buddy.h"
Buddy *mem;					/*Buddy CBT*/
uint8_t MAX_ORD;			/*The maximum  order of memory block*/
bool mem_init;				/*The CBT initialized flag*/
unsigned int nr_free;		/*Free memory block*/
/* update - Update ancestors' allocable status when alloc or free a
 * block
 * i : Buddy index in mem;
 */

static void update(int i){
	while(i){
		i = par(i);
		set_lft(mem[i], allocable(mem[lc(i)]));
		set_rht(mem[i], allocable(mem[rc(i)]));
	}
}
/*get_ord - Get the order of a memory block with n bytes. 
 */
static uint8_t get_ord(size_t n){
	uint8_t o = 0;
	while( (MIN_BLOCK << o) < n)o++;
	return o + 2;
}
/*idx2ord - Get the order of mem[i].
 */
static uint8_t idx2ord(size_t i){
	uint8_t cnt = 0;
	while(i){
		i = par(i);
		cnt++;
	}
	return MAX_ORD - cnt;
}
/*buddy_fill - Flag all buddy as unallocatable which belong subtree i.
 *             Used to initialize memory.
 */
static void buddy_fill(uint32_t i){
	uint8_t o = idx2ord(i);
	uint32_t k,j;
	for(k = 0;k<o;k++,i<<=1)
		for(j = 0;j < (1 << k); j++)
			set_buddy(mem[i + j + (1 << k) - 1],o - k, 0, 0);
}
/*buddy_init - Buddy memory manager initialization*/
void buddy_init(){
	MAX_ORD = get_ord(KMEMSIZE);
	mem_init = 0;
	nr_free = 0;
}
/*buddy_init_memmap - Set the detected physical memory block allocable*/
static void buddy_init_memmap(struct Page* page, size_t n){
    assert(n > 0);
    if(!mem_init){
		mem = page2kva(page);
		uint32_t mem_sz = (shl(1, MAX_ORD-1)-1) * sizeof(Buddy);
		uint32_t pgnum = ROUNDUP(mem_sz, PGSIZE) / PGSIZE;
		page += pgnum;
		n -= pgnum;
		mem_init = 1;
		buddy_fill(0);
    }
    struct Page *p = page;
    nr_free += n;
    uint32_t idx;
    for (; p != page + n; p++) {
        assert(PageReserved(p));
        p->flags = p->property = 0;
		idx = page2buddy(p, 2);
		set_buddy(mem[idx], 2, 1, 1);
		update(idx);
        set_page_ref(p, 0);
    }
}
/*buddy_alloc_pages - alloc n page from free area and return the first page.
 *	                  The header page should set page property flag, and the
 *					  property field should assign to n which means next 
 *					  follow n pages has been allocated.
 */
struct Page* buddy_alloc_pages(size_t n){
	assert(n > 0);
	if(n > nr_free){
		return NULL;
	}
	struct Page* page = NULL;
	uint8_t o = get_ord(n * MIN_BLOCK);
	uint32_t i = 0;
	while(1){
		if( lft(mem[i]) >= o )
			i = lc(i);
		else if( rht(mem[i]) >= o )
			i = rc(i);
		else
			break;
	}	
	if( o == ord(mem[i]) ){
		set_rht(mem[i], 0);
		set_lft(mem[i], 0);
		page = buddy2page(mem[i], i);
		assert(!PageReserved(page));
		SetPageProperty(page);
		nr_free -= page->property = 1 << (ord(mem[i]) - 2);
		update(i);
	}
	return page;
}
/*buddy_free_pages - Free continous pages start with page.The header page's
					 property flag should be unset, and the property field 
					 should assign to zero.
 */
void buddy_free_pages(struct Page* page){
	assert(!PageReserved(page) && PageProperty(page));
	uint32_t i = page2buddy(page, get_ord(page->property * MIN_BLOCK));
	set_rht(mem[i], ord(mem[i])-1);
	set_lft(mem[i], ord(mem[i])-1);
	update(i);
	nr_free += page->property;
	ClearPageProperty(page);
	page->property = 0;
}
void __buddy_free_pages(struct Page* page, uint32_t n){
	buddy_free_pages(page);
}
static size_t
buddy_nr_free_pages(void) {
    return nr_free;
}

/*buddy_check - Validate the correctness of buddy system.
 */
static void buddy_check(void){
	assert((1 << (MAX_ORD-2)) >= KMEMSIZE/MIN_BLOCK && (1 << (MAX_ORD-3)) < KMEMSIZE/MIN_BLOCK);
	uint32_t backup = nr_free;
	struct Page* p1 = buddy_alloc_pages(1);
	assert(backup == nr_free + 1);
	assert(PageProperty(p1));
	assert(p1->property == 1);
	backup = nr_free;
	struct Page* p2 = buddy_alloc_pages(3);
	assert(backup == nr_free + 4);
	assert(PageProperty(p2));
	assert(p2->property == 4);
	backup = nr_free;
	buddy_free_pages(p1);
	assert(backup == nr_free - 1);
	assert(!PageProperty(p1));
	assert(p1->property == 0);
	backup = nr_free;
	buddy_free_pages(p2);
	assert(backup == nr_free - 4);
	backup  = nr_free;
	p1 = buddy_alloc_pages(53);
	assert(backup == nr_free + 64);
	assert(PageProperty(p1));
	assert(p1->property == 64);
	backup = nr_free;
	p2 = buddy_alloc_pages(896);
	assert(backup == nr_free + 1024);
	assert(PageProperty(p2));
	assert(p2->property == 1024);
	backup = nr_free;
	buddy_free_pages(p1);
	assert(backup == nr_free - 64);
	assert(!PageProperty(p1));
	assert(p1->property == 0);
	backup = nr_free;
	buddy_free_pages(p2);
	assert(backup == nr_free - 1024);
	assert(!PageProperty(p2));
	assert(p2->property == 0);
	
	
	backup = nr_free;
	p1 = buddy_alloc_pages((1<<13) - 13);
	assert(backup == nr_free + (1<<13));
	assert(PageProperty(p1));
	assert(p1->property == (1<<13));
	
	backup = nr_free;
	p2 = buddy_alloc_pages((1<<13) - 13);
	assert(backup == nr_free + (1<<13));
	assert(PageProperty(p2));
	assert(p1->property == (1<<13));

	struct Page *p3, *p4;
	backup = nr_free;
	p3 = buddy_alloc_pages((1<<12) - 12);
	assert(backup == nr_free + (1<<12));
	assert(PageProperty(p3));
	assert(p3->property == (1<<12));

	backup = nr_free;
	p4 = buddy_alloc_pages((1<<12) - 12);
	assert(backup == nr_free + (1<<12));
	assert(PageProperty(p4));
	assert(p4->property == (1<<12));
	
	backup = nr_free;
	buddy_free_pages(p1);
	assert(backup == nr_free - (1<<13));
	assert(!PageProperty(p1));
	assert(p1->property == 0);	
	
	backup = nr_free;	
	buddy_free_pages(p2);
	assert(backup == nr_free - (1<<13));
	assert(!PageProperty(p2));
	assert(p2->property == 0);	
	
	backup = nr_free;	
	buddy_free_pages(p3);
	assert(backup == nr_free - (1<<12));
	assert(!PageProperty(p3));
	assert(p3->property == 0);	

	backup = nr_free;	
	buddy_free_pages(p4);
	assert(backup == nr_free - (1<<12));
	assert(!PageProperty(p4));
	assert(p4->property == 0);
	
	struct Page* p[100];
	int i;
	for(i = 0;i<100;i++){
		p[i] = buddy_alloc_pages(1);
		assert(!PageReserved(p[i]));
		assert(PageProperty(p[i]));
		assert(p[i]->property == 1);
	}
	for(i = 0;i<100;i++){
		buddy_free_pages(p[i]);
		assert(!PageProperty(p[i]));
		assert(p[i]->property == 0);
	}
}

const struct pmm_manager tiny_buddy_pmm_manager = {
	.name = "tiny_buddy_pmm_manager",
	.init = buddy_init,
	.init_memmap = buddy_init_memmap,
	.alloc_pages = buddy_alloc_pages,
	.free_pages = __buddy_free_pages,
	.nr_free_pages = buddy_nr_free_pages,
	.check = buddy_check,
};
