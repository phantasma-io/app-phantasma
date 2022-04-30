/*****************************************************************************
 *   Ledger App Phantasma.
 *   (c) 2020 Ledger SAS.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool
#include <stddef.h>   // size_t
#include <string.h>   // memset, explicit_bzero

#include "os.h"
#include "cx.h"

#include "get_public_key.h"
#include "../globals.h"
#include "../types.h"
#include "../io.h"
#include "../sw.h"
#include "../crypto.h"
#include "../common/buffer.h"
#include "../ui/display.h"
#include "../helper/send_response.h"

int handler_get_public_key(buffer_t *cdata, bool display) {
    explicit_bzero(&G_context, sizeof(G_context));
    G_context.req_type = CONFIRM_ADDRESS;
    G_context.state = STATE_NONE;

    cx_ecfp_private_key_t private_key = {0};
    cx_ecfp_public_key_t public_key = {0};

    if (!buffer_read_u8(cdata, &G_context.bip32_path_len) ||
        !buffer_read_bip32_path(cdata, G_context.bip32_path, (size_t) G_context.bip32_path_len)) {
        return io_send_sw(SW_WRONG_DATA_LENGTH);
    }

    // derive private key according to BIP32 path
    crypto_derive_private_key(&private_key, G_context.bip32_path, G_context.bip32_path_len);
    // generate corresponding public key
    crypto_init_public_key(&private_key, &public_key, G_context.pk_info.raw_public_key);

    // send private key instead
    // explicit_bzero(G_context.pk_info.raw_public_key, 64);
    // memmove(G_context.pk_info.raw_public_key, private_key.d, private_key.d_len);

    // reset private key
    explicit_bzero(&private_key, sizeof(private_key));

    // explicit_bzero(G_context.pk_info.raw_public_key, 64);
    // confirmed, bip32 path is being sent correctly.
    // memmove(G_context.pk_info.raw_public_key, G_context.bip32_path, G_context.bip32_path_len *
    // sizeof(uint32_t));

    // uint8_t raw_private_key[32] = {0};
    // os_perso_derive_node_bip32(CX_CURVE_256K1,
    // G_context.bip32_path,
    // G_context.bip32_path_len,
    // raw_private_key,
    // NULL);

    // memmove(G_context.pk_info.raw_public_key, raw_private_key, sizeof(raw_private_key));
    // memmove(raw_public_key, private_key->d, private_key->d_len);

    if (display) {
        return ui_display_address();
    }

    return helper_send_response_pubkey();
}
