param(
    [string]$ModelsDir = (Join-Path $PSScriptRoot "..\models")
)

$ErrorActionPreference = "Stop"

New-Item -ItemType Directory -Force -Path $ModelsDir | Out-Null

$haarUrl = "https://raw.githubusercontent.com/opencv/opencv/4.x/data/haarcascades/haarcascade_frontalface_default.xml"
$dlibUrl = "https://github.com/davisking/dlib-models/raw/master/shape_predictor_68_face_landmarks.dat.bz2"

$haarPath = Join-Path $ModelsDir "haarcascade_frontalface_default.xml"
$dlibArchivePath = Join-Path $ModelsDir "shape_predictor_68_face_landmarks.dat.bz2"
$dlibModelPath = Join-Path $ModelsDir "shape_predictor_68_face_landmarks.dat"

if(-not (Test-Path $haarPath)) {
    Write-Host "Downloading OpenCV Haar model..."
    Invoke-WebRequest -Uri $haarUrl -OutFile $haarPath
} else {
    Write-Host "OpenCV Haar model already exists."
}

if(-not (Test-Path $dlibModelPath)) {
    if(-not (Test-Path $dlibArchivePath)) {
        Write-Host "Downloading dlib 68-point model archive..."
        Invoke-WebRequest -Uri $dlibUrl -OutFile $dlibArchivePath
    }

    $python = Get-Command python -ErrorAction SilentlyContinue
    if($python) {
        Write-Host "Extracting dlib model with Python..."
        & $python.Source -c "import bz2, pathlib; src=pathlib.Path(r'$dlibArchivePath'); dst=pathlib.Path(r'$dlibModelPath'); dst.write_bytes(bz2.decompress(src.read_bytes()))"
        Remove-Item -Path $dlibArchivePath -Force
    } else {
        Write-Host "Python was not found. Please extract $dlibArchivePath manually to $dlibModelPath."
    }
} else {
    Write-Host "dlib 68-point model already exists."
}

Write-Host "Model directory: $ModelsDir"
