import express from "express";
const app = express();
const port = 8080;

import path from "path";
import { getData } from "./firebase.js";

app.use(express.static("static"));

app.get("/get", async (_, res) => {
  res.json(await getData());
});

app.get("/", (_, res) => {
  res.sendFile(path.resolve("html/index.html"));
});

export function init() {
  app.listen(port, () => {
    console.log(`App listening on port ${port}`);
  });
}
