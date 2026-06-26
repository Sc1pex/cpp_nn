<script lang="ts">
  import Button from "./Button.svelte";
  import Card from "./Card.svelte";

  let { networkId }: { networkId: number } = $props();

  let epochs = $state(1);
  let batchSize = $state(32);
  let learningRate = $state(0.01);

  let training = $state(false);
  let currentEpoch = $state(0);
  let result = $state<{ message?: string; error?: string } | null>(null);

  async function train() {
    training = true;
    result = null;
    currentEpoch = 0;
    try {
      const res = await fetch(
        `http://localhost:8080/api/networks/${networkId}/train`,
        {
          method: "POST",
          headers: {
            "Content-Type": "application/json",
          },
          body: JSON.stringify({
            epochs,
            batch_size: batchSize,
            learning_rate: learningRate,
          }),
        },
      );

      if (!res.ok) {
        const data = await res.json();
        throw new Error(data.error || "Failed to train network");
      }

      const reader = res.body!.getReader();
      const decoder = new TextDecoder();
      let buffer = "";

      while (true) {
        const { done, value } = await reader.read();
        if (done) break;

        buffer += decoder.decode(value, { stream: true });
        const lines = buffer.split("\n");
        buffer = lines.pop() || "";

        for (const line of lines) {
          const trimmed = line.trim();
          if (!trimmed) continue;

          if (trimmed.startsWith("error:")) {
            throw new Error(trimmed.replace("error:", "").trim());
          }

          const epochNum = parseInt(trimmed, 10);
          if (!isNaN(epochNum)) {
            currentEpoch = epochNum;
          }
        }
      }

      result = { message: "Network trained successfully" };
    } catch (e: any) {
      result = { error: e.message || "Failed to train network" };
    } finally {
      training = false;
    }
  }
</script>

<Card class="flex flex-col gap-4">
  <h2 class="text-xl font-bold text-text">Train Network</h2>
  <div class="grid grid-cols-1 md:grid-cols-3 gap-4">
    <label class="flex flex-col gap-1">
      <span class="text-sm font-medium text-text">Epochs</span>
      <input
        type="number"
        bind:value={epochs}
        min={1}
        max={64}
        class="border border-border rounded-lg px-3 py-2 bg-background focus:outline-none focus:ring-2 focus:ring-primary"
      />
    </label>
    <label class="flex flex-col gap-1">
      <span class="text-sm font-medium text-text">Batch Size</span>
      <input
        type="number"
        bind:value={batchSize}
        min={1}
        max={784}
        class="border border-border rounded-lg px-3 py-2 bg-background focus:outline-none focus:ring-2 focus:ring-primary"
      />
    </label>
    <label class="flex flex-col gap-1">
      <span class="text-sm font-medium text-text">Learning Rate</span>
      <input
        type="number"
        step="0.001"
        bind:value={learningRate}
        min={1}
        class="border border-border rounded-lg px-3 py-2 bg-background focus:outline-none focus:ring-2 focus:ring-primary"
      />
    </label>
  </div>

  <div class="flex items-center gap-4 mt-2">
    <Button onclick={train} disabled={training}>
      {training ? "Training..." : "Start Training"}
    </Button>

    {#if training}
      <span class="text-sm text-muted"
        >Training (Epoch {currentEpoch}/{epochs})...</span
      >
    {/if}

    {#if result?.message}
      <span class="text-sm text-primary font-medium">{result.message}</span>
    {/if}
    {#if result?.error}
      <span class="text-sm text-red-500 font-medium">{result.error}</span>
    {/if}
  </div>
</Card>
