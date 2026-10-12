#!/bin/sh
set -e
REPO="guillemetall/Angel-Protocol"
TOKEN=$(sed 's|.*://[^:]*:\([^@]*\)@.*|\1|' ~/.git-credentials)
RUN_ID=$(curl -s -H "Authorization: token $TOKEN" "https://api.github.com/repos/$REPO/actions/runs?per_page=1" | grep -oE '"id": [0-9]+' | head -1 | grep -oE '[0-9]+')
echo "Run: $RUN_ID"
curl -sL -H "Authorization: token $TOKEN" "https://api.github.com/repos/$REPO/actions/runs/$RUN_ID/logs" -o /tmp/run.zip
command -v unzip >/dev/null || apk add unzip
rm -rf /tmp/runlog && mkdir -p /tmp/runlog
unzip -o /tmp/run.zip -d /tmp/runlog >/dev/null
echo "--- total warnings ---"
grep -rh "warning:" /tmp/runlog 2>/dev/null | wc -l
echo "--- top 20 archivos con más warnings ---"
grep -rh "warning:" /tmp/runlog 2>/dev/null | sed 's/.*\(main\/[^:]*\).*/\1/' | sort | uniq -c | sort -rn | head -20
