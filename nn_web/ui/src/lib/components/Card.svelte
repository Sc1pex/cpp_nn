<script lang="ts">
  import type { Snippet } from "svelte";

  interface Props {
    class: string;
    children?: Snippet;
  }

  let { class: cls = "", children }: Props = $props();
</script>

<div class="bg-surface border border-border rounded-xl shadow-sm {cls}">
  {@render children?.()}
</div>
