#! /bin/bash

export DOCKER_BUILDKIT=1

kvcpp_registry="registry.met.no/met/obsklim/bakkeobservasjoner/data-og-kvalitet/kvalobs/kvbuild"
kvcpp_tag=latest
kvuser=kvalobs
kvuserid=5010
mode="test"
targets=kvbufrd
tag=latest
tag_and_latest="false"
os=focal
registry="registry.met.no/met/obsklim/bakkeobservasjoner/data-og-kvalitet/kvalobs/kvbuild"
nocache=
only_build=
VERSION="$(./version.sh)"
BUILDDATE=$(date +'%Y%m%d')

gitref=$(git rev-parse --show-toplevel)/gitref.sh

use() {

  usage="\
Usage: $0 [--help] [--staging|--prod|--test] [--tag tag] [--no-cache] [--only-build] 
          [--tag-and-latest tag] [--tag-version] [--tag-with-build-date] target-list

This script build a kvbufrd container. 
Stop at build stage 'stage'. Default $target.

If --staging or --prod is given it is copied to the 
container registry at $registry. 
If --test, the default, is used it will not be copied 
to the registry.


Options:
  --help        display this help and exit.
  --tag tagname tag the image with the name tagname, default $tag.
  --tag-and-latest tagname tag the image with the name tagname  and also create latest tag.
  --tag-version Use version from configure.ac as tag. Also tag latest.
  --tag-with-build-date 
                tag with version and build date on the form version-YYYYMMDD 
                and set latest.
  --staging     build and push to staging.
  --prod        build and push to prod.
  --test        only build. Default.
  --stage stage stop at build stage. Default $targets.
  --no-cache    Do not use the docker build cache.
  --only-build  Stop after building.
  --kvcpp-local Use local docker registry for kvcpp. Default: $kvcpp_registry
  --kvcpp-tag tagname Use tagname. Default: $kvcpp_tag


"
echo -e "$usage\n\n"

}


while test $# -ne 0; do
  case $1 in
    --tag) tag=$2; shift;;
    --tag-and-latest) 
        tag="$2"
        tag_and_latest=true
        shift;;
    --tag-version) 
        tag="$VERSION"
        tag_and_latest=true;;
    --tag-with-build-date)
        tag="$VERSION-$BUILDDATE"
        tag_and_latest=true;;
    --help) 
        use
        exit 0;;
    --kvcpp-local) kvcpp_registry="";;
    --kvcpp-tag) 
        kvcpp_tag=$2; shift;;
    --staging) mode=staging;;
    --prod) mode=prod;;
    --test) mode=test;;
    --no-cache) nocache="--no-cache";;
    --only-build)
        only_build="--target build";;
    -*) use
      echo "Invalid option $1"
      exit 1;;  
    *) targets="$targets $1";;
  esac
  shift
done

echo "VERSION: $VERSION"
echo "mode: $mode"
echo "os: $os"
echo "Build mode: $mode"
echo "targets: $targets"


if [ "$mode" = "test" ]; then 
  kvuserid=$(id -u)
  registry=""
  kvcpp_registry=""
  # if [ -n "$kvcpp_registry" ]; then
  #   kvcpp_registry="$kvcpp_registry/staging/"
  # fi
else 
  if [ -n "$kvcpp_registry" ]; then
    kvcpp_registry="$kvcpp_registry/$mode/"
  fi
  registry="$registry/$mode/"
fi

echo "registry: $registry"
echo "tag: $tag"
echo "kvcpp registry: $kvcpp_registry"
echo "kvcpp tag: $kvcpp_tag"


$gitref 

for target in $targets ; do
  docker build $nocache --target $target --build-arg "REGISTRY=${kvcpp_registry}" --build-arg="BASE_IMAGE_TAG=${kvcpp_tag}" \
    --build-arg "kvuser=$kvuser" --build-arg "kvuserid=$kvuserid" \
    -f docker/${os}/${target}.dockerfile ${only_build} --tag ${registry}${target}:$tag .

  if [ "$tag_and_latest" = "true" ]; then
      docker tag "${registry}${target}:$tag" "${registry}${target}:latest"
  fi


  if [ $mode != test ]; then 
    docker push ${registry}${target}:$tag
    if [ "$tag_and_latest" = "true" ]; then
      docker push "${registry}${target}:latest"
    fi
  fi
done


