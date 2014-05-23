/*	SCCS Id: @(#)rnd.c	3.4	1996/02/07	*/
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* "Rand()"s definition is determined by [OS]conf.h */
#if defined(LINT) && defined(UNIX)	/* rand() is long... */
extern int NDECL(rand);
#define RND(x)	(rand() % x)
#else /* LINT */
# if defined(UNIX) || defined(RANDOM)
#define RND(x)	(int)(Rand() % (long)(x))
# else
/* Good luck: the bottom order bits are cyclic. */
#define RND(x)	(int)((Rand()>>3) % (x))
# endif
#endif /* LINT */

#ifdef OVL0

int
rn2(x)		/* 0 <= rn2(x) < x */
register int x;
{
	if(x <= 0) x = 1; //I prefer this behavior -CM
#ifdef DEBUG
	if (x <= 0) {
		impossible("rn2(%d) attempted", x);
		return(0);
	}
	x = RND(x);
	return(x);
#else
	return(RND(x));
#endif
}

#endif /* OVL0 */
#ifdef OVLB

int
rnl(x)		/* 0 <= rnl(x) < x; sometimes subtracting Luck */
register int x;	/* good luck approaches 0, bad luck approaches (x-1) */
{
	register int i;

#ifdef DEBUG
	if (x <= 0) {
		impossible("rnl(%d) attempted", x);
		return(0);
	}
#endif
	i = RND(x);

	if (Luck && rn2(50 - Luck)) {
	    i -= (x <= 15 && Luck >= -5 ? Luck/3 : Luck);
	    if (i < 0) i = 0;
	    else if (i >= x) i = x-1;
	}

	return i;
}

#endif /* OVLB */
#ifdef OVL0

int
rnd(x)		/* 1 <= rnd(x) <= x */
register int x;
{
	if(x<=0) x=1; //fixes a crash from feeding rnd a negative number.  I'd rather have this behavior.
#ifdef DEBUG
	if (x <= 0) {
		impossible("rnd(%d) attempted", x);
		return(1);
	}
	x = RND(x)+1;
	return(x);
#else
	return(RND(x)+1);
#endif
}

#endif /* OVL0 */
#ifdef OVL1

int
d(n,x)		/* n <= d(n,x) <= (n*x) */
register int n, x;
{
	register int tmp = n;

#ifdef DEBUG
	if (x < 0 || n < 0 || (x == 0 && n != 0)) {
		impossible("d(%d,%d) attempted", n, x);
		return(1);
	}
#endif
	while(n--) tmp += RND(x);
	return(tmp); /* Alea iacta est. -- J.C. */
}

int
exploding_d(n,x,m)
register int n, x, m;
{
	register int tmp=0, cur;
	if(x < 2) return d(n,x) + m*n; //A die of size 1 or 0 would always explode.
#ifdef DEBUG
	if (x < 0 || n < 0 || (x == 0 && n != 0)) {
		impossible("exploding d(%d,%d) attempted", n, x);
		return(1);
	}
#endif
	while(n--){
		cur = RND(x) + 1;
		while(cur == x){
			tmp += cur+m;
			cur = RND(x) + 1;
		}
		tmp += cur+m;
	}
	return(tmp); /* How do you get the average of an exploding die? 
				 If you ask a friend who's good at math, 
				 he'll tell you a long explanation about Sequences and Limits, 
				 of which you'll forget half. 
				 What you'll remember is the conclusion: 
				 
				 The average of an exploding Dn tends to the average of a 
				 regular Dn (which is (n+1)/2) multiplied by n/(n-1)

				 -1d4chan wiki entry on exploding dice, retrieved 2/13/2012 
				 
				 .:, the average of this function tends to n*( ((x+1)/2 + m) * x/(x-1) ) 
					The minimum value is n + n*m, and the maximum is unbounded
					-CM */
}

int
lucky_exploding_d(n,x,m)
register int n, x, m;
{
	register int tmp=0, cur;
	if(x < 2) return d(n,x) + m*n; //A die of size 1 or 0 would always explode.
#ifdef DEBUG
	if (x < 0 || n < 0 || (x == 0 && n != 0)) {
		impossible("exploding d(%d,%d) attempted", n, x);
		return(1);
	}
#endif
	while(n--){
		cur = x-rnl(x);
//		pline("damage %d",cur);
		while(cur == x){
			tmp += cur+m;
			cur = x-rnl(x);
//			pline("looping %d",cur);
		}
		tmp += cur+m;
	}
//	pline("damage total %d",tmp);
	return(tmp);
}

int
unlucky_exploding_d(n,x,m)
register int n, x, m;
{
	register int tmp=0, cur;
	if(x < 2) return d(n,x) + m*n; //A die of size 1 or 0 would always explode.
#ifdef DEBUG
	if (x < 0 || n < 0 || (x == 0 && n != 0)) {
		impossible("exploding d(%d,%d) attempted", n, x);
		return(1);
	}
#endif
	while(n--){
		cur = rnl(x);
//		pline("damage %d",cur);
		while(cur == x){
			tmp += cur+m;
			cur = rnl(x);
//			pline("looping %d",cur);
		}
		tmp += cur+m;
	}
//	pline("damage total %d",tmp);
	return(tmp);
}

#endif /* OVL1 */
#ifdef OVLB

int
rne(x)
register int x;
{
	register int tmp, utmp;

	utmp = (u.ulevel < 15) ? 5 : u.ulevel/3;
	tmp = 1;
	while (tmp < utmp && !rn2(x))
		tmp++;
	return tmp;

	/* was:
	 *	tmp = 1;
	 *	while(!rn2(x)) tmp++;
	 *	return(min(tmp,(u.ulevel < 15) ? 5 : u.ulevel/3));
	 * which is clearer but less efficient and stands a vanishingly
	 * small chance of overflowing tmp
	 */
}

int
rnz(i)
int i;
{
#ifdef LINT
	int x = i;
	int tmp = 1000;
#else
	register long x = i;
	register long tmp = 1000;
#endif
	tmp += rn2(1000);
	tmp *= rne(4);
	if (rn2(2)) { x *= tmp; x /= 1000; }
	else { x *= 1000; x /= tmp; }
	return((int)x);
}

#endif /* OVLB */

/*rnd.c*/
