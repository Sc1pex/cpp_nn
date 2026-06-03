<script lang="ts">
  import NetworkCard from "$lib/components/network/NetworkCard.svelte";
  import CreateNetworkDialog from "$lib/components/CreateNetworkDialog.svelte";
  import type { Network } from "$lib/data/network";
  import type { FieldError } from "$lib/types";
  import type { Layer } from "$lib/data/layer";

  let networks = $state<Network[]>([
    {
      name: "Network 1",
      id: 1,
      layers: [784, 16, 16, 10],
      activations: ["ReLU", "ReLU", "SoftMax"],
      loss_function: "Cross Entropy",
      loss: 0.2,
      epocs: 67,
      accuracy: 6.7,
    },
    {
      name: "Network 2",
      id: 2,
      layers: [784, 32, 16, 10],
      activations: ["Sigmoid", "ReLU", "Sigmoid"],
      loss_function: "MSE",
      loss: 0.1,
      epocs: 45,
      accuracy: 87.3,
    },
    {
      name: "Network 3",
      id: 3,
      layers: [784, 64, 32, 10],
      activations: ["Tanh", "ReLU", "SoftMax"],
      loss_function: "Cross Entropy",
      loss: 0.05,
      epocs: 30,
      accuracy: 12.3,
    },
    {
      name: "Network 4",
      id: 4,
      layers: [784, 128, 64, 10],
      activations: ["ReLU", "ReLU", "SoftMax"],
      loss_function: "Cross Entropy",
      loss: 0.01,
      epocs: 20,
      accuracy: 15.6,
    },
  ]);

  const defaultName = $derived.by(() => {
    let smallest = 0;
    for (const network of networks) {
      const match = network.name.match(/Network (\d+)/);
      if (match) {
        const number = parseInt(match[1], 10);
        if (number >= smallest) {
          smallest = number;
        }
      }
    }
    return `Network ${smallest + 1}`;
  });

  function deleteNetwork(id: number) {
    networks = networks.filter((n) => n.id !== id);
  }
  function addNetwork(
    name: string,
    layers: Layer[],
    loss: string,
  ): FieldError[] {
    if (name.length < 3 || name.length > 32) {
      return [
        {
          field: "name",
          error: "Name must be between 3 and 32 characters long",
        },
      ];
    }

    let max_id = 0;
    for (let i = 0; i < networks.length; i++) {
      max_id = Math.max(max_id, networks[i].id);
    }
    networks = [
      ...networks,
      {
        name,
        id: max_id + 1,
        layers: layers.map((l) => l.neurons),
        activations: layers
          .filter((l) => l.kind !== "input")
          .map((l) => l.activation),
        loss_function: loss,
        loss: Math.random(),
        epocs: Math.round(Math.random() * 20),
        accuracy: Math.random() * 100,
      },
    ];
    return [];
  }
</script>

<div class="flex items-center justify-between mb-6">
  <h1 class="text-2xl font-bold text-text">Networks</h1>
  <CreateNetworkDialog onsubmit={addNetwork} {defaultName} />
</div>

<div class="flex flex-col gap-8">
  {#each networks as network}
    <NetworkCard {network} onDelete={deleteNetwork} />
  {/each}
</div>
