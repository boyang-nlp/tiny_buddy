/*
 * tiny_buddy.c
 *
 *  Created on: 2016年1月29日
 *      Author: boyang
 */
#include <pmm.h>
#include <string.h>
#include "tiny_buddy.h"
Buddy *mem;
uint8_t MAX_ORD;
unsigned int nr_free;

static void update(Buddy* mem, int i){
	while(i){
		if(i % 2){
			if(free(mem[i]) && free(mem[sib(i)]))
				set_left( mem[par(i)], ord( mem[par(i)]) - 1 );
			else
				set_left( mem[par(i)], max( allocable(mem[i]), allocable(mem[sib(i)])) );
		}
		else{
			if(free(mem[i]) && free(mem[sib(i)]))
				set_right( mem[par(i)], ord(mem[par(i)]) - 1 );
			else
				set_right( mem[par(i)], max( allocable(mem[i]), allocable(mem[sib(i)]) ) );
		}
		i = par(i);
	}
}
static uint8_t get_ord(size_t n){//MIN_BLOCK << (_ord - 1) >= n
	uint8_t _ord = 0;
	while( (MIN_BLOCK<<_ord) < n)_ord++;
	return _ord;
}
static void buddy_fill(uint32_t i){
	uint8_t _ord = ord(mem[i]);
	uint32_t k,j;
	for(k = 0;k<_ord;k++)
		for(j = 0;j < (1 << k); j++)
			set_buddy(mem[i + j + (1 << k) - 1],_ord, 0, 0);
}
static void buddy_clean(uint32_t i){
	uint8_t _ord = ord(mem[i]);
	uint32_t k,j;
	for(k = 0;k<_ord;k++)
		for(j = 0;j < (1<<k); j++)
			set_buddy(mem[i + j + (1<<k) - 1],_ord, _ord - 1, _ord - 1);
}
void buddy_init(){
	MAX_ORD = get_ord(KMEMSIZE);
	nr_free = 0;
	buddy_fill(0);//unusable
}
static void buddy_init_memmap(struct Page* page, size_t n){
    assert(n > 0);
    struct Page *p = page;
    for (; p != page + n; p ++) {
        assert(PageReserved(p));
        p->flags = p->property = 0;
        set_page_ref(p, 0);
    }
	uint8_t _ord = get_ord(n << MIN_AOFF);
	do{
		if( ( 1 << ( _ord - 1 ) ) & n ){
			buddy_clean(page2buddy(page, _ord));
			page += ( 1 << (_ord - 1) ) & n;
		}
	}while(--_ord > 0);
}
struct Page* buddy_alloc_pages(size_t n){
	struct Page* page = NULL;
	uint8_t _ord = get_ord(n << MIN_AOFF);
	uint32_t i = 0;
	while(1){
		if( left(mem[i]) >= _ord )
			i = lc(i);
		else if( right(mem[i]) >= _ord )
			i = rc(i);
		else
			break;
	}
	if( _ord == order(mem[i]) ){
		set_rht(mem[i], 0);
		set_lft(mem[i], 0);
		page = buddy2page(mem[i], i);
		SetPageProperty(page);
		nr_free -= page->property = 1 << (order(mem[i]) - 1);
		update(mem, i);
	}
	return page;
}
void buddy_free_pages(struct Page* page){
	uint32_t i = page2buddy(page, get_order(page->property * MIN_BLOCK));
	set_rht(mem[i], order(mem[i])-1);
	set_lft(mem[i], order(mem[i])-1);
	update(mem, i);
	nr_free += page->property;
	ClearPageProperty(page);
	page->property = 0;
}

static size_t
buddy_nr_free_pages(void) {
    return nr_free;
}

static void basic_check(void){
	//
}

static void buddy_check(void){
	//
}

const struct pmm_manager tiny_buddy_pmm_manager = {
	.name = "tiny_buddy_pmm_manager",
	.init = buddy_init,
	.init_memmap = buddy_init_memmap,
	.alloc_pages = buddy_alloc_pages,
	.free_pages = buddy_free_pages,
	.nr_free_pages = buddy_nr_free_pages,
	.check = buddy_check,
};
