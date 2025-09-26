import X from "lucide-solid/icons/x";
import { createSignal } from "solid-js";

interface CreateNetworkModalProps {
  isOpen: boolean;
  onClose: () => void;
  onSubmit: (
    name: string,
    shape: string,
  ) => { field: string; message: string }[] | null;
}

function CreateNetworkModal(props: CreateNetworkModalProps) {
  const [networkName, setNetworkName] = createSignal("");
  const [networkShape, setNetworkShape] = createSignal("");
  const [errors, setErrors] = createSignal<{ field: string; message: string }[]>([]);

  const getFieldError = (fieldName: string) => {
    return errors().find(error => error.field === fieldName)?.message;
  };

  const handleSubmit = (e: Event) => {
    e.preventDefault();

    const validationErrors = props.onSubmit(networkName(), networkShape());

    if (validationErrors && validationErrors.length > 0) {
      setErrors(validationErrors);
      return;
    }

    // Clear errors and reset form on successful submission
    setErrors([]);
    setNetworkName("");
    setNetworkShape("");
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
                  class={`input input-bordered w-full ${getFieldError('name') ? 'input-error' : ''}`}
                  value={networkName()}
                  onInput={(e) => setNetworkName(e.currentTarget.value)}
                />
                {getFieldError('name') && (
                  <label class="label">
                    <span class="label-text-alt text-error">{getFieldError('name')}</span>
                  </label>
                )}
              </div>

              <div class="form-control w-full mb-4">
                <label class="label">
                  <span class="label-text">Network Shape</span>
                </label>
                <input
                  type="text"
                  placeholder="e.g. 784,128,64,10"
                  class={`input input-bordered w-full ${getFieldError('shape') ? 'input-error' : ''}`}
                  value={networkShape()}
                  onInput={(e) => setNetworkShape(e.currentTarget.value)}
                />
                {getFieldError('shape') && (
                  <label class="label">
                    <span class="label-text-alt text-error">{getFieldError('shape')}</span>
                  </label>
                )}
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
