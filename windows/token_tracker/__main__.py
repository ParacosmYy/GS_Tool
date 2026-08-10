"""CLI module entry point.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Delegate ``python -m token_tracker`` to the CLI parser.
"""

from .cli import main


if __name__ == "__main__":
    raise SystemExit(main())
