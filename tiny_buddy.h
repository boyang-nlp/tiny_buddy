/*Tiny Buddy System.
 *Author : dingboyang
 *Date   : 2016.1.29
 */
#ifndef LAB2_KERN_MM_TINY_BUDDY_H_
#define LAB2_KERN_MM_TINY_BUDDY_H_
typedef uint16_t Buddy;

/****************************************************************************
 *Buddy Data Type is an abstract of memory.
 *We organize whole memory space as complete binary tree(CBT), every node of it is
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
#define        MIN_AOFF        12//
#define        MIN_BLOCK       shl(1, MIN_AOFF)//

#define        shr(e, i)       ( (e) >> (i) )
#define        shl(e, i)       ( (e) << (i) )
#define         or(a, b)       ( (a) |  (b) )
#define        and(a, b)       ( (a) &  (b) )
#define        not(e)          (~(e))
#define        _max(a, b)       ( (a) > (b) ? (a) : (b))
#define	       set(e, mask, v) ( (e) = or( and(e, not(mask)), v ) )
/*Get the order,left,right field of buddy*/
#define        ord(buddy)     and( shr(buddy, 10), 0x001f )
#define        lft(buddy)     and( shr(buddy,  5), 0x001f )
#define        rht(buddy)     and( buddy, 0x001f )

/*Set right,left or order field of buddy*/
#define	   set_ord(buddy, o)  ( set( buddy, shl( 0x1f, 10 ), shl( o, 10 ) ) )
#define    set_lft(buddy, l)  ( set( buddy, shl( 0x1f,  5 ), shl( l,  5 ) ) ) 

#define    set_rht(buddy, r)  ( set( buddy, 0x1f, r ) )

#define  set_buddy(buddy, o, l, r)\
							  ( set_ord(buddy, o), set_lft(buddy, l), set_rht(buddy, r))

/*Get a buddy's right,left child index*/
#define         rc(i)         shl( i + 1, 1)
#define         lc(i)         ( rc(i) - 1 )
/*Get a parent,sibling index of a buddy*/
#define        par(i)         shr(i-1, 1)
#define        sib(i)         ( ( i % 2 ) ? ( i+1 ) : ( i-1 ) )
/*Judge if a buddy is totally free*/
#define 	 free(buddy)	  ( ( lft(buddy) == rht(buddy) ) && ( rht(buddy) == (ord(buddy)-1) ) )
/*Judge if a buddy could still alloc a memory chunk*/
#define  allocable(buddy)     ( free(buddy) ? ord(buddy) : _max( lft(buddy), rht(buddy) ) )

/*Get the buddy's start physical address*/
#define buddy2pa(buddy, i)    ( shl(and( i + 1 , shl(1, MAX_ORD - ord(buddy)) - 1 ) , ord( buddy ) - 2 +MIN_AOFF) )
/*Get the buddy's start physical address, and convert it to
 *page struct
 */
#define buddy2page(buddy, i)  pa2page( buddy2pa( buddy, i ) )
/*Get the corresponding buddy index in mem of a memory with the 
  start physical pa and the order o.*/
#define pa2buddy(pa, o)       ( shr( pa,  (o) - 2 + MIN_AOFF ) + shl( 1, MAX_ORD - (o) ) - 1 )
/*Get the corresponding buddy index in mem of a memory block  
  with order o and start with page.*/
#define page2buddy(page, o)   ( pa2buddy( page2pa( page ), o ) )

extern const struct pmm_manager tiny_buddy_pmm_manager;
#endif /* LAB2_KERN_MM_TINY_BUDDY_H_ */
