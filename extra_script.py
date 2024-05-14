import os
from os.path import join
from dotenv import load_dotenv

# Import env from SCons
Import("env")

# Use the current working directory
dotenv_path = join(env["PROJECT_DIR"], '.env')
load_dotenv(dotenv_path)

# Read environment variables
wifi_ssid = os.getenv('WIFI_SSID')
wifi_password = os.getenv('WIFI_PASSWORD')

# Add Build Flags
if wifi_ssid and wifi_password:
    print(f'Using WIFI_SSID={wifi_ssid} and WIFI_PASSWORD={wifi_password}')
    env.Append(
        BUILD_FLAGS=[
            f'-DWIFI_SSID=\\"{wifi_ssid}\\"',
            f'-DWIFI_PASSWORD=\\"{wifi_password}\\"'
        ]
    )
else:
    print('WIFI_SSID or WIFI_PASSWORD is not set in .env file')
