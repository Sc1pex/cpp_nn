<script lang="ts">
  import NetworkSummary from "$lib/components/NetworkSummary.svelte";
  import * as Dialog from "$lib/components/ui/dialog/index";
  import { Button } from "$lib/components/ui/button/index";
  import { NetworkSummaries } from "$lib/data/networks.svelte";
  import { Plus } from "@lucide/svelte";
  import CreateNetworkModal from "$lib/components/CreateNetworkModal.svelte";

  const data = new NetworkSummaries();
  let showCreateModal = $state(false);

  async function handleCreateNetwork(
    name: string,
    layer_sizes: number[],
    activations: string[],
  ) {
    const err = await data.add(name, layer_sizes, activations);
    if (err == null) {
      showCreateModal = false;
      await data.fetch();
    }
    // TODO: Show error if it's not on the name field

    return err;
  }

  const defaultName = $derived(`Network ${data.networks.length + 1}`);
</script>

<div class="flex flex-col gap-6">
  <Dialog.Root
    open={showCreateModal}
    onOpenChange={(open) => (showCreateModal = open)}
  >
    <Dialog.Trigger class="flex justify-end">
      <Button>
        <Plus class="w-4 h-4" />
        Create Network
      </Button>
    </Dialog.Trigger>

    <CreateNetworkModal onSubmit={handleCreateNetwork} {defaultName} />
  </Dialog.Root>

  {#if data.loading}
    <div>Loading networks...</div>
  {:else}
    {#each data.networks as network (network.id)}
      <NetworkSummary {network} />
    {/each}
  {/if}
</div>
