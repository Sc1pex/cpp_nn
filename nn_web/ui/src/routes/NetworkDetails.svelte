<script lang="ts">
  import { NetworkDetails } from "$lib/data/network.svelte";
  import { Button } from "$lib/components/ui/button";
  import * as Card from "$lib/components/ui/card";
  import { ArrowLeft, MoveRight } from "@lucide/svelte";
  import { navigate, route } from "../router";
  import Prediction from "$lib/components/Prediction.svelte";

  const networkId = parseInt(route.getParams("/:id").id);
  const networkDetails = new NetworkDetails(networkId);
  const network = $derived(networkDetails.network);
</script>

{#if networkDetails.loading}
  <div>Loading...</div>
{:else if networkDetails.error}
  <div class="text-red-500">Error: {networkDetails.error}</div>
{:else if network}
  <div class="mb-6 flex items-center gap-4">
    <Button variant="ghost" size="icon" onclick={() => navigate("/")}>
      <ArrowLeft />
    </Button>
    <h1 class="text-3xl font-bold">{network.name}</h1>
  </div>

  <Card.Root>
    <Card.Header>
      <Card.Title>Architecture</Card.Title>
    </Card.Header>
    <Card.Content>
      <div class="overflow-x-auto">
        <div
          class="inline-flex w-full min-w-max items-center justify-center gap-4 py-8"
        >
          <div class="flex flex-col items-center gap-2">
            <span class="text-sm text-muted-foreground">Input</span>
            <div class="rounded-lg bg-primary/20 px-8 py-6 text-center">
              <div class="text-2xl font-bold">{network.layer_sizes[0]}</div>
            </div>
            <span class="text-xs text-muted-foreground">-</span>
          </div>

          {#each network.layer_sizes.slice(1, -1) as hiddenSize, i}
            <div class="text-muted-foreground">
              <MoveRight class="h-6 w-6" />
            </div>

            <div class="flex flex-col items-center gap-2">
              <span class="text-sm text-muted-foreground">Hidden {i + 1}</span>
              <div class="rounded-lg bg-primary/20 px-8 py-6 text-center">
                <div class="text-2xl font-bold">{hiddenSize}</div>
              </div>
              <span class="text-xs text-muted-foreground"
                >{network.activations[i]}</span
              >
            </div>
          {/each}

          <div class="text-muted-foreground">
            <MoveRight class="h-6 w-6" />
          </div>

          <div class="flex flex-col items-center gap-2">
            <span class="text-sm text-muted-foreground">Output</span>
            <div class="rounded-lg bg-primary/20 px-8 py-6 text-center">
              <div class="text-2xl font-bold">
                {network.layer_sizes[network.layer_sizes.length - 1]}
              </div>
            </div>
            <span class="text-xs text-muted-foreground"
              >{network.activations[network.activations.length - 1]}</span
            >
          </div>
        </div>
      </div>
    </Card.Content>
  </Card.Root>

  <div class="my-8"></div>
  <Prediction {networkId} />
{/if}
