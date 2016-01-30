/*
 * tiny_buddy.h
 *
 *  Created on: 2016年1月29日
 *      Author: lenovo8
 */

#ifndef LAB2_KERN_MM_TINY_BUDDY_H_
#define LAB2_KERN_MM_TINY_BUDDY_H_
typedef uint16_t Buddy;
/****************************************************************************
 *Buddy Data Type as an abstract of memory.
 *We organize whole memory space as complete binary tree, every node of it is
 *a buddy. Buddy is a memory chunk with the size of power of 2.
 *The data type Buddy is actually an unsigned short interger with three field
 *represented as follow:
 *+-1 bit-+-----5 bits-----+-----5 bits-----+-----5 bits-----+
 *| unused|      order     |      left      |      right     |
 *+-------+----------------+----------------+----------------+
 *order : The size of this buddy is 2 ^ (order-1) * MIN_BLOCK
 * left : The max allocable buddy in left subtree is 2^(left-1) * MIN_BLOCK
 *right : The max allocable buddy in right subtree is 2^(right-1) * MIN_BLOCK
 *If the left or right field is 0, means there is no memory chunk allocable.
 ****************************************************************************/

/*Minium memory block is 4KB(1 PAGE)*/
#define        MIN_AOFF       12
#define        MIN_BLOCK      shl(1, MIN_AOFF)

#define        shr(e, i)      ( (e) >> (i) )
#define        shl(e, i)      ( (e) << (i) )
#define         or(a, b)      ( (a) |  (b) )
#define        and(a, b)      ( (a) &  (b) )
#define        not(e)         (~(e))

/*Get the order,left,right field of buddy*/
#define        ord(buddy)     and( shr(buddy, 10), 0x001f )
#define        lft(buddy)     and( shr(buddy,  5), 0x001f )
#define        rht(buddy)     and( buddy, 0x001f )

/*Set right,left or order field of buddy*/
#define    set_rht(buddy, i)  ( buddy = or( and(buddy, not(0x03e0) ), shl( and(i, 0x1f), 5) ) )

#define    set_lft(buddy, i)  ( buddy = or( and(buddy, not(0x001f) ), and( i, 0x1f) ) )

#define  set_buddy(buddy, ord, lft, rht)\
							  ( set_rht(buddy, rht), set_lft(buddy, lft), \
							    buddy = or( and(buddy, not(0x7c00) ), shl( and(ord, 0x1f), 10) ) )

/*Get a buddy's right,left child index*/
#define         rc(i)         shr( i + 1, 1)
#define         lc(i)         ( rc(i) - 1 )
/*Get a parent,sibling index of a buddy*/
#define        par(i)         shr(i-1, 1)
#define        sib(i)         ( ( i % 2 ) ? ( i-1 ) : ( i+1 ) )
/*Judge if a buddy is totally free*/
#define 	 free(buddy)	  ( lft(buddy) == rht(buddy) == (ord(buddy)-1) )
/*Judge if a buddy could still alloc a chunk*/
#define  allocable(buddy)     ( free(buddy) ? order(buddy) : max( left(buddy), right(buddy) ) )

/*Get the address prefix of a buddy*/
#define     prefix(buddy)     ( MAX_ORD - order(buddy) )
/*Convert buddy to physical address or page*/
#define   buddy2pa(buddy, i)  ( ( ( ( 1 << prefix(buddy) ) - 1) & (i + 1) ) << (32 - prefix(buddy) - 1) )
#define buddy2page(buddy, i)  pa2page( buddy2pa( buddy, i ) )
/*Convert physical address or page to buddy index in mem*/
#define   pa2buddy(pa, ord)   ( ( shr( pa, ord +  MIN_AOFF - 2) + 1 << ( MAX_ORD - ord ) ) - 1 )//
#define page2buddy(page, ord) pa2buddy( page2pa( page ), ord )

#endif /* LAB2_KERN_MM_TINY_BUDDY_H_ */
