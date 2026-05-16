<script lang="ts">
  import type { Snippet } from "svelte";

  type Variant = "primary" | "danger" | "outline";

  let {
    variant = "primary",
    children,
    class: cls = "",
    ...props
  }: {
    variant?: Variant;
    children: Snippet;
    class?: string;
    [key: string]: unknown;
  } = $props();

  const variants: Record<Variant, string> = {
    primary: "bg-primary hover:bg-primary-hover text-white",
    danger: "bg-danger hover:bg-danger-hover text-white",
    outline: "border border-border hover:bg-background text-text",
  };
</script>

<button
  class="flex items-center gap-2 px-4 py-2 rounded-xl transition-colors {variants[
    variant
  ]} {cls}"
  {...props}
>
  {@render children()}
</button>
