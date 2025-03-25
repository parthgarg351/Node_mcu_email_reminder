require("dotenv").config();
const express = require("express");
const Imap = require("node-imap");
const cors = require("cors");
const fetch = require("node-fetch");
const { simpleParser } = require("mailparser"); // 📩 Parse email content
const os = require("os");

const app = express();
app.use(cors());
app.use(express.json());

let nodemcuIp = null; // Store NodeMCU's latest IP
process.env.NODE_TLS_REJECT_UNAUTHORIZED = "0"; // ⚠️ TEMPORARY - DISABLES SSL CHECK

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

// 📌 NodeMCU Registers Its IP
app.post("/register-nodemcu", (req, res) => {
    const { ip } = req.body;
    if (!ip) return res.status(400).send("IP required");

    nodemcuIp = ip;
    console.log("✅ NodeMCU IP Updated:", nodemcuIp);
    res.send("NodeMCU IP Registered");
});

// 📌 Notify NodeMCU When a New Email is Detected
function notifyNodeMCU(email, subject) {
    if (!nodemcuIp) {
        console.error("❌ No NodeMCU IP found!");
        return;
    }

    const encodedEmail = encodeURIComponent(email);
    const encodedSubject = encodeURIComponent(subject);
    
    fetch(`http://${nodemcuIp}/email-alert?email=${encodedEmail}&subject=${encodedSubject}`)
        .then(() => console.log("📩 ✅ NodeMCU Notified!"))
        .catch((err) => console.error("❌ Error notifying NodeMCU:", err));
}

// 📌 Check Emails Using `node-imap`
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
                const allowedSenders = ["parthgarg351@gmail.com", "example2@company.com"];
                let newMail = false;

                const fetcher = imap.fetch(latestEmails, { bodies: "" });
                const emailPromises = []; // Store promises for each email

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
                                    notifyNodeMCU(from, subject);
                                }
                                // const from = parsed.from.text;
                                // const subject = parsed.subject;
                                // console.log(`📧 New email from: ${from} - Subject: ${subject}`);

                                // notifyNodeMCU(from, subject);

                                resolve(); // Mark parsing as done
                            });
                        });
                    });

                    emailPromises.push(emailPromise); // Add promise to array
                });

                fetcher.on("end", async function () {
                    await Promise.all(emailPromises); // ✅ Wait for all parsing to finish
                    imap.end();
                    if (newMail) {
                        console.log("📢 Triggering NodeMCU notification...");
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

// 📌 Run Email Check Every 60 Seconds
setInterval(checkEmails, 60000);

const PORT = 5000;

app.listen(PORT, () => {
    console.log(`🚀 Server running at http://${localIP}:${PORT}`);
});
