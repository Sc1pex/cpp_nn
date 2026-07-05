type Datapoint = {
  idx: number;
  value: number;
};

export class TrainingHistoryStore {
  lossPoints = $state<Datapoint[]>([]);
  epochSummaries = $state<Datapoint[]>([]);
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
        fetch(`/api/networks/${this.networkId}/test-points`),
      ]);

      if (!lossRes.ok || !sessionRes.ok) {
        console.error("Failed to fetch training history");
        return;
      }

      const [lossPoints, sessions] = await Promise.all([
        lossRes.json() as Promise<number[]>,
        sessionRes.json() as Promise<number[]>,
      ]);

      this.lossPoints = lossPoints.map((value, i) => ({ idx: i + 1, value }));
      this.epochSummaries = sessions.map((value, i) => ({ idx: i + 1, value }));


    } catch (e) {
      console.error("Error fetching training history:", e);
    } finally {
      this.loading = false;
    }
  }

  appendLoss(loss: number) {
    this.lossPoints = [
      ...this.lossPoints,
      { idx: this.lossPoints.length + 1, value: loss },
    ];
  }

  appendEpoch(tests: number) {
    this.epochSummaries = [
      ...this.epochSummaries,
      { idx: this.epochSummaries.length + 1, value: tests },
    ];
  }

  clear() {
    this.lossPoints = [];
    this.epochSummaries = [];
  }
}
