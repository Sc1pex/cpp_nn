<script lang="ts">
  import { Button } from "$lib/components/ui/button/index";
  import * as Card from "$lib/components/ui/card/index";
  import type { TNetworkListItem } from "$lib/data/networks.svelte";
  import { ExternalLink, Trash2 } from "@lucide/svelte";
  import { navigate } from "../../router";

  type Props = {
    network: TNetworkListItem;
    onDelete: (network: TNetworkListItem) => void;
  };
  let { network, onDelete }: Props = $props();

  const correctCount = $derived(network.correct_predictions);
  const accuracy = $derived(correctCount / 10000);
  const accuracyPercent = $derived((accuracy * 100).toFixed(1));
</script>

<Card.Root class="w-full">
  <Card.Header class="pb-4">
    <Card.Title class="text-xl font-semibold">
      {network.name}
    </Card.Title>
    <Card.Description class="text-sm">
      {network.layer_sizes.join(" ➜ ")}
    </Card.Description>
  </Card.Header>

  <Card.Content class="pb-4">
    <div class="grid grid-cols-2 gap-4">
      <div class="bg-muted rounded-lg p-4">
        <div class="text-4xl font-bold">
          {accuracyPercent}%
        </div>
        <div class="text-xs text-muted-foreground mt-1">
          {correctCount} / 10000
        </div>
      </div>

      <div class="flex flex-col gap-3">
        <div class="bg-muted rounded-lg p-3">
          <div class="text-sm font-semibold">
            {network.cost}
          </div>
          <div class="text-xs text-muted-foreground">Cost</div>
        </div>

        <div class="bg-muted rounded-lg p-3">
          <div class="text-sm font-semibold">
            {network.training_epochs}
          </div>
          <div class="text-xs text-muted-foreground">Iterations</div>
        </div>
      </div>
    </div>
  </Card.Content>

  <Card.Footer class="flex justify-between pt-4">
    <Button
      variant="destructive"
      size="sm"
      class="gap-2"
      onclick={() => onDelete(network)}
    >
      <Trash2 class="w-4 h-4" />
      Delete
    </Button>
    <Button
      size="sm"
      class="gap-2"
      type="button"
      onclick={() =>
        navigate("/:id", { params: { id: network.id.toString() } })}
    >
      <ExternalLink class="w-4 h-4" />
      View Details
    </Button>
  </Card.Footer>
</Card.Root>
