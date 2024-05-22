#!/bin/bash
echo "Deleting old local server files"
rm -rf ./__pycache__/
rm -rf ./agent/
rm -rf ./command/
rm -rf ./html/
rm -rf ./install/
rm -rf ./interactive/
rm -rf ./payload/
rm -f ./tsunami.py
rm -f ./tsunami_warning.py

echo "Copying remote server files"
if [ "--new" == "$1" ]
then
	echo "Creating local agent files from scratch"
	mkdir ./agent/
else
	cp -r /mnt/hgfs/git/TsunamiWave/TsunamiServer/agent/ .
fi
cp -r /mnt/hgfs/git/TsunamiWave/TsunamiServer/command/ .
cp -r /mnt/hgfs/git/TsunamiWave/TsunamiServer/html/ .
cp -r /mnt/hgfs/git/TsunamiWave/TsunamiServer/install/ .
cp -r /mnt/hgfs/git/TsunamiWave/TsunamiServer/interactive/ .
cp -r /mnt/hgfs/git/TsunamiWave/TsunamiServer/payload/ .
cp -r /mnt/hgfs/git/TsunamiWave/TsunamiServer/tsunami.py .
cp -r /mnt/hgfs/git/TsunamiWave/TsunamiServer/tsunami_warning.py .

echo "Fixing permissions for webserver"
#set the group for all folders/files so the webserver (www-data) can access
chown --recursive developer:www-data -R ./*
#set default file permissions
chmod --recursive 664 ./*
chmod --recursive 775 ./*.sh
#set the permissions for the agent folder so new folders can be created by webserver
#chmod --recursive 775 ./agent/
find . -type d -exec chmod 775 {} \;
#set group sticky bit for agent folder
#chmod --recursive g+s ./agent/
find ./agent -type d -exec chmod g+s {} \;
#set group to rwx default
setfacl --default --modify g::rwx ./agent

