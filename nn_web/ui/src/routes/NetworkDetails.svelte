<script lang="ts">
  import { route, navigate } from "../router";
  import { ArrowLeft } from "@lucide/svelte";
  import Button from "$lib/components/Button.svelte";
  import { networkStore } from "$lib/data/network.svelte";
  import NetworkArchitecture from "$lib/components/network/NetworkArchitecture.svelte";

  const networkId = parseInt(route.getParams("/:id").id);

  const network = $derived(
    networkStore.networks.find((n) => n.id === networkId),
  );

  const outputBars = [96, 4, 12, 31, 22, 8, 3, 5, 2, 1];
  const predictedDigit = 0;
  const actualDigit = 0;
</script>

<div class="flex flex-col gap-6">
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
      <NetworkArchitecture {network} variant="detailed" />
    </div>

    <div
      class="bg-surface border border-border rounded-2xl shadow-sm p-6 flex flex-col gap-4"
    >
      <h2 class="text-xl font-bold text-text">Prediction Output</h2>

      <div class="mt-4 grid grid-cols-1 md:grid-cols-2 gap-8 items-center">
        <div class="flex flex-col gap-3">
          {#each outputBars as score, digit}
            <div class="flex items-center gap-4">
              <span class="w-4 font-bold text-text">{digit}</span>
              <div
                class="h-3 flex-1 rounded-full bg-background border border-border overflow-hidden"
              >
                <div
                  class="h-full bg-primary transition-all duration-500"
                  style="width: {score}%"
                ></div>
              </div>
              <span class="w-12 text-right text-sm font-medium text-muted"
                >{score}%</span
              >
            </div>
          {/each}
        </div>

        <div class="flex flex-col gap-4">
          <div
            class="bg-background border border-border rounded-xl p-8 flex items-center justify-between shadow-sm"
          >
            <div>
              <p class="text-sm font-bold text-muted uppercase tracking-wider">
                Predicted
              </p>
              <p class="text-6xl font-bold text-primary mt-2">
                {predictedDigit}
              </p>
            </div>
            <div class="text-right">
              <p class="text-sm font-bold text-muted uppercase tracking-wider">
                Actual
              </p>
              <p class="text-6xl font-bold text-text mt-2">{actualDigit}</p>
            </div>
          </div>
        </div>
      </div>
    </div>
  {/if}
</div>
