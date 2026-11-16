# Dining Philosophers Problem — Independent Processes & POSIX Named Semaphores

[![Live Interactive Visualizer](https://img.shields.io/badge/Live_Demo-Interactive_Semaphore_Visualizer-2c9a58?style=for-the-badge)](https://diganta18.github.io/dining-philosophers-ipc/)
![Platform](https://img.shields.io/badge/Platform-Linux_(Ubuntu_24.04_LTS)-3a7bbf?style=for-the-badge)
![Language](https://img.shields.io/badge/Language-C_(POSIX_IPC)-d19015?style=for-the-badge)

A Linux C simulation of **Edsger W. Dijkstra’s Dining Philosophers problem** using **five completely independent (unrelated) OS processes**—without `fork()` or threads—synchronized exclusively through **POSIX named semaphores** (`/dev/shm`), paired with a step-by-step **interactive browser visualizer** that executes the exact C synchronization logic line by line.

---

## Live Interactive Visualizer

Explore how `/room` and `/chopstick0`–`/chopstick4` change state under the hood without compiling anything:

> **[Launch the Live Semaphore Visualizer](https://diganta18.github.io/dining-philosophers-ipc/)**

* **Step-by-step execution:** Advance one line of C code at a time for any selected philosopher (`P0`–`P4`).
* **Live kernel semaphore table:** Inspect real-time counter values, current lock holders, and FIFO wait queues.
* **Deadlock toggle (`Use the /room semaphore`):** Switch `/room` off and trigger a 5-way circular wait on demand to observe why `N - 1` admission control is required.

---

## Repository Structure

```text
dining-philosophers-ipc/
├── src/
│   ├── philosopher.c                  # Independent philosopher process (run 5 times with IDs 0–4)
│   └── cleanup.c                      # Unlinks /chopstick0..4 and /room from /dev/shm
├── docs/
│   └── OS_Lab_Mini_Project_Report.pdf # Complete 27-page academic project report & test logs
├── index.html                         # Interactive POSIX semaphore visualizer (GitHub Pages)
├── .gitignore                         # Excludes compiled ELF binaries
└── README.md                          # Project documentation