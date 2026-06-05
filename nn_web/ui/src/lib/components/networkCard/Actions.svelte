<script lang="ts">
  import type { Network } from "$lib/data/network.svelte";
  import { ExternalLink, Trash2 } from "@lucide/svelte";
  import { Dialog } from "bits-ui";
  import Button from "../Button.svelte";
  import { cx } from "$lib/utils";
  import { navigate } from "../../../router";

  interface Props {
    network: Network;
    onDelete: (networkId: number) => Promise<void>;
  }

  const { network, onDelete }: Props = $props();

  let dialogOpen = $state(false);
  let isDeleting = $state(false);

  async function handleDelete() {
    isDeleting = true;
    await onDelete(network.id);
    isDeleting = false;
    dialogOpen = false;
  }
</script>

<Dialog.Root bind:open={dialogOpen}>
  <div class="flex items-center justify-between pt-2 border-t border-border">
    <Dialog.Trigger>
      <Button variant="danger">
        <Trash2 size={16} />
        Delete
      </Button>
    </Dialog.Trigger>
    <Button
      onclick={() =>
        navigate("/:id", { params: { id: network.id.toString() } })}
    >
      <ExternalLink size={16} />
      View Details
    </Button>
  </div>

  <Dialog.Portal>
    <Dialog.Overlay class="fixed inset-0 bg-black/50 backdrop-blur-sm" />
    <Dialog.Content
      class={cx(
        "fixed left-1/2 top-1/2 -translate-x-1/2 -translate-y-1/2",
        "bg-surface border border-border rounded-2xl shadow-lg",
        "p-6",
        "flex flex-col gap-4",
        "w-full max-w-md",
      )}
    >
      <div>
        <Dialog.Title class="text-lg font-bold text-text"
          >Delete {network.name}?</Dialog.Title
        >
        <Dialog.Description class="text-muted text-sm mt-1">
          This action cannot be undone. The network and all its data will be
          permanently removed.
        </Dialog.Description>
      </div>

      <div
        class="flex items-center justify-end gap-3 pt-2 border-t border-border"
      >
        <Dialog.Close>
          <Button variant="outline" class="text-sm" disabled={isDeleting}
            >Cancel</Button
          >
        </Dialog.Close>
        <Button
          variant="danger"
          class="text-sm"
          onclick={handleDelete}
          disabled={isDeleting}
        >
          <Trash2 size={16} />
          {#if isDeleting}
            Deleting...
          {:else}
            Delete
          {/if}
        </Button>
      </div>
    </Dialog.Content>
  </Dialog.Portal>
</Dialog.Root>
