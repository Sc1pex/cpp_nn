import { createRouter } from "sv-router";
import Networks from "./routes/Networks.svelte";

export const { p, navigate, isActive, route } = createRouter({
  "/": Networks,
});
