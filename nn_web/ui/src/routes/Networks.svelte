<script lang="ts">
  import NetworkSummary from "$lib/components/NetworkSummary.svelte";
  import { Button } from "$lib/components/ui/button/index.js";
  import { NetworkSummaries } from "$lib/data/networks.svelte";
  import { Plus } from "@lucide/svelte";

  const data = new NetworkSummaries();
  $inspect(data.networks);
</script>

<div class="flex flex-col gap-6">
  <div class="flex justify-end">
    <Button>
      <Plus class="w-4 h-4" />
      Create Network
    </Button>
  </div>

  {#if data.loading}
    <div>Loading networks...</div>
  {:else}
    {#each data.networks as network (network.id)}
      <NetworkSummary {network} />
    {/each}
  {/if}
</div>
