<script lang="ts">
  import * as Dialog from "$lib/components/ui/dialog/index";
  import { Button, buttonVariants } from "./ui/button";
  import * as Select from "$lib/components/ui/select/index";
  import { Plus, Trash2 } from "@lucide/svelte";
  import { Label } from "./ui/label";
  import { Input } from "./ui/input";
  import type { FieldError } from "$lib/data/common";

  type Props = {
    onSubmit: (
      name: string,
      layer_sizes: number[],
      activations: string[],
    ) => Promise<FieldError | null>;
    defaultName: string;
  };

  let { onSubmit, defaultName }: Props = $props();
  let field_error = $state<FieldError | null>(null);

  type Layer = {
    neurons: number;
    activation: string;
    fixed: boolean;
  };
  const activationOptions = ["relu", "sigmoid", "none"] as const;

  $effect(() => {
    name = defaultName;
  });

  let name = $state("");
  let layers = $state<Layer[]>([
    { neurons: 784, activation: "none", fixed: true },
    { neurons: 10, activation: "sigmoid", fixed: true },
  ]);

  function addLayer() {
    layers = [
      ...layers.slice(0, -1),
      { neurons: 16, activation: "relu", fixed: false },
      layers[layers.length - 1],
    ];
  }

  function removeLayer(index: number) {
    if (!layers[index].fixed) {
      layers = layers.filter((_, i) => i !== index);
    }
  }

  async function handleSubmit(e: Event) {
    e.preventDefault();

    const layer_sizes = layers.map((layer) => layer.neurons);
    const activations = layers.slice(1).map((layer) => layer.activation);

    field_error = await onSubmit(name, layer_sizes, activations);
  }
</script>

<Dialog.Content>
  <Dialog.Header>
    <Dialog.Title>Create New Network</Dialog.Title>
  </Dialog.Header>
  <form class="grid gap-4 py-4" onsubmit={handleSubmit}>
    <div class="grid gap-2">
      <Label for="network-name">Network Name</Label>
      <Input
        id="network-name"
        type="text"
        bind:value={name}
        placeholder="Enter network name"
      />
      {#if field_error && field_error.field === "name"}
        <p class="text-sm text-destructive">{field_error.error}</p>
      {/if}
    </div>

    <div class="grid gap-2">
      <div class="flex items-center justify-between">
        <Label class="font-medium">Network Layers</Label>
        <Button type="button" size="sm" variant="outline" onclick={addLayer}>
          <Plus class="mr-1 size-4" />
          Add Hidden Layer
        </Button>
      </div>

      <div class="space-y-2">
        {#each layers as layer, i}
          <div class="flex items-center gap-2 rounded border bg-muted/30 p-3">
            <div class="w-16 text-sm text-muted-foreground">
              {#if i === 0}
                <span class="font-medium">Input</span>
              {:else if i === layers.length - 1}
                <span class="font-medium">Output</span>
              {:else}
                <span>Hidden {i}</span>
              {/if}
            </div>

            <div class="flex-1 {i === 0 ? '' : 'grid grid-cols-2 gap-2'}">
              <div>
                <Label for="neurons-{i}" class="mb-2">Neurons</Label>
                <Input
                  id="neurons-{i}"
                  type="number"
                  bind:value={layer.neurons}
                  disabled={layer.fixed}
                  min="1"
                />
              </div>

              {#if i !== 0}
                <div>
                  <Label for="activation-{i}" class="mb-2">Activation</Label>
                  <Select.Root type="single" bind:value={layer.activation}>
                    <Select.Trigger id="activation-{i}" class="w-full">
                      {layer.activation || "Select activation"}
                    </Select.Trigger>
                    <Select.Content>
                      {#each activationOptions as option}
                        <Select.Item value={option} label={option}>
                          {option}
                        </Select.Item>
                      {/each}
                    </Select.Content>
                  </Select.Root>
                </div>
              {/if}
            </div>

            {#if !layer.fixed}
              <Button
                type="button"
                size="sm"
                variant="ghost"
                onclick={() => removeLayer(i)}
                class="shrink-0"
              >
                <Trash2 class="size-4 text-destructive" />
              </Button>
            {:else}
              <div class="w-10"></div>
            {/if}
          </div>
        {/each}
      </div>
    </div>

    <Dialog.Footer>
      <Dialog.Close class={buttonVariants({ variant: "outline" })}>
        Cancel
      </Dialog.Close>
      <Button type="submit">Create Network</Button>
    </Dialog.Footer>
  </form>
</Dialog.Content>
