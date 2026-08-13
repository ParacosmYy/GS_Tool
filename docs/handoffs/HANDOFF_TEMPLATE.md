# Handoff: <HANDOFF_ID>

| Field | Value |
|---|---|
| ID | `<HANDOFF_ID>` |
| Delivery / slice | `<DELIVERY>` |
| Status | `<STATUS>` |
| Owner | `<OWNER>` |
| Checkout | Current local checkout only |
| Created | `<CREATED_AT>` |

## User outcome

<State the user-visible or governance outcome in one or two concrete sentences.>

## Scope and boundaries

### In scope

- <item>

### Out of scope

- <item>

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `<OWNER>` | Integration, final review, verification, and handoff decision |
| Project Manager | `<PM_OWNER>` | Plan, dependencies, risks, and status |
| Product | `<PRODUCT_OWNER>` | User outcome and acceptance |
| Developer 1 | `<DEV1_OWNER>` | Domain/application/infrastructure slice |
| Developer 2 | `<DEV2_OWNER>` | Presentation/integration/packaging slice |
| QA | `<QA_OWNER>` | Read-only verification and unrun evidence |

## Changed files and modules

- `<path>` — <why it changed>

## Decisions and constraints

- <architecture, compatibility, security, or process decision>
- Shared checkout writer: <owner and exact scope>.
- Runtime launch policy: <state whether launch was allowed and why>.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `<command>` | `<PASS / FAIL / NOT RUN>` | <actual observation>

## Unrun checks and reason

- <check not run> — <specific reason and owner for follow-up>

## Known risks and limits

- <remaining risk or explicit support boundary>

## Acceptance and evidence IDs

- Acceptance: `<ID>`
- Evidence: `<path, command, review, or artifact identity>`

## Next owner and next action

- Owner: `<OWNER>`
- Action: <single concrete next step>

## Artifact information

- Artifact path: `<path or none>`
- Version: `<version or not changed>`
- SHA-256 / size: `<identity or not changed>`
- Packaging note: <portable, rebuilt, unchanged, or not applicable>

## Disposition

<Choose one: in-progress, accepted-with-limits, completed, blocked. Explain the
decision and the condition for the next handoff.>
