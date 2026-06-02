<script lang="ts">
  import { Plus, Trash2 } from "@lucide/svelte";
  import { Dialog } from "bits-ui";
  import Button from "./Button.svelte";
  import Select from "./Select.svelte";
  import { cx } from "$lib/utils";
  import type { Layer } from "$lib/data/network";
  import type { FieldError } from "$lib/types";

  const lossValues = [
    { value: "cross_entropy", label: "Cross Entropy" },
    { value: "mse", label: "MSE" },
  ];

  function activationOptionsForLayer(layer: Layer) {
    if (layer.kind === "input") {
      return [];
    } else if (layer.kind === "hidden") {
      return ["relu", "sigmoid", "linear"];
    } else {
      return ["softmax", "sigmoid", "linear"];
    }
  }

  function defaultLayers(): Layer[] {
    return [
      { kind: "input", neurons: 784, fixed: true },
      { kind: "output", neurons: 10, activation: "softmax", fixed: true },
    ];
  }

  let layers = $state<Layer[]>(defaultLayers());
  let networkName = $state("");
  let lossFunction = $state("cross_entropy");
  let errors = $state<FieldError[]>([]);
  let dialogOpen = $state<boolean>(false);

  function defaultState() {
    layers = defaultLayers();
    networkName = "";
    errors = [];
    dialogOpen = false;
  }

  let maxLayers = 6;
  let addLayerDisabled = $derived(layers.length >= maxLayers);

  function addHiddenLayer() {
    if (layers.length >= maxLayers) {
      return;
    }

    layers = [
      ...layers.slice(0, -1),
      { kind: "hidden", neurons: 16, activation: "relu", fixed: false },
      layers[layers.length - 1],
    ];
  }
  function removeLayer(idx: number) {
    if (!layers[idx].fixed) {
      layers = layers.filter((_, i) => i !== idx);
    }
  }

  const {
    onsubmit: handleSubmit,
  }: {
    onsubmit: (name: string, layers: Layer[], loss: string) => FieldError[];
  } = $props();

  function onsubmit(e: SubmitEvent) {
    e.preventDefault();
    errors = handleSubmit(networkName, layers, lossFunction);

    if (errors.length != 0) {
      return;
    }

    defaultState();
  }

  const getFieldError = (fieldName: string) =>
    errors.find((f) => f.field === fieldName);

  $inspect(errors);
</script>

<Dialog.Root bind:open={dialogOpen}>
  <Dialog.Trigger>
    <Button>
      <Plus size={16} />
      New Network
    </Button>
  </Dialog.Trigger>

  <Dialog.Portal>
    <Dialog.Overlay class="fixed inset-0 bg-black/50 backdrop-blur-sm" />
    <Dialog.Content
      class={cx(
        "fixed left-1/2 top-1/2 -translate-x-1/2 -translate-y-1/2",
        "bg-surface border border-border rounded-2xl shadow-lg",
        "p-6",
        "w-full max-w-lg",
        "data-placeholder:text-text/50",
      )}
    >
      <form class="flex flex-col gap-4" {onsubmit}>
        <div>
          <Dialog.Title class="text-xl font-bold text-text"
            >Create new network</Dialog.Title
          >
        </div>

        <div class="flex gap-4">
          <div class="flex-1">
            <p class="font-semibold">Network name</p>
            <input
              bind:value={networkName}
              maxlength={32}
              class={cx(
                "bg-background rounded-lg border border-border",
                "px-3 py-2",
                "text-sm",
                "transition-colors hover:border-primary",
                "focus:outline-none focus:border-primary",
              )}
            />
            <span class="text-danger text-sm"
              >{getFieldError("name")?.error}</span
            >
          </div>
          <div class="flex-1">
            <p class="font-semibold">Loss Function</p>
            <Select
              items={lossValues}
              class="w-full"
              default_value="cross_entropy"
              bind:selected={lossFunction}
            />
          </div>
        </div>

        <div>
          <div class="flex justify-between items-center">
            <p class="font-semibold">Network layers</p>

            <Button
              disabled={addLayerDisabled}
              type="button"
              variant="primary"
              onclick={addHiddenLayer}>Add hidden layer</Button
            >
          </div>

          <div class="flex flex-col gap-2 mt-2">
            {#each layers as layer, i}
              <div
                class="flex items-center gap-2 rounded-lg border border-border bg-background p-3"
              >
                <div class="w-16 text-muted">
                  {#if layer.kind == "input"}
                    <span>Input</span>
                  {:else if layer.kind == "hidden"}
                    <span>Hidden</span>
                  {:else}
                    <span>Output</span>
                  {/if}
                </div>

                <div class="flex-1">
                  Neurons
                  <input
                    bind:value={layer.neurons}
                    disabled={layer.fixed}
                    type="number"
                    min={1}
                    max={1024}
                    required={true}
                    class={cx(
                      "rounded-lg border border-border bg-background",
                      "text-text text-sm",
                      "w-full",
                      "px-3 py-2",
                      "transition-colors hover:!disabled:border-primary",
                      "focus:outline-none focus:border-primary",
                      "disabled:opacity-50 disabled:cursor-not-allowed disabled:hover:border-border disabled:bg-background/50",
                    )}
                  />
                </div>
                {#if layer.kind != "input"}
                  <div class="flex-1">
                    Activation
                    <Select
                      items={activationOptionsForLayer(layer)}
                      placeholder="Activation"
                      class="w-full"
                      bind:selected={layer.activation}
                    />
                  </div>
                {/if}

                {#if !layer.fixed}
                  <Button
                    type="button"
                    variant="ghost"
                    onclick={() => removeLayer(i)}
                    class="px-2 py-2"
                  >
                    <Trash2 size={16} class="text-danger" />
                  </Button>
                {:else}
                  <div class="w-8"></div>
                {/if}
              </div>
            {/each}
          </div>
        </div>

        <div
          class="flex items-center justify-end gap-3 pt-2 border-t border-border"
        >
          <Dialog.Close>
            <Button type="reset" variant="outline" onclick={defaultState}
              >Cancel</Button
            >
          </Dialog.Close>
          <Button type="submit">Create network</Button>
        </div>
      </form>
    </Dialog.Content>
  </Dialog.Portal>
</Dialog.Root>
