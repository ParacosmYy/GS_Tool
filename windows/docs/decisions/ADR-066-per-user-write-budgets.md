# ADR-066: Shared per-user write budgets

## Status

Accepted

## Date

2026-08-10

## Context

The project is intended to be shared with classmates through one Windows
service. Usage records, work events, diagnostic logs, and administrator CSV
exports are authenticated writes or expensive read operations. Authentication
and provider routes already have rate limits, but a client could otherwise
repeat the small write endpoints until the local SQLite file or audit surface
grew without bound.

The Web and Android clients also share one account and must not receive two
different safety policies that can be bypassed by switching clients.

## Decision

Keep resource budgets in the central `rate_limit` policy boundary and expose
`allow_user_write(resource, user_id)` to both Web and `/api/v1` controllers.
The current sliding-window budgets are:

| Resource | Budget |
| --- | ---: |
| Usage record | 120 requests / 60 seconds |
| Work event | 180 requests / 60 seconds |
| Application log | 300 requests / 60 seconds |
| Administrator export | 12 requests / 60 seconds |

The bucket identity is a server-side user ID. In shared, LAN, and production
modes it is stored only as a hash in the existing SQLite limiter table; local
mode retains the process-local adapter. Unknown policy names fail closed.

## Alternatives considered

### Per-IP-only throttling

Rejected: authenticated classmates can share an IP behind one router, while a
single user can rotate addresses or use two clients to bypass a user budget.

### Controller-local constants

Rejected: duplicated limits drift between Web and Android and make new write
routes easy to ship without a safety budget.

### Unlimited writes with only a payload-size cap

Rejected: a bounded request body does not bound the number of SQLite rows or
export operations over time.

## Consequences

- Clients receive a stable `429 RATE_LIMITED` response and should back off.
- Browser double-submit is additionally disabled while the manual form is in flight.
- Limits are intentionally generous for normal use and can be replaced by a
  dedicated distributed limiter when the service outgrows one Windows host.
- The policy does not replace data retention, backup, or operational monitoring.
