# Generating the Documentation

!!! note
    This is only useful if you want to host your own `AutonomySim` documentation site (e.g., in a secure enclave).

The `AutonomySim` documentation [website](https://nervosys.github.io/AutonomySim/) HTML and CSS files are automatically generated from Markdown files and deployed to GitHub Pages using the `mkdocs` package for Python. You can also self-host the documentation or redirect GitHub Pages to your own domain. The choice is yours.

Compared to `AirSim` and its forks, we offer simpler and cleaner documentation in a modern style.

## Configure GitHub Repository

Configure the GitHub repository to automatically deploy documentation to a GitHub project site from the `gh-pages` branch:

* Web browser &rarr; GitHub repository URL
* `Settings` &rarr; `Build and deployment`
* `Source` &rarr; `Deploy from a branch`
* `Branch` &rarr; `gh-pages/(root)`
* `Save`

For more information on this process, see the GitHub Pages [documentation](https://pages.github.com/).

## Install Python `mkdocs` and `mkdocstrings`

Using a package manager such as Anaconda or Mamba, install `mkdocs`:

```bash
micromamba create -n mkdocs python=3.11 pip
micromamba activate mkdocs
pip install mkdocs mkdocs-material pymdown-extensions mkdocstrings[python]
mkdocs --version
```

## Edit the Configuration File

```bash
vim mkdocs.yaml
```

## Build and Deploy Documentation

Build and deploy the documentation to GitHub Pages:

```bash
mkdocs gh-deploy
```

The documentation should soon be viewable on GitHub Pages. For more information, see the `mkdocs` documentation [here](https://www.mkdocs.org/getting-started/) and [here](https://www.mkdocs.org/user-guide/deploying-your-docs/).
