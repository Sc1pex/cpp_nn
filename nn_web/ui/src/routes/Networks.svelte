<script lang="ts">
  import NetworkCard from "$lib/components/network/NetworkCard.svelte";
  import CreateNetworkDialog from "$lib/components/CreateNetworkDialog.svelte";
  import { networkStore } from "$lib/data/network.svelte";

  const defaultName = $derived.by(() => {
    let smallest = 0;
    for (const network of networkStore.networks) {
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
</script>

<div class="flex items-center justify-between mb-6">
  <h1 class="text-2xl font-bold text-text">Networks</h1>
  <CreateNetworkDialog onsubmit={networkStore.create} {defaultName} />
</div>

{#if networkStore.loading && networkStore.networks.length === 0}
  <div class="flex flex-col items-center justify-center p-12 text-muted gap-4">
    <div
      class="w-8 h-8 border-4 border-border border-t-primary rounded-full animate-spin"
    ></div>
    <p>Loading networks...</p>
  </div>
{:else}
  <div class="flex flex-col gap-8">
    {#each networkStore.networks as network}
      <NetworkCard {network} onDelete={networkStore.delete} />
    {/each}
  </div>
{/if}
