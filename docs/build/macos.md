# Setup your Keyman build environment on macOS

This document describes prerequisites and tools required for building various
Keyman projects on macOS. Each project will also have additional notes linked
below.

## Target Projects

On macOS, you can build the following projects:

* Keyman for Android ([additional details](../../android/README.md))
* Keyman for iOS ([additional details](../../ios/README.md))
* Keyman for macOS ([additional details](../../mac/README.md))
* KeymanWeb ([additional details](../../web/README.md))
* Keyman Developer Command Line Tools (kmc)

The following libraries can also be built:

* core (macOS, wasm targets)
* Common/Web

The following projects **cannot** be built on macOS:

* Keyman for Linux
* Keyman for Windows
* Keyman Developer (IDE component)

## System Requirements

* Minimum macOS version: macOS Catalina 10.15 or Big Sur 11.0

**Note:** to make a fully M1-compatible release build of Keyman for macOS (for
the setup Applescript), Big Sur 11.0 is required, as osacompile on earlier
versions does not build for arm64 (M1). The build will still work on earlier
versions, but the installer won't be able to run on M1 Macs that do not have
Rosetta 2 installed.

## Prerequisites

Many dependencies are only required for specific projects.

* XCode (iOS, macOS) 12.4 or later is needed only for Keyman for macOS and Keyman
  for iOS.
  * Install from App Store
  * Accept the Xcode license: `sudo xcodebuild -license accept`

The remaining dependencies can be installed via script:
  `resources/devbox/macos/macos.sh`

This script will also update your environment to the values in:
  `resources/devbox/macos/keyman.macos.env.sh`

These dependencies are also listed below if you'd prefer to install manually.

### Example shell profile script

`keyman.macos.env.sh` may be `source`d within your `~/.bashrc` / `~/.zprofile` in
order to regularly apply its path settings to the terminal during local development.

For a `.zprofile`...

```zsh
# Adds homebrew-managed dependencies to $PATH
eval "$(/opt/homebrew/bin/brew shellenv)"

# Replace right-hand side with the path to your local copy of the repo
export KEYMAN_ROOT=~/keymanapp/keyman

# May be called to apply build-path settings, such as those allowing
# direct call of npm scripts without `npm run`.
set_keyman_standard_build_path() {
  PATH="${KEYMAN_ROOT}/node_modules/.bin:$PATH"
}

# builder-script tab autocompletion
autoload -Uz compinit && compinit
autoload bashcompinit
bashcompinit
source "${KEYMAN_ROOT}/resources/builder_completion.sh"

# environment variable initialization
source "${KEYMAN_ROOT}/resources/devbox/macos/keyman.macos.env.sh"
```

## Shared Dependencies

* HomeBrew, Bash 5.0+, jq, Python 2.7, Python 3, Meson, Ninja, coreutils, Pandoc

```shell
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
brew install bash jq python3 meson ninja coreutils pandoc pyenv
# Python 2.7 required for DeviceKit (among others?) at present
pyenv install 2.7.18
pyenv global 2.7.18
echo 'eval "$(pyenv init --path)"' >> ~/.bash_profile
```

On macOS, you will need to adjust your PATH so that coreutils’ `realpath` takes
precedence over the BSD one:

```shell
# Credit: brew info coreutils
PATH="$HOMEBREW_PREFIX/opt/coreutils/libexec/gnubin:$PATH"
```

## KeymanWeb Dependencies

* node.js, emscripten 3.1.46 or later

### node.js

Our recommended way to install node.js is with
[nvm](https://github.com/nvm-sh/nvm). This makes it easy to switch between
versions of node.js.

Alternatively, you can install node.js with `brew`.

See [node.md](node.md) for more information.

### emscripten

The recommended way to install emscripten is with the official method, as that
then allows the build scripts to select the appropriate version automatically.

To install emscripten, `cd` to an appropriate path, and then:

```bash
git clone https://github.com/emscripten-core/emsdk
cd emsdk
emsdk install 3.1.58
emsdk activate 3.1.58
cd upstream/emscripten
npm install
export EMSCRIPTEN_BASE="$(pwd)"
echo "export EMSCRIPTEN_BASE=\"$EMSCRIPTEN_BASE\"" >> .bashrc
```

If you are updating an existing install of Emscripten:

```bash
cd emsdk
git pull
emsdk install 3.1.58
emsdk activate 3.1.58
cd upstream/emscripten
npm install
```

You will want to add `EMSCRIPTEN_BASE` to your .bashrc.

> ![WARNING]
> Don't put EMSDK on the path, i.e. don't source `emsdk_env.sh`.
>
> Emscripten very unhelpfully overwrites `JAVA_HOME`, and adds its own
> versions of Python, Node and Java to the `PATH`. For best results, restart
> your shell after installing Emscripten so that you don't end up with the
> wrong versions.

**Optional environment variables**:

To let the Keyman build scripts control the version of Emscripten installed on
your computer:

```shell
export KEYMAN_USE_EMSDK=1
```

## Keyman for iOS Dependencies

* XCode, swiftlint, carthage

```shell
brew install swiftlint carthage
```

## Keyman for Mac Dependencies

* XCode, carthage, cocoapods

```shell
brew install carthage cocoapods
```

## Keyman for Android Dependencies

* openjdk 21, Android SDK, Android Studio, Ant, Gradle, Maven

```shell
brew install openjdk@21 android-sdk android-studio ant gradle maven
# update path
source ../resources/devbox/macos/keyman.macos.env.sh
# optionally install sdk images
sdkmanager "system-images;android-30;google_apis;armeabi-v7a"
sdkmanager --update
sdkmanager --licenses
```

* Note: Run Android Studio once after installation to install additional
components such as emulator images and SDK updates.

## Keyman Developer Command Line (kmc)

* node.js, emscripten

```shell
brew install node emscripten
```

## Optional Tools

* sentry-cli: Uploading symbols for Sentry-based error reporting

  ```shell
  brew install getsentry/tools/sentry-cli
  ```
