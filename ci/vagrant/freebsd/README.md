# Vagrant

All command must be run in this directory (ci/vagrant/freebsd)

## Lexical

Docker: image
Vagrant: box

## Build box (image)

To build a Vagrant Box (aka Docker image)

```sh
vagrant destroy -f
vagrant box update
vagrant up
```

## SSH to a freebsd_<lang> vagrant machine (debug).

```sh
vagrant up
vagrant ssh
```

## Clean FreeBSD vagrant machine

```sh
vagrant destroy -f
rm -rf .vagrant
```
