# Contributing

Contributions are welcome through GitHub issues and pull requests.

## Development

1. Build with `make build-docker`.
2. Validate shell scripts with `bash -n scripts/*.sh` and `sh -n device/*.sh`.
3. Validate JSON manifests with `jq empty device/hn-reader/*.json`.
4. Run `make smoke-test` against a connected reMarkable 2 after deployment.

Keep the interface monochrome, avoid animation, and test changes on the e-paper display. Device
integration changes must retain a documented path back to stock `xochitl`.
