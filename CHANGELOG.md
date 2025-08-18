# Changelog

All notable changes to this project will be documented in this file.

The format is based on Keep a Changelog and this project adheres to Semantic Versioning.

## [0.0.10] - 2025-08-17
### Added
- Initial project scaffolding (CMake, C++17, optional Qt Widgets placeholder).
- Doxygen documentation pipeline with custom theme and sidebar/search enabled.
- Project logo and visual polish for docs (`docs/assets/customdox.css`).
- Documentation sources included in site: `README.md`, `REQUISITOS.md`, `PLANO-DE-DESENVOLVIMENTO.md`.
- New docs: `CHANGELOG.md`, `ROADMAP.md`.

### Changed
- Set project version to 0.0.10 in `CMakeLists.txt`, `README.md` and headers.
- Improved Doxyfile to avoid copy-to-self issues and keep default header.

### Known Issues
- If Qt is not installed, a console placeholder is built (see `CMakeLists.txt` message).
- One unresolved reference in `README.md` badge/anchor noted by Doxygen (harmless).

[0.0.10]: https://github.com/<your-org-or-user>/GenAi-E-Book-Reader/releases/tag/v0.0.10
