<script lang="ts">
  import { Plus, Trash2 } from "@lucide/svelte";
  import { Dialog } from "bits-ui";
  import Button from "./Button.svelte";
  import Select from "./Select.svelte";
  import { cx } from "$lib/utils";

  const lossFunctions = [
    { value: "cross_entropy", label: "Cross Entropy" },
    { value: "mse", label: "MSE" },
  ];

  type HiddenActivation = "relu" | "sigmoid" | "linear";
  type OutputActivation = "softmax" | "sigmoid" | "linear";

  type InputLayer = {
    kind: "input";
    neurons: 784;
    fixed: true;
  };
  type HiddenLayer = {
    kind: "hidden";
    neurons: number;
    fixed: false;
    activation: HiddenActivation;
  };
  type OutputLayer = {
    kind: "output";
    neurons: 10;
    fixed: true;
    activation: OutputActivation;
  };

  function activationOptionsForLayer(layer: Layer) {
    if (layer.kind === "input") {
      return [];
    } else if (layer.kind === "hidden") {
      return ["relu", "sigmoid", "linear"];
    } else {
      return ["softmax", "sigmoid", "linear"];
    }
  }

  type Layer = InputLayer | HiddenLayer | OutputLayer;
  let layers = $state<Layer[]>([
    { kind: "input", neurons: 784, fixed: true },
    { kind: "output", neurons: 10, activation: "softmax", fixed: true },
  ]);
  function addHiddenLayer() {
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

  $inspect(layers);
</script>

<Dialog.Root>
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
        "flex flex-col gap-4",
        "w-full max-w-lg",
        "data-placeholder:text-text/50",
      )}
    >
      <div>
        <Dialog.Title class="text-xl font-bold text-text"
          >Create new network</Dialog.Title
        >
      </div>

      <div class="flex gap-4">
        <div class="flex-1">
          <p class="font-semibold">Network name</p>
          <input
            class={cx(
              "bg-background rounded-lg border border-border",
              "px-3 py-2",
              "text-sm",
              "transition-colors hover:border-primary",
              "focus:outline-none focus:border-primary",
            )}
          />
        </div>
        <div class="flex-1">
          <p class="font-semibold">Loss Function</p>
          <Select
            items={lossFunctions}
            placeholder="Loss function"
            class="w-full"
          />
        </div>
      </div>

      <div>
        <div class="flex justify-between items-center">
          <p class="font-semibold">Network layers</p>

          <Button variant="primary" onclick={addHiddenLayer}
            >Add hidden layer</Button
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
                  type="number"
                  min={1}
                  class={cx(
                    "rounded-lg border border-border bg-background",
                    "text-text text-sm",
                    "w-full",
                    "px-3 py-2",
                    "transition-colors hover:border-primary",
                    "focus:outline-none focus:border-primary",
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
          <Button variant="outline">Cancel</Button>
        </Dialog.Close>
        <Dialog.Close>
          <Button>Create network</Button>
        </Dialog.Close>
      </div>
    </Dialog.Content>
  </Dialog.Portal>
</Dialog.Root>
