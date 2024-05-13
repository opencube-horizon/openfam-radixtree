
# Contrib

- the docker/ directory contains a Dockerfile to build a build container for both meadowlark itself and the RPM package
- this directory contains an RPM .spec file, to build a package directly out of a checked out source tree, run this in the main dir:

    rpmbuild --build-in-place -bb contrib/openfam-radixtree.spec
