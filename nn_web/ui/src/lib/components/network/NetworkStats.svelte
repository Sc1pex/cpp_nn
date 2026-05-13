<script lang="ts">
  import type { Network } from "$lib/data/network";

  interface Props {
    network: Network;
  }

  const { network }: Props = $props();
</script>

<div class="flex gap-8 bg-background rounded-xl p-4">
  <div class="flex-1">
    <p class="text-muted text-sm">Accuracy</p>
    <p class="text-2xl font-bold">{network.accuracy}%</p>
    <div class="h-1.5 bg-border rounded-full mt-2">
      <div
        class="h-1.5 bg-primary rounded-full transition-all"
        style="width: {network.accuracy}%"
      ></div>
    </div>
  </div>
  <div>
    <p class="text-muted text-sm">Loss</p>
    <p class="text-2xl font-bold mt-1">{network.loss}</p>
  </div>
  <div>
    <p class="text-muted text-sm">Epochs</p>
    <p class="text-2xl font-bold mt-1">
      {network.epocs}
    </p>
  </div>
</div>
