#include <hwloc.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

int main(void) {
    hwloc_topology_t topology;

    if (hwloc_topology_init(&topology) == -1) {
        perror("failed to initialize topology");
        return -1;
    }

    if (hwloc_topology_load(topology) == -1) {
        perror("failed to load topology");
        return -1;
    }

    hwloc_bitmap_t set = hwloc_bitmap_alloc_full();
    void *mem = hwloc_alloc_membind_policy(topology, 1, set, HWLOC_MEMBIND_DEFAULT, 0);
    if (mem == -1 || mem == NULL) {
        perror("hwloc_alloc_membind_policy() failed");
        return -1;
    }
    hwloc_free(topology, mem, 1);

    if (hwloc_set_membind(topology, set, HWLOC_MEMBIND_DEFAULT, 0) == -1) {
        perror("hwloc_set_membind() failed");
        return -1;
    }

    return 0;
}
