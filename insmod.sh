sudo insmod ./nvmev.ko memmap_start=8G memmap_size=4G cpus=2,3
sudo mkfs -t ext4 /dev/nvme0n1
sudo mount /dev/nvme0n1 /home/candyuser/nvmetest
