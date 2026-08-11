# 🌲 EdgeWake — Tiered Cascade Guardian System

<p align="center">
  <img src="docs/images/edgewake-banner.png" alt="EdgeWake Banner" width="800">
</p>

<p align="center">
  <strong>An Ultra-Low-Power Edge AI System for Real-Time Forest Threat Detection</strong><br>
  <em>Smart India Hackathon 2026 | VIT-AP University</em>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/SIH-2026-green?style=for-the-badge&logo=hackthebox&logoColor=white" alt="SIH 2026">
  <img src="https://img.shields.io/badge/ESP32--CAM-Powered-blue?style=for-the-badge&logo=espressif&logoColor=white" alt="ESP32-CAM">
  <img src="https://img.shields.io/badge/Edge_AI-ZCR_+_RMS-orange?style=for-the-badge&logo=tensorflow&logoColor=white" alt="Edge AI">
  <img src="https://img.shields.io/badge/Battery-2+_Years-brightgreen?style=for-the-badge&logo=battledotnet&logoColor=white" alt="Battery Life">
  <img src="https://img.shields.io/badge/Cost-₹1,140-yellow?style=for-the-badge&logo=cashapp&logoColor=white" alt="Cost">
</p>

<p align="center">
  <a href="#-quick-start">Quick Start</a> •
  <a href="#-the-problem">Problem</a> •
  <a href="#-our-solution">Solution</a> •
  <a href="#-architecture">Architecture</a> •
  <a href="#-team-roles--responsibilities">Team Roles</a> •
  <a href="#-development-setup">Dev Setup</a> •
  <a href="#-project-structure">Project Structure</a> •
  <a href="#-contributing">Contributing</a>
</p>

---

## 📣 Hey Team! Read This First

> **Welcome to EdgeWake!** This README has **everything** you need to get started.
> 
> 1. Read the [Problem](#-the-problem) and [Solution](#-our-solution) sections so you deeply understand what we're building
> 2. Find your name in [Team Roles](#-team-roles--responsibilities) to know your specific responsibilities
> 3. Follow the [Development Setup](#-development-setup) guide for your role
> 4. Read the [Contributing](#-contributing) section so you know the Git workflow
> 5. Check `docs/` folder for detailed documentation on each subsystem

---

## 🔥 The Problem

India loses **thousands of hectares** of forest every year to fires and illegal logging. The existing solutions all have critical flaws:

| Current Method | What's Wrong |
|:---|:---|
| 🛰️ **Satellite Imaging** | Hours of latency, blocked by clouds, can't detect small fires early |
| 🚁 **Drone Surveillance** | 20-40 min battery life, needs active human piloting, expensive |
| 📡 **Always-On IoT Sensors** | "Always-on" cameras/mics drain batteries in **days**, making deep-forest deployment impossible |

**The core challenge:** How do you put smart sensors deep in a forest where there's no power grid, no internet infrastructure, and no one to change batteries?

---

## 💡 Our Solution

**EdgeWake** solves this with a **5-Tier Cascade Architecture** — a system that keeps all the power-hungry components (camera, AI processor, WiFi) **completely turned off** until a near-zero-power hardware sensor confirms something suspicious is happening.

### The 5-Tier Cascade — How It Works

Think of it like a sleeping guard dog that only wakes up when it hears something, then looks, then barks:

```
╔══════════════════════════════════════════════════════════════════════╗
║                                                                      ║
║   TIER 1: DEEP SLEEP  ⏳ (~115µA — near-zero power)                 ║
║   ┃  ESP32 is sleeping. Camera OFF. Mic OFF. WiFi OFF.               ║
║   ┃  Only tiny flame/vibration sensors are powered.                  ║
║   ┃                                                                  ║
║   ┃  💡 Battery drain: ~1mAh per day → 2+ YEARS on one battery      ║
║   ┃                                                                  ║
║   ▼ ── flame detected OR vibration detected ──                       ║
║                                                                      ║
║   TIER 2: HARDWARE INTERRUPT  ⚡ (<5mA, <100ms)                     ║
║   ┃  ESP32 wakes up instantly.                                       ║
║   ┃  Checks: "Was this a real sensor trigger or just noise?"         ║
║   ┃  If noise → go back to sleep immediately.                        ║
║   ┃                                                                  ║
║   ▼ ── valid trigger confirmed ──                                    ║
║                                                                      ║
║   TIER 3: EDGE AI AUDIO VERIFY  🎤 (~40mA, ~2 seconds)             ║
║   ┃  Powers ON the INMP441 microphone.                               ║
║   ┃  Records 2 seconds of audio.                                     ║
║   ┃  Computes ZCR (Zero-Crossing Rate) + RMS (loudness).            ║
║   ┃                                                                  ║
║   ┃  Decision:                                                       ║
║   ┃    High ZCR + High RMS  →  🪚 CHAINSAW detected                 ║
║   ┃    Low ZCR + Med RMS   →  🔥 FIRE detected                      ║
║   ┃    Otherwise            →  ✅ False alarm → sleep                ║
║   ┃                                                                  ║
║   ▼ ── threat confirmed by AI ──                                     ║
║                                                                      ║
║   TIER 4: VISUAL CAPTURE  📸 (~120mA, ~1 second)                    ║
║   ┃  Powers ON the OV2640 camera.                                    ║
║   ┃  Snaps a JPEG photo as evidence.                                 ║
║   ┃  Powers OFF camera immediately.                                  ║
║   ┃                                                                  ║
║   ▼ ── photo captured ──                                             ║
║                                                                      ║
║   TIER 5: CLOUD HANDOFF  📡 (~160mA, ~3 seconds)                    ║
║   ┃  Powers ON WiFi.                                                 ║
║   ┃  Sends HTTP POST to cloud webhook:                               ║
║   ┃    → threat type, confidence %, photo, GPS, battery level        ║
║   ┃  Powers OFF WiFi.                                                ║
║   ┃                                                                  ║
║   ▼ ── alert sent ──                                                 ║
║                                                                      ║
║   🔄 Returns to TIER 1 (Deep Sleep)                                  ║
║                                                                      ║
╚══════════════════════════════════════════════════════════════════════╝
```

### Why This Matters

| Metric | EdgeWake | Competitor (Dryad) | Competitor (Pyronear) |
|:---|:---|:---|:---|
| **Cost per node** | **₹1,140 ($15)** | $500 | $200 |
| **Battery life** | **2+ years** | Solar dependent | Mains power |
| **Detection method** | Audio AI + Camera + IR | Gas sensors only | Camera only |
| **Visual evidence** | ✅ Yes (JPEG) | ❌ No | ✅ Yes |
| **Works offline** | ✅ Yes (edge AI) | ⚠️ Partial | ❌ No |

**We are 33x cheaper than the closest competitor with MORE features.**

---

## 🏗️ Architecture

### System Overview
```
                    ┌─────────────────────────────────────────────┐
                    │              FOREST DEPLOYMENT               │
                    │                                             │
                    │   [EW-001]    [EW-002]    [EW-003]         │
                    │      │           │           │              │
                    │      └─────WiFi──┴───────────┘              │
                    │               │                             │
                    └───────────────┼─────────────────────────────┘
                                    │ HTTP POST (JSON + Image)
                                    ▼
                    ┌───────────────────────────────────┐
                    │         CLOUD LAYER                │
                    │                                   │
                    │  ┌──────────┐    ┌─────────────┐ │
                    │  │ n8n      │───▶│ Firebase    │ │
                    │  │ Webhook  │    │ Firestore   │ │
                    │  └────┬─────┘    └──────┬──────┘ │
                    │       │                 │        │
                    │  ┌────▼─────┐    ┌──────▼──────┐ │
                    │  │ Alerts   │    │   Image     │ │
                    │  │ SMS/Email│    │   Storage   │ │
                    │  │ Telegram │    │   (S3/FB)   │ │
                    │  │ WhatsApp │    └─────────────┘ │
                    │  └──────────┘                    │
                    └───────────────┬───────────────────┘
                                    │ WebSocket / REST
                                    ▼
                    ┌───────────────────────────────────┐
                    │        MONITORING DASHBOARD        │
                    │                                   │
                    │  🗺️ Live Map    📊 Analytics      │
                    │  🚨 Alert Feed  🔋 Node Health    │
                    │  📸 Evidence    📈 Charts          │
                    │                                   │
                    │  (React/Next.js + Leaflet.js)     │
                    └───────────────────────────────────┘
```

### Hardware Block Diagram
```
                    ┌──────────────────────────────────────────┐
                    │            ESP32-CAM Module               │
                    │  ┌──────────────────────────────────┐    │
                    │  │  ESP32 (240MHz, 520KB SRAM)      │    │
                    │  │  + PSRAM (4MB)                    │    │
                    │  │  + OV2640 Camera (built-in)       │    │
                    │  │  + WiFi 802.11 b/g/n (built-in)   │    │
                    │  └──────────┬───────────────────────┘    │
                    │             │                             │
                    │  GPIO 13 ───┤◄── KY-026 Flame Sensor     │
                    │  GPIO 14 ───┤◄── SW-420 Vibration Sensor │
                    │             │                             │
                    │  I2S Bus ───┤◄── INMP441 MEMS Mic        │
                    │  (via MOSFET     (powered only when       │
                    │   power gate)     needed)                 │
                    │             │                             │
                    │  5V ────────┤◄── TP4056 Charger           │
                    │                  ◄── 18650 Battery        │
                    └──────────────────────────────────────────┘
```

---

## 👥 Team Roles & Responsibilities

### Team Structure

| # | Role | Member | Primary Focus | Key Deliverables |
|:---|:---|:---|:---|:---|
| 1️⃣ | **Team Lead + Hardware** | Parth Arsid | Project management, ESP32-CAM integration, system architecture | Working prototype, integration of all tiers |
| 2️⃣ | **Circuit & Sensor Engineer** | TBD | Circuit design, PCB, power profiling, sensor calibration | Schematic, power analysis, weatherproof enclosure |
| 3️⃣ | **Edge AI & Firmware Dev** | TBD | ZCR/RMS algorithm, INMP441 driver, deep sleep code | Audio classification code, accuracy benchmarks |
| 4️⃣ | **Cloud Backend Dev** | TBD | Firebase, n8n webhooks, API, notification pipeline | REST API, webhook automation, SMS/email alerts |
| 5️⃣ | **UI/UX & Pitch Lead** | TBD | Dashboard design, presentation, business model | Figma designs, React dashboard, pitch deck |
| 6️⃣ | **Research & Docs** | TBD | Literature review, testing protocols, documentation | Research report, test results, SIH documents |

### Who Works on What Folder

```
EdgeWake/
├── firmware/      ← Members 1, 2, 3
├── hardware/      ← Member 2
├── cloud/         ← Member 4
├── dashboard/     ← Members 4, 5
├── datasets/      ← Members 3, 6
├── docs/          ← Member 6
└── presentation/  ← Members 5, 6
```

---

## 🛠️ Tech Stack

| Layer | Technologies | Owner |
|:---|:---|:---|
| **Microcontroller** | ESP32-CAM (AI Thinker) + OV2640 Camera | Members 1, 2 |
| **Sensors** | KY-026 (flame), SW-420 (vibration), INMP441 (I2S mic) | Member 2 |
| **Power** | 18650 Li-ion 3000mAh + TP4056 charger + MOSFET power gating | Member 2 |
| **Firmware** | Arduino/ESP-IDF + PlatformIO + FreeRTOS | Members 1, 3 |
| **Edge AI** | ZCR + RMS audio classification (C/C++ on ESP32) | Member 3 |
| **Backend** | Firebase Firestore + Cloud Functions + n8n webhooks | Member 4 |
| **Storage** | Firebase Storage / AWS S3 (for captured images) | Member 4 |
| **Notifications** | Twilio SMS + Telegram Bot + Email | Member 4 |
| **Dashboard** | Next.js + Leaflet.js + Recharts + Framer Motion | Members 4, 5 |
| **Design** | Figma (wireframes + design system) | Member 5 |

---

## 💰 Bill of Materials (BOM)

| # | Component | Model / Spec | Est. Cost (₹) | Buy Link |
|:---|:---|:---|:---|:---|
| 1 | Microcontroller + Camera | ESP32-CAM + OV2640 | ₹500 | [Amazon](https://amazon.in) / [Robu.in](https://robu.in) |
| 2 | Acoustic Sensor | INMP441 I2S MEMS Mic | ₹200 | [Robu.in](https://robu.in) |
| 3 | Thermal Sensor | KY-026 IR Flame Module | ₹50 | [Robu.in](https://robu.in) |
| 4 | Seismic Sensor | SW-420 Vibration Module | ₹40 | [Robu.in](https://robu.in) |
| 5 | Battery + Charger | 18650 3000mAh + TP4056 | ₹200 | [Amazon](https://amazon.in) |
| 6 | Miscellaneous | Wires, breadboard, enclosure | ₹150 | Local electronics shop |
| | | **TOTAL** | **₹1,140** | |

> **💡 Tip:** Order 2 sets of everything so you have a backup during the hackathon!

---

## 🚀 Development Setup

### For Everyone (Required)

```bash
# 1. Install Git (if not installed)
# Download from: https://git-scm.com/downloads

# 2. Clone this repository
git clone https://github.com/Partharsid/EdgeWake.git
cd EdgeWake

# 3. Create your feature branch
git checkout -b feature/your-name-component
# Example: git checkout -b feature/rahul-cloud-backend
```

### Required Software by Role

| Software | Members | Download |
|:---|:---|:---|
| **VS Code** | All | [code.visualstudio.com](https://code.visualstudio.com) |
| **Git** | All | [git-scm.com](https://git-scm.com) |
| **PlatformIO** (VS Code extension) | 1, 2, 3 | Install from VS Code Extensions |
| **Node.js v20 LTS** | 4, 5 | [nodejs.org](https://nodejs.org) |
| **Python 3.12+** | 3, 6 | [python.org](https://python.org) |
| **Figma** | 5 | [figma.com](https://figma.com) |
| **KiCad** | 2 | [kicad.org](https://kicad.org) |

### VS Code Extensions (Install These)

```
✅ PlatformIO IDE           ← Firmware team (Members 1, 2, 3)
✅ C/C++ Extension Pack      ← Firmware team
✅ ESLint                    ← All
✅ Prettier                  ← All
✅ GitLens                   ← All
✅ Live Share                ← All (for pair programming)
✅ Thunder Client            ← Member 4 (API testing)
```

---

### ⚡ Firmware Setup (Members 1, 2, 3)

```bash
# Open the firmware folder in VS Code with PlatformIO
cd firmware

# 1. Copy the config template and fill in your WiFi credentials
cp include/config.h.example include/config.h
# Edit config.h with your WiFi SSID and password

# 2. Connect ESP32-CAM via USB-to-TTL adapter
# 3. In VS Code → PlatformIO sidebar → Build → Upload
# 4. Open Serial Monitor at 115200 baud to see output
```

**⚠️ NEVER commit `config.h` — it contains your WiFi password! It's in `.gitignore`.**

---

### ☁️ Cloud Backend Setup (Member 4)

```bash
# 1. Install Firebase CLI
npm install -g firebase-tools

# 2. Login to Firebase
firebase login

# 3. Set up the cloud project
cd cloud/functions
npm install

# 4. For local development
firebase emulators:start

# 5. Deploy to production
firebase deploy --only functions
```

---

### 🎨 Dashboard Setup (Members 4, 5)

```bash
# 1. Navigate to dashboard folder
cd dashboard

# 2. Install dependencies
npm install

# 3. Start development server
npm run dev

# 4. Open http://localhost:3000 in browser
```

---

## 📁 Project Structure

```
EdgeWake/
│
├── 📄 README.md                  ← You are here!
├── 📄 .gitignore                 ← Protects secrets & ignores build files
├── 📄 PROPOSAL.pdf               ← Original project proposal
│
├── 🔧 firmware/                  ← ESP32-CAM firmware code
│   ├── platformio.ini            ← Build configuration
│   ├── src/
│   │   └── main.cpp              ← Main 5-tier cascade code (START HERE)
│   ├── include/
│   │   └── config.h              ← WiFi & webhook settings (DON'T COMMIT)
│   └── test/                     ← Unit tests
│
├── 🔌 hardware/                  ← Circuit & physical design
│   ├── schematic/                ← KiCad/Fritzing circuit diagrams
│   ├── pcb/                      ← PCB layout files
│   └── enclosure/                ← 3D models for weatherproof case
│
├── ☁️ cloud/                     ← Backend & automation
│   ├── functions/                ← Firebase Cloud Functions / Express API
│   ├── n8n-workflows/            ← n8n webhook workflow exports
│   └── api/                      ← API documentation
│
├── 🖥️ dashboard/                 ← Monitoring web dashboard
│   └── (Next.js project)         ← React-based monitoring UI
│
├── 🎵 datasets/                  ← Audio samples for AI testing
│   ├── chainsaw/                 ← Chainsaw audio clips
│   ├── fire/                     ← Fire crackling audio clips
│   ├── wind/                     ← Wind/rain background noise
│   └── animals/                  ← Bird/animal sounds
│
├── 📚 docs/                      ← Technical documentation
│   ├── images/                   ← Architecture diagrams, photos
│   ├── architecture.md           ← Detailed system architecture
│   ├── api-reference.md          ← API endpoint documentation
│   └── power-analysis.md         ← Battery life calculations
│
└── 🎤 presentation/              ← SIH presentation materials
    ├── slides.pptx               ← Pitch deck
    ├── demo-video.mp4            ← Recorded demo (backup)
    └── poster.pdf                ← SIH booth poster
```

---

## 🔄 Contributing (Git Workflow)

### Branch Naming Convention
```
feature/your-name-what-you-are-building
```
Examples:
- `feature/parth-deep-sleep-integration`
- `feature/rahul-firebase-webhook`
- `feature/priya-dashboard-map`

### Workflow

```bash
# 1. Always start from the latest develop branch
git checkout develop
git pull origin develop

# 2. Create your feature branch
git checkout -b feature/your-name-thing

# 3. Make your changes, commit often with clear messages
git add .
git commit -m "feat: add ZCR calculation for audio classification"
# Use these prefixes:
#   feat:     New feature
#   fix:      Bug fix
#   docs:     Documentation
#   test:     Adding tests
#   refactor: Code cleanup

# 4. Push your branch
git push origin feature/your-name-thing

# 5. Go to GitHub → Create Pull Request → develop branch
# 6. Request review from at least 1 teammate
# 7. After approval, merge (Team Lead merges to main)
```

### Commit Message Examples
```
✅ Good:
  feat: implement INMP441 I2S microphone driver
  fix: resolve false wake-up from vibration sensor bounce
  docs: add power consumption measurements to docs
  feat: add Leaflet map component to dashboard

❌ Bad:
  fixed stuff
  update
  asdfg
  test123
```

### ⚠️ Rules
1. **NEVER push directly to `main` or `develop`** — always use Pull Requests
2. **NEVER commit secrets** (WiFi passwords, API keys) — use `config.h` which is gitignored
3. **Pull before you push** — always `git pull origin develop` before starting work
4. **Write clear commit messages** — your teammates need to understand your changes

---

## 📅 Development Timeline

```
Week 1-2  🔨 FOUNDATION
          Hardware setup, sensor testing, audio pipeline, cloud infra, wireframes

Week 3-4  🧠 INTELLIGENCE
          ZCR/RMS algorithm, threshold calibration, webhook pipeline, dashboard UI

Week 5    🔗 INTEGRATION
          End-to-end testing, enclosure, demo preparation

Week 6    🎤 PRESENTATION
          Pitch rehearsal, documentation, final polish
```

---

## 📖 Learning Resources

| Topic | Resource | For |
|:---|:---|:---|
| ESP32 Deep Sleep | [RandomNerdTutorials](https://randomnerdtutorials.com/esp32-deep-sleep-arduino-ide-wake-up-sources/) | Members 1, 3 |
| ESP32-CAM Photo | [RandomNerdTutorials](https://randomnerdtutorials.com/esp32-cam-take-photo-save-microsd-card/) | Member 1 |
| INMP441 I2S Audio | [GitHub: atomic14](https://github.com/atomic14/esp32-i2s-mic-test) | Member 3 |
| Firebase + ESP32 | [RandomNerdTutorials](https://randomnerdtutorials.com/esp32-firebase-realtime-database/) | Members 1, 4 |
| Next.js Basics | [nextjs.org/learn](https://nextjs.org/learn) | Members 4, 5 |
| Leaflet.js Maps | [leafletjs.com](https://leafletjs.com/examples.html) | Member 5 |
| n8n Webhooks | [n8n docs](https://docs.n8n.io/integrations/builtin/core-nodes/n8n-nodes-base.webhook/) | Member 4 |
| KiCad PCB Design | [KiCad Docs](https://docs.kicad.org/7.0/en/getting_started_in_kicad/getting_started_in_kicad.html) | Member 2 |
| ZCR Explained | [Wikipedia](https://en.wikipedia.org/wiki/Zero-crossing_rate) | Member 3 |

---

## 🤝 Team Communication

| Channel | Purpose |
|:---|:---|
| 💬 **WhatsApp Group** | Daily standups, quick questions, urgencies |
| 🎙️ **Discord Server** | Voice calls during coding sessions, screen share |
| 📋 **GitHub Issues** | Bug tracking, feature requests |
| 📊 **GitHub Projects** | Kanban board (To Do → In Progress → Done) |

**Daily Standup (10 PM, 15 min):** Each member answers:
1. What did I finish today?
2. What will I work on tomorrow?
3. Am I blocked on anything?

---

## 📄 License

This project is built for **Smart India Hackathon 2026**.

**Team EdgeWake** — VIT-AP University

---

<p align="center">
  <strong>Made with 💚 for India's Forests</strong><br>
  <em>Every tree saved is a future preserved 🌲</em>
</p>
