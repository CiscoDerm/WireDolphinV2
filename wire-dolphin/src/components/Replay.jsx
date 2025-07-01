import React, { useEffect, useState } from "react";
import "./Replay.css";

function Replay({ data }) {
  const [replay, setReplay] = useState("");
  const [index, setIndex] = useState(0);

  useEffect(() => {
    if (!data || data.length === 0) return;

    const flatKeys = data.flatMap(log => log.keystrokes || []);
    const interval = setInterval(() => {
      if (index < flatKeys.length) {
        setReplay(prev => prev + flatKeys[index] + " ");
        setIndex(i => i + 1);
      } else {
        clearInterval(interval);
      }
    }, 300); // vitesse de la "frappe"

    return () => clearInterval(interval);
  }, [data, index]);

  return (
    <div className="replay-container">
      <h2>⌨️ Relecture des frappes</h2>
      <div className="terminal-box">
        <pre>{replay}</pre>
      </div>
    </div>
  );
}

export default Replay;
