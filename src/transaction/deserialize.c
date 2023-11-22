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

#include <string.h>  // memset
#include "deserialize.h"
#include "utils.h"
#include "types.h"
#include "../common/buffer.h"

parser_status_e read_opcode(buffer_t *script_buf, uint8_t *opcode) {
    bool flag = buffer_read_u8(script_buf, opcode);
    return flag;
}

parser_status_e read_ctx(buffer_t *script_buf, ctx_t *ctx) {
    if (!read_opcode(script_buf, &ctx->opcode)) {
        return SCRIPT_PARSING_ERROR;
    }
    if (ctx->opcode != 45) {
        // expected opcode CTX(45)
        return SCRIPT_PARSING_ERROR_OPCODE_CTX45;
    }
    if (!buffer_read_u8(script_buf, &ctx->src_reg)) {
        return SCRIPT_PARSING_ERROR;
    }
    if (!buffer_read_u8(script_buf, &ctx->dest_reg)) {
        return SCRIPT_PARSING_ERROR;
    }
    return PARSING_OK;
}

parser_status_e read_extcall(buffer_t *script_buf, extcall_t *extcall) {
    if (!read_opcode(script_buf, &extcall->opcode)) {
        return SCRIPT_PARSING_ERROR;
    }
    if (extcall->opcode != 7) {
        // expected opcode EXTCALL(7)
        return SCRIPT_PARSING_ERROR_OPCODE_EXTCALL7;
    }
    if (!buffer_read_u8(script_buf, &extcall->dest_reg)) {
        return SCRIPT_PARSING_ERROR;
    }
    return PARSING_OK;
}

parser_status_e read_switch(buffer_t *script_buf, switch_t *swwitch) {
    if (!read_opcode(script_buf, &swwitch->opcode)) {
        return SCRIPT_PARSING_ERROR;
    }
    if (swwitch->opcode != 46) {
        // expected opcode SWITCH(46)
        return SCRIPT_PARSING_ERROR_OPCODE_SWITCH46;
    }
    if (!buffer_read_u8(script_buf, &swwitch->dest_reg)) {
        return SCRIPT_PARSING_ERROR;
    }
    return PARSING_OK;
}

parser_status_e read_end(buffer_t *script_buf, end_t *end) {
    if (!read_opcode(script_buf, &end->opcode)) {
        return SCRIPT_PARSING_ERROR;
    }
    if (end->opcode != 11) {
        // expected opcode RET(11)
        return SCRIPT_PARSING_ERROR_OPCODE_RET11;
    }
    return PARSING_OK;
}

parser_status_e read_load(buffer_t *script_buf, load_t *load) {
    uint8_t opcode;
    uint8_t reg;
    uint8_t type;
    uint8_t buf_len;
    if (!read_opcode(script_buf, &opcode)) {
        return SCRIPT_PARSING_ERROR;
    }
    if (opcode != 13) {
        // expected opcode LOAD(13)
        return SCRIPT_PARSING_ERROR_OPCODE_LOAD13;
    }
    if (!buffer_read_u8(script_buf, &reg)) {
        return SCRIPT_PARSING_ERROR;
    }
    if (!buffer_read_u8(script_buf, &type)) {
        return SCRIPT_PARSING_ERROR;
    }
    if (!buffer_read_u8(script_buf, &buf_len)) {
        return SCRIPT_PARSING_ERROR;
    }
    buffer_t buf = {.ptr = script_buf->ptr + script_buf->offset, .size = buf_len, .offset = 0};
    buffer_seek_cur(script_buf, buf_len);

    load->opcode = opcode;
    load->reg = reg;
    load->type = type;
    load->buf = buf;

    return PARSING_OK;
}

parser_status_e read_push(buffer_t *script_buf, push_t *push) {
    uint8_t opcode;
    uint8_t reg;
    if (!read_opcode(script_buf, &opcode)) {
        return SCRIPT_PARSING_ERROR;
    }
    if (opcode != 3) {
        // expected opcode PUSH(2)
        return SCRIPT_PARSING_ERROR_OPCODE_PUSH2;
    }
    if (!buffer_read_u8(script_buf, &reg)) {
        return SCRIPT_PARSING_ERROR;
    }
    push->opcode = opcode;
    push->reg = reg;

    return PARSING_OK;
}

parser_status_e read_load_push(buffer_t *script_buf, load_push_t *arg) {
    parser_status_e load_status = read_load(script_buf, &arg->load);
    if (load_status != PARSING_OK) {
        return load_status;
    }
    parser_status_e push_status = read_push(script_buf, &arg->push);
    if (push_status != PARSING_OK) {
        return push_status;
    }
    return PARSING_OK;
}

parser_status_e read_method_args(buffer_t *script_buf, load_push_t *args, const size_t args_len) {
    for (size_t ix = 0; ix < args_len; ix++) {
        parser_status_e arg_status = read_load_push(script_buf, &args[ix]);
        if (arg_status != PARSING_OK) {
            return arg_status;
        }
    }
    return PARSING_OK;
}

parser_status_e read_contract(buffer_t *script_buf, contract_t *contract) {
    parser_status_e method_args_status =
        read_method_args(script_buf, contract->args, ALLOW_GAS_ARGS_LEN);
    if (method_args_status != PARSING_OK) {
        return method_args_status;
    }
    parser_status_e method_status = read_load_push(script_buf, &contract->method);
    if (method_status != PARSING_OK) {
        return method_status;
    }
    parser_status_e contract_name_status = read_load(script_buf, &contract->name);
    if (contract_name_status != PARSING_OK) {
        return contract_name_status;
    }
    parser_status_e ctx_status = read_ctx(script_buf, &contract->ctx);
    if (ctx_status != PARSING_OK) {
        return ctx_status;
    }
    parser_status_e switch_op_status = read_switch(script_buf, &contract->swwitch);
    if (switch_op_status != PARSING_OK) {
        return switch_op_status;
    }
    return PARSING_OK;
}

parser_status_e read_interop(buffer_t *script_buf, interop_t *interop) {
    parser_status_e method_args_status =
        read_method_args(script_buf, interop->args, interop->args_len);
    if (method_args_status != PARSING_OK) {
        return method_args_status;
    }
    parser_status_e method_status = read_load(script_buf, &interop->method);
    if (method_status != PARSING_OK) {
        return method_status;
    }
    parser_status_e extcall_status = read_extcall(script_buf, &interop->extcall);
    if (extcall_status != PARSING_OK) {
        return extcall_status;
    }
    return PARSING_OK;
}

parser_status_e script_deserialize(buffer_t *script_buf,
                                   contract_t *allow_gas,
                                   interop_t *transfer_tokens,
                                   contract_t *spend_gas,
                                   end_t *end) {
    memset(allow_gas, 0, sizeof(contract_t));
    memset(transfer_tokens, 0, sizeof(interop_t));
    memset(spend_gas, 0, sizeof(contract_t));
    memset(end, 0, sizeof(end_t));
    allow_gas->args_len = ALLOW_GAS_ARGS_LEN;
    transfer_tokens->args_len = TRANSFER_TOKENS_ARGS_LEN;
    spend_gas->args_len = SPEND_GAS_ARGS_LEN;

    parser_status_e allow_gas_status = read_contract(script_buf, allow_gas);
    if (allow_gas_status != PARSING_OK) {
        // return allow_gas_status;
    }
    parser_status_e read_transfer_tokens_status = read_interop(script_buf, transfer_tokens);
    if (read_transfer_tokens_status != PARSING_OK) {
        // return read_transfer_tokens_status;
    }
    parser_status_e read_spend_gas_status = read_contract(script_buf, spend_gas);
    if (read_spend_gas_status != PARSING_OK) {
        // return read_spend_gas_status;
    }
    parser_status_e read_end_status = read_end(script_buf, end);
    if (read_end_status != PARSING_OK) {
        // return read_end_status;
    }
    return PARSING_OK;
    //    return (script_buf->offset == script_buf->size) ? PARSING_OK : SCRIPT_UNDERFLOW_ERROR;
}

parser_status_e transaction_deserialize(buffer_t *buf, transaction_t *tx) {
    if (buf->size > MAX_TRANSACTION_LEN) {
        return LENGTH_OVERFLOW_ERROR;
    }

    if (buf->size == 0) {
        return LENGTH_ZERO_ERROR;
    }

    // get nexus name
    buffer_read_varint(buf, &tx->nexus_len);
    if (tx->nexus_len > NEXUS_LEN) {
        return NEXUS_OVERFLOW_ERROR;
    }
    if (tx->nexus_len == 0) {
        return NEXUS_ZERO_ERROR;
    }
    tx->nexus = (uint8_t *) (buf->ptr + buf->offset);
    if (!buffer_seek_cur(buf, tx->nexus_len)) {
        return NEXUS_PARSING_ERROR;
    }

    // get chain name
    buffer_read_varint(buf, &tx->chain_len);
    if (tx->chain_len > CHAIN_LEN) {
        return CHAIN_OVERFLOW_ERROR;
    }
    if (tx->chain_len == 0) {
        return CHAIN_ZERO_ERROR;
    }
    tx->chain = (uint8_t *) (buf->ptr + buf->offset);
    if (!buffer_seek_cur(buf, tx->chain_len)) {
        return CHAIN_PARSING_ERROR;
    }

    // get script
    buffer_read_varint(buf, &tx->script_len);
    if (tx->script_len > SCRIPT_LEN) {
        return SCRIPT_OVERFLOW_ERROR;
    }
    if (tx->script_len == 0) {
        return SCRIPT_ZERO_ERROR;
    }
    tx->script = (uint8_t *) (buf->ptr + buf->offset);
    if (!buffer_seek_cur(buf, tx->script_len)) {
        return SCRIPT_PARSING_ERROR;
    }
    tx->script_buf.ptr = tx->script;
    tx->script_buf.size = tx->script_len;
    tx->script_buf.offset = 0;
    parser_status_e script_deserialize_status = script_deserialize(&tx->script_buf,
                                                                   &tx->allow_gas,
                                                                   &tx->transfer_tokens,
                                                                   &tx->spend_gas,
                                                                   &tx->end);
    if (script_deserialize_status == PARSING_OK) {
        //'Runtime.TransferTokens', [from(3), to(2), tokenName(1), amount(0)])
        tx->from = (uint8_t *) tx->transfer_tokens.args[3].load.buf.ptr;
        tx->from_len = tx->transfer_tokens.args[3].load.buf.size;
        tx->to = (uint8_t *) tx->transfer_tokens.args[2].load.buf.ptr; // 2
        tx->to_len = tx->transfer_tokens.args[2].load.buf.size; //2

        tx->value = (uint8_t *) tx->transfer_tokens.args[0].load.buf.ptr; // Was 0 
        tx->value_len = tx->transfer_tokens.args[0].load.buf.size; // Was 0

        tx->token = (uint8_t *) tx->transfer_tokens.args[1].load.buf.ptr;
        tx->token_len = tx->transfer_tokens.args[1].load.buf.size;

        tx->gas_price = (uint8_t *) tx->allow_gas.args[1].load.buf.ptr;
        tx->gas_price_len = tx->allow_gas.args[1].load.buf.size;

        tx->gas_limit = (uint8_t *) tx->allow_gas.args[0].load.buf.ptr;
        tx->gas_limit_len = tx->allow_gas.args[0].load.buf.size;
    } else {
        return script_deserialize_status;
    }

    // get expiration
    buffer_read_u32(buf, &tx->expiration, BE);

    // get payload
    buffer_read_varint(buf, &tx->payload_len);
    if (tx->payload_len > PAYLOAD_LEN) {
        return PAYLOAD_OVERFLOW_ERROR;
    }
    if (tx->payload_len == 0) {
        return PAYLOAD_ZERO_ERROR;
    }
    tx->payload = (uint8_t *) (buf->ptr + buf->offset);
    if (!buffer_seek_cur(buf, tx->payload_len)) {
        return PAYLOAD_PARSING_ERROR;
    }

    // // amount value
    // if (!buffer_read_u64(buf, &tx->value, BE)) {
    //    return VALUE_PARSING_ERROR;
    //}

    // return (buf->offset == buf->size) ? PARSING_OK : LENGTH_UNDERFLOW_ERROR;
    return PARSING_OK;
}
