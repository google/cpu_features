# GitHub-CI Status
| Os                    | amd64                 | AArch64               | ARM                   | MIPS                  |
| :--                   | --:                   | --:                   | --:                   | --:                   |
| FreeBSD               | ![][img_na]           | ![][img_na]           | ![][img_na]           | ![][img_na]           |
| Linux                 | [![][img_2a]][lnk_2a] | ![][img_na]           | ![][img_na]           | ![][img_na]           |
| MacOS                 | ![][img_na]           | ![][img_na]           | ![][img_na]           | ![][img_na]           |
| Windows               | ![][img_na]           | ![][img_na]           | ![][img_na]           | ![][img_na]           |

[img_na]: https://img.shields.io/badge/build-N%2FA-lightgrey
[lnk_2a]: https://github.com/google/cpu_features/actions/workflows/amd64_linux_cmake.yml
[img_2a]: https://img.shields.io/github/workflow/status/google/cpu_features/amd64%20Linux%20Bazel/main

### Usage
To get the help simply type:
```sh
bazel test -s --verbose_failures //...
```
