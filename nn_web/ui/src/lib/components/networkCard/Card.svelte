<script lang="ts">
  import { type Network } from "$lib/data/network.svelte";
  import Card from "../Card.svelte";
  import Actions from "./Actions.svelte";
  import Architecture from "./Architecture.svelte";
  import Stats from "./Stats.svelte";

  interface Props {
    network: Network;
    onDelete: (networkId: number) => Promise<void>;
  }

  const { network, onDelete }: Props = $props();
</script>

<Card class="flex flex-col gap-4">
  <h2 class="font-bold text-xl">{network.name}</h2>
  <Stats {network} />
  <Architecture {network} />
  <Actions {network} {onDelete} />
</Card>
