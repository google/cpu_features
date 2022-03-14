# GitHub-CI Status
| Os                    | amd64                 | AArch64               | ARM                   | MIPS                  |
| :--                   | --:                   | --:                   | --:                   | --:                   |
| FreeBSD               | [![][img_1a]][lnk_1a] | ![][img_na]           | ![][img_na]           | ![][img_na]           |
| Linux                 | [![][img_2a]][lnk_2a] | [![][img_2b]][lnk_2b] | [![][img_2c]][lnk_2c] | [![][img_2d]][lnk_2d] |
| MacOS                 | [![][img_3a]][lnk_3a] | ![][img_na]           | ![][img_na]           | ![][img_na]           |
| Windows               | [![][img_4a]][lnk_4a] | ![][img_na]           | ![][img_na]           | ![][img_na]           |

[img_na]: https://img.shields.io/badge/build-N%2FA-lightgrey
[lnk_1a]: https://github.com/google/cpu_features/actions/workflows/amd64_FreeBSD.yml
[img_1a]: https://img.shields.io/github/workflow/status/google/cpu_features/amd64%20FreeBSD/main
[lnk_2a]: https://github.com/google/cpu_features/actions/workflows/amd64_Linux.yml
[img_2a]: https://img.shields.io/github/workflow/status/google/cpu_features/amd64%20Linux/main
[lnk_3a]: https://github.com/google/cpu_features/actions/workflows/amd64_macOS.yml
[img_3a]: https://img.shields.io/github/workflow/status/google/cpu_features/amd64%20macOS/main
[lnk_4a]: https://github.com/google/cpu_features/actions/workflows/amd64_Windows.yml
[img_4a]: https://img.shields.io/github/workflow/status/google/cpu_features/amd64%20Windows/main
[lnk_2b]: https://github.com/google/cpu_features/actions/workflows/aarch64_Linux.yml
[img_2b]: https://img.shields.io/github/workflow/status/google/cpu_features/aarch64%20Linux/main
[lnk_2c]: https://github.com/google/cpu_features/actions/workflows/arm_Linux.yml
[img_2c]: https://img.shields.io/github/workflow/status/google/cpu_features/arm%20Linux/main
[lnk_2d]: https://github.com/google/cpu_features/actions/workflows/mips_Linux.yml
[img_2d]: https://img.shields.io/github/workflow/status/google/cpu_features/mips%20Linux/main

## Makefile/Docker testing
To test the build on various distro, we are using docker containers and a Makefile for orchestration.

pros:
* You are independent of third party CI runner config
  (e.g. [github action virtual-environnments](https://github.com/actions/virtual-environments)).
* You can run it locally on your linux system.
* Most CI provide runners with docker and Makefile installed.

cons:
* Only GNU/Linux distro supported.

### Usage
To get the help simply type:
```sh
make
```

note: you can also use from top directory
```sh
make --directory=ci
```

### Example
For example to test mips32 inside an container:
```sh
make mips32_test
```

### Docker layers
Dockerfile is splitted in several stages.

![docker](doc/docker.svg)


## Makefile/Vagrant testing
To test build for FreeBSD we are using Vagrant and VirtualBox box.

This is similar to the docker stuff but use `vagrant` as `docker` cli and
VirtuaBox to replace the docker engine daemon.
