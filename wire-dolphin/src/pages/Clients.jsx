import React, { useEffect, useState } from "react";
import Replay from "../components/Replay";
import SessionReplay from "../components/SessionReplay";
import "./Clients.css";

function Clients() {
  const [data, setData] = useState([]);
  const [search, setSearch] = useState("");
  const [selectedVictim, setSelectedVictim] = useState(null);

  useEffect(() => {
    fetch("http://localhost:3001/api/data")
      .then(res => res.json())
      .then(setData);
  }, []);

  const victims = [...new Map(data.map(item => [item.ip, item])).values()];

  const filteredVictims = victims.filter(v =>
    v.ip.includes(search) ||
    v.hostname?.toLowerCase().includes(search.toLowerCase()) ||
    v.country?.toLowerCase().includes(search.toLowerCase()) ||
    v.mac?.toLowerCase().includes(search.toLowerCase())
  );

  return (
    <div className="clients-page">
      <h2>🧍 Liste des victimes</h2>

      {selectedVictim ? (
        <div className="replay-wrapper">
          <button onClick={() => setSelectedVictim(null)} className="back-button">
            🔙 Revenir à la liste
          </button>

          {selectedVictim.type === "replay" && (
            <Replay data={data.filter(d => d.ip === selectedVictim.data.ip)} />
          )}

          {selectedVictim.type === "session" && (
            <SessionReplay data={data.filter(d => d.ip === selectedVictim.data.ip)} />
          )}
        </div>
      ) : (
        <>
          <input
            type="text"
            placeholder="🔍 "
            className="search-input"
            value={search}
            onChange={(e) => setSearch(e.target.value)}
          />

          <table className="clients-table">
            <thead>
              <tr>
                <th>IP</th>
                <th>Pays</th>
                <th>Hostname</th>
                <th>Adresse MAC</th>
                <th>Dernier contact</th>
                <th>Actions</th>
              </tr>
            </thead>
            <tbody>
              {filteredVictims.map((v, i) => (
                <tr key={i}>
                  <td>{v.ip}</td>
                  <td>{v.country}</td>
                  <td>{v.hostname}</td>
                  <td>{v.mac || "–"}</td>
                  <td>{v.timestamp ? new Date(v.timestamp).toLocaleString("fr-FR") : "–"}</td>
                  <td>
                    <button
                      onClick={() => setSelectedVictim({ type: "replay", data: v })}
                      style={{ marginRight: "10px" }}
                    >
                      ⌨️ Replay
                    </button>
                    <button
                      onClick={() => setSelectedVictim({ type: "session", data: v })}
                    >
                      🎥 Session
                    </button>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </>
      )}
    </div>
  );
}

export default Clients;
