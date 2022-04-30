from hashlib import sha256

# from ecdsa.keys import VerifyingKey
from ecdsa.util import sigdecode_der

from boilerplate_client.transaction import Transaction

def test_sign_tx(cmd, button):
    bip32_path: str = "m/44'/627'/0'/0/0"

    pub_key = cmd.get_public_key(
        bip32_path=bip32_path,
        display=False
    )  # type: bytes, bytes

    # pk: VerifyingKey = VerifyingKey.from_string(
    #     pub_key,
    #     curve=Ed25519,
    #     hashfunc=sha256
    # )

    tx = Transaction(
        nexus="0x01",
        chain="0x0203",
        script="0x040506",
    )

    sig = cmd.sign_tx(bip32_path=bip32_path,
                        transaction=tx,
                        button=button)

    # assert pk.verify(signature=der_sig,
                     # data=tx.serialize(),
                     # hashfunc=sha256,
                     # sigdecode=sigdecode_der) is True
