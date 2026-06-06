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

async function delay(ms: number) {
  return new Promise((res) => setTimeout(res, ms));
}

class NetworkStore {
  networks = $state<Network[]>([]);
  loading = $state(false);

  constructor() {
    this.fetch().then(() => {
      this.networks = [
        {
          id: 1,
          name: "Network 1",
          created_at: new Date().toISOString(),
          layer_sizes: [784, 16, 16, 10],
          activations: ["relu", "relu", "softmax"],
          loss: "cross_entropy",
          correct_predictions: 670,
          training_epochs: 67,
          cost: 0.2,
        },
        {
          id: 2,
          name: "Network 2",
          created_at: new Date().toISOString(),
          layer_sizes: [784, 32, 16, 10],
          activations: ["sigmoid", "relu", "sigmoid"],
          loss: "mse",
          correct_predictions: 8730,
          training_epochs: 45,
          cost: 0.1,
        },
        {
          id: 3,
          name: "Network 3",
          created_at: new Date().toISOString(),
          layer_sizes: [784, 16, 16, 16, 16, 16, 16, 10],
          activations: [
            "tanh",
            "relu",
            "relu",
            "relu",
            "relu",
            "relu",
            "softmax",
          ],
          loss: "cross_entropy",
          correct_predictions: 1230,
          training_epochs: 30,
          cost: 0.05,
        },
        {
          id: 4,
          name: "Network 4",
          created_at: new Date().toISOString(),
          layer_sizes: [784, 128, 64, 10],
          activations: ["relu", "relu", "softmax"],
          loss: "cross_entropy",
          correct_predictions: 1560,
          training_epochs: 20,
          cost: 0.01,
        },
      ];
    });
  }

  async fetch() {
    this.loading = true;
    await delay(1000);
    this.loading = false;
  }

  delete = async (id: number) => {
    await this.fetch();

    this.networks = this.networks.filter((n) => n.id !== id);
  };

  create = async (
    name: string,
    layers: Layer[],
    loss: string,
  ): Promise<FieldError[]> => {
    await this.fetch();

    if (name.length < 3 || name.length > 32) {
      return [
        {
          field: "name",
          error: "Name must be between 3 and 32 characters long",
        },
      ];
    }

    let max_id = 0;
    for (let i = 0; i < this.networks.length; i++) {
      max_id = Math.max(max_id, this.networks[i].id);
    }
    this.networks = [
      ...this.networks,
      {
        name,
        id: max_id + 1,
        created_at: new Date().toISOString(),
        layer_sizes: layers.map((l) => l.neurons),
        activations: layers
          .filter((l) => l.kind !== "input")
          .map((l) => l.activation),
        loss: loss,
        correct_predictions: Math.round(Math.random() * 10000),
        training_epochs: Math.round(Math.random() * 20),
        cost: Math.random(),
      },
    ];

    return [];
  };
}

export const networkStore = new NetworkStore();
