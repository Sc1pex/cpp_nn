import { For, Suspense, createSignal } from "solid-js";
import Plus from "lucide-solid/icons/plus";
import CreateNetworkModal from "../components/CreateNetworkModal";
import { networkState } from "../data/network";
import NetworkComponent from "../components/NetworkComponent";
import DeleteNetworkConfirm from "../components/DeleteNetworkConfirm";

function Networks() {
  const { getNetworks, addNetwork, isFetching, deleteNetwork } = networkState;
  const [showCreateModal, setShowCreateModal] = createSignal(false);
  const [deleteModalNetworkName, setDeleteModalNetworkName] = createSignal<
    string | null
  >(null);

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
            <button
              class="btn btn-primary"
              onClick={() => setShowCreateModal(true)}
              disabled={isFetching()}
            >
              <Plus class="w-4 h-4" />
              Create Network
            </button>
          </div>

          <Suspense fallback={<div>Loading...</div>}>
            <For each={getNetworks()}>
              {(network) => (
                <NetworkComponent
                  network={network}
                  onView={() => {}}
                  onDelete={() => {
                    setDeleteModalNetworkName(network.name);
                  }}
                />
              )}
            </For>
          </Suspense>
        </div>
      </div>

      {showCreateModal() && (
        <CreateNetworkModal
          onClose={() => setShowCreateModal(false)}
          onSubmit={handleNewNetwork}
        />
      )}

      {deleteModalNetworkName() && (
        <DeleteNetworkConfirm
          onClose={() => setDeleteModalNetworkName(null)}
          name={deleteModalNetworkName()!}
          onConfirm={(name) => {
            deleteNetwork(name);
          }}
        />
      )}
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
