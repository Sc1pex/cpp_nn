<script lang="ts">
  import { Button } from "$lib/components/ui/button";
  import * as Card from "$lib/components/ui/card";
  import { Input } from "$lib/components/ui/input";
  import { Label } from "$lib/components/ui/label";
  import * as RadioGroup from "$lib/components/ui/radio-group";
  import * as Progress from "$lib/components/ui/progress";
  import { getSample, Prediction } from "$lib/data/predict.svelte";

  type Props = {
    networkId: number;
  };
  let { networkId }: Props = $props();

  let predictor = new Prediction(networkId);

  let datasetType = $state<"train" | "test">("train");
  let imageIndex = $state(42);
  let maxIndex = $derived(datasetType === "train" ? 60000 : 10000);

  let expectedDigit = $state<number | null>(null);
  let inputPixels = $state<number[]>([]);

  let predictedDigit = $state<number | null>(null);
  let confidenceScores = $state<number[]>([]);

  let canvasElement: HTMLCanvasElement;

  function renderDigit(canvas: HTMLCanvasElement, pixels: number[]) {
    const ctx = canvas.getContext("2d");
    if (!ctx) return;

    for (let y = 0; y < 28; y++) {
      for (let x = 0; x < 28; x++) {
        const pixelValue = pixels[y * 28 + x];
        ctx.fillStyle = `rgb(${pixelValue}, ${pixelValue}, ${pixelValue})`;
        ctx.fillRect(x * 10, y * 10, 10, 10);
      }
    }
  }

  function runPrediction() {
    predictor.predict(inputPixels).then((result) => {
      console.log(result);
      predictedDigit = result.prediction;
      confidenceScores = result.scores;
    });
  }

  $effect(() => {
    getSample(datasetType, imageIndex - 1).then((sample) => {
      expectedDigit = sample.expected_output;
      inputPixels = sample.input;

      renderDigit(canvasElement, sample.input);
      runPrediction();
    });
  });
</script>

<Card.Root>
  <Card.Header>
    <Card.Title>Test Network</Card.Title>
  </Card.Header>
  <Card.Content>
    <div class="mb-6">
      <Label class="mb-3">Source</Label>
      <RadioGroup.Root bind:value={datasetType} class="flex gap-6">
        <div class="flex items-center space-x-2">
          <RadioGroup.Item value="train" id="train-data" />
          <Label for="train-data">Training Data (60,000)</Label>
        </div>
        <div class="flex items-center space-x-2">
          <RadioGroup.Item value="test" id="test-data" />
          <Label for="test-data">Test Data (10,000)</Label>
        </div>
      </RadioGroup.Root>
    </div>

    <div class="mb-6 flex items-center gap-4">
      <Label for="image-index">Image Index:</Label>
      <Input
        id="image-index"
        type="number"
        bind:value={imageIndex}
        min={1}
        max={maxIndex}
        class="w-32"
      />
      <Button
        variant="outline"
        size="sm"
        onclick={() => {
          imageIndex = Math.floor(Math.random() * maxIndex) + 1;
          runPrediction();
        }}
      >
        Random
      </Button>
    </div>

    <div class="grid grid-cols-1 gap-6 md:grid-cols-2">
      <div>
        <h3 class="mb-3 text-sm font-semibold">Input Image</h3>
        <div class="flex flex-col items-center gap-4">
          <div class="rounded-lg border bg-muted/30 p-4">
            <canvas
              bind:this={canvasElement}
              width="280"
              height="280"
              class="rounded"
              style="image-rendering: pixelated;"
            ></canvas>
          </div>
        </div>
      </div>

      <div>
        <h3 class="mb-3 text-sm font-semibold">Prediction Results</h3>
        <div class="rounded-lg border bg-muted/30 p-4">
          <div class="mb-4 flex items-center justify-between">
            <div>
              <span class="text-sm text-muted-foreground">Predicted:</span>
              <span class="ml-2 text-2xl font-bold">
                {predictedDigit === null ? "-" : predictedDigit}
              </span>
            </div>
            <div>
              <span class="text-sm text-muted-foreground">Actual:</span>
              <span class="mx-2 text-2xl font-bold">{expectedDigit}</span>
            </div>
          </div>

          <div class="space-y-2">
            {#each { length: 10 }, digit}
              <div class="flex items-center gap-2">
                <span class="w-4 text-xs font-medium">{digit}:</span>
                <Progress.Root
                  value={confidenceScores[digit] || 0}
                  max={100}
                  class="h-3 flex-1"
                />
              </div>
            {/each}
          </div>
        </div>
      </div>
    </div>
  </Card.Content>
</Card.Root>
