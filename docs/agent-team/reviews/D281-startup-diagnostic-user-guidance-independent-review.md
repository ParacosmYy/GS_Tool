# D281 independent review — startup diagnostic user guidance

## Result

Initial review: `REVISE`. The reviewer found that README omitted argument
parse failures from the exit-code description and that “only” could be read as
excluding the outer generic `status`/error metadata.

Follow-up review: `PASS`. The README was corrected to cover argument parsing,
report-write, and check failures returning `2`, and to distinguish the four
settings business fields from the outer diagnostic record. The same reviewer
confirmed the correction against `app.py` in a bounded follow-up.

## Assigned scope

Check README command syntax, `settings_preflight` field accuracy, exit-code
wording, privacy statement, and the distinction between static/source
diagnostic evidence and native startup.

## Parent evidence retained

The parent review recorded PASS after the correction; source diagnostic and
project checks passed. The initial revise finding remains recorded rather than
being erased by the follow-up.

## Applicability and limits

Python standard-library documentation and the existing QuillForge source
contract are applicable. No manufacturer or embedded requirement applies;
native EXE/Qt behavior remains unverified.
