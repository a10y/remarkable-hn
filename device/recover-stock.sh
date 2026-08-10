#!/bin/sh
set -u

/home/root/xovi/stock || true
systemctl reset-failed xochitl.service
systemctl start xochitl.service
