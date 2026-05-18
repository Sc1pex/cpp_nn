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

export type HiddenActivation = "relu" | "sigmoid" | "linear";
export type OutputActivation = "softmax" | "sigmoid" | "linear";

export type InputLayer = {
  kind: "input";
  neurons: 784;
  fixed: true;
};
export type HiddenLayer = {
  kind: "hidden";
  neurons: number;
  fixed: false;
  activation: HiddenActivation;
};
export type OutputLayer = {
  kind: "output";
  neurons: 10;
  fixed: true;
  activation: OutputActivation;
};

export type Layer = InputLayer | HiddenLayer | OutputLayer;
