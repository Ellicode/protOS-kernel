while true; do
    printf "\033[2mEnter address to find line number (type \"q\" to quit): \033[0m"
    read -r addr
    if [ "$addr" = "q" ]; then
        break # This cleanly exits the loop
    else
        addr2line -e build/kernel.elf "$addr"
    fi
done