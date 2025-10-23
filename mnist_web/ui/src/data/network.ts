import { createResource } from "solid-js";

export type Network = {
  name: string;
  shape: number[];
  training_iterations: number;
  cost: number;
};

const [networks, { refetch }] = createResource(getNetroks);

export const networkState = {
  getNetworks: networks,
  isFetching: () => networks.loading,
  addNetwork: async (name: string, shape: number[]) => {
    const err = await addNetwork(name, shape);
    if (!err) {
      await refetch();
    }
    return err;
  },
  deleteNetwork: async (name: string) => {
    const success = await deleteNetwork(name);
    if (success) {
      await refetch();
    }
    return success;
  },
};

async function addNetwork(name: string, shape: number[]) {
  const url = `${import.meta.env.VITE_SERVER_URL}/api/addNetwork`;
  const res = await fetch(url, {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify({ name, shape }),
  });

  if (!res.ok) {
    const err = await res.json();
    return err;
  }

  return null;
}

async function getNetroks(): Promise<Network[]> {
  const url = `${import.meta.env.VITE_SERVER_URL}/api/networks`;
  const res = await fetch(url);
  return await res.json();
}

async function deleteNetwork(name: string) {
  const url = `${import.meta.env.VITE_SERVER_URL}/api/deleteNetwork`;
  const res = await fetch(url, {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify({ name }),
  });

  if (!res.ok) {
    return false;
  }
  return true;
}
