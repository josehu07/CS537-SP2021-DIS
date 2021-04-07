#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>


#define CLOCKSIZE 8

#define PTE_P 0x001
#define PTE_A 0x020
#define PTE_E 0x200

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

// Insert a page (should be guaranteed not already in queue)
// into the clock queue.
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
        } else if (!(*(clk_queue[clk_hand].pte) & PTE_A)) {
            // Encrypt the evicted page.
            mencrypt(clk_queue[clk_hand].vpn, clk_queue[clk_hand].pte);
            // Put in the new page.
            clk_queue[clk_hand].vpn = vpn;
            clk_queue[clk_hand].pte = pte;
            break;

        // Else, clear the ref bit of the page in slot.
        } else {
            *(clk_queue[clk_hand].pte) &= (~PTE_A);
        }
    }

    // Decrypt the new page.
    mdecrypt(vpn, pte);
}

// Removing a page forcefully is tricky because you need to
// shift things around.
// This happens at page deallocation.
static void
clk_remove(int vpn)
{
    int prev_tail = clk_hand;
    int match_idx = -1;

    // Search for the matching element.
    for (int i = 0; i < CLOCKSIZE; ++i) {
        int idx = (clk_hand + i) % CLOCKSIZE;
        if (clk_queue[idx].vpn == vpn) {
            match_idx = idx;
            break;
        }
    }

    if (match_idx == -1)
        return;

    // Shift everything from match_idx+1 to prev_tail to
    // one slot to the left.
    for (int idx = match_idx;
         idx != prev_tail;
         idx = (idx + 1) % CLOCKSIZE) {
        int next_idx = (idx + 1) % CLOCKSIZE;
        clk_queue[idx].vpn = clk_queue[next_idx].vpn;
        clk_queue[idx].pte = clk_queue[next_idx].pte;
    }

    // Clear the element at prev_tail. Set clk_hand to
    // one entry to the left.
    clk_queue[prev_tail].vpn = -1;
    clk_hand = clk_hand == 0 ? CLOCKSIZE - 1
                             : clk_hand - 1;
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
                (*(clk_queue[print_idx].pte) & PTE_A) > 0);
        }
    }
    printf("\n\n");
}


// Now, referencing a page (that triggered a page falt) means
// inserting this page into the clock queue, possibly evicting
// another.
static void
do_ref_page(int vpn, pte_t *pte)
{
    printf("Ref page %1X\n", vpn);

    if (*pte & PTE_P) {
        *pte |= PTE_A;  // mimick the HW setting ref bit
        return;     // if has PTE_P, hardware won't trigger page fault
    }
    
    clk_insert(vpn, pte);
    *pte |= PTE_A;  // mimick the HW setting ref bit
}

// Removing a page gets tricky and may involve shifting things
// in the queue.
static void
do_remove_page(int vpn)
{
    printf("Remove page %1X\n", vpn);
    clk_remove(vpn);
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
    // A page starts with PTE_E set (encrypted), and so
    // PTE_P is not set.
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

    do_remove_page(10);
    do_remove_page(11);
    wait_for_enter();
    clk_print();
}
