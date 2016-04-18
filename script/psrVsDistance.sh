###########################################################
#             Wireless Access Networks@NTU HW2            #
###########################################################
# Author: Teng-Hui Huang                                  #
# Required file: myWifiPhyTest.cc                         #
###########################################################
# USAGE: user@term:~$ sh psrVsDistance [loss model]       
###########################################################
WAF_EXE=./waf
experiment=myTest

TxPowerLevel=0.0
NPackets=400
PacketSize=2304
WifiStand=80211a
WifiClass=Ofdm

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
ErrRateMd=Yans


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
      echo "please specify correct loss model"
  ;;
esac

$WAF_EXE --run "myWifiPhyTest $experiment \
      --TxPowerLevel=$TxPowerLevel --NPackets=$NPackets \
      --PacketSize=$PacketSize --WifiClass=$WifiClass --WifiStand=$WifiStand \
      --ErrRateMd=$ErrRateMd \
      --LossMd=$lossMd $Arg_Loss"
