printf "启动总数量=%s\n" $1

totalcmd=""
ffmpegpath="C:/nonsys/tools/ffmpeg-20180505-0803233-win64-static/bin/ffmpeg"
function startonestream() {
	cmd=$ffmpegpath" -re -stream_loop -1 -i ./1080p.mp4 -c copy -f flv rtmp://192.168.0.85:1936/live/test"$1
	echo $cmd
	totalcmd=$totalcmd$cmd
	start $cmd
#	start /b "C:/nonsys/tools/ffmpeg-20180505-0803233-win64-static/bin/ffmpeg"
}

for((i=0;i<$1;i++))
do   
startonestream $i
done  

read -n 1