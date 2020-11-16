# test with different directory sizes


# compile
cd ..
make && sudo make install clean
cd test

# prepare environment
echo [test] creating environment
mkdir 1000_files_test
cd 1000_files_test
touch file{0001..1000}.txt

# execute efm
efm

# clean environment
echo [test] cleaning
cd ..
rm -r 1000_files_test

echo [test] done