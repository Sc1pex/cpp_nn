<script lang="ts">
  import { Check, ChevronDown } from "@lucide/svelte";
  import { Select } from "bits-ui";

  type Props = {
    items: { value: string; label: string }[] | string[];
    class?: string;
    selected?: string;
  } & (
    | { placeholder: string; default_value?: never }
    | { placeholder?: never; default_value: string }
  );

  let {
    items: items_prop,
    placeholder,
    default_value,
    class: cls = "",
    selected = $bindable(default_value),
  }: Props = $props();

  const items = $derived(
    items_prop.map((i) => (typeof i === "string" ? { value: i, label: i } : i)),
  );
</script>

<Select.Root type="single" {items} bind:value={selected}>
  <Select.Trigger
    class="flex items-center justify-between px-3 py-2 rounded-lg border border-border bg-background text-sm hover:border-primary transition-colors focus:border-primary focus:outline-none group {cls}"
  >
    <Select.Value {placeholder} />
    <ChevronDown
      size={16}
      class="text-muted transition-transform group-data-[state=open]:rotate-180"
    />
  </Select.Trigger>

  <Select.Content
    class="bg-surface border border-border rounded-lg shadow-lg overflow-hidden z-50 w-(--bits-select-anchor-width)
    transition-all duration-100 data-[state=open]:opacity-100 data-[state=closed]:opacity-0 data-[state=open]:translate-y-0 data-[state=closed]:-translate-y-1"
  >
    {#each items as item}
      <Select.Item
        value={item.value}
        label={item.label}
        class="px-3 py-2 text-sm text-text hover:bg-background cursor-pointer transition-colors data-highlighted:bg-background data-[state=checked]:text-primary flex"
      >
        {#snippet children({ selected })}
          {item.label}
          {#if selected}
            <div class="ml-auto">
              <Check class="text-muted" />
            </div>
          {/if}
        {/snippet}
      </Select.Item>
    {/each}
  </Select.Content>
</Select.Root>
