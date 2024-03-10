import argparse
import asyncio
import contextlib
from datetime import datetime
from typing import AsyncGenerator

import bleak

CHAR_UID = '19B10001-E8F2-537E-4F6C-D104768A1215'


@contextlib.asynccontextmanager
async def _connect() -> AsyncGenerator[bleak.BleakClient, None]:
    arduino = await bleak.BleakScanner.find_device_by_name('Nixie-Clock')
    if not arduino:
        print('Could not connect to Arduino')
        return

    async with bleak.BleakClient(arduino) as arduino_client:
        yield arduino_client


async def _set_time(arduino_client: bleak.BleakClient, hour: int, minute: int):
    hour_bytes = hour.to_bytes(1, 'little')
    minute_bytes = minute.to_bytes(1, 'little')
    await arduino_client.write_gatt_char(CHAR_UID,
                                         hour_bytes + minute_bytes,
                                         response=True)


async def set_time():
    async with _connect() as arduino_client:
        now = datetime.now()
        await _set_time(arduino_client, now.hour, now.minute)


async def test_nixies(dwell_time: float):
    async with _connect() as arduino_client:
        for i in range(10):
            hour = 10 * i + i
            minute = 10 * i + i
            await _set_time(arduino_client, hour, minute)
            await asyncio.sleep(dwell_time)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--dwell-time',
                        type=float,
                        default=10,
                        help='How long to spend on each digit')
    group = parser.add_mutually_exclusive_group(required=True)

    group.add_argument('--set-time', action='store_true', help="Set the time")
    group.add_argument('--test-nixies',
                       action='store_true',
                       help="Test each digit of each tube")

    args = parser.parse_args()

    if args.set_time:
        asyncio.run(set_time())
    elif args.test_nixies:
        asyncio.run(test_nixies(args.dwell_time))
    else:
        assert False, "Impossible!"
