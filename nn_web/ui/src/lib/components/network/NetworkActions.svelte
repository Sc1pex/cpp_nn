<script lang="ts">
  import type { Network } from "$lib/data/network";
  import { ExternalLink, Trash2 } from "@lucide/svelte";
  import { Dialog } from "bits-ui";
  import Button from "../Button.svelte";

  interface Props {
    network: Network;
    onDelete: (networkId: number) => void;
  }

  const { network, onDelete }: Props = $props();
</script>

<Dialog.Root>
  <div class="flex items-center justify-between pt-2 border-t border-border">
    <Dialog.Trigger>
      <Button variant="danger">
        <Trash2 size={16} />
        Delete
      </Button>
    </Dialog.Trigger>
    <Button>
      <ExternalLink size={16} />
      View Details
    </Button>
  </div>

  <Dialog.Portal>
    <Dialog.Overlay class="fixed inset-0 bg-black/50 backdrop-blur-sm" />
    <Dialog.Content
      class="fixed left-1/2 top-1/2 -translate-x-1/2 -translate-y-1/2 bg-surface border border-border rounded-2xl p-6 flex flex-col gap-4 w-full max-w-md shadow-lg"
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
          <button
            class="px-4 py-2 rounded-xl border border-border text-text hover:bg-background transition-colors text-sm"
          >
            Cancel
          </button>
        </Dialog.Close>
        <Dialog.Close>
          <button
            class="flex items-center gap-2 bg-danger hover:bg-danger-hover text-white px-4 py-2 rounded-xl transition-colors text-sm"
          >
            <Trash2 size={16} />
            Delete
          </button>
        </Dialog.Close>
      </div>
    </Dialog.Content>
  </Dialog.Portal>
</Dialog.Root>
