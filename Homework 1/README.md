# Homework 01 — Setting up a Linux Machine on CloudVeneto for C Development

## 1. What this is and how to use it

This guide walks through the full setup of a CloudVeneto virtual machine (VM) for
compiling and running C code, starting from nothing and ending with a working
toolchain (`gcc`, `make`, `git`, plus the libraries used later in the course:
GSL and HDF5).

It is organized as:

1. **Connecting to the VM** (SSH, keys, permissions, common pitfalls on Windows/WSL).
2. **Installing the development environment** (compiler, build tools, libraries).
3. **Compiling and running a first C program**, to verify everything works.
4. **Good practices** for working remotely (persistent sessions, file transfer, Git).

> **Course policy reminder:** every file, comment, and commit message in the
> repository must be written in English.

---

## 2. Connecting to the VM via SSH

CloudVeneto provisions a virtual machine (in this course, running **AlmaLinux**)
reachable only via **SSH** (Secure Shell). You need:

- the **public IP / hostname** of the VM (provided by CloudVeneto once the
  instance is created),
- your **username** on the VM,
- a **private SSH key** matching a public key registered on the VM (or, if
  enabled, password authentication — key-based access is strongly recommended).

### 2.1 Generating an SSH key pair (if you don't have one yet)

On your **local machine** (not on the VM):

```bash
ssh-keygen -t ed25519 -C "your_email@example.com" -f ~/.ssh/cloudveneto_key
```

This creates two files:

- `~/.ssh/cloudveneto_key` — the **private** key (never share this).
- `~/.ssh/cloudveneto_key.pub` — the **public** key (copy this into the VM's
  `~/.ssh/authorized_keys`, or paste it into the CloudVeneto dashboard when
  creating/configuring the instance).

### 2.2 File permissions — the most common source of connection failures

SSH is strict about key permissions on **both ends**:

- **Locally**, your private key must not be readable by anyone else:

  ```bash
  chmod 600 ~/.ssh/cloudveneto_key
  ```

- **On the VM**, the `~/.ssh` directory and the `authorized_keys` file must
  also have restrictive permissions:

  ```bash
  chmod 700 ~/.ssh
  chmod 600 ~/.ssh/authorized_keys
  ```

Permissions are expressed as a 3-digit octal number: `600` means
*read+write for the owner, nothing for group or others*. If these
permissions are too open (e.g. `644` or `777`), the SSH daemon will
**silently refuse** the key-based login and fall back to asking for a
password (or reject the connection entirely) — this is by far the most
common reason a "correct" key stops working.

### 2.3 Streamlining the connection with `~/.ssh/config`

Instead of typing the full `ssh -i <key> user@host` command every time, add
an entry to `~/.ssh/config` on your local machine:

```sshconfig
Host cloudveneto
    HostName <VM_PUBLIC_IP_OR_HOSTNAME>
    User <your_username>
    IdentityFile ~/.ssh/cloudveneto_key
    ServerAliveInterval 60
```

From now on, you can simply run:

```bash
ssh cloudveneto
```

### 2.4 Known issue: Windows Subsystem for Linux (WSL) routing bug

On **Windows 11** with **WSL2**, a known networking/routing bug can prevent
the SSH tunnel from reaching the CloudVeneto gateway directly. If you connect
from WSL and the connection hangs or times out (while it works from a plain
Windows terminal, or vice versa), add a `ProxyCommand` to force the
connection through a working path, e.g.:

```sshconfig
Host cloudveneto
    HostName <VM_PUBLIC_IP_OR_HOSTNAME>
    User <your_username>
    IdentityFile ~/.ssh/cloudveneto_key
    ProxyCommand ssh.exe -i C:/Users/<you>/.ssh/cloudveneto_key -W %h:%p <jump_host_if_any>
```

The exact `ProxyCommand` depends on your network setup; if the direct
connection from WSL fails, try connecting from a native Windows OpenSSH
client first to confirm the credentials are correct, then adapt the
`ProxyCommand`/network settings accordingly (or use the Windows-native
client instead of WSL for this specific connection).

---

## 3. First steps on the VM (AlmaLinux)

Once connected, you operate as a **standard (non-root) user**. Installing
software requires temporarily escalating privileges with `sudo`, which asks
for your password and runs a single command as root — without permanently
switching user.

AlmaLinux is RHEL-based, so the package manager is `dnf` (the modern
replacement for `yum`).

### 3.1 Updating the system

```bash
sudo dnf update -y
```

### 3.2 Installing the C development toolchain

```bash
sudo dnf groupinstall -y "Development Tools"
sudo dnf install -y gcc gcc-c++ make cmake git vim
```

This installs:

- `gcc` / `gcc-c++` — the C/C++ compilers.
- `make` — to run Makefiles (used throughout this course's homework).
- `cmake` — for more structured, multi-file projects.
- `git` — for version control and submitting homework.
- `vim` — a lightweight terminal text editor (feel free to install `nano` or
  set up a remote-editing workflow with VS Code's Remote-SSH extension
  instead, if preferred).

Verify the installation:

```bash
gcc --version
make --version
git --version
```

### 3.3 Installing scientific libraries used later in the course

Some homework assignments require the **GNU Scientific Library (GSL)** and
**HDF5**. Install their development packages (headers + linkable libraries)
now, so they are ready when needed:

```bash
sudo dnf install -y gsl-devel hdf5-devel
```

If `hdf5-devel` is not found under that exact name, check available
packages with:

```bash
dnf search hdf5
dnf search gsl
```

(On some AlmaLinux/EPEL configurations the `epel-release` repository must be
enabled first: `sudo dnf install -y epel-release`.)

---

## 4. Compiling and running a first C program

Create a small test file to confirm the toolchain works end-to-end.

```bash
mkdir -p ~/sandbox && cd ~/sandbox
cat << 'EOF' > hello.c
#include <stdio.h>

int main(void) {
    printf("Hello from the CloudVeneto VM!\n");
    return 0;
}
EOF
```

Compile it:

```bash
gcc -Wall -Wextra -O2 -std=c11 hello.c -o hello
```

Run it:

```bash
./hello
```

Expected output:

```
Hello from the CloudVeneto VM!
```

### 4.1 Compiler flags used throughout this repository

Every homework in this repository compiles with (at least) the following
flags, consistently with the course's recommendations:

| Flag            | Purpose                                                          |
|-----------------|-------------------------------------------------------------------|
| `-std=c11`      | Targets the C11 language standard explicitly.                    |
| `-Wall -Wextra` | Enables (almost) all compiler warnings — catches likely bugs.     |
| `-O2`           | Enables standard optimizations (used for benchmarking exercises). |
| `-lm`           | Links the math library (`sqrt`, `pow`, etc.) when needed.        |
| `-lgsl -lgslcblas -lm` | Links GSL where used (e.g. high-precision reference values). |
| `-lhdf5`        | Links HDF5 where used (binary output of large datasets).         |

---

## 5. Good practices for working remotely

- **Use `tmux` or `screen`** for long-running jobs (e.g. `N = 10^8` vector
  benchmarks): they keep your session alive even if your SSH connection
  drops.

  ```bash
  sudo dnf install -y tmux
  tmux new -s scicomp
  # ... run your long job ...
  # detach with: Ctrl+b then d
  # reattach later with: tmux attach -t scicomp
  ```

- **Transfer files** with `scp` or `rsync` rather than copy-pasting code
  through the terminal:

  ```bash
  scp -r ./Homework_02_Vector_Sum cloudveneto:~/scientific-computing/
  rsync -avz ./Homework_02_Vector_Sum cloudveneto:~/scientific-computing/
  ```

- **Use Git from the start.** Initialize the repository locally or on the
  VM, commit early and often, and push to the course's GitHub remote. All
  commit messages and code comments must be in English (course policy).

- **Keep large/generated files out of Git.** Compiled binaries, `.o` files,
  and large data files (e.g. the `.h5` outputs from Homework 06) belong in
  `.gitignore`, not in version control. A minimal `.gitignore` for this
  repository:

  ```gitignore
  *.o
  *.out
  *.h5
  /bin/
  /build/
  ```

---

## 6. Summary checklist

- [ ] Generated an SSH key pair and set `600` permissions on the private key.
- [ ] Connected successfully with `ssh cloudveneto` (using `~/.ssh/config`).
- [ ] Solved the WSL `ProxyCommand` issue, if applicable.
- [ ] Updated the VM and installed `gcc`, `make`, `cmake`, `git`.
- [ ] Installed `gsl-devel` and `hdf5-devel` for later homework.
- [ ] Compiled and ran a "Hello World" C program successfully.
- [ ] Set up `tmux` and a `.gitignore` for the course repository.

With this environment in place, every subsequent homework in this repository
(`Homework_02` through `Homework_07`) can be compiled and run directly on the
CloudVeneto VM using the provided `Makefile`s.
