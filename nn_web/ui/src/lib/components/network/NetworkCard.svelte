<script lang="ts">
  import { type Network } from "$lib/data/network";
  import Card from "../Card.svelte";
  import NetworkActions from "./NetworkActions.svelte";
  import NetworkArchitecture from "./NetworkArchitecture.svelte";
  import NetworkStats from "./NetworkStats.svelte";

  interface Props {
    network: Network;
    onDelete: (networkId: number) => void;
  }

  const { network, onDelete }: Props = $props();
</script>

<Card class="p-6 flex flex-col gap-4">
  <h2 class="font-bold text-xl">{network.name}</h2>
  <NetworkStats {network} />
  <NetworkArchitecture {network} />
  <NetworkActions {network} {onDelete} />
</Card>
