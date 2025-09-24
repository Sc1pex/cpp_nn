import type { Network } from "../network";

const networks: Network[] = [
  {
    id: 1,
    shape: [784, 16, 16, 10],
    training_iterations: 1000,
    cost: 0.2,
  },
  {
    id: 2,
    shape: [784, 12, 13, 14, 10],
    training_iterations: 2000,
    cost: 0.15,
  },
  {
    id: 3,
    shape: [784, 32, 16, 10],
    training_iterations: 1500,
    cost: 0.1,
  },
  {
    id: 4,
    shape: [784, 64, 32, 16, 10],
    training_iterations: 3000,
    cost: 0.05,
  },
  {
    id: 5,
    shape: [784, 128, 64, 32, 16, 10],
    training_iterations: 5000,
    cost: 0.03,
  },
  {
    id: 6,
    shape: [784, 256, 128, 64, 32, 16, 10],
    training_iterations: 8000,
    cost: 0.02,
  },
  {
    id: 7,
    shape: [784, 12, 12, 10],
    training_iterations: 1200,
    cost: 0.18,
  },
  {
    id: 8,
    shape: [784, 20, 15, 10],
    training_iterations: 2500,
    cost: 0.12,
  },
  {
    id: 9,
    shape: [784, 40, 20, 10],
    training_iterations: 3500,
    cost: 0.08,
  },
  {
    id: 10,
    shape: [784, 80, 40, 20, 10],
    training_iterations: 6000,
    cost: 0.04,
  },
];

export function getNetroks(): Promise<Network[]> {
  return new Promise((resolve) => {
    setTimeout(() => {
      resolve(networks);
    }, 500);
  });
}
