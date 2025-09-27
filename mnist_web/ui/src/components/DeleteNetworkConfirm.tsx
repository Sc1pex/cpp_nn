interface DeleteNetworkConfirmProps {
  onClose: () => void;
  onConfirm: (name: string) => void;
  name: string;
}

function DeleteNetworkConfirm(props: DeleteNetworkConfirmProps) {
  return (
    <div class="modal modal-open">
      <div class="modal-box">
        <h3 class="font-bold text-lg">
          Are you sure you want to delete {props.name}?
        </h3>
        <p class="py-4">
          This action cannot be undone. All associated data will be permanently
          deleted.
        </p>

        <div class="modal-action">
          <button class="btn btn-primary" on:click={props.onClose}>
            Cancel
          </button>
          <button
            class="btn btn-error"
            on:click={() => props.onConfirm(props.name)}
          >
            Delete
          </button>
        </div>
      </div>
      <div class="modal-backdrop" on:click={props.onClose}></div>
    </div>
  );
}

export default DeleteNetworkConfirm;
