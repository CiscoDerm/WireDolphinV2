import React, { useEffect, useState } from "react";
import "./SessionReplay.css";

function SessionReplay({ data }) {
  const [timeline, setTimeline] = useState([]);
  const [index, setIndex] = useState(0);

  useEffect(() => {
    if (!data || data.length === 0) return;

    const sorted = [...data].sort(
      (a, b) => new Date(a.timestamp) - new Date(b.timestamp)
    );
    setTimeline(sorted);
  }, [data]);

  useEffect(() => {
    if (timeline.length === 0) return;

    const interval = setInterval(() => {
      setIndex((prev) => {
        if (prev + 1 < timeline.length) return prev + 1;
        clearInterval(interval);
        return prev;
      });
    }, 1000);

    return () => clearInterval(interval);
  }, [timeline]);

  return (
    <div className="session-replay">
      <h2>🎥 Session complète : {data[0]?.ip}</h2>
      <ul className="session-log">
        {timeline.slice(0, index + 1).map((entry, i) => (
          <li key={i}>
            <strong>{new Date(entry.timestamp).toLocaleTimeString()} :</strong>{" "}
            {Array.isArray(entry.keystrokes) ? entry.keystrokes.join(" ") : "(aucune frappe)"}
            {detectCommand(entry.keystrokes) && (
              <span className="cmd">[commande détectée]</span>
            )}
          </li>
        ))}
      </ul>
    </div>
  );
}

function detectCommand(keystrokes) {
  const joined = keystrokes.join(" ").toLowerCase();
  const dangerous = ["cd", "rm", "sudo", "curl", "wget", "nano", "shutdown"];
  return dangerous.some((cmd) => joined.includes(cmd));
}

export default SessionReplay;
