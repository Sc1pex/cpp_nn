import { For, Suspense, createSignal } from "solid-js";
import ExternalLink from "lucide-solid/icons/external-link";
import Plus from "lucide-solid/icons/plus";
import CreateNetworkModal from "../components/CreateNetworkModal";
import { networkState } from "../data/network";

function Networks() {
  const { getNetworks, addNetwork } = networkState;
  const [showModal, setShowModal] = createSignal(false);

  const handleNewNetwork = (name: string, shape: string) => {
    const errors = [];
    const shapeArray = validateShape(shape);
    if (shapeArray === null) {
      errors.push({
        field: "shape",
        message: "Shape must be a comma-separated list of positive integers",
      });
    }
    name = name.trim();
    if (name === "") {
      errors.push({ field: "name", message: "Name cannot be empty" });
    }
    if (errors.length > 0) {
      return errors;
    }

    const err = addNetwork(name, shapeArray!);
    if (err != null) {
      return [err];
    }

    return null;
  };

  return (
    <>
      <div class="w-2xl mx-auto">
        <div class="flex flex-col gap-6 my-8">
          <div class="flex justify-end">
            <button class="btn btn-primary" onClick={() => setShowModal(true)}>
              <Plus class="w-4 h-4" />
              Create Network
            </button>
          </div>

          <Suspense fallback={<div>Loading...</div>}>
            <For each={getNetworks()}>
              {(network) => (
                <div class="card bg-base-200 shadow-sm cursor-pointer hover:shadow-md hover:bg-base-300 transition duration-200">
                  <div class="card-body text-lg">
                    <h2 class="card-title text-2xl">{network.name}</h2>
                    <p> Shape: [{network.shape.join(", ")}]</p>
                    <p> Training iterations: {network.training_iterations}</p>
                    <p> Cost: {network.cost.toFixed(4)}</p>
                    <div class="card-actions justify-end">
                      <ExternalLink />
                    </div>
                  </div>
                </div>
              )}
            </For>
          </Suspense>
        </div>
      </div>

      <CreateNetworkModal
        isOpen={showModal()}
        onClose={() => setShowModal(false)}
        onSubmit={handleNewNetwork}
      />
    </>
  );
}

function validateShape(shape: string): number[] | null {
  const parts = shape.split(",").map((s) => s.trim());
  const shapeArray: number[] = [];

  for (const part of parts) {
    const num = parseInt(part, 10);
    if (isNaN(num)) {
      return null;
    }
    shapeArray.push(num);
  }

  if (shapeArray.length !== 0) {
    return shapeArray;
  } else {
    return null;
  }
}

export default Networks;
