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

    // Read BIP32 path from incoming data and handle errors
    if (!buffer_read_u8(cdata, &G_context.bip32_path_len) ||
        !buffer_read_bip32_path(cdata, G_context.bip32_path, (size_t) G_context.bip32_path_len)) {
        return io_send_sw(SW_WRONG_DATA_LENGTH);
    }

    // Derive private key according to BIP32 path
    if (crypto_derive_private_key(&private_key, G_context.bip32_path, G_context.bip32_path_len) != 0) {
        explicit_bzero(&private_key, sizeof(private_key));
        return io_send_sw(SW_DENY); // or appropriate error code
    }

    // Generate corresponding public key
    if (crypto_init_public_key(&private_key, &public_key, G_context.pk_info.raw_public_key) != 0) {
        explicit_bzero(&private_key, sizeof(private_key));
        return io_send_sw(SW_DISPLAY_ADDRESS_FAIL); // or appropriate error code
    }

    // Reset private key after use for security
    explicit_bzero(&private_key, sizeof(private_key));

    // Display logic if required
    if (display) {
        // Implement display logic or call to UI function here
        return ui_display_address();
    }

    // Send response with public key
    return helper_send_response_pubkey();
}


#define HARDENED_KEY_OFFSET 0x80000000

int handler_get_public_key_menu() {
    // Define the BIP32 path as an array of 32-bit integers
    const uint32_t bip32_path[] = {
        0x8000002C, // 44' with 0x80000000 added for hardening
        0x8000003C, // 60' with 0x80000000 added for hardening
        0x80000000, // 0'  with 0x80000000 added for hardening
        0x00000000, // 0
        0x00000000  // 0
    };
    size_t bip32_path_len = sizeof(bip32_path) / sizeof(bip32_path[0]);

    // Initialize the buffer
    buffer_t cdata;
    cdata.ptr = (const char*)bip32_path; // Cast to char* if necessary
    cdata.size = sizeof(bip32_path);
    cdata.offset = 0;

    explicit_bzero(&G_context, sizeof(G_context));
    G_context.req_type = CONFIRM_ADDRESS;
    G_context.state = STATE_NONE;


    cx_ecfp_private_key_t private_key = {0};
    cx_ecfp_public_key_t public_key = {0};

    // Read BIP32 path from incoming data and handle errors
    if (!buffer_read_u8(&cdata, &G_context.bip32_path_len) || !buffer_read_bip32_path(&cdata, G_context.bip32_path, (size_t) G_context.bip32_path_len)) {
        return -3;
    }

    // Derive private key according to BIP32 path
    if (crypto_derive_private_key(&private_key, G_context.bip32_path, G_context.bip32_path_len) != 0) {
        explicit_bzero(&private_key, sizeof(private_key));
        return -2; // or appropriate error code
    }

    // Generate corresponding public key
    if (crypto_init_public_key(&private_key, &public_key, G_context.pk_info.raw_public_key) != 0) {
        explicit_bzero(&private_key, sizeof(private_key));
        return -1; // or appropriate error code
    }

    // Reset private key after use for security
    explicit_bzero(&private_key, sizeof(private_key));
    return 0;
}