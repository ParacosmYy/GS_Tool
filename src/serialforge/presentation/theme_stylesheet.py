"""Composed stylesheet assembled from bounded theme modules."""

from __future__ import annotations

from .theme_stylesheet_base import BASE_STYLESHEET
from .theme_stylesheet_controls import CONTROLS_STYLESHEET
from .theme_stylesheet_extension import EXTENSION_STYLESHEET

THEME_STYLESHEET = BASE_STYLESHEET + EXTENSION_STYLESHEET + CONTROLS_STYLESHEET

__all__ = ["THEME_STYLESHEET"]
