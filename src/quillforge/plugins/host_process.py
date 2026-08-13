"""Qt-free diagnostic process entrypoint for the plugin-host protocol."""

import sys
from collections.abc import Sequence

from .host_protocol import (
    PLUGIN_HOST_MAX_FRAME_BYTES,
    PluginHostProtocolError,
    decode_frame,
    encode_frame,
    make_error,
    make_hello,
    make_probe_result,
    parse_probe_request,
)


def run_plugin_host(arguments: Sequence[str] | None = None) -> int:
    """Run one bounded probe exchange and exit without importing Qt or plugins."""
    command_arguments = tuple(arguments if arguments is not None else sys.argv)
    if not _is_exact_probe_invocation(command_arguments):
        return 2
    output = sys.stdout.buffer
    try:
        output.write(encode_frame(make_hello(_current_pid())))
        output.flush()
        raw_request = sys.stdin.buffer.readline(PLUGIN_HOST_MAX_FRAME_BYTES + 1)
        if not raw_request:
            raise PluginHostProtocolError("Host probe request is missing")
        parse_probe_request(decode_frame(raw_request))
        output.write(encode_frame(make_probe_result(_current_pid())))
        output.flush()
    except (BrokenPipeError, OSError):
        return 3
    except PluginHostProtocolError as error:
        try:
            output.write(encode_frame(make_error(str(error))))
            output.flush()
        except (BrokenPipeError, OSError, ValueError):
            pass
        return 4
    return 0


def _current_pid() -> int:
    """Keep the PID lookup local to the process entrypoint."""
    import os

    return os.getpid()


def _is_exact_probe_invocation(arguments: tuple[str, ...]) -> bool:
    """Accept only the documented executable plus probe-mode arguments."""
    return len(arguments) == 3 and arguments[1:] == ("--plugin-host", "--probe")


if __name__ == "__main__":
    raise SystemExit(run_plugin_host())
