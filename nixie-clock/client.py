import asyncio
from datetime import datetime

import bleak

CHAR_UID = '19B10001-E8F2-537E-4F6C-D104768A1215'


async def main():
    arduino = await bleak.BleakScanner.find_device_by_name('Nixie-Clock')
    if not arduino:
        print('Could not connect to Arduino')
        return

    async with bleak.BleakClient(arduino) as arduino_client:
        now = datetime.now()
        hour_bytes = now.hour.to_bytes(1, 'little')
        minute_bytes = now.minute.to_bytes(1, 'little')
        await arduino_client.write_gatt_char(CHAR_UID,
                                             hour_bytes + minute_bytes,
                                             response=True)


if __name__ == '__main__':
    asyncio.run(main())
