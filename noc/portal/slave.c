// #include <mppaipc.h>
#include <mppa/osconfig.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <inttypes.h>

#include <mppa_power.h>
#include <mppa_noc.h>
#include <mppa_routing.h>

#define MASK_0 0xF
#define MASK_1 0x10
#define MASK_2 0x20

static int portal_tx;

void portal_open(int source_cluster, int target_cluster)
{
    unsigned aux;
    assert(mppa_noc_dnoc_tx_alloc_auto(0, &aux, MPPA_NOC_BLOCKING) == MPPA_NOC_RET_SUCCESS);
    portal_tx = aux;

    mppa_dnoc_channel_config_t config = { 0 };
    mppa_dnoc_header_t header = { 0 };
    header._.tag = 7;
    header._.valid = 1;

    MPPA_NOC_DNOC_TX_CONFIG_INITIALIZER_DEFAULT(config, 0);

    assert(mppa_routing_get_dnoc_unicast_route(source_cluster, target_cluster, &config, &header) == 0);
    mppa_noc_dnoc_tx_configure(0, portal_tx, header, config);
}

void portal_write(char *buffer, int size, int offset)
{
    mppa_dnoc_push_offset_t off;
    off._.offset = offset;
    off._.protocol = 0x1; //! absolute
    off._.valid = 1;

    mppa_noc_dnoc_tx_flush(0, portal_tx); //! need?
    __builtin_k1_fence();
    mppa_noc_dnoc_tx_set_push_offset(0, portal_tx, off);
    mppa_noc_dnoc_tx_send_data(0, portal_tx, size, buffer);
    mppa_noc_dnoc_tx_flush_eot(0, portal_tx);
}

void portal_close(void)
{
    mppa_noc_dnoc_tx_free(0, portal_tx);
}

int main(__attribute__((unused)) int argc, const char **argv)
{
    int id = __k1_get_cluster_id();
    // char id = atoi(argv[0]);

    printf("[IODDR0] Cluster %d: Start portal\n", id);

    portal_open(id, 128);

    printf("[IODDR0] Cluster %d: send\n", id);

    if (id == 1)
    {
        char buffer[4];
        sprintf(buffer, "C1& ");
        portal_write(buffer, 4, 0);
    }
    else
    {
        char buffer[7];
        sprintf(buffer, " Clus2");
        portal_write(buffer, 7, 4);
    }
    
    printf("[IODDR0] Cluster %d: Sync\n", id);

    portal_close();

    printf("[IODDR0] Cluster %d: End Sync\n", id);
	printf("[IODDR0] Cluster %d: Goodbye\n", id);

	return 0;
}
