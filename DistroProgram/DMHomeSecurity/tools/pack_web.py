#!/usr/bin/env python3
"""DMHomeSecurity - rebuilds web_assets.h from the files in the web/ folder.

You only need this if you change the website (web/index.html, app.css, app.js).
Run it from the DMHomeSecurity folder:   python3 tools/pack_web.py
Then upload to the master camera again.
"""
import pathlib, sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
ASSETS = [("WEB_INDEX_HTML", "web/index.html"),
          ("WEB_APP_CSS",    "web/app.css"),
          ("WEB_APP_JS",     "web/app.js")]
DELIM = "DMWEB"

out = ["// Made automatically by tools/pack_web.py - edit the files in web/ instead, then re-run it.",
       "#pragma once", ""]
total = 0
for name, rel in ASSETS:
    text = (ROOT / rel).read_text(encoding="utf-8")
    if f"){DELIM}\"" in text:
        sys.exit(f"{rel} contains the raw-string delimiter")
    total += len(text.encode())
    out.append(f"static const char {name}[] = R\"{DELIM}({text}){DELIM}\";")
    out.append("")
(ROOT / "web_assets.h").write_text("\n".join(out), encoding="utf-8")
print(f"web_assets.h written ({total} bytes of assets)")
