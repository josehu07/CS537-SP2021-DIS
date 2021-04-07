#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>


#define CLOCKSIZE 8

#define PTE_P 0x001
#define PTE_E 0x200
#define PTE_R 0x400

typedef uint32_t pte_t;

typedef struct clk_node {
    int vpn;
    pte_t *pte;
} node_t;

// There are many ways of implementing a CLOCK eviction algorithm.
// I'm showing the most classical way:
// A clock struture is composed of a fixed ring buffer and
// an index of the current tail (clock hand).
node_t clk_queue[CLOCKSIZE];
int clk_hand = -1;


//////////////////////////////////////////
// Placeholders for mem encrypt/decrypt //
//////////////////////////////////////////

static void
mencrypt(int vpn, pte_t *pte)
{
    if (*pte & PTE_E)
        return;
    *pte |= PTE_E;
    *pte &= (~PTE_P);
    // Flip the bits...
}

static void
mdecrypt(int vpn, pte_t *pte)
{
    if (!(*pte & PTE_E))
        return;
    *pte &= (~PTE_E);
    *pte |= PTE_P;
    // Flip the bits...
}


////////////////////////////
// Clock queue operations //
////////////////////////////

// Search for a page without changing the hand position.
// This is the first thing you do when you try to ref a page.
static node_t *
clk_search(int vpn)
{
    for (int i = 0; i < CLOCKSIZE; ++i) {
        if (clk_queue[i].vpn == vpn) {
            // Ref the page and return;
            *(clk_queue[i].pte) |= PTE_R;
            *(clk_queue[i].pte) |= PTE_P;
            return &clk_queue[i];
        }
    }
    return NULL;
}

// Insert a page (should be guaranteed not already in queue)
// into the clock queue.
// This is the second thing you do when you try to ref a page
// and find that it is not in queue (so encrypted).
static void
clk_insert(int vpn, pte_t *pte)
{
    for (;;) {
        // First advance the hand.
        clk_hand = (clk_hand + 1) % CLOCKSIZE;

        // Found an empty slot.
        if (clk_queue[clk_hand].vpn == -1) {
            clk_queue[clk_hand].vpn = vpn;
            clk_queue[clk_hand].pte = pte;
            break;
        
        // Else if the page in this slot does not have its ref
        // bit set, evict this one.
        } else if (!(*(clk_queue[clk_hand].pte) & PTE_R)) {
            // Encrypt the evicted page.
            mencrypt(vpn, pte);
            // Put in the new page.
            clk_queue[clk_hand].vpn = vpn;
            clk_queue[clk_hand].pte = pte;
            break;

        // Else, clear the ref bit of the page in slot.
        // This should be accompanied by clearing the present
        // bit as well.
        } else {
            *(clk_queue[clk_hand].pte) &= (~PTE_R);
            *(clk_queue[clk_hand].pte) &= (~PTE_P);
        }
    }

    // Decrypt the new page and set reference bit.
    mdecrypt(vpn, pte);
    *(clk_queue[clk_hand].pte) |= PTE_R;
}

// Initialize the clock queue to an empty state.
static void
clk_clear(void)
{
    for (int i = 0; i < CLOCKSIZE; ++i)
        clk_queue[i].vpn = -1;
    clk_hand = -1;
}

// Print the clock queue in head->tail orderr, so starting
// from hand+1.
static void
clk_print(void)
{
    int print_idx = clk_hand;
    printf("CLK queue: | ");
    for (int i = 0; i < CLOCKSIZE; ++i) {
        print_idx = (print_idx + 1) % CLOCKSIZE;
        if (clk_queue[print_idx].vpn != -1) {
            printf("VPN %1X R %1d | ",
                clk_queue[print_idx].vpn,
                (*(clk_queue[print_idx].pte) & PTE_R) > 0);
        }
    }
    printf("\n\n");
}


// Now, referencing a page (that triggered a page falt) means:
//   1. Search for the page clock queue
//   2. If not found - insert this page, possibly evicting another
static void
do_ref_page(int vpn, pte_t *pte)
{
    printf("Ref page %1X\n", vpn);
    if (clk_search(vpn) != NULL)
        return;
    clk_insert(vpn, pte);
}


static inline void
wait_for_enter(void)
{
    printf("Hit [Enter] to print...");
    getchar();
}

int
main(void) {
    // Mimick a pgtable.
    // I omitted the physical frame number in the PTEs.
    // A page starts with PTE_E set (encrypted), and
    // PTE_P and PTE_R are not set.
    pte_t pgtable[12] = {0};
    for (int i = 0; i < 12; ++i)
        pgtable[i] |= PTE_E;

    // Make sure a process initially has an empty clock queue.
    clk_clear();

    do_ref_page(0, &pgtable[0]);
    do_ref_page(3, &pgtable[3]);
    do_ref_page(1, &pgtable[1]);
    do_ref_page(2, &pgtable[2]);
    clk_print();

    do_ref_page(9, &pgtable[9]);
    do_ref_page(7, &pgtable[7]);
    do_ref_page(4, &pgtable[4]);
    do_ref_page(6, &pgtable[6]);
    wait_for_enter();
    clk_print();

    do_ref_page(2, &pgtable[2]);
    wait_for_enter();
    clk_print();

    do_ref_page(10, &pgtable[10]);
    wait_for_enter();
    clk_print();

    do_ref_page(1, &pgtable[1]);
    do_ref_page(2, &pgtable[2]);
    do_ref_page(3, &pgtable[3]);
    wait_for_enter();
    clk_print();

    do_ref_page(11, &pgtable[11]);
    wait_for_enter();
    clk_print();
}
