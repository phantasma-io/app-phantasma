from hashlib import sha256

import ed25519
from ecdsa.util import sigdecode_der

from boilerplate_client.transaction import Transaction

def test_sign_tx(cmd, button):
    bip32_path: str = "m/44'/627'/0'/0/0"

    pub_key = cmd.get_public_key(
        bip32_path=bip32_path,
        display=False
    )  # type: bytes, bytes

    # pk = ed25519.VerifyingKey(pub_key);

    tx_hex = ("076d61696e6e6574046d61696efd73010d000402313003000d00040231300300"
            "0d00042353313131313131313131313131313131313131313131313131313131"
            "3131313131313103000d00042f50324b376e3170566679335a4d536578747258"
            "316e6a337859645251766a446e744547416631783262376836526b3403000d00"
            "0408416c6c6f7747617303000d0004036761732d00012e010d00040231300300"
            "0d0004074f50434f44455303000d00042f50324b376e3170566679335a4d5365"
            "78747258316e6a337859645251766a446e744547416631783262376836526b34"
            "03000d00042f50324b376e3170566679335a4d536578747258316e6a33785964"
            "5251766a446e744547416631783262376836526b3403000d00041652756e7469"
            "6d652e5472616e73666572546f6b656e7307000d00042f50324b376e31705666"
            "79335a4d536578747258316e6a337859645251766a446e744547416631783262"
            "376836526b3403000d0004085370656e6447617303000d0004036761732d0001"
            "2e010b000000000c7068616E7461736D612D747300");


    tx = Transaction(
        tx_hex=tx_hex,
    )

    # sig = cmd.sign_tx(bip32_path=bip32_path,
                        # transaction=tx,
                        # button=button)

    # https://github.com/warner/python-ed25519

    # assert pk.verify(signature=der_sig,
                     # data=tx.serialize(),
                     # hashfunc=sha256,
                     # sigdecode=sigdecode_der) is True
