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
build="true"
push="true"
VERSION="$(./version.sh)"
BUILDDATE=$(date +'%Y%m%d')
KV_BUILD_DATE=${KV_BUILD_DATE:-}

if [ -n "${KV_BUILD_DATE}" ]; then
  BUILDDATE=$KV_BUILD_DATE
fi

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
                and set latest. If the enviroment variable KV_BUILD_DATE is set use
                this as the build date. Format KV_BUILD_DATE YYYYMMDD.
  --staging     build and push to staging.
  --prod        build and push to prod.
  --test        only build. Default.
  --stage stage stop at build stage. Default $targets.
  --no-cache    Do not use the docker build cache.
  --only-build  Stop after building.
  --only-push   Push previous build to registry. Must use the same flag as when building.
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
    --only-build) push="false" ;;
    --only-push) build="false" ;;
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
echo "build: $build"
echo "push: $push"


if [ "$mode" = "test" ]; then 
  kvuserid=$(id -u)
  registry=""
  kvcpp_registry=""
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
  if [ "$build" = "true" ]; then
    docker build $nocache --target $target --build-arg "REGISTRY=${kvcpp_registry}" --build-arg="BASE_IMAGE_TAG=${kvcpp_tag}" \
      --build-arg "kvuser=$kvuser" --build-arg "kvuserid=$kvuserid" \
      -f docker/${os}/${target}.dockerfile --tag ${registry}${target}:$tag .

    if [ "$tag_and_latest" = "true" ] && [ "$tag" != "latest" ]; then
      docker tag "${registry}${target}:$tag" "${registry}${target}:latest"
    fi
  fi


  if [ $mode != test ] && [ "$push" = "true" ]; then 
    docker push ${registry}${target}:$tag
    if [ "$tag_and_latest" = "true" ] && [ "$tag" != "latest" ]; then
      docker push "${registry}${target}:latest"
    fi
  fi
done


