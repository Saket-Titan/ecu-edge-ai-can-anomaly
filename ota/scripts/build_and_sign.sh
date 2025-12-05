west build --sysbuild -p always -b nucleo_h723zg ~/firmware/ids_node

imgtool sign \
    --key ota/keys/root-rsa-2048.pem \
    --header-size 0x200 \
    --align 4 \
    --version 2.0.0 \
    --slot-size 0x60000 \
    --pad \
    --confirm \
    build/ids_node/zephyr/zephyr.bin \
    update.bin

objcopy --change-addresses 0x08080000 -I binary -O ihex update.bin update.hex
west flash --hex-file update.hex --skip-rebuild
