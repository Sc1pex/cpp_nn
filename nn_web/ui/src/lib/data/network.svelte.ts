import { apiUrl } from "./common";

export type TNetworkDetails = {
  id: number;
  name: string;
  created_at: string;
  layer_sizes: number[];
  correct_predictions: number;
  training_epochs: number;
  cost: number;
  activations: string[];
};

export class NetworkDetails {
  network = $state<TNetworkDetails | null>(null);
  loading = $state(false);
  error = $state<string | null>(null);

  constructor(public id: number) {
    this.fetch();
  }

  async fetch() {
    this.loading = true;
    this.error = null;
    try {
      const response = await fetch(apiUrl(`networks/${this.id}`));
      if (!response.ok) {
        throw new Error("Network not found");
      }
      this.network = await response.json();
    } catch (e) {
      this.error = e instanceof Error ? e.message : "Failed to load network";
    } finally {
      this.loading = false;
    }
  }
}
