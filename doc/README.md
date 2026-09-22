# libopenwch documentation

This directory builds one project-wide landing site plus one self-contained
HTML API reference per supported family:

```sh
make html                 # root site + both family sites
make html TARGETS=ch32v0  # root site + CH32V00x site
```

Outputs:

| Site | Path | Warning log |
| --- | --- | --- |
| project root | `doc/html/index.html` | `doc/doxygen-root.log` |
| CH32V00x API | `doc/ch32v0/html/index.html` | `doc/ch32v0/doxygen.log` |
| CH58x API | `doc/ch5xx58x/html/index.html` | `doc/ch5xx58x/doxygen.log` |

The generated HTML sites are intentionally not committed; `.gitignore` covers
them and the warning logs.

## Requirements

* Doxygen (the `DOXYGEN` variable can point at a non-system binary)
* No Graphviz required: `HAVE_DOT = NO` in both Doxyfiles

The sites use Doxygen Awesome's optional JavaScript extensions, so a recent
Doxygen is recommended (the theme itself lists 1.9.6 and later).

## How it is put together

| Path | Purpose |
| --- | --- |
| `Doxyfile.in` | per-family template, substituted by the Makefile |
| `Doxyfile-root` | project-wide root-site configuration |
| `Makefile` | builds the generated headers, root site, and family sites |
| `source/` | per-family main page, group hierarchy, and licence page |
| `root/` | cross-family overview, architecture, build, API map, licence pages |
| `templates/` | header template, project logo, and small project stylesheet |
| `awesome/` | vendored Doxygen Awesome assets and their MIT licence |

The top-level `make html`/`make doc` target declares `irq` as a prerequisite
(note: `irq` itself does not need a RISC-V toolchain), then delegates to
`doc/Makefile`.  Doxygen runs from the repository root so source paths in the
HTML are clean and stable.

If you invoke `doc/Makefile` directly (for example `make -C doc html`), first
run `make irq` from the repository root; otherwise the generated `nvic.h`
files may be missing and the interrupt API will be incomplete.

The per-family input set is selected by substitution: the shared QingKe core,
the dispatch and mini-libc headers, and only the requested
`include/libopenwch/<family>` and `lib/<family>` trees.  For `ch5xx58x` the BLE
wrapper is added; WCH's vendored header under `include/libopenwch/ble/wch/` is
excluded because it is third-party documentation, not part of this project's
API.

The root site deliberately does **not** merge the two families' API symbols.
The family references use the same group names for the same peripheral (for
example `gpio_defines`), and each family is documented separately.  Instead,
`doc/root/` provides cross-family prose and `doc/root/api_index.dox` links
directly into the generated group pages of both family sites.
