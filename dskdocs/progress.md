# OpenTyrian 60fps Project — Progress Log

## Goal
Take OpenTyrian (open-source port of Tyrian) and get it running smoothly at 60fps,
using Claude Code to do the actual implementation work.

## Key context / decisions
- Original Tyrian's game logic is tied directly to its ~35fps timer (based on the
  original DOS PC timer frequency). This isn't a rendering bottleneck — the frame
  rate and gameplay speed are the same variable in the original code.
- Upstream maintainers closed a feature request for 60fps as "won't fix" — they
  said decoupling logic from rendering would require rearchitecting the whole game.
- Two possible approaches were identified:
  - **Option A (recommended starting point):** Keep game logic at ~35Hz, but
    interpolate sprite positions for rendering so the display runs at 60fps+ and
    looks smooth, without touching gameplay balance. Bounded, safer scope.
  - **Option B:** Actually run all game logic at 60Hz — requires retuning every
    velocity, timer, and enemy pattern currently expressed in ~35Hz ticks. Much
    larger scope, real risk of changing game feel even if "the logic is the same."
- Starting with Option A.

## Machine / environment setup
- Headless Ubuntu box, hostname `docker`.
- Connected via SSH + VS Code Remote-SSH.
- No physical monitor — xrdp + XFCE installed so the game can actually be watched
  running (SSH X11 forwarding was avoided since network-tunneled frames would
  corrupt any judgment of real frame smoothness).

## Repo status
- Forked to: `github.com/d0sk3y/opentyrian` (private — upstream maintainer has no
  visibility into this fork unless a PR is explicitly opened)
- Local clone: `~/tyrian` on the Ubuntu box
  - `origin` → your fork
  - `upstream` → `opentyrian/opentyrian` (the real project, for pulling future
    fixes if wanted)
- Branch: `interpolated-60fps` — created and pushed, 1 commit ahead of fork's
  `master` (just the branch-creation commit so far, no real changes yet)

## Build status
- Dependencies installed: `build-essential`, `libsdl2-dev`, `libsdl2-net-dev`,
  `unzip`, `curl`
- `make` runs clean, produces working `opentyrian` binary
- Data files: Tyrian 2.1 freeware data (`tyrian21.zip` from camanis.net)
  - Gotcha #1: zip extracts into a nested `tyrian21/` subfolder — had to `mv
    tyrian21/* .` inside `data/` to flatten it
  - Gotcha #2: original DOS filenames are uppercase; had to run the repo's
    included `lower-script.sh` from `~/tyrian` (not from inside `data/`) to
    lowercase everything
- Confirmed working: `./opentyrian` launches, loads data, plays correctly via
  the xrdp desktop session

## Claude Code status
- Installed via `npm install -g @anthropic-ai/claude-code`
- **Blocked:** subscription-based login requires Pro/Max plan (or an Anthropic
  API key as an alternative). Was on Free — upgrading to Pro to resolve.

## Next steps (once Claude Code is authenticated)
1. `cd ~/tyrian`, run `claude`, log in with Pro
2. Confirm still on `interpolated-60fps` branch (`git branch`)
3. First prompt: ask Claude Code to read `src/nortsong.c` and explain the
   current frame timing / game loop — specifically `jasondelay`, `target`,
   `target2`, and the `SDL_Delay()` calls — **without changing anything yet**.
   This is a review checkpoint before any code gets written.
4. Once the explanation checks out, scope the actual interpolation change as a
   small, reviewable diff — not "make it 60fps" as one big ask.
