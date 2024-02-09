# conf.py
#
# Sphinx configuration file.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html
#
# Copyright 2024 Nervosys, LLC
#

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = "AutonomySim"
copyright = 'Copyright &copy; 2024 <a href="https://nervosys.ai/">Nervosys, LLC</a>'
author = "Nervosys"

version = "latest"
release = "latest"

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = []
extensions.append("sphinx_immaterial")

templates_path = ["_templates"]
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output
# https://jbms.github.io/sphinx-immaterial/customization.html
# see `theme.conf` for more information

html_theme = "sphinx_immaterial"
html_static_path = ["_static"]

html_theme_options = {
    "site_url": "https://nervosys.github.io/AutonomySim",
    "repo_url": "https://github.com/nervosys/AutonomySim",
    "repo_name": "AutonomySim",
    "edit_uri": "blob/master/docs",
    "features": [
        "announce.dismiss",
        "content.code.annotate",
        "content.code.copy",
        "content.tabs.link",
        # 'header.autohide',
        # 'mkdocstrings',
        "navigation.expand",
        # 'navigation.instant',
        "navigation.top",
        # 'navigation.tabs',
        # 'navigation.sections',
        "navigation.instant",
        "navigation.path",
        "navigation.footer",
        # 'navigation.tracking',
        # 'search.highlight',
        "search.suggest",
        "search.share",
        "toc.follow",
        "toc.integrate",
    ],
    "font": {
        "text": "Roboto",
        "code": "Roboto Mono",
    },
    "language": "en",
    "palette": [
        {
            "media": "(prefers-color-scheme)",
            "toggle": {
                "icon": "material/brightness-auto",
                "name": "Switch to light mode",
            },
        },
        {
            "media": "(prefers-color-scheme: light)",
            "scheme": "nervosys",
            "toggle": {
                "icon": "material/brightness-7",
                "name": "Switch to dark mode",
            },
        },
        {
            "media": "(prefers-color-scheme: dark)",
            "scheme": "slate",
            "toggle": {
                "icon": "material/brightness-4",
                "name": "Switch to system preference",
            },
        },
    ],
    "favicon": "../images/rune.svg",
    "icon": {
        "logo": "material/shield-airplane",
        "repo": "fontawesome/brands/git-alt",
        "edit": "material/pencil",
        "view": "material/eye",
    },
    "globaltoc_collapse": True,
    "toc_title_is_page_title": True,
    "version_dropdown": True,
    "version_info": [
        {
            "version": "https://nervosys.github.io/AutonomySim",
            "title": "Github Pages",
            "aliases": [],
        },
    ],
    # icon links (not necessarily limited to social media)
    "social": [
        {
            "icon": "fontawesome/brands/github-alt",
            "link": "https://github.com/nervosys/AutonomySim",
        },
        {
            "icon": "fontawesome/brands/discord",
            "link": "https://discord.gg/x84JXYje",
        },
        {
            "icon": "fontawesome/brands/x-twitter",
            "link": "https://x.com/nervosys",
        },
    ],
}
