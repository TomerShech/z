#!/usr/bin/env bash

set -e
set -u
set -o pipefail

function githubLatestTag {
    finalUrl=$(curl "https://github.com/$1/releases/latest" -s -L -I -o /dev/null -w '%{url_effective}')
    echo "${finalUrl##*v}"
}

UNKNOWN_OS_MSG=<<-'EOM'
/=====================================\
|      COULD NOT DETECT PLATFORM      |
\=====================================/

Uh oh! I couldn't automatically detect your operating system.

To continue with installation, please choose from one of the following values:

- freebsd32
- freebsd64
- linux-arm
- linux32
- linux64
- netbsd32
- netbsd64
- openbsd32
- openbsd64
- osx
- win32
- win64
EOM


platform=''
machine=$(uname -m)

if [[ "$OSTYPE" == "linux"* ]]; then
  if [[ "$machine" == "arm"* || "$machine" == "aarch"* ]]; then
    platform='linux-arm'
  elif [[ "$machine" == *"86" ]]; then
    platform='linux32'
  elif [[ "$machine" == *"64" ]]; then
    platform='linux64'
  fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
  platform='osx'
elif [[ "$OSTYPE" == "freebsd"* ]]; then
  if [[ "$machine" == *"64" ]]; then
    platform='freebsd64'
  elif [[ "$machine" == *"86" ]]; then
    platform='freebsd32'
  fi
elif [[ "$OSTYPE" == "openbsd"* ]]; then
  if [[ "$machine" == *"64" ]]; then
    platform='openbsd64'
  elif [[ "$machine" == *"86" ]]; then
    platform='openbsd32'
  fi
elif [[ "$OSTYPE" == "netbsd"* ]]; then
  if [[ "$machine" == *"64" ]]; then
    platform='netbsd64'
  elif [[ "$machine" == *"86" ]]; then
    platform='netbsd32'
  fi
fi

if test "x$platform" = "x"; then
  cat <<EOM
/=====================================\\
|      COULD NOT DETECT PLATFORM      |
\\=====================================/

Uh oh! I couldn't automatically detect your operating system.

To continue with installation, please choose from one of the following values:

- freebsd32
- freebsd64
- linux-arm
- linux32
- linux64
- netbsd32
- netbsd64
- openbsd32
- openbsd64
- osx
- win32
- win64
EOM
  read -rp "> " platform
else
  echo "Detected platform: $platform"
fi

TAG=$(githubLatestTag TomerShech/z)

echo "Downloading https://github.com/TomerShech/z/releases/download/v$TAG/z-$TAG-$platform.tar.gz"
curl -L "https://github.com/TomerShech/z/releases/download/v$TAG/z-$TAG-$platform.tar.gz" > z.tar.gz

tar -xvzf z.tar.gz "z-$TAG/z"
mv "z-$TAG/z" ./z

rm z.tar.gz
rm -rf "z-$TAG"

cat <<-'EOM'

 _____  ___           _        _ _          _ _
|__  / |_ _|_ __  ___| |_ __ _| | | ___  __| | |
  / /   | || '_ \/ __| __/ _` | | |/ _ \/ _` | |
 / /_   | || | | \__ \ || (_| | | |  __/ (_| |_|
/____| |___|_| |_|___/\__\__,_|_|_|\___|\__,_(_)

z has been downloaded to the current directory. You can run it with:

./z [-s str] [files]


EOM
