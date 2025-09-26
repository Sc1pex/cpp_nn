import ExternalLink from "lucide-solid/icons/external-link";
import Trash2 from "lucide-solid/icons/trash-2";
import { type Network } from "../data/network";

interface NetworkComponentProps {
  network: Network;
  onView: (name: string) => void;
  onDelete: (name: string) => void;
}

function NetworkComponent({
  network,
  onView,
  onDelete,
}: NetworkComponentProps) {
  const handleViewClick = (e: MouseEvent) => {
    e.stopPropagation();
    onView(network.name);
  };

  const handleDeleteClick = (e: MouseEvent) => {
    e.stopPropagation();
    onDelete(network.name);
  };

  return (
    <div class="card bg-base-200 shadow-sm hover:shadow-lg transition duration-200">
      <div class="card-body text-lg">
        <h2 class="card-title text-2xl">{network.name}</h2>
        <p> Shape: [{network.shape.join(", ")}]</p>
        <p> Training iterations: {network.training_iterations}</p>
        <p> Cost: {network.cost.toFixed(4)}</p>
        <div class="card-actions justify-end gap-2">
          <button
            class="btn btn-sm btn-error btn-square"
            onClick={handleDeleteClick}
            title="Delete network"
          >
            <Trash2 />
          </button>
          <button
            class="btn btn-sm btn-ghost btn-square"
            onClick={handleViewClick}
            title="View network"
          >
            <ExternalLink />
          </button>
        </div>
      </div>
    </div>
  );
}

export default NetworkComponent;
