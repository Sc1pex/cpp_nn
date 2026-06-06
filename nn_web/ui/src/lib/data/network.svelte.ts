import type { FieldError } from "$lib/types";
import type { Layer } from "./layer";

export type Network = {
  name: string;
  id: number;
  layers: number[];
  activations: string[];
  loss_function: string;
  loss: number;
  epocs: number;
  accuracy: number;
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
          name: "Network 1",
          id: 1,
          layers: [784, 16, 16, 10],
          activations: ["ReLU", "ReLU", "SoftMax"],
          loss_function: "Cross Entropy",
          loss: 0.2,
          epocs: 67,
          accuracy: 6.7,
        },
        {
          name: "Network 2",
          id: 2,
          layers: [784, 32, 16, 10],
          activations: ["Sigmoid", "ReLU", "Sigmoid"],
          loss_function: "MSE",
          loss: 0.1,
          epocs: 45,
          accuracy: 87.3,
        },
        {
          name: "Network 3",
          id: 3,
          layers: [784, 16, 16, 16, 16, 16, 16, 10],
          activations: [
            "Tanh",
            "ReLU",
            "ReLU",
            "ReLU",
            "ReLU",
            "ReLU",
            "SoftMax",
          ],
          loss_function: "Cross Entropy",
          loss: 0.05,
          epocs: 30,
          accuracy: 12.3,
        },
        {
          name: "Network 4",
          id: 4,
          layers: [784, 128, 64, 10],
          activations: ["ReLU", "ReLU", "SoftMax"],
          loss_function: "Cross Entropy",
          loss: 0.01,
          epocs: 20,
          accuracy: 15.6,
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
        layers: layers.map((l) => l.neurons),
        activations: layers
          .filter((l) => l.kind !== "input")
          .map((l) => l.activation),
        loss_function: loss,
        loss: Math.random(),
        epocs: Math.round(Math.random() * 20),
        accuracy: Math.random() * 100,
      },
    ];

    return [];
  };
}

export const networkStore = new NetworkStore();
