export type Sample = {
  expected_output: number;
  input: number[];
};

export type PredictionResult = {
  prediction: number;
  scores: number[];
  loss: number;
};

export async function getSample(
  datasetType: "train" | "test",
  index: number,
): Promise<Sample> {
  const res = await fetch(`/api/data/${datasetType}/${index}`);
  if (!res.ok) throw new Error("Failed to fetch sample");
  return await res.json();
}

export async function getPrediction(
  networkId: number,
  datasetType: "train" | "test",
  index: number,
): Promise<PredictionResult> {
  const res = await fetch(
    `/api/networks/${networkId}/predict/${datasetType}/${index}`,
    {
      method: "POST",
    },
  );

  if (!res.ok) throw new Error("Failed to predict");
  const data = await res.json();

  const rawScores: number[] = data.output;

  let prediction = 0;
  let maxScore = -Infinity;
  for (let i = 0; i < rawScores.length; i++) {
    if (rawScores[i] > maxScore) {
      maxScore = rawScores[i];
      prediction = i;
    }
  }

  const scores = rawScores.map((s) =>
    Math.round(Math.max(0, Math.min(1, s)) * 100),
  );

  return {
    prediction,
    scores,
    loss: data.loss,
  };
}
