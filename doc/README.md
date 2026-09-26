# libopenwch documentation

Builds one project-wide site and one API site per family:

```sh
make html                 # root + both families
make html TARGETS=ch32v0  # root + CH32V00x
```

| Site | Output | Warning log |
|---|---|---|
| root | `doc/html/index.html` | `doc/doxygen-root.log` |
| CH32V00x | `doc/ch32v0/html/index.html` | `doc/ch32v0/doxygen.log` |
| CH58x | `doc/ch5xx58x/html/index.html` | `doc/ch5xx58x/doxygen.log` |

Generated HTML is not committed. Requirements: Doxygen (override with
`DOXYGEN=`); no Graphviz.

| Path | Purpose |
|---|---|
| `Doxyfile.in`, `Doxyfile-root` | per-family and root configuration |
| `Makefile` | builds generated headers and all sites |
| `source/`, `root/` | per-family and cross-family prose |
| `templates/` | header, logo, stylesheet |
| `awesome/` | vendored Doxygen Awesome (MIT) |

The top-level `make html` depends on `irq` so the `nvic.h` headers exist; the
generated headers need no RISC-V toolchain. The root site links into each family
site's group pages rather than merging duplicate symbol names.
