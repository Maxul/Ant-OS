#include "mm.h"

#include "ok.h"

enum {
    MEM_FREE,
    MEM_USED,
};

static u16 mem_map[PAGING_PAGES] = { 0 };

u64 free_page(u64 p)
{
    int location = (p - LOW_MEMORY / PAGE_SIZE);
    mem_map[location] = MEM_FREE;
}

u64 get_free_page()
{
    int i;
    
    for (i = 0; i< PAGING_PAGES; ++i) {
        if (MEM_FREE == mem_map[i]) {
            mem_map[i] = MEM_USED;
            return LOW_MEMORY + PAGE_SIZE * i;
        }
    }
    return 0;
}
