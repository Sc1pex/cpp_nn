export type Sample = {
  expected_output: number;
  input: number[];
};

export type PredictionResult = {
  prediction: number;
  scores: number[];
  loss: number;
};

async function delay(ms: number) {
  return new Promise((res) => setTimeout(res, ms));
}

export async function getSample(
  datasetType: "train" | "test",
  index: number,
): Promise<Sample> {
  await delay(200);

  const expected_output = index % 10;
  const input = new Array(28 * 28).fill(0);

  for (let y = 0; y < 28; y++) {
    for (let x = 0; x < 28; x++) {
      input[y * 28 + x] = Math.floor(Math.random() * 255);
    }
  }

  return { expected_output, input };
}

export async function getPrediction(
  networkId: number,
  datasetType: "train" | "test",
  index: number,
): Promise<PredictionResult> {
  await delay(400);

  const expected_output = index % 10;

  const prediction =
    Math.random() > 0.2 ? expected_output : Math.floor(Math.random() * 10);

  const rawScores = Array(10)
    .fill(0)
    .map(() => Math.random() * 10);

  rawScores[prediction] = 70 + Math.random() * 30;

  const sum = rawScores.reduce((a, b) => a + b, 0);
  const scores = rawScores.map((s) => Math.round((s / sum) * 100));

  return {
    prediction,
    scores,
    loss: Math.random() * 0.1,
  };
}
