// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU];

void
kinit()
{
    char buf[10];
    for (int i = 0; i < NCPU; i++)
    {
        printf(buf, 10, "kmem_CPU%d", i);
        initlock(&kmem[i].lock, buf);
    }
    freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void* pa)
{
    struct run* r;

    if (((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
        panic("kfree");

    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    r = (struct run*)pa;

    push_off();
    int cpu = cpuid();
    pop_off();
    acquire(&kmem[cpu].lock);
    r->next = kmem[cpu].freelist;
    kmem[cpu].freelist = r;
    release(&kmem[cpu].lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void*
kalloc(void)
{
    struct run* r;

    // Disable interrupts and get the current CPU ID
    push_off();
    int cpu = cpuid();
    pop_off();

    // Try to allocate a page from the current CPU's free list
    acquire(&kmem[cpu].lock);
    r = kmem[cpu].freelist;
    if (r)
        kmem[cpu].freelist = r->next;
    else // If no pages are available, attempt to steal from other CPUs
    {
        struct run* tmp;
        for (int i = 0; i < NCPU; ++i)
        {
            if (i == cpu) continue;
            acquire(&kmem[i].lock);
            tmp = kmem[i].freelist;
            if (tmp == 0) {
                release(&kmem[i].lock);
                continue;
            }
            else {
                for (int j = 0; j < 1024; j++) {
                    // Try to steal up to 1024 pages
                    if (tmp->next)
                        tmp = tmp->next;
                    else
                        break;
                }
                // Transfer pages from other CPU's free list to the current CPU's free list
                kmem[cpu].freelist = kmem[i].freelist;
                kmem[i].freelist = tmp->next;
                tmp->next = 0;
                release(&kmem[i].lock);
                break;
            }
        }
        r = kmem[cpu].freelist;
        if (r)
            kmem[cpu].freelist = r->next;
    }
    release(&kmem[cpu].lock);

    // If a page was allocated, fill it with a specific value (5) for debugging purposes
    if (r)
        memset((char*)r, 5, PGSIZE); // fill with junk
    return (void*)r;
}
