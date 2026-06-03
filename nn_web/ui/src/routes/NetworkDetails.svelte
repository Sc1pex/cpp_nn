<script lang="ts">
  import { route, navigate } from "../router";
  import { ArrowLeft } from "@lucide/svelte";
  import Button from "$lib/components/Button.svelte";
  import type { Network } from "$lib/data/network.svelte";
  import NetworkArchitecture from "$lib/components/network/NetworkArchitecture.svelte";

  const networkId = parseInt(route.getParams("/:id").id);

  const network: Network = {
    name: `Network ${networkId}`,
    id: networkId,
    layers: [784, 128, 64, 10],
    activations: ["ReLU", "ReLU", "SoftMax"],
    loss_function: "Cross Entropy",
    loss: 0.042,
    epocs: 14,
    accuracy: 96.8,
  };

  const outputBars = [96, 4, 12, 31, 22, 8, 3, 5, 2, 1];
  const predictedDigit = 0;
  const actualDigit = 0;
</script>

<div class="flex flex-col gap-6">
  <div class="flex items-center gap-4 mb-2">
    <Button variant="outline" onclick={() => navigate("/")} class="!px-3 !py-2">
      <ArrowLeft size={20} />
    </Button>
    <h1 class="text-3xl font-bold text-text">{network.name}</h1>
  </div>

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
            <p class="text-6xl font-bold text-primary mt-2">{predictedDigit}</p>
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
</div>
