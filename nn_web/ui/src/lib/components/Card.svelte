<script lang="ts">
  import { cx } from "$lib/utils";
  import type { Snippet } from "svelte";

  interface Props {
    class: string;
    children?: Snippet;
  }

  let { class: cls = "", children }: Props = $props();
</script>

<div class={cx("bg-surface border border-border rounded-xl shadow-sm", cls)}>
  {@render children?.()}
</div>
