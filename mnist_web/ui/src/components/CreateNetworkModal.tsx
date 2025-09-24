import X from "lucide-solid/icons/x";

interface CreateNetworkModalProps {
  isOpen: boolean;
  onClose: () => void;
}

function CreateNetworkModal(props: CreateNetworkModalProps) {
  const handleSubmit = (e: Event) => {
    e.preventDefault();
    // TODO: Handle form submission
    console.log("Creating network...");
    props.onClose();
  };

  return (
    <>
      {props.isOpen && (
        <div class="modal modal-open">
          <div class="modal-box">
            <div class="flex justify-between items-center mb-4">
              <h3 class="font-bold text-lg">Create New Network</h3>
              <button class="btn btn-circle btn-sm" on:click={props.onClose}>
                <X class="w-4 h-4" />
              </button>
            </div>

            <form onSubmit={handleSubmit}>
              <div class="form-control w-full mb-4">
                <label class="label">
                  <span class="label-text">Network Name</span>
                </label>
                <input
                  type="text"
                  placeholder="Enter network name"
                  class="input input-bordered w-full"
                />
              </div>

              <div class="form-control w-full mb-4">
                <label class="label">
                  <span class="label-text">Network Shape</span>
                </label>
                <input
                  type="text"
                  placeholder="e.g., 784,128,64,10"
                  class="input input-bordered w-full"
                />
              </div>

              <div class="modal-action">
                <button type="button" class="btn" onClick={props.onClose}>
                  Cancel
                </button>
                <button type="submit" class="btn btn-primary">
                  Create
                </button>
              </div>
            </form>
          </div>
          <div class="modal-backdrop" onClick={props.onClose}></div>
        </div>
      )}
    </>
  );
}

export default CreateNetworkModal;

