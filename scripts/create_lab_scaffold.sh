#!/usr/bin/env bash
set -euo pipefail

mkdir -p scripts
mkdir -p docs/lab1 docs/lab2 docs/devlog
mkdir -p assets/lab1 assets/lab2

for lab in lab1 lab2; do
  mkdir -p "labs/${lab}/task1"
  mkdir -p "labs/${lab}/task2"
  mkdir -p "labs/${lab}/docs"
  mkdir -p "labs/${lab}/setup"

  touch "labs/${lab}/README.md"

  touch "labs/${lab}/setup/ssh-setup.md"
  touch "labs/${lab}/setup/vitis-guide.md"
  touch "labs/${lab}/setup/vivado-guide.md"
  touch "labs/${lab}/setup/student-guide.md"
  touch "labs/${lab}/setup/troubleshooting.md"

  touch "labs/${lab}/task1/commands.md"
  touch "labs/${lab}/task1/instructor-notes.md"

  touch "labs/${lab}/task2/commands.md"
  touch "labs/${lab}/task2/instructor-notes.md"

  touch "docs/${lab}/README.md"
  touch "docs/${lab}/student-guide.md"
  touch "docs/${lab}/troubleshooting.md"
  touch "docs/devlog/${lab}-working-log.md"
done

# keep empty asset directories tracked by git
touch assets/lab1/.gitkeep
touch assets/lab2/.gitkeep

echo "lab scaffold created."
