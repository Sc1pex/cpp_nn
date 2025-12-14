export type FieldError = {
  field: string;
  error: string;
};

export function apiUrl(path: string) {
  return `${import.meta.env.VITE_API_BASE_URL}/api/${path}`;
}
