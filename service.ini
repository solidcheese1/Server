[Unit]
Description=TCP/UDP Echo Server with epoll
After=network.target

[Service]
Type=simple
User=nobody
Group=nogroup
ExecStart=/home/vboxuser/Server/my_program
Restart=on-failure
RestartSec=5s
LimitNOFILE=65536

[Install]
WantedBy=multi-user.target