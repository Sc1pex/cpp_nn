import type { FieldError } from "$lib/types";
import type { Layer } from "./layer";

export type Network = {
  id: number;
  name: string;
  created_at: string;
  layer_sizes: number[];
  activations: string[];
  loss: string;
  correct_predictions: number;
  training_epochs: number;
  cost: number;
};

class NetworkStore {
  networks = $state<Network[]>([]);
  loading = $state(false);

  constructor() {
    this.fetch();
  }

  async fetch() {
    this.loading = true;
    try {
      const res = await fetch("/api/networks");
      if (res.ok) {
        this.networks = await res.json();
      } else {
        console.error("Failed to fetch networks", await res.text());
      }
    } catch (e) {
      console.error(e);
    }
    this.loading = false;
  }

  delete = async (id: number) => {
    try {
      const res = await fetch("/api/networks/" + id, { method: "DELETE" });
      if (res.ok) {
        this.networks = this.networks.filter((n) => n.id !== id);
      }
    } catch (e) {
      console.error(e);
    }
  };

  create = async (
    name: string,
    layers: Layer[],
    loss: string,
  ): Promise<FieldError[]> => {
    try {
      const payload = {
        name,
        layer_sizes: layers.map((l) => l.neurons),
        activations: layers
          .filter((l) => l.kind !== "input")
          .map((l) => l.activation),
        loss,
      };

      const res = await fetch("/api/networks", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(payload),
      });

      if (res.ok) {
        await this.fetch();
        return [];
      } else {
        const data = await res.json();
        if (data.field && data.error) {
          return [data];
        }
        return [{ field: "name", error: data.message || "Unknown error" }];
      }
    } catch (e) {
      console.error(e);
      return [{ field: "name", error: "Failed to connect to backend" }];
    }
  };
}

export const networkStore = new NetworkStore();
