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
