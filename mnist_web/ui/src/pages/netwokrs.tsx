import { createResource, For, Suspense } from "solid-js";
import { getNetroks } from "../data/temp/networks";

function Networks() {
  const [data] = createResource(getNetroks);

  return (
    <div class="w-2xl mx-auto">
      <div class="flex flex-col gap-6 mt-8">
        <Suspense fallback={<div>Loading...</div>}>
          <For each={data()}>
            {(item) => (
              <div class="card bg-base-200 shadow-sm">
                <div class="card-body">
                  <h2 class="card-title">Network #{item.id}</h2>
                </div>
              </div>
            )}
          </For>
        </Suspense>
      </div>
    </div>
  );
}

export default Networks;
