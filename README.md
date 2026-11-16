# Dining Philosophers: Independent Processes and POSIX Named Semaphores

A Linux C simulation of Dijkstra's **Dining Philosophers** problem. The five philosophers are five completely independent (unrelated) OS processes. The program never calls `fork()` and uses no threads. The processes are synchronized only through **POSIX named semaphores**, which Linux keeps in `/dev/shm`.

The repository also includes a step-by-step browser visualizer, so you can see what every semaphore does, line by line, without compiling anything.



## Contents

 1. [Live visualizer](#live-visualizer)

 2. [Problem statement](#problem-statement)

 3. [What, why and how](#what-why-and-how)

 4. [How the solution works](#how-the-solution-works)

 5. [Repository structure](#repository-structure)

 6. [Requirements](#requirements)

 7. [Build and run](#build-and-run)

 8. [Sample output](#sample-output)

 9. [Tests](#tests)

10. [FAQ and viva questions](#faq-and-viva-questions)

11. [Troubleshooting](#troubleshooting)

12. [Limitations and future work](#limitations-and-future-work)

13. [References](#references)

14. [Author](#author)

## Live visualizer

[**Open the live semaphore visualizer**](https://diganta18.github.io/dining-philosophers-ipc/)

It is a JavaScript model that follows the same logic as `philosopher.c`, one line at a time. It does not run the C code or touch a real kernel.

* **Step by step:** press *Step* to run one line for the next philosopher, or press a philosopher to run that philosopher's next line yourself.

* **Semaphore table:** shows the value of each of the six semaphores, who holds it and who is asleep waiting for it. The wait queue is modelled as first-in first-out for clarity. A real kernel does not promise any particular wake-up order.

* **Deadlock demo:** choose *All hungry at once, room OFF* to watch all five philosophers grab their left chopstick and freeze. Choose *All hungry at once, room ON* to see the same start end safely. The *Use the /room semaphore* checkbox switches the room on and off.

## Problem statement

> Write programs in C on Linux to simulate the Dining Philosophers problem with the following considerations,
> i) the Philosophers (at least five) should be implemented as independent (unrelated) processes without using `fork()`
> ii) use POSIX named semaphores appropriately for proper synchronization between independent processes.

## What, why and how

**What.** Five philosophers sit at a round table. Between each pair there is one chopstick, so there are five chopsticks. A philosopher repeatedly thinks, gets hungry, picks up the left and right chopsticks, eats and puts them down. Each philosopher here is a separate running program.

**Why.** Programs that share resources must not conflict, and must not wait for each other forever. This problem is the classic small example of that. It shows three things:

* a **race condition**, when two philosophers grab the same chopstick,

* **deadlock**, when all five hold one chopstick and wait for the other,

* **starvation**, when a philosopher is always passed over.

**How.** The same program is started five times with an ID from 0 to 4. The chopsticks are named semaphores that every process can find by name. One extra semaphore, `/room`, stops all five from competing at once.

```
              P0
       c0           c1
    P4                  P1
    c4                  c2
       P3      c3      P2

```

Philosopher `i` uses `c[i]` on the left and `c[(i + 1) % 5]` on the right. Neighbours share exactly one chopstick, so they can never eat together.

## How the solution works

### The six named semaphores

| Name | Start value | Meaning | 
| ----- | ----- | ----- | 
| `/chopstick0` to `/chopstick4` | 1 each | 1 means the chopstick is free, 0 means in use. This gives mutual exclusion. | 
| `/room` | 4 (that is, N - 1) | How many philosophers may try to pick up chopsticks at once. | 

### What a philosopher does

```
flowchart TD
    A([Start: ./philosopher id]) --> B{id valid, 0 to 4?}
    B -- no --> X([Print error and exit])
    B -- yes --> C[sem_open: 5 chopsticks = 1, room = 4]
    C --> D[left = id, right = id + 1 mod 5]
    D --> E[Think: sleep]
    E --> F[sem_wait room]
    F --> G[sem_wait left chopstick]
    G --> H[sem_wait right chopstick]
    H --> I[Eat: sleep 2 s]
    I --> J[sem_post left, right and room]
    J --> K{more rounds?}
    K -- yes --> E
    K -- no --> L([sem_close all, done])

```

### The core of `philosopher.c`

```
sem_wait(room);               /* enter the room: at most 4 inside      */
sem_wait(chopstick[left]);    /* take the left chopstick (1 -> 0)      */
sem_wait(chopstick[right]);   /* take the right chopstick (1 -> 0)     */
/* ... eat ... */
printf("... finished eating ...\n");
sem_post(chopstick[left]);    /* put the left chopstick back (0 -> 1)  */
sem_post(chopstick[right]);   /* put the right chopstick back          */
sem_post(room);               /* leave the room                        */

```

* `sem_wait` decreases the value, and sleeps if it is 0.

* `sem_post` increases the value and wakes one sleeper.

* Both are atomic, so two processes cannot both take the last unit.

* The "finished eating" line is printed before the release, so the log order is always truthful.

### Why there is no deadlock

Deadlock needs four conditions at once: mutual exclusion, hold and wait, no pre-emption and circular wait. This solution breaks the **circular wait**.

Only four philosophers may be in the room, and there are five chopsticks. By the pigeonhole principle, at least one philosopher in the room can always get both chopsticks. He eats and releases them, and the others continue. Without `/room`, all five could take their left chopstick at the same moment and wait forever for the right one.

### Why neighbours never eat together

Neighbours share one chopstick, which is a semaphore with value 1. Whoever takes it first makes the other sleep until it is released. At most two philosophers can eat at once, since each needs two of the five chopsticks.

## Repository structure

```
dining-philosophers-ipc/
├── src/
│   ├── philosopher.c                  # Independent philosopher process (run 5 times with IDs 0-4)
│   └── cleanup.c                      # Unlinks /chopstick0..4 and /room from /dev/shm
├── docs/
│   └── OS_Lab_Mini_Project_Report.pdf # Full project report with test cases and output screenshots
├── index.html                         # Interactive semaphore visualizer (GitHub Pages)
├── .gitignore                         # Excludes compiled binaries
└── README.md

```

## Requirements

* Linux with a POSIX-compliant C library (glibc) and `/dev/shm`

* `gcc`

## Build and run

```
cd src
gcc philosopher.c -o philosopher -pthread
gcc cleanup.c -o cleanup -pthread
./cleanup        # start from a clean state

```

**Cleanest way, five terminals.** Open five terminals in the `src` folder and start one philosopher in each, one right after the other:

```
./philosopher 0
./philosopher 1
./philosopher 2
./philosopher 3
./philosopher 4

```

**Quick test, one terminal.** All five start together and their output is merged. Press Enter afterwards if the prompt looks missing:

```
for i in 0 1 2 3 4; do ./philosopher $i & done

```

**When all five print "is done"**, remove the semaphores:

```
./cleanup

```

While the simulation runs, `ls /dev/shm` shows `sem.chopstick0` to `sem.chopstick4` and `sem.room`.

The number of meals per philosopher is the `ROUNDS` constant (3) at the top of `philosopher.c`. The number of philosophers is `N` (5).

## Sample output

Lines from the five philosophers are interleaved. The exact order differs on every run because the operating system schedules the processes.

```
Philosopher 2 is thinking...
Philosopher 1 is thinking...
Philosopher 3 is thinking...
Philosopher 4 is thinking...
Philosopher 3 is hungry
Philosopher 0 is hungry
Philosopher 3 picked up chopstick 3
Philosopher 3 picked up chopstick 4
Philosopher 3 is EATING (round 1)
Philosopher 0 picked up chopstick 0
Philosopher 0 picked up chopstick 1
Philosopher 0 is EATING (round 1)
...
Philosopher 1 finished eating, putting down chopsticks
Philosopher 1 is done.

```

A healthy run has 15 "EATING" lines (3 meals times 5 philosophers), at most two philosophers eating at once, never two neighbours together, and an "is done" line for every philosopher.

## Tests

The full report in `docs/` has the output screenshot for each case.

| \# | Test case | Expected result | 
| ----- | ----- | ----- | 
| 1 | Compile both programs | No errors or warnings | 
| 2 | Run with no ID | Usage message, exits | 
| 3 | Run with an invalid ID, for example 7 | Error message, exits | 
| 4 | One philosopher alone | Finishes 3 meals without waiting | 
| 5 | Neighbours 0 and 1 together | Never eat at the same time | 
| 6 | Non-neighbours together | Can eat at the same time | 
| 7 | All five together | 15 meals, no deadlock | 
| 8 | Check the log of a full run | No two neighbours eating together, at most 2 eating at once | 
| 9 | `ls /dev/shm` while running | Six semaphore files are present | 
| 10 | Run again without `cleanup` after a normal finish | Works, because the values are restored | 
| 11 | Kill a philosopher while eating | Neighbours block until `cleanup` is run (known limitation) | 
| 12 | `./cleanup` | All six semaphores removed | 

## FAQ and viva questions

**Why named semaphores?**
Unrelated processes share no memory, so they need something with a system-wide name to find each other. A named semaphore provides that, and Linux keeps it as a file called `sem.<name>` in `/dev/shm`.

**Aren't the processes created by `fork()` anyway, since the shell starts them?**
A shell does fork internally to launch any program. The restriction in the problem is about the program: `philosopher.c` never calls `fork()`, and the five processes have no parent-child relationship with each other. For the cleanest demonstration, start them from five separate terminals.

**What does `/room` do, and why is its value 4?**
It admits at most four philosophers to the chopstick-picking stage. With five chopsticks and at most four competitors, at least one can always get both chopsticks, so the circular wait can never close. A value of 5 gives no protection. A value of 3 also works, but allows less parallelism.

**What happens without `/room`?**
Deadlock becomes possible: all five take their left chopstick, and each waits for the right one forever. The visualizer shows this with the room switched off.

**Why does every philosopher pick the left chopstick first?**
It keeps the code simple. It is safe only because `/room` prevents deadlock. Other deadlock-free designs make odd and even philosophers pick in different orders, or number the chopsticks and always pick the lower number first.

**What is the difference between `sem_close` and `sem_unlink`?**
`sem_close` only closes the semaphore in the calling process. `sem_unlink` removes the named semaphore from the system. That is why `cleanup.c` exists.

**Is the solution starvation-free?**
It is deadlock-free. A strict starvation guarantee would depend on the order in which the kernel wakes sleeping processes, which is not specified. In the tests every philosopher completed all meals.

**Why is "finished eating" printed before the chopsticks are released?**
So the log order is truthful. If it were printed after the release, a neighbour could print "EATING" before that line appeared, which would look like two neighbours eating together.

**Why `sleep()`?**
It simulates thinking and eating time. The thinking time of `1 + id % 3` seconds makes different philosophers hungry at different moments.

**What if a philosopher process is killed mid-meal?**
It never runs `sem_post`, so its chopsticks and its seat in the room stay taken, and its neighbours wait forever. Run `./cleanup` and start again.

**How would this differ with threads?**
Threads share memory, so unnamed semaphores (`sem_init`, `sem_destroy`) or mutexes are enough. The philosophers would be created with `pthread_create` and waited for with `pthread_join`, and no cleanup program would be needed.

**Does the visualizer run the C code?**
No. It is a JavaScript model of the same logic, written so each step matches a line of `philosopher.c`. It is meant for learning, not for measuring real kernel behaviour.

## Troubleshooting

| Problem | Cause and fix | 
| ----- | ----- | 
| The program seems stuck on "hungry" | An earlier run was stopped midway and left the semaphores locked. Stop it, run `./cleanup`, start again. | 
| Ctrl+C does nothing | The program was started with `&` and is a background job. Use `pkill philosopher`, or run it without `&`. | 
| Prompt seems missing after the one-terminal loop | The output keeps printing after the prompt. Press Enter. | 
| Output order differs from the report | Normal. The kernel schedules the processes differently on every run. | 
| Odd `sem_open` errors | Run `./cleanup` and try again. Make sure `/dev/shm` exists and is writable. | 

## Limitations and future work

* The number of philosophers is fixed at compile time (`N`).

* A killed philosopher leaves the semaphores locked until `./cleanup` is run. A `SIGINT`/`SIGTERM` handler that releases the chopsticks would fix this.

* There is no strict starvation guarantee.

* Semaphore names are system-wide, so two users running the program on one machine would interfere with each other.

* Possible extensions: take `N` and the number of meals from the command line, add a launcher script, print timestamps and waiting times, compare the asymmetric and resource-hierarchy solutions, and add a threaded version.





