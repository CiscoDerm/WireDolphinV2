# 🖥️ Wire_Dolphin Dashboard - Orange Cyberdefense

<p align="center">
  <img src="https://img.shields.io/badge/Framework-React.js-61DAFB?style=for-the-badge&logo=react" alt="React">
  <img src="https://img.shields.io/badge/Backend-Node.js-339933?style=for-the-badge&logo=node.js" alt="Node.js">
  <img src="https://img.shields.io/badge/WebSocket-Socket.io-010101?style=for-the-badge&logo=socket.io" alt="Socket.io">
  <img src="https://img.shields.io/badge/License-MIT-brightgreen?style=for-the-badge" alt="MIT License">
</p>

<p align="center">
  <strong>Dashboard de supervision en temps réel pour le simulateur de ransomware</strong><br>
  Développé par les étudiants de 2ème année Bachelor ISEN pour Orange Cyberdefense
</p>

---

## 📋 Table des matières

- [Vue d'ensemble](#-vue-densemble)
- [Fonctionnalités](#-fonctionnalités)
- [Architecture technique](#-architecture-technique)
- [Installation](#-installation)
- [Configuration](#-configuration)
- [Utilisation](#-utilisation)
- [API Endpoints](#-api-endpoints)
- [Sécurité](#-sécurité)
- [Développement](#-développement)

---

## 🎯 Vue d'ensemble

**Wire_Dolphin Dashboard** est une interface de supervision conçue pour centraliser et visualiser en temps réel les données collectées par le simulateur de ransomware. Il fait partie intégrante du projet WireDolphinV2 développé dans le cadre des formations Orange Cyberdefense.

### Objectifs principaux

- 📊 **Centralisation** des données récoltées par les malwares simulés
- 👁️ **Visualisation en temps réel** des attaques et comportements
- 📈 **Analyse rapide** des victimes et de leurs actions
- 🎮 **Faciliter le pilotage** cyber et la prise de décision

---

## ✨ Fonctionnalités

### 📊 Tableau de bord principal

| Fonctionnalité | Description |
|----------------|-------------|
| **Statistiques en temps réel** | Nombre de machines infectées, pays impactés, total des logs |
| **Graphiques dynamiques** | Logs par heure, répartition géographique, activité journalière |
| **Connexions WebSocket** | Affichage des clients connectés en direct |
| **Notifications toast** | Alertes pour chaque nouvelle victime détectée |

### 🗺️ Carte mondiale interactive

- Géolocalisation des victimes via **Leaflet.js** et **IP-API**
- Popups détaillés : IP, hostname, pays, ville
- Mise à jour en temps réel des nouvelles infections

### 👥 Gestion des clients

- **Table des victimes** : IP, hostname, pays, adresse MAC, timestamp
- **Replay des sessions** : Historique complet des frappes clavier
- **Reconstruction des saisies** : Analyse du comportement utilisateur

### 🔐 Système d'authentification

- Login sécurisé avec **bcrypt**
- Protection contre le brute-force via **rate limiting**
- Gestion des sessions utilisateur

---

## 🏗️ Architecture technique

### Stack technologique

**Frontend:**
- React.js 18.x - Framework SPA
- Recharts - Graphiques statistiques  
- Leaflet - Cartographie interactive
- Socket.io-client - Communication temps réel

**Backend:**
- Node.js / Express - Serveur API
- Socket.io - WebSocket server
- bcrypt - Hachage des mots de passe
- express-rate-limit - Protection anti brute-force

**Stockage:**
- JSON files - Base de données légère
- Système de fichiers local - Logs et données

### Structure des dossiers

```
wire-dolphin-dashboard/
├── backend/
│   ├── server.js           # Serveur Express principal
│   ├── secure-data/        # Données sensibles
│   │   ├── users.json      # Utilisateurs (hashés)
│   │   └── data.json       # Données des victimes
│   └── package.json
├── frontend/
│   ├── src/
│   │   ├── components/     # Composants React
│   │   ├── pages/          # Pages de l'application
│   │   └── App.js
│   └── package.json
└── README.md
```

---

## 🚀 Installation

### Prérequis

- Node.js >= 14.x
- npm ou yarn
- Debian/Ubuntu (pour la production)

### Installation locale

1. **Cloner le repository**
```bash
git clone https://github.com/orange-cyberdefense/wire-dolphin-dashboard.git
cd wire-dolphin-dashboard
```

2. **Installer les dépendances backend**
```bash
cd backend
npm install
```

3. **Installer les dépendances frontend**
```bash
cd ../frontend
npm install
```

4. **Créer les fichiers de configuration**
```bash
# Dans backend/secure-data/
touch users.json data.json
```

5. **Initialiser users.json**
```json
[
  {
    "username": "admin",
    "password": "$2b$10$..." // Hash bcrypt du mot de passe
  }
]
```

### Déploiement sur VM Debian

```bash
# 1. Installer Node.js et npm
sudo apt update
sudo apt install nodejs npm

# 2. Transférer le projet via SCP
scp -r wire-dolphin-dashboard/ user@192.168.x.x:/home/user/

# 3. Sur la VM
cd wire-dolphin-dashboard
npm install # Dans backend/ et frontend/

# 4. Build du frontend
cd frontend
npm run build

# 5. Lancer le serveur
cd ../backend
node server.js
```

---

## ⚙️ Configuration

### Variables d'environnement

Créer un fichier `.env` dans le dossier backend :

```env
PORT=3001
JWT_SECRET=votre_secret_jwt
RATE_LIMIT_WINDOW=60000
RATE_LIMIT_MAX=5
```

### Configuration réseau

Le dashboard est accessible sur :
```
http://192.168.240.249:3001
```

---

## 📖 Utilisation

### Connexion

1. Accéder à l'interface : `http://192.168.240.249:3001`
2. Se connecter avec les identifiants fournis
3. Navigation via la barre supérieure

### Sections principales

- **Dashboard** : Vue d'ensemble et statistiques
- **Carte** : Localisation géographique des victimes
- **Clients** : Détails des machines infectées
- **Logs** : Historique complet des événements
- **Paramètres** : Configuration du dashboard

### Réception des données

Le simulateur envoie les données au format JSON :

```json
{
  "ip": "193.168.5.77",
  "hostname": "VICTIM-001",
  "country": "France",
  "mac": "AA:BB:CC:DD:EE:FF",
  "timestamp": "2025-06-26T10:30:00Z",
  "keystrokes": ["ctrl", "alt", "del", "enter"]
}
```

---

## 🔌 API Endpoints

| Endpoint | Méthode | Description |
|----------|---------|-------------|
| `/api/login` | POST | Authentification utilisateur |
| `/api/data` | POST | Réception des données malware |
| `/api/victims` | GET | Liste des victimes |
| `/api/logs` | GET | Historique des logs |
| `/api/stats` | GET | Statistiques globales |

### WebSocket Events

- `connection` : Nouvelle connexion client
- `victim-data` : Données d'une victime
- `new-victim` : Notification nouvelle infection
- `disconnect` : Déconnexion client

---

## 🔒 Sécurité

### Mesures implémentées

- ✅ **Authentification bcrypt** avec salt rounds = 10
- ✅ **Rate limiting** : 5 tentatives par minute
- ✅ **Validation des données** entrantes
- ✅ **Stockage sécurisé** dans `/secure-data`
- ✅ **CORS configuré** pour les domaines autorisés
- ✅ **Pas de base de données externe** (plus discret)

### Protection contre les attaques

```javascript
// Rate limiter configuration
const loginLimiter = rateLimit({
  windowMs: 60 * 1000, // 1 minute
  max: 5,
  message: "Trop de tentatives, réessayez plus tard."
});
```

---

## 🛠️ Développement

### Lancer en mode développement

```bash
# Terminal 1 - Backend
cd backend
npm run dev

# Terminal 2 - Frontend
cd frontend
npm start
```

### Build de production

```bash
cd frontend
npm run build
```

### Tests

```bash
npm test
```

---

<p align="center">
  <strong>Orange Cyberdefense - ISEN Méditerranée</strong><br>
  <em>Former aujourd'hui les experts cyber de demain</em><br><br>
  <sub>© 2025 Grep2Raisin - Tous droits réservés</sub>
</p>