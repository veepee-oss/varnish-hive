# varnish-hive ![License][license-img]

1. [Overview](#overview)
2. [Description](#description)
3. [Requirements](#requirements)
4. [Setup](#setup)
5. [Usage](#usage)
6. [Limitations](#limitations)
7. [Contributors](#contributors)
8. [Author Information](#author)
9. [Miscellaneous](#miscellaneous)

## Overview

Description of `varnish-hive`

[domain.tld](https://www.domain.tld/)

## Description

Replicate varnish cache between all the active varnishes present in the configuration file.

## Requirements

varnish 4.1.7 (no, varnish 4.1.1 won't work).

## Setup

### From source

```bash
./autogen.sh
./configure
make
sudo make install
```
## Create varnish-user

```bash
utils/create_user.sh
```

### Create .deb package

```bash
bash utils/deb_packager.sh
```

You will find the .deb package in varnish-hive parent folder 

## Usage

### Configuration

/etc/varnish/varnishhive.ini configuration file format:

See etc/varnish/varnishhive.ini

### Daemon options

```bash
sudo service varnish-hive start
```

### VCL modifications


```c
...
include "/etc/varnish/varnishhive.vcl";
...
sub vcl_recv {
...
	call varnishhive_vcl_recv;
...
}
```

## Limitations

Works only with varnish > 4.1.3

## Authors Information

jambon69 <lgiesen@vente-privee.com>

dave <david.sebaoun@epitech.eu>

sams <sfarhane@vente-privee.com>

sxbri <sabdellatif@vente-privee.com>

Alain ROMEYER <aromeyer@vente-privee.com>

(guidance) poolpOrg <gchehade@vente-privee.com>

## Development

Please read carefully [CONTRIBUTING]
(https://raw.githubusercontent.com/vente-privee/varnish-hive/master/CONTRIBUTING)
before making a merge request.

## Miscellaneous


```
    ╚⊙ ⊙╝
  ╚═(███)═╝
 ╚═(███)═╝
╚═(███)═╝
 ╚═(███)═╝
  ╚═(███)═╝
   ╚═(███)═╝
```

[license-img]: http://pgloader.io/img/bsd.svg
