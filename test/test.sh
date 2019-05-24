canDetectHandles=$(node -e "console.log(require('compare-versions')(process.version, '8'))")

if [ $canDetectHandles == "1" ]; then
    jest # --detectOpenHandles # --runInBand --verbose
else
    jest # --runInBand --verbose 
fi

