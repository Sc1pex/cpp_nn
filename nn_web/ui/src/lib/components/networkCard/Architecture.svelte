<script lang="ts">
  import type { Network } from "$lib/data/network.svelte";
  import { ArrowRight } from "@lucide/svelte";

  interface Props {
    network: Network;
    variant?: "compact" | "detailed";
  }

  const { network, variant = "compact" }: Props = $props();
</script>

{#if variant === "compact"}
  <div class="bg-background rounded-xl p-4 flex flex-col gap-3">
    <div class="flex items-end justify-between">
      <p class="text-muted text-sm">Architecture</p>
      <p class="text-muted text-sm">Loss function</p>
    </div>
    <div class="flex justify-between">
      <div class="flex items-center gap-1 flex-wrap">
        {#each network.layers as layer, i}
          <div class="flex flex-col items-center">
            <span class="text-xs opacity-0">·</span>
            <span
              class="border border-border rounded-lg px-3 py-1 text-sm text-text"
              >{layer}</span
            >
          </div>
          {#if i < network.layers.length - 1}
            <div class="flex flex-col items-center mx-1">
              <span class="text-xs text-muted">{network.activations[i]}</span>
              <span class="text-muted">→</span>
            </div>
          {/if}
        {/each}
      </div>
      <p class="text-xl font-bold">{network.loss_function}</p>
    </div>
  </div>
{:else}
  <div class="overflow-x-auto py-2">
    <div class="inline-flex min-w-max items-center justify-start gap-4">
      {#each network.layers as size, i}
        <div class="flex flex-col items-center gap-2">
          <span class="text-sm font-semibold text-muted tracking-wider">
            {#if i === 0}
              INPUT
            {:else if i === network.layers.length - 1}
              OUTPUT
            {:else}
              HIDDEN {i}
            {/if}
          </span>
          <div
            class="rounded-xl border border-border bg-background px-6 py-5 text-center shadow-sm"
          >
            <div class="text-3xl font-bold text-text">{size}</div>
          </div>
        </div>

        {#if i < network.layers.length - 1}
          <div class="flex flex-col items-center gap-2 mt-6">
            <span class="text-sm font-medium text-muted"
              >{network.activations[i]}</span
            >
            <div class="text-primary opacity-60">
              <ArrowRight size={28} strokeWidth={2.5} />
            </div>
          </div>
        {/if}
      {/each}
    </div>
  </div>
{/if}
