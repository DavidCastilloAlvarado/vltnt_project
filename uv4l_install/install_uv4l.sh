
# Agregar la siguiente linea al archivo /etc/apt/sources.list
# deb https://www.linux-projects.org/listing/uv4l_repo/raspbian/stretch stretch main
# despues correr este script

curl https://www.linux-projects.org/listing/uv4l_repo/lpkey.asc | sudo apt-key add -
sudo apt-get update
sudo apt-get install uv4l uv4l-webrtc uv4l-uvc uv4l-server
