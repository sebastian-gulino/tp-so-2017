#!/bin/bash

cd /home/utnso/workspace
git clone https://github.com/sisoputnfrba/so-commons-library.git
cd so-commons-library
sudo make install

cd /home/utnso/workspace
git clone https://github.com/sisoputnfrba/ansisop-parser.git
cd ansisop-parser
cd parser
sudo make install

cd /home/utnso/workspace/tp-2017-1c-Codeando-por-un-sueldo/cpus-commons/Debug
make clean
make all

cd /home/utnso/workspace
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/workspace/tp-2017-1c-Codeando-por-un-sueldo/cpus-commons/Debug

cd /home/utnso/workspace/tp-2017-1c-Codeando-por-un-sueldo/kernel/Debug
make clean
make all

cd /home/utnso/workspace/tp-2017-1c-Codeando-por-un-sueldo/cpu/Debug
make clean
make all

cd /home/utnso/workspace/tp-2017-1c-Codeando-por-un-sueldo/memoria/Debug
make clean
make all

cd /home/utnso/workspace/tp-2017-1c-Codeando-por-un-sueldo/filesystem/Debug
make clean
make all

cd /home/utnso/workspace/tp-2017-1c-Codeando-por-un-sueldo/consola/Debug
make clean
make all

cd /home/utnso/workspace/ansisop-parser/programas-ejemplo/evaluacion-final-esther/FS-ejemplo
tar -xzvf SADICA_FS_V2.tar.gz
mv SADICA_FS /home/utnso/FS_SADICA
