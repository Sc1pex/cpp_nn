import { apiUrl } from "./common";

export type PredictionResult = {
  scores: number[];
  prediction: number;
};

export class Prediction {
  networkId = $state<number>();
  output = $state<number[] | null>(null);
  loading = $state(false);

  constructor(networkId: number) {
    this.networkId = networkId;
  }

  async predict(input: number[]) {
    if (this.networkId === null) {
      throw new Error("Network ID is not set");
    }

    this.loading = true;
    const response = await fetch(apiUrl(`networks/${this.networkId}/predict`), {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify({ input: input }),
    });

    if (!response.ok) {
      this.loading = false;
    }

    const data = await response.json();
    const scores = data.output.map((x: number) => x * 100);

    let bestScore = scores[0];
    let prediction = 0;
    for (let i = 1; i < scores.length; i++) {
      if (scores[i] > bestScore) {
        bestScore = scores[i];
        prediction = i;
      }
    }

    return {
      scores: scores,
      prediction: prediction,
    };
  }
}

export type TSample = {
  input: number[];
  expected_output: number;
};

export async function getSample(source: "test" | "train", index: number) {
  const response = await fetch(apiUrl(`data/${source}/${index}`));
  if (!response.ok) {
    throw new Error(`Error fetching image: ${response.statusText}`);
  }
  const data = await response.json();
  return data as TSample;
}
