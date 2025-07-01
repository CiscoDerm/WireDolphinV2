import React from "react";
import "./Settings.css";

function Settings() {
  const handleReset = async () => {
    const confirm = window.confirm("Tu es sûr de vouloir supprimer toutes les données ?");
    if (!confirm) return;

    await fetch("http://localhost:3001/api/reset", {
      method: "POST"
    });

    alert("Données réinitialisées !");
    window.location.reload();
  };

  const handleLogout = () => {
    localStorage.removeItem("isAuth");
    window.location.href = "/login";
  };

  return (
    <div className="settings-page">
      <h2>⚙️ Paramètres</h2>

      <div className="settings-actions">
        <button onClick={handleReset} className="danger">🗑️ Vider les données</button>
        <button onClick={handleLogout}>🚪 Se déconnecter</button>
      </div>

      <p className="settings-info">
        Ce panneau vous permet d'effectuer des actions globales comme la réinitialisation de toutes les données ou votre déconnexion.
      </p>
    </div>
  );
}

export default Settings;
