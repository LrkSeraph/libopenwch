# libopenwch documentation

Builds one root site plus one API site per family:

```sh
make html                 # root + both families
make html TARGETS=ch32v0  # root + CH32V00x
```

| Site | Output | Log |
|---|---|---|
| root | `doc/html/index.html` | `doc/doxygen-root.log` |
| CH32V00x | `doc/ch32v0/html/index.html` | `doc/ch32v0/doxygen.log` |
| CH58x | `doc/ch5xx58x/html/index.html` | `doc/ch5xx58x/doxygen.log` |

Requires Doxygen (`DOXYGEN=` override); no Graphviz. Generated HTML is not committed.

| Path | Purpose |
|---|---|
| `Doxyfile.in`, `Doxyfile-root` | family/root config |
| `source/`, `root/` | family and cross-family prose |
| `templates/` | header, logo, stylesheet |
| `awesome/` | vendored Doxygen Awesome (MIT) |

Top-level `make html` depends on `irq` so `nvic.h` exists; root site links into
family group pages instead of merging duplicate symbols.
