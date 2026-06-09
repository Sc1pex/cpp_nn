<script lang="ts">
  import { route, navigate } from "../router";
  import { ArrowLeft } from "@lucide/svelte";
  import Button from "$lib/components/Button.svelte";
  import { networkStore } from "$lib/data/network.svelte";
  import Architecture from "$lib/components/networkCard/Architecture.svelte";
  import NetworkTester from "$lib/components/NetworkTester.svelte";
  import NetworkTrainer from "$lib/components/NetworkTrainer.svelte";

  const networkId = parseInt(route.getParams("/:id").id);

  const network = $derived(
    networkStore.networks.find((n) => n.id === networkId),
  );
</script>

<main class="flex flex-col gap-6 max-w-5xl mx-auto mt-8 px-4 mb-16">
  <div class="flex items-center gap-4 mb-2">
    <Button variant="outline" onclick={() => navigate("/")} class="!px-3 !py-2">
      <ArrowLeft size={20} />
    </Button>
    {#if network}
      <h1 class="text-3xl font-bold text-text">{network.name}</h1>
    {/if}
  </div>

  {#if !network}
    {#if networkStore.loading}
      <div
        class="flex flex-col items-center justify-center p-12 text-muted gap-4"
      >
        <div
          class="w-8 h-8 border-4 border-border border-t-primary rounded-full animate-spin"
        ></div>
        <p>Loading network details...</p>
      </div>
    {:else}
      <div
        class="flex flex-col items-center justify-center p-12 text-muted gap-4"
      >
        <p class="text-xl font-bold text-text">Network not found</p>
        <p>The network you are looking for does not exist.</p>
        <Button variant="outline" onclick={() => navigate("/")}
          >Return to Networks</Button
        >
      </div>
    {/if}
  {:else}
    <div
      class="bg-surface border border-border rounded-2xl shadow-sm p-6 flex flex-col gap-4"
    >
      <h2 class="text-xl font-bold text-text">Architecture</h2>
      <Architecture {network} variant="detailed" />
    </div>

    <NetworkTrainer networkId={network.id} />
    <NetworkTester networkId={network.id} />
  {/if}
</main>
