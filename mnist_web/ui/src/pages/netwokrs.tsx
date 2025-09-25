import { createResource, For, Suspense, createSignal } from "solid-js";
import { getNetroks } from "../data/temp/networks";
import ExternalLink from "lucide-solid/icons/external-link";
import Plus from "lucide-solid/icons/plus";
import CreateNetworkModal from "../components/CreateNetworkModal";

function Networks() {
  const [data] = createResource(getNetroks);
  const [showModal, setShowModal] = createSignal(false);

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
            <For each={data()}>
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
      />
    </>
  );
}

export default Networks;
