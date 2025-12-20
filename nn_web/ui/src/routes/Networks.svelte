<script lang="ts">
  import NetworkSummary from "$lib/components/NetworkSummary.svelte";
  import * as Dialog from "$lib/components/ui/dialog/index";
  import { Button, buttonVariants } from "$lib/components/ui/button/index";
  import {
    NetworkList,
    type TNetworkListItem,
  } from "$lib/data/networks.svelte";
  import { Plus } from "@lucide/svelte";
  import CreateNetworkModal from "$lib/components/CreateNetworkModal.svelte";

  const data = new NetworkList();
  let showCreateModal = $state(false);

  let networkToDelete: TNetworkListItem | null = $state(null);

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

  const defaultName = $derived.by(() => {
    let smallest = 0;
    for (const network of data.networks) {
      const match = network.name.match(/Network (\d+)/);
      if (match) {
        const number = parseInt(match[1], 10);
        if (number >= smallest) {
          smallest = number;
        }
      }
    }
    return `Network ${smallest + 1}`;
  });

  const handleDeleteNetwork = async () => {
    if (networkToDelete) {
      await data.delete(networkToDelete.id);
      networkToDelete = null;
      await data.fetch();
    }
  };
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
      <NetworkSummary
        {network}
        onDelete={(network) => (networkToDelete = network)}
      />
    {/each}
  {/if}

  <Dialog.Root
    open={networkToDelete !== null}
    onOpenChange={(open) => {
      if (!open) networkToDelete = null;
    }}
  >
    <Dialog.Content>
      <Dialog.Header>
        <Dialog.Title
          >Are you sure you want to delete {networkToDelete?.name}</Dialog.Title
        >
        <Dialog.Description>This action cannot be undone.</Dialog.Description>
      </Dialog.Header>

      <Dialog.Footer>
        <Dialog.Close class={buttonVariants({ variant: "outline" })}>
          Cancel
        </Dialog.Close>
        <Button variant="destructive" onclick={handleDeleteNetwork}
          >Delete Network</Button
        >
      </Dialog.Footer>
    </Dialog.Content>
  </Dialog.Root>
</div>
