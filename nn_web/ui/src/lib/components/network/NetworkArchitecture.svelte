<script lang="ts">
  import type { Network } from "$lib/data/network";

  interface Props {
    network: Network;
  }

  const { network }: Props = $props();
</script>

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
