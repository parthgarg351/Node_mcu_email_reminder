require("dotenv").config();
const express = require("express");
const Imap = require("node-imap");
const cors = require("cors");
const fetch = require("node-fetch");
const { simpleParser } = require("mailparser"); // 📩 Parse email content
const os = require("os");
const axios = require("axios");

const app = express();
app.use(cors());
app.use(express.json());

let storedEmails = []; // Store allowed sender emails temporarily

process.env.NODE_TLS_REJECT_UNAUTHORIZED = "0"; //TEMPORARY - DISABLES SSL CHECK

function getLocalIP() {
  const interfaces = os.networkInterfaces();
  for (let iface of Object.values(interfaces)) {
    for (let entry of iface) {
      if (entry.family === "IPv4" && !entry.internal) {
        return entry.address;
      }
    }
  }
  return "127.0.0.1"; // Default to localhost if not found
}

const localIP = getLocalIP();

let matchId = "115030"; // Default match ID

// Web UI to update matchId
app.get("/", (req, res) => {
  res.send(`
        <h2>Match ID Update</h2>
        <form method="POST" action="/update-match">
            <input type="text" name="matchId" value="${matchId}" required />
            <button type="submit">Update Match ID</button>
        </form>
    `);
});

// Update match ID from UI
app.post(
  "/update-match",
  express.urlencoded({ extended: true }),
  (req, res) => {
    matchId = req.body.matchId;
    res.send(
      `<h3>✅ Match ID updated to ${matchId}</h3><a href="/">Go Back</a>`
    );
  }
);

// Fetch match data from API
app.get("/get-match", async (req, res) => {
  try {
    const apiUrl = `${process.env.CRICKET_API_URL}?id=${matchId}`;
    const response = await axios.get(
      apiUrl,
      {
        headers: {
          accept:
            "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7",
          "accept-encoding": "gzip, deflate, br, zstd",
          "accept-language": "en-US,en;q=0.9",
          "cache-control": "max-age=0",
          cookie:
            "_vercel_jwt=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VySWQiOiI0RnpRc1l4S3YwRDA3Vlc0bHlZSXRtNnYiLCJpYXQiOjE3NDMzNDgzMjgsIm93bmVySWQiOiJ0ZWFtX0s2Z3o3Vkgyd0w3cHppVUdWanlWand4byIsImF1ZCI6ImNyaWNrZXQtcG9jMDNsdTZ0LXBhcnRoLWdhcmdzLXByb2plY3RzLTcxYzdlZTRmLnZlcmNlbC5hcHAiLCJ1c2VybmFtZSI6InBhcnRoZ2FyZzM1MSIsInN1YiI6InNzby1wcm90ZWN0aW9uIn0.Abzp7WKN6miWr1DbQtFZrlrx0_MGm6YiZOfvLgInbkQ",
          priority: "u=0, i",
          "sec-ch-ua": `"Chromium";v="134", "Not:A-Brand";v="24", "Google Chrome";v="134"`,
          "sec-ch-ua-mobile": "?0",
          "sec-ch-ua-platform": `"Windows"`,
          "sec-fetch-dest": "document",
          "sec-fetch-mode": "navigate",
          "sec-fetch-site": "none",
          "sec-fetch-user": "?1",
          "upgrade-insecure-requests": "1",
          "user-agent":
            "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/134.0.0.0 Safari/537.36",
        },
      }
    );

    const data = await response.data;
    console.log("✅ Received Data:", data); 

    if (data.livescore === "Data Not Found") {
      return res.json({ message: "Match is not live" });
    }

    res.json({
      title: data.title || "Unknown Match",
      update: data.update || "No Update",
      livescore: data.livescore || "No Score",
      runrate: data.runrate || "No Runrate",
      batterone: data.batterone || "Unknown",
      battertwo: data.battertwo || "Unknown",
    });
  } catch (error) {
    console.error("❌ Error fetching match data:", error);
    res.status(500).json({ error: "Failed to fetch match details" });
  }
});

app.get("/get-emails", (req, res) => {
  res.json(storedEmails);
  storedEmails = []; 
});

function checkEmails() {
  const imap = new Imap({
    user: process.env.EMAIL_USER,
    password: process.env.EMAIL_PASS,
    host: "imap.gmail.com",
    port: 993,
    tls: true,
    tlsOptions: { rejectUnauthorized: false },
  });

  imap.once("ready", function () {
    console.log("📬 Connected to IMAP, opening INBOX...");

    imap.openBox("INBOX", false, function (err, box) {
      if (err) {
        console.error("❌ Failed to open INBOX:", err);
        return imap.end();
      }

      imap.search(["UNSEEN"], function (err, results) {
        if (err) {
          console.error("❌ Search Error:", err);
          return imap.end();
        }

        if (!results || results.length === 0) {
          console.log("📭 No new emails.");
          return imap.end();
        }

        console.log(`📩 Found ${results.length} new email(s).`);

        const latestEmails = results.slice(-50);
        const allowedSenders = [
          "parthgarg351@gmail.com",
          "no-reply@classroom.google.com",
          "info.tpc@lnmiit.ac.in",
          "manager.placement@lnmiit.ac.in",
          "rajbirkaur@lnmiit.ac.in",
          "mohit.gupta@lnmiit.ac.in",
          "sunil@lnmiit.ac.in",
        ];
        let newEmails = [];

        let newMail = false;

        const fetcher = imap.fetch(latestEmails, { bodies: "" });
        const emailPromises = []; 

        fetcher.on("message", function (msg) {
          const emailPromise = new Promise((resolve) => {
            msg.on("body", function (stream) {
              simpleParser(stream, async (err, parsed) => {
                if (err) {
                  console.error("❌ Email Parsing Error:", err);
                  return resolve();
                }

                const from = parsed.from.text;
                const subject = parsed.subject || "No Subject";

                console.log(`📧 New email from: ${from}`);
                console.log(`📜 Subject: ${subject}`);

                if (allowedSenders.some((sender) => from.includes(sender))) {
                  console.log("✅ Email from allowed sender:", from);
                  newMail = true;
                  // notifyNodeMCU(from, subject);
                  const emailExists = storedEmails.some(
                    (e) => e.from === from && e.subject === subject
                  );

                  if (!emailExists) {
                    console.log("✅ New email added to storage.");
                    newEmails.push({ from, subject });
                  } else {
                    console.log("⚠️ Duplicate email ignored.");
                  }
                }

                resolve(); // Mark parsing as done
              });
            });
          });

          emailPromises.push(emailPromise); 
        });

        fetcher.on("end", async function () {
          await Promise.all(emailPromises); 
          imap.end();
          if (newEmails.length > 0) {
            console.log("📢 Triggering NodeMCU notification...");
            storedEmails = [...storedEmails, ...newEmails];
            // notifyNodeMCU();
          } else {
            console.log("📭 No allowed sender email detected.");
          }
        });
      });
    });
  });

  imap.once("error", function (err) {
    console.error("❌ IMAP Connection Error:", err);
  });

  imap.once("end", function () {
    console.log("📤 IMAP Connection Closed.");
  });

  imap.connect();
}

// Run Email Check Every 60 Seconds
setInterval(checkEmails, 60000);

const PORT = 5000;

app.listen(PORT, () => {
  console.log(`🚀 Server running at http://${localIP}:${PORT}`);
});
