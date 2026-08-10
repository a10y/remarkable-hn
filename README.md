# HN Reader for reMarkable 2

A Qt Quick app for reading Hacker News posts on the reMarkable 2.

## Compatibility

The current build has been tested on:

- reMarkable 2
- reMarkable software 3.26.0.68 / OS 5.6.75
- Qt 6.8.2
- Xovi 0.3.3 and AppLoad 0.5.3

## Build With Docker

The official reMarkable SDK runs on `x86_64` Linux. The included container also works through Docker
x86_64 emulation on Apple Silicon.

1. Download the reMarkable 2 SDK matching OS 5.6.75 from the
   [reMarkable developer portal](https://developer.remarkable.com/links).
2. Initialize the build image and persistent SDK volume:

   ```sh
   scripts/setup-sdk-container.sh /path/to/remarkable-production-image-5.6.75-rm2-public-x86_64-toolchain.sh
   ```

3. Build HN Reader and the AppLoad-compatible Qt `linuxfb` plugin:

   ```sh
   make build-docker
   ```

Build outputs are written below `build/` and are ignored by Git.

## Build On Linux

With the SDK installed at `/opt/codex/rm2/5.6.75`:

```sh
make build
scripts/build-linuxfb-plugin.sh
```

Set `SDK_ROOT` or `REMARKABLE_SDK_VERSION` to use another compatible SDK location.

## Standalone Deployment

The standalone launcher temporarily stops the stock reMarkable UI while HN Reader owns the e-paper
display, then restarts it when HN Reader exits.

```sh
make deploy
ssh -t root@10.11.99.1 /home/root/hn-reader/run.sh
```

Set `REMARKABLE_HOST` to use a different SSH destination.

## AppLoad Deployment

For a tablet with a working Xovi/AppLoad installation:

```sh
make deploy-appload
```

This installs the application, icon, external-app manifest, and bundled `linuxfb` plugin, then
restarts `xochitl` so AppLoad discovers the registration. HN Reader uses AppLoad's QTFB shim and a
small frame-update bridge rather than acquiring the display's exclusive SWTCON lock.

Run the on-device smoke test at any time with:

```sh
make smoke-test
```

## Persistent Xovi Startup

On installations that provide `/home/root/xovi/start` and `/home/root/xovi/stock`, the optional
service files in `device/` can restore Xovi after every reboot:

```sh
make install-xovi-persistence
```

The service waits for `xochitl`, activates Xovi, and confirms that `xochitl` remains healthy. A
failed startup invokes `xovi-recovery.service`, which restores the stock interface.

To disable the integration over USB SSH:

```sh
systemctl disable --now xovi-persistent.service
/home/root/xovi/stock
```

Before a reMarkable OS update, check all installed Vellum packages against the new software version:

```sh
/home/root/.vellum/bin/vellum check-os <version>
```

System updates may remove custom systemd units. Reinstall or re-enable the integration only after
confirming that Xovi and AppLoad support the new version.

## Privacy

HN Reader makes requests to the Hacker News Firebase API and to the source URL of a selected story.
It has no accounts, analytics, cookies, or remote readability service, and stores no browsing data.

## License

Original project code is available under the MIT License. The optional `linuxfb` build uses source
from Qt 6.8.2 under Qt's own licensing terms. See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
