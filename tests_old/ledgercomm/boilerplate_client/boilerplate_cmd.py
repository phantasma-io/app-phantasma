import struct
from typing import Tuple

from ledgercomm import Transport

from boilerplate_client.boilerplate_cmd_builder import BoilerplateCommandBuilder, InsType
from boilerplate_client.button import Button
from boilerplate_client.exception import DeviceException
from boilerplate_client.transaction import Transaction


class BoilerplateCommand:
    def __init__(self,
                 transport: Transport,
                 debug: bool = False) -> None:
        self.transport = transport
        self.builder = BoilerplateCommandBuilder(debug=debug)
        self.debug = debug

    def get_app_and_version(self) -> Tuple[str, str]:
        sw, response = self.transport.exchange_raw(
            self.builder.get_app_and_version()
        )  # type: int, bytes

        if sw != 0x9000:
            raise DeviceException(error_code=sw, ins=0x01)

        # response = format_id (1) ||
        #            app_name_len (1) ||
        #            app_name (var) ||
        #            version_len (1) ||
        #            version (var) ||
        offset: int = 0

        format_id: int = response[offset]
        offset += 1
        app_name_len: int = response[offset]
        offset += 1
        app_name: str = response[offset:offset + app_name_len].decode("ascii")
        offset += app_name_len
        version_len: int = response[offset]
        offset += 1
        version: str = response[offset:offset + version_len].decode("ascii")
        offset += version_len

        return app_name, version

    def get_version(self) -> Tuple[int, int, int]:
        sw, response = self.transport.exchange_raw(
            self.builder.get_version()
        )  # type: int, bytes

        if sw != 0x9000:
            raise DeviceException(error_code=sw, ins=InsType.INS_GET_VERSION)

        # response = MAJOR (1) || MINOR (1) || PATCH (1)
        assert len(response) == 3

        major, minor, patch = struct.unpack(
            "BBB",
            response
        )  # type: int, int, int

        return major, minor, patch

    def get_app_name(self) -> str:
        sw, response = self.transport.exchange_raw(
            self.builder.get_app_name()
        )  # type: int, bytes

        if sw != 0x9000:
            raise DeviceException(error_code=sw, ins=InsType.INS_GET_APP_NAME)

        return response.decode("ascii")

    def get_public_key(self, bip32_path: str, display: bool = False) -> Tuple[bytes, bytes]:
        print("STARTING get_public_key")
        print(bip32_path)

        sw, response = self.transport.exchange_raw(
            self.builder.get_public_key(bip32_path=bip32_path,
                                        display=display)
        )  # type: int, bytes

        if sw != 0x9000:
            raise DeviceException(error_code=sw, ins=InsType.INS_GET_PUBLIC_KEY)

        print(response.hex())
        print("FINISHED get_public_key")

        pub_key: bytes = response

        return pub_key

    def sign_tx(self, bip32_path: str, transaction: Transaction, button: Button) -> Tuple[int, bytes]:
        print("STARTING sign_tx")
        sw: int
        response: bytes = b""

        for is_last, chunk in self.builder.sign_tx(bip32_path=bip32_path, transaction=transaction):
            self.transport.send_raw(chunk)

            print(chunk.hex())

            if is_last:
                # Review Transaction
                button.right_click()
                # TX/Nexus/Chain/Script
                button.right_click()
                button.right_click()
                button.right_click()
                # Address/Token/Amount
                button.right_click()
                button.right_click()
                button.right_click()
                # Approve
                button.both_click()

            sw, response = self.transport.recv()  # type: int, bytes

            if sw != 0x9000:
                raise DeviceException(error_code=sw, ins=InsType.INS_SIGN_TX)

        # response = der_sig_len (1) ||
        #            der_sig (var) ||
        #            v (1)
        sig: bytes = response[:64]

        print("FINISHED sign_tx")

        return sig
