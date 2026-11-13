# Preparing a release

## Before export

1. Build the client and installer using [BUILD_WINDOWS.md](BUILD_WINDOWS.md).
2. Complete a co-op battle and duel with two clients running the exact new EXE.
3. Check title/lobby/setup navigation and configuration errors in real game windows.
4. Keep connection settings, credentials, game data and local build outputs outside the published package.

## Export

```powershell
py -3 scripts/package_release.py --output C:\Releases\Touhou09-Netplay-v1.1.0
```

Use a new directory outside the development project. The exporter copies an allowlist of source/docs/tools, the client, installer and `.env.example`. It uses locally built binaries when available and otherwise uses the bundled `mod/` binaries. It excludes actual `.env` files, game archives, saves, SDKs, test files, reports, build outputs and private logs.

For an additional private-data check, pass `--private-values PATH_TO_PRIVATE_JSON` to the exporter. That local JSON contains sensitive strings to reject in every exported text/binary file. It must remain outside the release.

The archive contains matching GPL-3.0 source, the mod EXE, installer, configuration templates, guides and SHA-256 manifest. The exporter validates file hashes, archive contents, configuration templates and private data. Inspect the exported folder before publishing it.

## Fresh repository

Initialize Git only in the clean export when replacing a deleted repository:

```powershell
git init -b main
git add .
git commit -m "Initial clean release"
gh repo create YOUR_ACCOUNT/YOUR_REPOSITORY --private --source . --remote origin --push
```

This starts a new history. Do not copy a previous `.git` directory or import its tags. Check `git rev-list --all --count` and `git status` before publishing. Keep the repository private until its contents and intended access are reviewed. A private GitHub repository requires explicit invitations; a link alone does not grant access.

Do not bundle infrastructure addresses, SSH credentials, host fingerprints or DNS-provider tokens. People configure their own client `.env` after installation. Keep license notices and provide the source corresponding to every distributed EXE.
