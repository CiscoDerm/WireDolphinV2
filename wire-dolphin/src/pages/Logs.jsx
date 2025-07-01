import React, { useEffect, useState } from "react";
import "./Logs.css";

function Logs() {
  const [logs, setLogs] = useState([]);
  const [search, setSearch] = useState("");

  useEffect(() => {
    fetch("http://localhost:3001/api/data")
      .then(res => res.json())
      .then(data => {
        const formatted = data.map((d, i) => ({
          id: i,
          ip: d.ip,
          timestamp: d.timestamp,
          keys: formatKeys(d.keystrokes)
        }));
        setLogs(formatted);
      });
  }, []);

  const formatKeys = (keystrokes) => {
    if (Array.isArray(keystrokes)) {
      return keystrokes.join(" + ");
    } else if (typeof keystrokes === "string") {
      return keystrokes;
    } else {
      return "(aucune frappe)";
    }
  };

  const filteredLogs = logs.filter(log =>
    log.ip.includes(search) ||
    log.keys.toLowerCase().includes(search.toLowerCase())
  );

  return (
    <div className="logs-page">
      <h2>📜 Historique des Logs</h2>

      <input
        type="text"
        placeholder="🔍 Rechercher IP ou frappes..."
        className="search-input"
        value={search}
        onChange={(e) => setSearch(e.target.value)}
      />

      <table className="logs-table">
        <thead>
          <tr>
            <th>IP</th>
            <th>Frappes clavier</th>
            <th>Heure</th>
          </tr>
        </thead>
        <tbody>
          {filteredLogs.map(log => (
            <tr key={log.id}>
              <td>{log.ip}</td>
              <td>{log.keys}</td>
              <td>{new Date(log.timestamp).toLocaleTimeString()}</td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
}

export default Logs;
