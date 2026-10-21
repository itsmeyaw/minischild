# Minischild

Use miniagents for implementation, menial, or simple tasks.

## Workflow

1. Create a git worktree inside .worktrees/ and ensure the directory is gitignored.
2. Create GitHub stack using skill `gh-stack`.
3. Perform the implementation using miniagents with `tdd` skill.
4. Format and lint.
5. Review the implementation using miniagents with skill `code-review` and following the `CODING_STANDARDS.md`.
6. Commit and sync the stack. Use `pr` skill top create the proper PR content.
7. Update all related tickets with comments.

## Development Note

- Commit in small functional change instead of one big change if possible to make review easier.
- When designing UI, ensure to use `impeccable` skill.

## Agent skills

### Issue tracker

Issues are tracked as GitHub Issues in itsmeyaw/minischild via the `gh` CLI. See `docs/agents/issue-tracker.md`.

### Domain docs

Single-context layout: `GLOSSARY.md` + `docs/adr/` at the repo root. See `docs/agents/domain.md`.
