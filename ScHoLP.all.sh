#!/bin/bash

folder="../ScHoLP-Data"

declare -a data=(
  #  "coauth-DBLP" "coauth-MAG-Geology" "coauth-MAG-History"
  "coauth-DBLP-full" "coauth-MAG-Geology-full" "coauth-MAG-History-full"
  "contact-high-school" "contact-primary-school"
  #  "email-Enron" "email-Eu"
  "email-Enron-full" "email-Eu-full"
  #  "NDC-classes" "NDC-substances"
  "NDC-classes-full" "NDC-substances-full"
  "tags-ask-ubuntu" "tags-math-sx"
  "threads-ask-ubuntu" "threads-math-sx"
)

for d in "${data[@]}"; do
  path="${folder}/${d}"
  echo '================================================================================'
  set -x
  g++ -std=c++17 -O3 -Iinclude ./src/GraphInfo.cpp -o GraphInfo
  ./GraphInfo $path $d
  { set +x; } 2>/dev/null
  echo '================================================================================'
  set -x
  g++ -std=c++17 -O3 -Iinclude ./src/Baseline.cpp -o ./Baseline
  ./Baseline $path $d
  { set +x; } 2>/dev/null
  echo '++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++'
  set -x
  g++ -std=c++17 -O3 -Iinclude ./src/PushForward.cpp -o ./PushForward
  ./PushForward $path $d
  { set +x; } 2>/dev/null
  echo '--------------------------------------------------------------------------------'
  set -x
  g++ -std=c++17 -O3 -Iinclude -DSKIP_INTERSECT_VERTEX ./src/PushForward.cpp -o ./PushForward
  ./PushForward $path $d
  { set +x; } 2>/dev/null
  echo '++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++'
  set -x
  g++ -std=c++17 -O3 -Iinclude ./src/PrefixForest.cpp -o ./PrefixForest
  ./PrefixForest $path $d
  { set +x; } 2>/dev/null
  echo '--------------------------------------------------------------------------------'
  set -x
  g++ -std=c++17 -O3 -Iinclude -DPROGRESSIVE_COUNTING ./src/PrefixForest.cpp -o ./PrefixForest
  ./PrefixForest $path $d
  { set +x; } 2>/dev/null
  echo '--------------------------------------------------------------------------------'
  set -x
  g++ -std=c++17 -O3 -Iinclude -DPROGRESSIVE_COUNTING -DUSING_BITMAP ./src/PrefixForest.cpp -o ./PrefixForest
  ./PrefixForest $path $d
  { set +x; } 2>/dev/null
done

echo '********************************************************************************'

for d in "${data[@]}"; do
  path="${folder}/${d}"
  echo '================================================================================'
  set -x
  g++ -std=c++17 -O3 -Iinclude ./src/Baseline.cpp -o ./Baseline
  ./Baseline $path $d
  ./Baseline $path $d
  { set +x; } 2>/dev/null
  echo '++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++'
  set -x
  g++ -std=c++17 -O3 -Iinclude ./src/PushForward.cpp -o ./PushForward
  ./PushForward $path $d
  ./PushForward $path $d
  { set +x; } 2>/dev/null
  echo '--------------------------------------------------------------------------------'
  set -x
  g++ -std=c++17 -O3 -Iinclude -DSKIP_INTERSECT_VERTEX ./src/PushForward.cpp -o ./PushForward
  ./PushForward $path $d
  ./PushForward $path $d
  { set +x; } 2>/dev/null
  echo '++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++'
  set -x
  g++ -std=c++17 -O3 -Iinclude ./src/PrefixForest.cpp -o ./PrefixForest
  ./PrefixForest $path $d
  ./PrefixForest $path $d
  { set +x; } 2>/dev/null
  echo '--------------------------------------------------------------------------------'
  set -x
  g++ -std=c++17 -O3 -Iinclude -DPROGRESSIVE_COUNTING ./src/PrefixForest.cpp -o ./PrefixForest
  ./PrefixForest $path $d
  ./PrefixForest $path $d
  { set +x; } 2>/dev/null
  echo '--------------------------------------------------------------------------------'
  set -x
  g++ -std=c++17 -O3 -Iinclude -DPROGRESSIVE_COUNTING -DUSING_BITMAP ./src/PrefixForest.cpp -o ./PrefixForest
  ./PrefixForest $path $d
  ./PrefixForest $path $d
  { set +x; } 2>/dev/null
done

echo '********************************************************************************'

declare -a data2=(
  #  "congress-bills"
  "congress-bills-full"
  "DAWN"
  "tags-stack-overflow"
  "threads-stack-overflow"
)

for d in "${data2[@]}"; do
  path="${folder}/${d}"
  echo '================================================================================'
  set -x
  g++ -std=c++17 -O3 -Iinclude ./src/GraphInfo.cpp -o GraphInfo
  ./GraphInfo $path $d
  { set +x; } 2>/dev/null
  echo '================================================================================'
  set -x
  g++ -std=c++17 -O3 -Iinclude ./src/Baseline.cpp -o ./Baseline
  timeout -v 25h ./Baseline $path $d
  { set +x; } 2>/dev/null
  echo '++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++'
  set -x
  g++ -std=c++17 -O3 -Iinclude ./src/PushForward.cpp -o ./PushForward
  timeout -v 25h ./PushForward $path $d
  { set +x; } 2>/dev/null
  echo '--------------------------------------------------------------------------------'
  set -x
  g++ -std=c++17 -O3 -Iinclude -DSKIP_INTERSECT_VERTEX ./src/PushForward.cpp -o ./PushForward
  timeout -v 25h ./PushForward $path $d
  { set +x; } 2>/dev/null
  echo '++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++'
  set -x
  g++ -std=c++17 -O3 -Iinclude ./src/PrefixForest.cpp -o ./PrefixForest
  timeout -v 25h ./PrefixForest $path $d
  { set +x; } 2>/dev/null
  echo '--------------------------------------------------------------------------------'
  set -x
  g++ -std=c++17 -O3 -Iinclude -DPROGRESSIVE_COUNTING ./src/PrefixForest.cpp -o ./PrefixForest
  timeout -v 25h ./PrefixForest $path $d
  { set +x; } 2>/dev/null
  echo '--------------------------------------------------------------------------------'
  set -x
  g++ -std=c++17 -O3 -Iinclude -DPROGRESSIVE_COUNTING -DUSING_BITMAP ./src/PrefixForest.cpp -o ./PrefixForest
  timeout -v 25h ./PrefixForest $path $d
  { set +x; } 2>/dev/null
done

echo '********************************************************************************'

for d in "${data2[@]}"; do
  path="${folder}/${d}"
  echo '================================================================================'
  set -x
  g++ -std=c++17 -O3 -Iinclude ./src/Baseline.cpp -o ./Baseline
  timeout -v 25h ./Baseline $path $d
  timeout -v 25h ./Baseline $path $d
  { set +x; } 2>/dev/null
  echo '++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++'
  set -x
  g++ -std=c++17 -O3 -Iinclude ./src/PushForward.cpp -o ./PushForward
  timeout -v 25h ./PushForward $path $d
  timeout -v 25h ./PushForward $path $d
  { set +x; } 2>/dev/null
  echo '--------------------------------------------------------------------------------'
  set -x
  g++ -std=c++17 -O3 -Iinclude -DSKIP_INTERSECT_VERTEX ./src/PushForward.cpp -o ./PushForward
  timeout -v 25h ./PushForward $path $d
  timeout -v 25h ./PushForward $path $d
  { set +x; } 2>/dev/null
  echo '++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++'
  set -x
  g++ -std=c++17 -O3 -Iinclude ./src/PrefixForest.cpp -o ./PrefixForest
  timeout -v 25h ./PrefixForest $path $d
  timeout -v 25h ./PrefixForest $path $d
  { set +x; } 2>/dev/null
  echo '--------------------------------------------------------------------------------'
  set -x
  g++ -std=c++17 -O3 -Iinclude -DPROGRESSIVE_COUNTING ./src/PrefixForest.cpp -o ./PrefixForest
  timeout -v 25h ./PrefixForest $path $d
  timeout -v 25h ./PrefixForest $path $d
  { set +x; } 2>/dev/null
  echo '--------------------------------------------------------------------------------'
  set -x
  g++ -std=c++17 -O3 -Iinclude -DPROGRESSIVE_COUNTING -DUSING_BITMAP ./src/PrefixForest.cpp -o ./PrefixForest
  timeout -v 25h ./PrefixForest $path $d
  timeout -v 25h ./PrefixForest $path $d
  { set +x; } 2>/dev/null
done
