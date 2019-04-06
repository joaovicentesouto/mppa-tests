// #include <mppaipc.h>
#include <mppa/osconfig.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include <mppa_power.h>
#include <mppa_noc.h>
#include <mppa_routing.h>

#include <spawn.h>
#include <noc.h>

#define MASK ~0x3
#define BUFFER_SIZE 128

int main(__attribute__((unused)) int argc,__attribute__((unused)) const char **argv)
{
    printf("====== NoC Portal: 1 IO to 2 Clusters ======\n");
    
    int id = 129;
    int tag_in = 42, tag_out;
    int interface = 1;
    
    
    char buffer_out[BUFFER_SIZE];
    char buffer_in[BUFFER_SIZE];

    printf("Alloc and config Portals\n");

    sprintf(buffer_out, "My message IO");
    sprintf(buffer_in, "Not Works");

    dnoc_rx_alloc(interface, tag_in);
    dnoc_rx_config(interface, tag_in, buffer_in, BUFFER_SIZE, 0);

    tag_out = dnoc_tx_alloc_auto(interface);

    printf("Send to myself: %s\n", buffer_out);

    dnoc_tx_config(interface, tag_out, id, tag_in, id);
    dnoc_tx_write(interface, tag_out, buffer_out, id, 0);

    printf("Wait to myself\n");

    dnoc_rx_wait(interface, tag_in, BUFFER_SIZE);

    printf("Receive from myself: %s\n", buffer_in);

    printf("Done - Spawn slave\n");

    spawn();

    join();

    printf("Goodbye\n");

	return 0;
};
