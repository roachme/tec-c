# Jira-style PM features missing from tec

Gap analysis comparing tec against a typical Jira-like project manager CLI,
cross-checked against `TODO` so this doesn't duplicate already-scoped work.

## Already covered by tec, just under different names

- Custom fields -> the `unit` key/val store already handles this generically
  (more flexible than Jira's rigid custom-field config)
- Boards/workflow columns -> `TODO` Milestone #1 already decided this is a
  plugin's job, not core
- Issue type / priority -> `type`/`prio` in `cli/set.c`

## Real gaps, not yet in TODO

| Jira feature | Gap in tec |
|---|---|
| Assignee / multi-user | No concept of "who owns this" - everything is single-user local state (`TODO` Milestone #4 flags this but it's undecided) |
| Comments / activity feed | No append-only history per task - closest is idea #42 (undo log), but that's for recovery, not a conversation trail |
| Subtasks / issue hierarchy | env->desk->task is a fixed 3-level structure, not arbitrary parent/child links between tasks |
| Sprints / backlog | No concept of time-boxed iterations grouping tasks - desks are the closest analog but aren't date-scoped |
| Search / JQL-style filtering | `ls` has no `--filter key=val` or query language over units; idea #30 (tags) and #43 (`--overdue`) are narrow slices of this |
| Story points / estimation | No numeric-effort unit or burndown-style aggregation (idea #33 `stats` is closest, but counts only) |
| Issue linking (blocks/relates-to) | No cross-task relationship at all, even informally |
| Attachments | Notes/sketches are files in the task dir already, but nothing structured tracks "attached files" as metadata |
| Notifications / watchers | Only plugin idea #5 (due-date reminders) touches this; no "watch this task" concept |
| Permissions / roles | N/A - single local user, no ACL concept exists anywhere |
| Bulk edit | `set`/`mv` operate one task (or a couple IDs) at a time; no `tec set --filter type=bug prio=high` batch update |

## Already on TODO, mapped to the Jira gap they cover

- Tags/labels -> idea #30
- Due dates + overdue filter -> idea #43
- Archive instead of hard delete -> idea #31/#37
- Stats/dashboard -> idea #33
- JSON output for scripting -> idea #35
- Op log / undo -> idea #42

## Takeaway

The biggest structural gap is **multi-user + assignee + comments** - that's
the one place Jira's whole value prop (team coordination) has no analog in
tec at all, versus everything else being "a plugin or a TODO item away."
