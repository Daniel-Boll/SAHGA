#!/usr/bin/env bash

rm -rf build .cache .xmake

xmake project -k cmakelists

pushd ..

rm -rf sahga.zip
zip -r sahga.zip sahga-api-xmake

sshpass -e sftp -oBatchMode=no -b - daniel.bol@login.sdumont.lncc.br <<!
  put sahga.zip /scratch/ppar_unioeste/daniel.bol
  bye
!

echo "Waiting for the job to finish..."
sleep 2

## Run scripts into the ssh
sshpass -e ssh daniel.bol@login.sdumont.lncc.br 'bash -s' <<!
  cd /scratch/ppar_unioeste/daniel.bol
  rm -rf sahga-api-xmake

 # check if there is any sahga.zip
  if [ -f sahga.zip ]; then
    # unzip the sahga.zip
    unzip sahga.zip
    # remove the sahga.zip
    rm sahga.zip

    # cd sahga-api-xmake
    # mkdir build
    # cd build
    #
    # module load gcc/11.1
    # module load cmake/3.17.3
    #
    # cmake ..
    # make -j24
    # cd ..

    # OMP_NUM_THREADS=4 ./build/linux/x86_64/release/SAHGA
    echo "Deployed" > deploy.log
    exit 0
  else
    # write to deploy.log the error
    echo "There is no sahga.zip" > deploy.log
    exit 1
  fi
!

popd || exit
