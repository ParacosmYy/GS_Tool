"""Compatibility exports for split component codec modules.

New code should import configuration, helper, decoder, or registry APIs
from their owning module; this facade keeps the established import path.
"""

from .codecs_config import *
from .codecs_decoders import *
from .codecs_helpers import *
from .codecs_router import *
