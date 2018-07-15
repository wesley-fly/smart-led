echo ""
echo "local make start..."
echo ""

make COMPILE=gcc BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=2
