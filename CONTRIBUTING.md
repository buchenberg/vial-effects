# Contributing to Vial Effects

## Commit Messages

This project follows the [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/) specification. All commit messages are validated in CI.

### Format

```
<type>[optional scope][optional !]: <description>

[optional body]

[optional footer(s)]
```

### Types

| Type | When to use |
|------|-------------|
| `feat` | New feature or capability |
| `fix` | Bug fix |
| `docs` | Documentation only |
| `style` | Formatting, whitespace (no code change) |
| `refactor` | Code restructuring, no behavior change |
| `perf` | Performance improvement |
| `test` | Adding or updating tests |
| `build` | Build system or external dependencies |
| `ci` | CI/CD configuration |
| `chore` | Maintenance, tooling |
| `revert` | Reverts a previous commit |

### Examples

```
feat(dsp): add stereo width control to chorus
```

```
fix(ui): resolve WebView2 crash on plugin close
```

```
docs: update build instructions for Windows installer
```

```
refactor(framework): extract processor routing logic

Moves routing into a dedicated ProcessorRouter class to reduce
branching in the main processBlock.
```

```
feat(api)!: change preset format to JSON

BREAKING CHANGE: Preset files are now JSON instead of binary.
Old presets must be re-exported from the plugin.
```

### Breaking Changes

Add `!` after the type/scope or include `BREAKING CHANGE:` in the footer:

```
feat(preset)!: migrate to JSON preset format
```

### Scope (optional)

Use a short identifier for the area of the codebase:
- `dsp` — audio processing / effects
- `ui` — web UI (React/Vite)
- `plugin` — JUCE plugin wrapper
- `build` — CMake / build system
- `ci` — GitHub Actions / workflows
- `installer` — Windows installer (Inno Setup)

### Rules

- Use imperative mood in the description: "add feature" not "added feature"
- Don't capitalize the first letter
- No period at the end
- Keep the first line under 72 characters
- Use the body to explain *what* and *why*, not *how*

## Pull Requests

When your PR is merged, the squash commit message should follow Conventional Commits. The title of your PR will be used as the squash commit message, so name it accordingly:

- `feat: add MIDI learn support`
- `fix(ui): prevent dropdown flicker on resize`
- `refactor(dsp): simplify delay line interpolation`
