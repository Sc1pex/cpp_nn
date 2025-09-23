/* @refresh reload */
import { render } from "solid-js/web";
import "./index.css";

const root = document.getElementById("root");

function App() {
  return <div>Hello world</div>;
}

render(() => <App />, root!);
