import React, { useState, useEffect } from "react";
import { BrowserRouter as Router, Routes, Route, NavLink } from "react-router-dom";
import Dashboard from "./pages/Dashboard";
import GlobalMap from "./pages/GlobalMap";
import Clients from "./pages/Clients";
import Logs from "./pages/Logs";
import Settings from "./pages/Settings";
import Login from "./pages/Login";
import PrivateRoute from "./components/PrivateRoute";
import "./index.css";

import { ToastContainer } from "react-toastify";
import "react-toastify/dist/ReactToastify.css";

function App() {
  const [darkMode, setDarkMode] = useState(localStorage.getItem("dark") === "true");
  const isAuth = localStorage.getItem("isAuth") === "true";

  useEffect(() => {
    document.body.className = darkMode ? "dark-mode" : "";
    localStorage.setItem("dark", darkMode);
  }, [darkMode]);

  const toggleDark = () => setDarkMode(prev => !prev);

  const handleLogout = () => {
    localStorage.removeItem("isAuth");
    window.location.href = "/login";
  };

  return (
    <Router>
      <div>
        {isAuth && (
          <header className="navbar">
            <h1 className="logo">Wire_Dolphin</h1>
            <nav className="nav-links">
              <NavLink to="/" end>Dashboard</NavLink>
              <NavLink to="/global-map">Carte</NavLink>
              <NavLink to="/clients">Clients</NavLink>
              <NavLink to="/logs">Logs</NavLink>
              <NavLink to="/settings">Paramètres</NavLink>
            </nav>
            <div className="nav-buttons">
              <button className="theme-toggle" onClick={toggleDark}>
                {darkMode ? "☀️" : "🌙"}
              </button>
              <button className="logout-button" onClick={handleLogout}>Logout</button>
            </div>
          </header>
        )}

        <main className="main-container">
          <Routes>
            <Route path="/login" element={<Login />} />
            <Route path="/" element={<PrivateRoute><Dashboard /></PrivateRoute>} />
            <Route path="/global-map" element={<PrivateRoute><GlobalMap /></PrivateRoute>} />
            <Route path="/clients" element={<PrivateRoute><Clients /></PrivateRoute>} />
            <Route path="/logs" element={<PrivateRoute><Logs /></PrivateRoute>} />
            <Route path="/settings" element={<PrivateRoute><Settings /></PrivateRoute>} />
          </Routes>
        </main>

        <ToastContainer
          position="top-right"
          autoClose={4000}
          hideProgressBar={false}
          newestOnTop
          closeOnClick
          pauseOnHover
          draggable
        />
      </div>
    </Router>
  );
}

export default App;
