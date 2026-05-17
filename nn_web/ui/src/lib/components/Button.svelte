<script lang="ts">
  import { cx } from "$lib/utils";
  import type { Snippet } from "svelte";
  import type { HTMLButtonAttributes } from "svelte/elements";

  type Variant = "primary" | "danger" | "ghost" | "outline";
  interface Props extends HTMLButtonAttributes {
    variant?: Variant;
    children: Snippet;
  }

  let {
    variant = "primary",
    children,
    class: cls = "",
    ...props
  }: Props = $props();

  const variants: Record<Variant, string> = {
    primary: "bg-primary hover:bg-primary-hover text-white",
    danger: "bg-danger hover:bg-danger-hover text-white",
    ghost: "border border-transparent hover:border-border text-text",
    outline: "border border-border hover:bg-background text-text",
  };
</script>

<button
  class={cx(
    "flex items-center gap-2",
    "px-4 py-2 rounded-xl",
    "transition-colors",
    variants[variant],
    cls,
  )}
  {...props}
>
  {@render children()}
</button>
