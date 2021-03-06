###########################################################
#             Wireless Access Networks@NTU HW2            #
###########################################################
# Author: Teng-Hui Huang                                  #
# Required file: myWifiPhyTest.cc                         #
###########################################################
# USAGE: user@term:~$ sh psrVsDistance [loss model] [error rate model]      
###########################################################
WAF_EXE=./waf
experiment=myTest

TxPowerLevel=0.0
NPackets=400
PacketSize=1000
TxAntennaGain=6.0

WifiStand=80211a
WifiClass=Ofdm         #Dsss ErpOfdm Ofdm Ofdm_Bw10 Ofdm_Bw5 HtMcs VhtMcs

# distance setup for psr vs distance experiment
IterMin=10.0
IterMax=165.0
IterStep=10.0

# parameters for loss model
Frequency=5.150e9        # Friis, TwoRay 
SystemLoss=1.0           # Friis, TwoRay
MinLoss=0.0              # Friis
MinDistance=1.0          # TwoRay
HeightAboveZ=0.0         # TwoRay
PathLossExponent=3.0     # Log
ReferenceDistance=1.0    # Log
ReferenceLoss=46.6777    # Log
Rss=-150.0               # Rss
DefaultLoss=100000000    # Matrix



#container for loss model
lossMd=$1
Arg_Loss=
#container for error rate model (in development)
ErrRateMd=$2


case $1 in
  "Log")
      echo "Loss model = Log"
#      --ReferenceLoss=$ReferenceLoss --PathLossExponent=$PathLossExponent \
#      --ReferenceDistance=$ReferenceDistance "
      Arg_Loss="--ReferenceLoss=$ReferenceLoss --PathLossExponent=$PathLossExponent --ReferenceDistance=$ReferenceDistance"
  ;;
  "Random")
      echo "Loss model = Random"
  ;;
  "Friis")
      echo "Loss model = Friis"
      Arg_Loss="--Frequency=$Frequency --SystemLoss=$SystemLoss --MinLoss=$MinLoss"
  ;;
  "TwoRay")
      echo "Loss model = TwoRay"
      Arg_Loss="--Frequency=$Frequency --SystemLoss=$SystemLoss --MinDistance=$MinDistance --HeightAboveZ=$HeightAboveZ"
  ;;
  "ThreeLog")
      echo "Loss model = ThreeLog"
  ;;
  "Nakagami")
      echo "Loss model = Nakagami"
  ;;
  "FixedRss")
      echo "Loss model = FixedRss"
      Arg_Loss="--Rss=$Rss"
  ;;
  "Matrix")
      echo "Loss model = Maxtrix"
      Arg_Loss="--DefaultLoss=$DefaultLoss"
  ;;
  "Range")
      echo "Loss model = Range"
  ;;
  *)
      lossMd=Log
      echo "please specify correct loss model [Random, Log, TwoRay, ThreeLog, Matrix, FixedRss, Nakagami, Range, Friis]"
  ;;
esac

case $2 in
  "Yans")
        echo "Error rate model = Yans"
        ;;
  "Nist")
        echo "Error rate model = Nist"
        ;;
  *)
        ErrRateMd="Yans"
        echo "please specify correct error rate model [Yans, Nist]"
        ;;
esac

$WAF_EXE --run "myWifiPhyTest $experiment \
      --TxPowerLevel=$TxPowerLevel --NPackets=$NPackets --TxAntennaGain=$TxAntennaGain\
      --PacketSize=$PacketSize --WifiClass=$WifiClass --WifiStand=$WifiStand \
      --IteratorMinDistance=$IterMin  --IteratorMaxDistance=$IterMax --IteratorDistanceStep=$IterStep \
      --ErrRateMd=$ErrRateMd \
      --LossMd=$lossMd $Arg_Loss"
