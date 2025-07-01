import React, { useEffect, useState } from "react";
import {
  MapContainer,
  TileLayer,
  Marker,
  Popup,
  ZoomControl
} from "react-leaflet";
import L from "leaflet";
import "leaflet/dist/leaflet.css";
import { io } from "socket.io-client";
import { toast } from "react-toastify";
import "./GlobalMap.css";

// 🧩 Fix icônes Leaflet
delete L.Icon.Default.prototype._getIconUrl;
L.Icon.Default.mergeOptions({
  iconRetinaUrl: require("leaflet/dist/images/marker-icon-2x.png"),
  iconUrl: require("leaflet/dist/images/marker-icon.png"),
  shadowUrl: require("leaflet/dist/images/marker-shadow.png")
});

function GlobalMap() {
  const [locations, setLocations] = useState([]);

  // 📦 Charger les victimes existantes
  useEffect(() => {
    const fetchData = async () => {
      try {
        const res = await fetch("http://localhost:3001/api/data");
        const data = await res.json();

        if (!Array.isArray(data)) {
          console.error("❌ Le backend a retourné un format inattendu :", data);
          setLocations([]);
          return;
        }

        const uniqueIPs = [...new Set(data.map(v => v.ip))];
        const results = await Promise.all(
          uniqueIPs.map(async ip => {
            try {
              const res = await fetch(`http://ip-api.com/json/${ip}`);
              const geo = await res.json();
              if (geo.status === "success") {
                return {
                  ip,
                  lat: geo.lat,
                  lng: geo.lon,
                  city: geo.city,
                  country: geo.country
                };
              }
            } catch {
              return null;
            }
          })
        );

        setLocations(results.filter(Boolean));
      } catch (err) {
        console.error("❌ Erreur de chargement :", err);
        setLocations([]);
      }
    };

    fetchData();
  }, []);

  // 🌐 WebSocket + toast live
  useEffect(() => {
    const socket = io("http://localhost:3001");

    socket.on("new_attack", async (data) => {
      try {
        const res = await fetch(`http://ip-api.com/json/${data.ip}`);
        const geo = await res.json();
        if (geo.status === "success") {
          const newVictim = {
            ip: data.ip,
            lat: geo.lat,
            lng: geo.lon,
            city: geo.city,
            country: geo.country
          };

          setLocations(prev => [...prev, newVictim]);

          toast.success(`🧨 Nouvelle victime : ${geo.country} - ${data.hostname}`, {
            position: "top-right",
            autoClose: 4000,
            closeOnClick: true,
            pauseOnHover: true,
            draggable: true,
            className: "toast-dark"
          });
        }
      } catch (err) {
        console.warn("Erreur géoloc live:", err);
      }
    });

    return () => {
      socket.disconnect();
    };
  }, []);

  return (
    <div className="map-fullscreen">
      <MapContainer
        center={[20, 0]}
        zoom={2}
        minZoom={2}
        maxZoom={4}
        scrollWheelZoom={true}
        zoomControl={false}
        maxBounds={[
          [90, -180],
          [-90, 180]
        ]}
        className="map-leaflet"
      >
        <ZoomControl position="topright" />
        <TileLayer
          url="https://tiles.stadiamaps.com/tiles/alidade_smooth_dark/{z}/{x}/{y}{r}.png"
        />
        {locations.map((loc, i) => (
          <Marker key={i} position={[loc.lat, loc.lng]}>
            <Popup>
              {loc.city}, {loc.country}
              <br />
              IP : {loc.ip}
            </Popup>
          </Marker>
        ))}
      </MapContainer>
    </div>
  );
}

export default GlobalMap;
