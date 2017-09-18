#! /bin/sh
wget https://repo.varnish-cache.org/source/varnish-4.1.7.tar.gz && tar xvf varnish-4.1.7.tar.gz && mkdir lib && mv varnish-4.1.7/lib/libvarnish lib/ && mv varnish-4.1.7/lib/libvarnishcompat lib/ && mv varnish-4.1.7/lib/libvarnishtools lib/ && mv varnish-4.1.7/include . && rm -rf varnish-4.1.7.tar.gz varnish-4.1.7
aclocal && automake --add-missing && autoconf
