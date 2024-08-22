tarNames=`ls 20*.tar`
for eachfile in $tarNames
do
    dirName="${eachfile%.*}"
    mkdir $dirName
    tar xvf ${eachfile} -C $dirName
    gcc ./$dirName/server.c -o ./$dirName/server
    gcc ./$dirName/client.c -o ./$dirName/client
    echo $eachfile
done