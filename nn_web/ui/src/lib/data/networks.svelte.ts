import { apiUrl, type FieldError } from "./common";

export type NetworkSummary = {
  id: number;
  name: string;
  layer_sizes: number[];
  correct_predictions: number;
  training_epochs: number;
  cost: number;
};

export class NetworkSummaries {
  networks = $state<NetworkSummary[]>([]);
  loading = $state(false);

  constructor() {
    this.fetch();
  }

  async fetch() {
    this.loading = true;
    const response = await fetch(apiUrl("networks")).then((res) => res.json());
    this.networks = response.networks;
    this.loading = false;
  }

  async add(name: string, layer_sizes: number[], activations: string[]): Promise<FieldError | null> {
    try {
      const res = await fetch(apiUrl("networks"), {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({ name, layer_sizes, activations }),
      });

      if (!res.ok) {
        const err = await res.json();
        return err;
      }
      return null;
    } catch (e) {
      return {
        field: "network",
        error: "Failed to add network",
      };
    }
  }

  async delete(id: number): Promise<FieldError | null> {
    try {
      const res = await fetch(apiUrl(`networks/${id}`), {
        method: "DELETE",
      });

      if (!res.ok) {
        const err = await res.json();
        return err;
      }
      return null;
    } catch (e) {
      return {
        field: "network",
        error: "Failed to delete network",
      };
    }
  }
}
