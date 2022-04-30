from io import BytesIO
from typing import Union

from boilerplate_client.utils import (read, read_uint, read_varint,
                                      write_varint, UINT64_MAX)


class TransactionError(Exception):
    pass


class Transaction:
    def __init__(self, nexus: str, chain: str, script: str) -> None:
        self.nexus: bytes = bytes.fromhex(nexus[2:]) if isinstance(nexus, str) else nexus
        self.chain: bytes = bytes.fromhex(chain[2:]) if isinstance(chain, str) else chain
        self.script: bytes = bytes.fromhex(script[2:]) if isinstance(script, str) else script

    def serialize(self) -> bytes:
        return b"".join([
            write_varint(len(self.nexus)),
            self.nexus,
            write_varint(len(self.chain)),
            self.chain,
            write_varint(len(self.script)),
            self.script,
        ])

    @classmethod
    def from_bytes(cls, hexa: Union[bytes, BytesIO]):
        buf: BytesIO = BytesIO(hexa) if isinstance(hexa, bytes) else hexa
        nexus_len: int = read_varint(buf)
        nexus: str = read(buf, nexus_len)
        chain_len: int = read_varint(buf)
        chain: str = read(buf, chain_len)
        script_len: int = read_varint(buf)
        script: str = read(buf, script_len)

        return cls(nexus=nexus,chain=chain,script=script)
