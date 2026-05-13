<script lang="ts">
  import { ExternalLink, Trash2 } from "@lucide/svelte";
  import Card from "../lib/components/Card.svelte";
  import { Dialog } from "bits-ui";
  import NetworkCard from "$lib/components/network/NetworkCard.svelte";
  import type { Network } from "$lib/data/network";

  const data: Network[] = [
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
  ];
</script>

<div class="flex flex-col gap-12">
  {#each data as network}
    <NetworkCard {network} onDelete={(id) => console.log(`Deleting ${id}`)} />
  {/each}
</div>
