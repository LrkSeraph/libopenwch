# libopenwch API documentation

This directory builds one self-contained HTML API site per supported family:

```sh
make html                 # both families
make html TARGETS=ch32v0  # one family
```

The generated sites are written to `doc/<family>/html/` and are intentionally
not committed; `.gitignore` covers them.  Doxygen warnings are collected in
`doc/<family>/doxygen.log`.

## Requirements

* Doxygen (the `DOXYGEN` variable can point at a non-system binary)
* No Graphviz required: `HAVE_DOT = NO` in the template

The Doxyfile also uses Doxygen Awesome's optional JavaScript extensions, so a
recent Doxygen is recommended (the theme itself lists 1.9.6 and later).

## How it is put together

| Path | Purpose |
| --- | --- |
| `Doxyfile.in` | one template, substituted by the Makefile for each family |
| `Makefile` | per-family substitution and Doxygen invocation |
| `source/` | main page, group hierarchy, and licence page |
| `templates/` | header template, project logo, and small project stylesheet |
| `awesome/` | vendored Doxygen Awesome assets and their MIT licence |

`doc/Makefile` first builds the generated interrupt headers with
`make -C .. irq` (no RISC-V toolchain needed), then runs Doxygen from the
repository root so source paths in the HTML are clean and stable.

The input set is selected per family: the shared QingKe core, the dispatch and
mini-libc headers, and only the requested `include/libopenwch/<family>` and
`lib/<family>` trees.  For `ch5xx58x` the BLE wrapper is added; WCH's vendored
vendor header under `include/libopenwch/ble/wch/` is excluded because it is
third-party documentation, not part of this project's API.
