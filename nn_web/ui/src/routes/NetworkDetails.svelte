<script lang="ts">
  import { route, navigate } from "../router";
  import { ArrowLeft } from "@lucide/svelte";
  import Button from "$lib/components/Button.svelte";
  import { networkStore } from "$lib/data/network.svelte";
  import NetworkArchitecture from "$lib/components/network/NetworkArchitecture.svelte";
  import { getPrediction, getSample } from "$lib/data/prediction.svelte";

  const networkId = parseInt(route.getParams("/:id").id);

  const network = $derived(
    networkStore.networks.find((n) => n.id === networkId),
  );

  let datasetType = $state<"train" | "test">("test");
  let imageIndex = $state(42);
  let maxIndex = $derived(datasetType === "train" ? 60000 : 10000);

  let expectedDigit = $state(0);
  let predictedDigit = $state(0);
  let confidenceScores = $state<number[]>(Array(10).fill(0));
  let isPredicting = $state(false);

  let canvasElement: HTMLCanvasElement;

  function renderPixels(pixels: number[]) {
    if (!canvasElement) return;
    const ctx = canvasElement.getContext("2d");
    if (!ctx) return;

    for (let y = 0; y < 28; y++) {
      for (let x = 0; x < 28; x++) {
        const pixelValue = pixels[y * 28 + x];
        ctx.fillStyle = `rgb(${pixelValue}, ${pixelValue}, ${pixelValue})`;
        ctx.fillRect(x * 10, y * 10, 10, 10);
      }
    }
  }

  $effect(() => {
    const type = datasetType;
    const idx = imageIndex;

    if (network) {
      const id = network.id;
      isPredicting = true;

      getSample(type, idx).then((sample) => {
        expectedDigit = sample.expected_output;
        renderPixels(sample.input);

        getPrediction(id, type, idx).then((result) => {
          predictedDigit = result.prediction;
          confidenceScores = result.scores;
          isPredicting = false;
        });
      });
    }
  });

  function randomize() {
    imageIndex = Math.floor(Math.random() * maxIndex) + 1;
  }
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
      <NetworkArchitecture {network} variant="detailed" />
    </div>

    <div
      class="bg-surface border border-border rounded-2xl shadow-sm p-6 flex flex-col gap-6"
    >
      <div
        class="flex flex-col sm:flex-row items-start sm:items-center justify-between border-b border-border pb-4 gap-4"
      >
        <h2 class="text-xl font-bold text-text">Test Network</h2>

        <div class="flex items-center gap-4 flex-wrap">
          <select
            bind:value={datasetType}
            disabled={isPredicting}
            class="bg-background border border-border rounded-lg px-3 py-1.5 text-sm outline-none focus:border-primary text-text disabled:opacity-50 disabled:cursor-not-allowed transition-opacity"
          >
            <option value="train">Training Data (60,000)</option>
            <option value="test">Test Data (10,000)</option>
          </select>

          <div class="flex items-center gap-2">
            <span
              class="text-sm font-semibold text-text"
              class:opacity-50={isPredicting}>Index:</span
            >
            <input
              type="number"
              bind:value={imageIndex}
              min="1"
              max={maxIndex}
              disabled={isPredicting}
              class="w-20 bg-background border border-border rounded-lg px-2 py-1.5 text-sm outline-none focus:border-primary text-text disabled:opacity-50 disabled:cursor-not-allowed transition-opacity"
            />
            <Button
              variant="outline"
              class="!px-3 !py-1.5 text-sm"
              onclick={randomize}
              disabled={isPredicting}>Random</Button
            >
          </div>
        </div>
      </div>

      <div class="grid grid-cols-1 lg:grid-cols-12 gap-8 items-stretch mt-4">
        <!-- Left Column: Canvas and Summary (5 columns wide) -->
        <div class="lg:col-span-5 flex flex-col gap-3">
          <div class="h-8 flex items-center">
            <h3
              class="text-sm font-semibold text-muted tracking-wider uppercase"
            >
              Input Image
            </h3>
          </div>
          <div
            class="border border-border rounded-xl bg-background flex flex-col items-center justify-center p-8 shadow-sm flex-1"
          >
            <!-- Canvas -->
            <div
              class="p-2 border border-border bg-surface rounded-xl shadow-inner w-full max-w-sm"
            >
              <canvas
                bind:this={canvasElement}
                width="280"
                height="280"
                class="rounded-lg bg-black w-full aspect-square shadow-md"
                style="image-rendering: pixelated;"
              ></canvas>
            </div>

            <!-- Summary Box -->
            <div
              class="w-full mt-10 pt-8 border-t border-border flex items-center justify-around"
            >
              <div class="text-center">
                <p
                  class="text-xs font-bold text-muted uppercase tracking-wider mb-2"
                >
                  Predicted
                </p>
                <div
                  class="w-20 h-20 mx-auto rounded-2xl bg-surface border-2 border-primary flex items-center justify-center shadow-lg shadow-primary/10"
                >
                  <span class="text-5xl font-bold text-primary"
                    >{predictedDigit}</span
                  >
                </div>
              </div>

              <div class="w-px h-16 bg-border"></div>

              <div class="text-center">
                <p
                  class="text-xs font-bold text-muted uppercase tracking-wider mb-2"
                >
                  Actual
                </p>
                <div
                  class="w-20 h-20 mx-auto rounded-2xl bg-background border border-border flex items-center justify-center shadow-sm opacity-80"
                >
                  <span class="text-5xl font-bold text-text"
                    >{expectedDigit}</span
                  >
                </div>
              </div>
            </div>
          </div>
        </div>

        <!-- Right Column: Confidence Breakdown (7 columns wide) -->
        <div class="lg:col-span-7 flex flex-col gap-3">
          <div class="flex items-center justify-between h-8">
            <h3
              class="text-sm font-semibold text-muted tracking-wider uppercase"
            >
              Confidence Breakdown
            </h3>
            <span
              class="text-xs text-primary font-bold uppercase tracking-wider flex items-center gap-2 bg-primary/10 px-3 py-1 rounded-full transition-opacity duration-200"
              class:opacity-0={!isPredicting}
              class:opacity-100={isPredicting}
            >
              <span class="w-2 h-2 bg-primary rounded-full animate-ping"></span>
              Computing...
            </span>
          </div>

          <div
            class="bg-background border border-border rounded-xl p-8 flex flex-col justify-between shadow-sm flex-1 gap-4"
          >
            {#each confidenceScores as score, digit}
              <div class="flex items-center gap-5 group">
                <div
                  class="flex items-center justify-center w-8 h-8 rounded-lg bg-surface border border-border text-text font-bold shadow-sm transition-all group-hover:border-primary group-hover:text-primary"
                >
                  {digit}
                </div>

                <div
                  class="h-3.5 flex-1 rounded-full bg-surface border border-border overflow-hidden shadow-inner relative"
                >
                  <div
                    class="absolute left-0 top-0 h-full transition-all duration-700 ease-out"
                    class:bg-primary={score === Math.max(...confidenceScores) &&
                      score > 0}
                    class:bg-muted={score !== Math.max(...confidenceScores) ||
                      score === 0}
                    class:opacity-40={score !== Math.max(...confidenceScores)}
                    style="width: {Math.max(score, 1)}%"
                  ></div>
                </div>

                <span
                  class="w-12 text-right text-sm font-bold transition-colors"
                  class:text-primary={score === Math.max(...confidenceScores) &&
                    score > 0}
                  class:text-muted={score !== Math.max(...confidenceScores) ||
                    score === 0}
                >
                  {score}%
                </span>
              </div>
            {/each}
          </div>
        </div>
      </div>
    </div>
  {/if}
</main>
