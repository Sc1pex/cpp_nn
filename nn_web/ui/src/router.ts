import { createRouter } from "sv-router";
import Networks from "./routes/Networks.svelte";
import NetworkDetails from "./routes/NetworkDetails.svelte";

export const { p, navigate, isActive, route } = createRouter({
  "/": Networks,
  "/:id": NetworkDetails,
});
