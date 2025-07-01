const fs = require("fs");
const path = require("path");
const bcrypt = require("bcrypt");

const usersPath = path.join(__dirname, "users.json");

const username = "admin";
const plainPassword = "dolphin123";

bcrypt.hash(plainPassword, 10, (err, hash) => {
  if (err) {
    console.error("Erreur lors du hash :", err);
    return;
  }

  const user = [{ username, password: hash }];
  fs.writeFileSync(usersPath, JSON.stringify(user, null, 2));
  console.log("✅ Utilisateur créé !");
});
