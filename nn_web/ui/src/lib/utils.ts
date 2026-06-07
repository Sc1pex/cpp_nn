import { twMerge } from "tailwind-merge";
import { clsx } from "clsx";
import type { ClassValue } from "clsx";

export const cx = (...args: ClassValue[]) => twMerge(clsx(...args));

export const FORMAT_LABELS: Record<string, string> = {
  cross_entropy: "Cross Entropy",
  mse: "MSE",
  relu: "ReLU",
  softmax: "Softmax",
  sigmoid: "Sigmoid",
  linear: "Linear",
  tanh: "Tanh"
};
