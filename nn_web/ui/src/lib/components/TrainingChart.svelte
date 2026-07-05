<script lang="ts">
  import { LineChart, defaultChartPadding } from "layerchart";
  import Card from "./Card.svelte";
  import type { TrainingHistoryStore } from "$lib/data/trainingHistory.svelte";

  let { history }: { history: TrainingHistoryStore } = $props();

  const padding = defaultChartPadding({
    left: 16,
    right: 16,
    top: 8,
    bottom: 8,
  });
</script>

<Card class="flex flex-col gap-6">
  <h2 class="text-xl font-bold text-text">Training Progress</h2>

  <div class="grid grid-cols-1 md:grid-cols-2 gap-8">
    <!-- Loss curve -->
    <div class="flex flex-col gap-2">
      <h3 class="text-sm font-medium text-muted">Training Loss</h3>
      {#if history.lossPoints.length > 0}
        <LineChart
          data={history.lossPoints}
          x="idx"
          y="value"
          height={220}
          {padding}
        />
      {:else}
        <div
          class="h-[220px] flex items-center justify-center text-muted text-sm"
        >
          No data yet
        </div>
      {/if}
    </div>

    <!-- Accuracy curve -->
    <div class="flex flex-col gap-2">
      <h3 class="text-sm font-medium text-muted">Test Accuracy</h3>
      {#if history.epochSummaries.length > 0}
        <LineChart
          data={history.epochSummaries}
          x="idx"
          y="value"
          height={220}
          {padding}
        />
      {:else}
        <div
          class="h-[220px] flex items-center justify-center text-muted text-sm"
        >
          No data yet
        </div>
      {/if}
    </div>
  </div>
</Card>
