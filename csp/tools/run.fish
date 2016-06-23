#! /bin/fish

set exe $argv[1]
echo "Executable: $exe" 1>&2

echo "Running: " 1>&2
for arg in $argv[2..-1]
    set cmd $exe \"$arg\"
    echo $cmd 1>&2
    eval $cmd
end

