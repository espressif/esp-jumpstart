# ESP-Jumpstart Documentation

This folder contains the source files for **ESP-Jumpstart Documentation**.

## Hosted Documentation

The ESP-Jumpstart documentation is hosted at:
- **English**: https://docs.espressif.com/projects/esp-jumpstart/en/latest/
- **Chinese**: https://docs.espressif.com/projects/esp-jumpstart/zh_CN/latest/

## Documentation Framework

ESP-Jumpstart uses the **esp-docs** framework for building documentation, which provides:
- Modern Sphinx-based documentation generation
- Espressif-specific themes and extensions
- Multi-language support (English and Chinese)
- Integration with GitLab CI for automatic builds and deployment
- Copy-to-clipboard functionality for code blocks

## Building Documentation Locally

### Prerequisites

1. Install Python dependencies:
   ```bash
   pip install -r requirements.txt
   ```

2. Install esp-docs build tool:
   ```bash
   pip install esp-docs
   ```

### Build Commands

To build the documentation locally:

```bash
# Build English documentation
cd docs
build-docs -l en -t esp32

# Build Chinese documentation
build-docs -l zh_CN -t esp32

# Build both languages
build-docs -l en,zh_CN -t esp32
```

The generated HTML files will be available in:
- English: `docs/_build/en/esp32/html/`
- Chinese: `docs/_build/zh_CN/esp32/html/`

### Supported Targets

ESP-Jumpstart documentation supports the following ESP32 targets:
- ESP32
- ESP32-S2
- ESP32-S3
- ESP32-C2
- ESP32-C3
- ESP32-C6

## Documentation Structure

```
docs/
├── README.md              # This file
├── requirements.txt       # Python dependencies
├── conf_common.py         # Common Sphinx configuration
├── en/                    # English documentation
│   ├── conf.py           # English-specific configuration
│   └── rst/              # English RST source files
└── zh_CN/                # Chinese documentation
    ├── conf.py           # Chinese-specific configuration
    └── rst/              # Chinese RST source files
```

## CI/CD Pipeline

The GitLab CI pipeline automatically:

1. **Builds documentation** on every commit using the `build_docs` job
2. **Creates preview deployments** for feature branches when docs are modified
3. **Deploys to production** when changes are merged to master branch

### CI Jobs

- `build_docs`: Builds documentation for both languages
- `deploy_docs_preview`: Deploys preview documentation for non-master branches
- `deploy_docs_production`: Deploys production documentation for master branch

## Contributing to Documentation

1. Make changes to the RST files in `en/rst/` or `zh_CN/rst/`
2. Test locally using `build-docs` command
3. Commit and push changes
4. GitLab CI will automatically build and deploy documentation

## Migration from ReadTheDocs

ESP-Jumpstart has been migrated from ReadTheDocs to the esp-docs framework to:
- Use modern Espressif documentation standards
- Improve build performance and reliability
- Enable better integration with ESP-IDF ecosystem
- Provide consistent styling with other Espressif projects

For more information about esp-docs, visit: https://docs.espressif.com/projects/esp-docs/