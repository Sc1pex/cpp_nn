/* @refresh reload */
import { render } from "solid-js/web";
import "./index.css";
import { Route, Router } from "@solidjs/router";
import Networks from "./pages/netwokrs";

const root = document.getElementById("root");

render(
  () => (
    <Router>
      <Route path="/" component={Networks} />
    </Router>
  ),
  root!,
);
