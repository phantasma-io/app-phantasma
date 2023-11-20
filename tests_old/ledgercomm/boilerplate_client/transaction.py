from io import BytesIO
from typing import Union

from boilerplate_client.utils import (read, read_uint, read_varint,
                                      write_varint, UINT64_MAX)


class TransactionError(Exception):
    pass


class Transaction:
    def __init__(self, tx_hex: str) -> None:
        self.tx_hex: str = tx_hex

    def serialize(self) -> bytes:
        return bytes.fromhex(self.tx_hex)

    @classmethod
    def from_bytes(cls, hexa: Union[bytes, BytesIO]):
        buf: BytesIO = BytesIO(hexa) if isinstance(hexa, bytes) else hexa
        tx_hex: str = buf.hex();
        return cls(tx_hex=tx_hex)
