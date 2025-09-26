import { createResource } from "solid-js";

export type Network = {
  name: string;
  shape: number[];
  training_iterations: number;
  cost: number;
};

const networksArr: Network[] = [
  {
    name: "Network #1",
    shape: [784, 16, 16, 10],
    training_iterations: 1000,
    cost: 0.2,
  },
];

const [networks, { refetch }] = createResource(getNetroks);

export const networkState = {
  getNetworks: () => networks(),
  addNetwork: (name: string, shape: number[]) => {
    const err = addNetwork(name, shape);
    if (err === null) {
      refetch();
    }
    return err;
  },
  deleteNetwork: (name: string) => {
    const index = networksArr.findIndex((net) => net.name === name);
    if (index !== -1) {
      networksArr.splice(index, 1);
      refetch();
    }
  },
};

function addNetwork(name: string, shape: number[]) {
  // Validate shape
  if (shape.length < 2) {
    return {
      field: "shape",
      message: "Shape must have at least two layers",
    };
  }
  if (shape[0] !== 784) {
    return {
      field: "shape",
      message: "Input layer must have 784 neurons (28x28 pixels)",
    };
  }
  if (shape[shape.length - 1] !== 10) {
    return {
      field: "shape",
      message: "Output layer must have 10 neurons (digits 0-9)",
    };
  }

  for (let i = 1; i < shape.length - 1; i++) {
    if (shape[i] <= 0 || !Number.isInteger(shape[i])) {
      return {
        field: "shape",
        message: "All layers must have a positive integer number of neurons",
      };
    }
  }

  // Names should be unique
  if (networksArr.some((net) => net.name === name)) {
    return {
      field: "name",
      message: "Network name must be unique",
    };
  }

  networksArr.push({
    name,
    shape,
    training_iterations: 0,
    cost: 0,
  });

  return null;
}

export function getNetroks(): Promise<Network[]> {
  return new Promise((resolve) => {
    setTimeout(() => {
      resolve([...networksArr]);
    }, 500);
  });
}
