export type LossPoint = {
  step: number;
  loss: number;
};

export type EpochSummary = {
  epoch: number;
  test_correct: number;
};

export class TrainingHistoryStore {
  lossPoints = $state<LossPoint[]>([]);
  epochSummaries = $state<EpochSummary[]>([]);
  loading = $state(false);
  networkId: number;

  constructor(networkId: number) {
    this.networkId = networkId;
    this.fetch();
  }

  async fetch() {
    this.loading = true;
    try {
      const [lossRes, sessionRes] = await Promise.all([
        fetch(`/api/networks/${this.networkId}/loss-points`),
        fetch(`/api/networks/${this.networkId}/training-sessions`),
      ]);

      if (!lossRes.ok || !sessionRes.ok) {
        console.error("Failed to fetch training history");
        return;
      }

      const [lossPoints, sessions] = await Promise.all([
        lossRes.json() as Promise<LossPoint[]>,
        sessionRes.json() as Promise<EpochSummary[]>,
      ]);

      this.lossPoints = lossPoints;
      this.epochSummaries = sessions;
    } catch (e) {
      console.error("Error fetching training history:", e);
    } finally {
      this.loading = false;
    }
  }

  appendLoss(point: LossPoint) {
    this.lossPoints = [...this.lossPoints, point];
  }

  appendEpoch(summary: EpochSummary) {
    this.epochSummaries = [...this.epochSummaries, summary];
  }

  clear() {
    this.lossPoints = [];
    this.epochSummaries = [];
  }
}
