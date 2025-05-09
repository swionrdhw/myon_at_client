/**
 * \copyright    Copyright 2019 - 2022 Fraunhofer Institute for Integrated Circuits IIS, Erlangen Germany
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
 
/**
 * \file
 * \version     0.0.2
 * \brief       Client side of communication with a MIOTY™ module via AT protocol v2.x.x
 */

#include "miotyAtClient.h"
#include "string_tools.h"

static miotyAtClient_returnCode get_info_bytes(char * AT_cmd, uint8_t sizeCmd, uint8_t * buffer, uint8_t * sizeBuf);
static miotyAtClient_returnCode set_info_bytes(char * AT_cmd, uint8_t sizeCmd, uint8_t * data, uint8_t size_data);
static miotyAtClient_returnCode get_info_int(char * AT_cmd, uint8_t sizeCmd, uint32_t * res);
static miotyAtClient_returnCode set_info_int(char * AT_cmd, uint8_t sizeCmd, uint32_t * info);
static miotyAtClient_returnCode get_info_string(char * AT_cmd, uint8_t sizeCmd, uint8_t * buffer, uint8_t * sizeBuf);
static miotyAtClient_returnCode checkATresponseMsg(uint32_t * packetCounter);
static void get_packet_counter(char * response_buf, uint32_t * packetCounter);
static void get_DLMPF(char * response_buf, uint8_t * DLMPF);
static void get_MSTA(char * response_buf, uint8_t * MSTA);
static void write_cmd_bytes(char * AT_cmd, uint8_t sizeCmd, uint8_t * data, uint8_t sizeData);
static miotyAtClient_returnCode get_int_data_AtResponse(char * AT_cmd, uint8_t sizeCmd, uint32_t * res, char * response_buf);
static miotyAtClient_returnCode get_data_AtResponse(char * AT_cmd, uint8_t sizeCmd, uint8_t * buffer, uint8_t * sizeBuf, char * response_buf);
static miotyAtClient_returnCode get_string_AtResponse(char * AT_cmd, uint8_t sizeCmd, uint8_t * buffer, uint8_t * sizeBuf, char * response_buf);
static miotyAtClient_returnCode check_AtResponse(char * response_buf);


miotyAtClient_returnCode miotyAtClient_reset(void) {
    char cmd[7] = "AT-RST\r";
    miotyAtClientWrite((uint8_t *)cmd, sizeof(cmd));
    /* this command has no answer */
    return MIOTYATCLIENT_RETURN_CODE_OK;
}

miotyAtClient_returnCode miotyAtClient_factoryReset(void) {
    char cmd[4] = "ATZ\r";
    miotyAtClientWrite((uint8_t *)cmd, sizeof(cmd));
    /* this command has no answer */
    return MIOTYATCLIENT_RETURN_CODE_OK;
}

miotyAtClient_returnCode miotyAtClient_startBootloader(void) {
    char cmd[8] = "AT-SBTL\r";
    miotyAtClientWrite((uint8_t *)cmd, sizeof(cmd));
    /* this command has no answer */
    return MIOTYATCLIENT_RETURN_CODE_OK;
}

miotyAtClient_returnCode miotyAtClient_shutdown(void) {
    char cmd[8] = "AT-SHDN\r";
    miotyAtClientWrite((uint8_t *)cmd, sizeof(cmd));
    /* this command has no answer */
    return MIOTYATCLIENT_RETURN_CODE_OK;
}

miotyAtClient_returnCode miotyAtClient_setNetworkKey(uint8_t * nwKey) {
    return set_info_bytes("AT-MNWK", 7, nwKey, 16);
}

miotyAtClient_returnCode miotyAtClient_getOrSetIPv6SubnetMask(uint8_t * ipv6, bool set) {
    if (set)
        return set_info_bytes("AT-MIP6", 7, ipv6, 8);
    uint8_t size_bytes = 8;
    return get_info_bytes("AT-MIP6", 7, ipv6, &size_bytes);
}

miotyAtClient_returnCode miotyAtClient_getOrSetEui(uint8_t * eui64, bool set) {
    if (set)
        return set_info_bytes("AT-MEUI", 7, eui64, 8);
    uint8_t size_bytes = 8;
    return get_info_bytes("AT-MEUI", 7, eui64, &size_bytes);
}

miotyAtClient_returnCode miotyAtClient_getOrSetShortAdress(uint8_t * shortAdress, bool set){
    if (set)
        return set_info_bytes("AT-MSAD", 7, shortAdress, 2);
    uint8_t size_bytes = 2;
    return get_info_bytes("AT-MSAD", 7, shortAdress, &size_bytes);
}

miotyAtClient_returnCode miotyAtClient_getPacketCounter(uint32_t * counter) {
    return get_info_int("AT-MPCT", 7, counter);
}

miotyAtClient_returnCode miotyAtClient_getOrSetTransmitPower(uint32_t * txPower, bool set) {
    if (set)
        return set_info_int("AT-UTPL", 7, txPower);
    return get_info_int("AT-UTPL", 7, txPower);
}

miotyAtClient_returnCode miotyAtClient_uplinkMode(uint32_t * ulMode, bool set) {
    if (set)
        return set_info_int("AT-UM", 5, ulMode);
    return get_info_int("AT-UM", 5, ulMode);
}

miotyAtClient_returnCode miotyAtClient_uplinkProfile(uint32_t * ulProfile, bool set) {
    if (set)
        return set_info_int("AT-UP", 5, ulProfile);
    return get_info_int("AT-UP", 5, ulProfile);
}

miotyAtClient_returnCode miotyAtClient_sendMessageUni(uint8_t * msg, uint8_t sizeMsg, uint32_t * packetCounter) {
    write_cmd_bytes("AT-U", 4, msg, sizeMsg);
    return checkATresponseMsg(packetCounter);
}

miotyAtClient_returnCode miotyAtClient_sendMessageUniMPF(uint8_t * msg, uint8_t sizeMsg, uint32_t * packetCounter) {
    write_cmd_bytes("AT-UMPF", 7, msg, sizeMsg);
    return checkATresponseMsg(packetCounter);
}

miotyAtClient_returnCode miotyAtClient_sendMessageBidi(uint8_t * msg, uint8_t sizeMsg, uint8_t * data, uint8_t * size_data, uint8_t * dl_mpf, uint32_t * packetCounter) {
    write_cmd_bytes("AT-B", 4, msg, sizeMsg);
    char response_buf[200];
    miotyAtClient_returnCode ret = get_data_AtResponse( "AT-B", 4, data, size_data, response_buf);
    get_packet_counter(response_buf, packetCounter);
    if (ret != MIOTYATCLIENT_RETURN_CODE_OK)
        return ret;
    get_DLMPF(response_buf, dl_mpf);

    return ret;
}

miotyAtClient_returnCode miotyAtClient_sendMessageBidiMPF(uint8_t * msg, uint8_t sizeMsg, uint8_t * data, uint8_t * size_data, uint8_t * dl_mpf, uint32_t * packetCounter) {
    write_cmd_bytes("AT-BMPF", 7, msg, sizeMsg);
    char response_buf[200];
    miotyAtClient_returnCode ret = get_data_AtResponse( "AT-B", 4, data, size_data, response_buf);
    get_packet_counter(response_buf, packetCounter);
    if (ret != MIOTYATCLIENT_RETURN_CODE_OK)
        return ret;
    get_DLMPF(response_buf, dl_mpf);

    return ret;
}

miotyAtClient_returnCode miotyAtClient_sendMessageUniTransparent(uint8_t * msg, uint8_t sizeMsg, uint32_t * packetCounter) {
    write_cmd_bytes("AT-TU", 5, msg, sizeMsg);
    return checkATresponseMsg(packetCounter);
}

miotyAtClient_returnCode miotyAtClient_sendMessageBidiTransparent(uint8_t * msg, uint8_t sizeMsg, uint8_t * data, uint8_t * size_data, uint32_t * packetCounter) {
    write_cmd_bytes("AT-TB", 5, msg, sizeMsg);
    char response_buf[200];
    miotyAtClient_returnCode ret = get_data_AtResponse( "AT-TB", 5, data, size_data, response_buf);
    get_packet_counter(response_buf, packetCounter);
    return ret;
}

miotyAtClient_returnCode miotyAtClient_macAttach(uint8_t * data, uint8_t * MSTA) {
    write_cmd_bytes("AT-MAOA", 7, data, 4);
    char response_buf[200];
    miotyAtClient_returnCode ret = check_AtResponse( response_buf);
    if ( ret != MIOTYATCLIENT_RETURN_CODE_OK)
        return ret;
    get_MSTA(response_buf, MSTA);

    return ret;
}

miotyAtClient_returnCode miotyAtClient_macDetach(uint8_t * data, uint8_t sizeData, uint8_t * MSTA) {
    write_cmd_bytes("AT-MDOA", 7, data, sizeData);
    char response_buf[200];
    miotyAtClient_returnCode ret = check_AtResponse( response_buf);
    if ( ret != MIOTYATCLIENT_RETURN_CODE_OK)
        return ret;
    get_MSTA(response_buf, MSTA);

    return ret;
}

miotyAtClient_returnCode miotyAtClient_macAttachLocal(uint8_t * MSTA) {
    miotyAtClientWrite((uint8_t *)"AT-MALO\r", 8);
    char response_buf[200];
    miotyAtClient_returnCode ret = check_AtResponse( response_buf);
    if (ret != MIOTYATCLIENT_RETURN_CODE_OK)
        return ret;
    get_MSTA(response_buf, MSTA);

    return ret;
}

miotyAtClient_returnCode miotyAtClient_macDetachLocal(uint8_t * MSTA) {
    miotyAtClientWrite((uint8_t *)"AT-MDLO\r", 8);
    char response_buf[200];
    miotyAtClient_returnCode ret = check_AtResponse( response_buf);
    if (ret != MIOTYATCLIENT_RETURN_CODE_OK)
        return ret;
    get_MSTA(response_buf, MSTA);

    return ret;
}

miotyAtClient_returnCode miotyAtClient_getAttachment(bool * attached_p) {
    uint32_t result;
    miotyAtClient_returnCode ret = get_info_int("AT-MAS", 6, &result);
    if (ret == MIOTYATCLIENT_RETURN_CODE_OK && attached_p != NULL)
    {
        *attached_p = result;
    }
    return ret;
}

miotyAtClient_returnCode miotyAtClient_downlinkRequestResponseFlag(bool * dl_flag, bool set) {
    uint32_t val;
    if (set) {
        val = *dl_flag;
        return set_info_int("AT-MRDR", 7, &val);
    }
    miotyAtClient_returnCode ret = get_info_int("AT-MRDR", 7, &val);
    if (ret == MIOTYATCLIENT_RETURN_CODE_OK)
    {
        *dl_flag = val;
    }
    return ret;
}

miotyAtClient_returnCode miotyAtClient_getEpInfo(uint8_t * buffer, uint8_t * sizeBuf) {
    return get_info_string("ATI", 3, buffer, sizeBuf);
}

miotyAtClient_returnCode miotyAtClient_getCoreLibInfo(uint8_t * buffer, uint8_t * sizeBuf) {
    return get_info_string("AT-LIBV", 7, buffer, sizeBuf);
}

miotyAtClient_returnCode miotyArClient_txInhibit(bool * enable, bool set) {
    uint32_t val;
    if (set) {
        val = *enable;
        return set_info_int("AT-TXINH", 8, &val);
    }
    miotyAtClient_returnCode ret = get_info_int("AT-TXINH", 8, &val);
    if (ret == MIOTYATCLIENT_RETURN_CODE_OK)
    {
        *enable = val;
    }
    return ret;
}

miotyAtClient_returnCode miotyArClient_txActive(bool * enable, bool set) {
    uint32_t val;
    if (set) {
        val = *enable;
        return set_info_int("AT-TXACT", 8, &val);
    }
    miotyAtClient_returnCode ret = get_info_int("AT-TXACT", 8, &val);
    if (ret == MIOTYATCLIENT_RETURN_CODE_OK)
    {
        *enable = val;
    }
    return ret;
}

miotyAtClient_returnCode miotyArClient_rxActive(bool * enable, bool set) {
    uint32_t val;
    if (set) {
        val = *enable;
        return set_info_int("AT-RXACT", 8, &val);
    }
    miotyAtClient_returnCode ret = get_info_int("AT-RXACT", 8, &val);
    if (ret == MIOTYATCLIENT_RETURN_CODE_OK)
    {
        *enable = val;
    }
    return ret;
}

miotyAtClient_returnCode miotyAtClient_startTxContUnmodulated(uint32_t frequency) {
    return set_info_int("AT$TXCU", 7, &frequency);
}

miotyAtClient_returnCode miotyAtClient_startTxContModulated(uint32_t frequency) {
    return set_info_int("AT$TXCMLP", 9, &frequency);
}

miotyAtClient_returnCode miotyAtClient_stopTxCont(void) {
    miotyAtClientWrite((uint8_t *)"AT$TXOFF\r", 9);
    char response_buf[10];
    return check_AtResponse( response_buf);
}

miotyAtClient_returnCode miotyAtClient_startRxCont(uint32_t frequency) {
    return set_info_int("AT$RXCONT", 9, &frequency);
}

miotyAtClient_returnCode miotyAtClient_stopRxCont(void) {
    miotyAtClientWrite((uint8_t *)"AT$RXOFF\r", 9);
    char response_buf[10];
    return check_AtResponse( response_buf);
}


static miotyAtClient_returnCode get_info_bytes(char * AT_cmd, uint8_t sizeCmd, uint8_t * buffer, uint8_t * sizeBuf) {
    char cmd[sizeCmd+2];
    strcpy(cmd, AT_cmd);
    cmd[sizeCmd] = '?';
    cmd[sizeCmd+1] = '\r';
    miotyAtClientWrite((uint8_t *)cmd, sizeof(cmd));
    char response_buf[200];
    return get_data_AtResponse( AT_cmd, sizeCmd, buffer, sizeBuf, response_buf);
}

static miotyAtClient_returnCode set_info_bytes(char * AT_cmd, uint8_t sizeCmd, uint8_t * data, uint8_t sizeData) {
    write_cmd_bytes(AT_cmd, sizeCmd, data, sizeData);
    char response_buf[200];
    return check_AtResponse(response_buf);
}

static miotyAtClient_returnCode get_info_int(char * AT_cmd, uint8_t sizeCmd, uint32_t * res) {
    char cmd[sizeCmd+2];
    strcpy(cmd, AT_cmd);
    cmd[sizeCmd] = '?';
    cmd[sizeCmd+1] = '\r';
    miotyAtClientWrite((uint8_t *)cmd, sizeof(cmd));
    char response_buf[200];
    return get_int_data_AtResponse( AT_cmd, sizeCmd, res, response_buf);
}

static miotyAtClient_returnCode set_info_int(char * AT_cmd, uint8_t sizeCmd, uint32_t * info) {
    char buf[12] = {0};
    uint8_t len_info =  string_uint2str_la_zt(*info, buf) - buf;
    char cmd[sizeCmd+2+len_info];
    strcpy(cmd, AT_cmd);
    cmd[sizeCmd] = '=';
    strcpy(cmd+sizeCmd+1, buf);
    cmd[sizeCmd+1+len_info] = '\r';
    miotyAtClientWrite((uint8_t *)cmd, sizeof(cmd));
    char response_buf[200];
    return check_AtResponse( response_buf);
}

static miotyAtClient_returnCode get_info_string(char * AT_cmd, uint8_t sizeCmd, uint8_t * buffer, uint8_t * sizeBuf) {
    char cmd[sizeCmd+1];
    strcpy(cmd, AT_cmd);
    cmd[sizeCmd] = '\r';
    miotyAtClientWrite((uint8_t *)cmd, sizeof(cmd));
    char response_buf[200];
    return get_string_AtResponse( AT_cmd, sizeCmd, buffer, sizeBuf, response_buf);
}

static miotyAtClient_returnCode checkATresponseMsg(uint32_t * packetCounter) {
    char response_buf[200];
    miotyAtClient_returnCode ret = check_AtResponse( response_buf);
    get_packet_counter(response_buf, packetCounter);
    return ret;
}

static void get_packet_counter(char * response_buf, uint32_t * packetCounter) {
    char * pos = strstr(response_buf, "-MPCT:");
    if( (pos != NULL) && (packetCounter != NULL) ) {
        *packetCounter = atoi((pos+6));
    }
}

static void get_DLMPF(char * response_buf, uint8_t * DLMPF) {
    if (!DLMPF) {
        return;
    }
    char * pos = strstr(response_buf, "-DLMPF:1\t");
    if(pos != NULL) {
        string_hex2byteArray((uint8_t *)pos + 9, 2, DLMPF, 1);
    }
    else {
        *DLMPF = 0;
    }
}

static void get_MSTA(char * response_buf, uint8_t * MSTA) {
    char * pos = strstr(response_buf, "-MSTA:");
    if( (pos != NULL) && (MSTA != NULL) )
        *MSTA = atoi(pos+6);
}

// converts uint8_t data to hexadecimal string representation
static void write_cmd_bytes(char * AT_cmd, uint8_t sizeCmd, uint8_t * data, uint8_t sizeData) {
    uint8_t const dataStringSize = 2*sizeData;
    char lenString[4];
    uint8_t digits = string_uint2str_la_zt(sizeData, lenString) - lenString;
    char dataString[dataStringSize];
    string_byteArray2hex(data, sizeData, dataString, dataStringSize);
    char cmd[sizeCmd+sizeof(dataString)+5+digits];
    strcpy(cmd, AT_cmd);
    cmd[sizeCmd] = '=';
    strcpy(cmd+sizeCmd+1, lenString);
    cmd[sizeCmd+1+digits] = '\t';
    strcpy(cmd+sizeCmd+digits+2, dataString);
    cmd[sizeCmd+dataStringSize+digits+2] = '\x1A';
    cmd[sizeCmd+dataStringSize+digits+3] = '\r';
    
    miotyAtClientWrite((uint8_t *) cmd, sizeof(cmd));
}

static miotyAtClient_returnCode get_int_data_AtResponse(char * AT_cmd, uint8_t sizeCmd, uint32_t * res, char * response_buf) {
    uint8_t pos=0;
    miotyAtClient_returnCode return_code = MIOTYATCLIENT_RETURN_CODE_ERR;
    while(1) {
        uint8_t buf[30];
        uint8_t len = 30;
        if(miotyAtClientRead(buf, &len) == true) {
        
            for (uint8_t i=0; i<len; i++) {
                if(isalpha(buf[i]))
                    buf[i] = toupper(buf[i]);
                response_buf[i+pos] = buf[i];
            }
            pos += len;
            response_buf[pos] = '\0';
            if (strstr(response_buf, "\r\n0\r\n") || strstr(response_buf, "0\r\n")==response_buf) {
                return_code = MIOTYATCLIENT_RETURN_CODE_OK;
                char * pos = strstr(response_buf, AT_cmd+2);
                pos += sizeCmd-1;
                *res = atoi(pos);
                break;
            } else if (strstr(response_buf, "\r\n1\r\n")) {
                char * err_pos = strstr(response_buf, "-MNFO:");
                if (err_pos == NULL)
                    err_pos = strstr(response_buf, "-MERR:");
                if (err_pos == NULL) {
                    return_code = MIOTYATCLIENT_RETURN_CODE_ERR;
                    break;
                }
                err_pos += 6;
                return_code = atoi(err_pos);
                break;
            } else if (strstr(response_buf, "\r\n2\r\n")) {
                char * err_pos = strstr(response_buf, "AT!ERR:");
                if (err_pos == NULL) {
                    return_code = MIOTYATCLIENT_RETURN_CODE_ATErr;
                    break;
                }
                err_pos += 7;
                return_code = atoi(err_pos) + MIOTYATCLIENT_RETURN_CODE_ATErr;
                break;
            }
        } else {
            return MIOTYATCLIENT_RETURN_CODE_ATReadFailed;
        }
    }
    return return_code;
}

static miotyAtClient_returnCode get_data_AtResponse(char * AT_cmd, uint8_t sizeCmd, uint8_t * buffer, uint8_t * sizeBuf, char * response_buf) {
    uint8_t pos=0;
    miotyAtClient_returnCode return_code = MIOTYATCLIENT_RETURN_CODE_ERR;
    while(1) {
        uint8_t buf[30];
        uint8_t len = 30;
        if(miotyAtClientRead(buf, &len)) {
            for (uint8_t i=0; i<len; i++) {
                if(isalpha(buf[i]))
                    buf[i] = toupper(buf[i]);
                response_buf[i+pos] = buf[i];
            }
            pos += len;
            response_buf[pos+1] = '\0';
            if (strstr(response_buf, "\r\n0\r\n") || strstr(response_buf, "0\r\n")==response_buf) {
                return_code = MIOTYATCLIENT_RETURN_CODE_OK;
                char * data_pos = strstr(response_buf, AT_cmd+2);
                data_pos += (sizeCmd-2);
                data_pos = strstr(data_pos, "\t");
                data_pos++;
                char * end_pos = strstr(data_pos, "\x1a\r");
                uint32_t len_data = (*sizeBuf)*2;
                if (end_pos != 0){
                    len_data = end_pos - data_pos;
                    *sizeBuf = len_data/2;
                }
                string_hex2byteArray((uint8_t *)data_pos, len_data, buffer, *sizeBuf);
                break;
            } else if (strstr(response_buf, "\r\n1\r\n")) {
                char * err_pos = strstr(response_buf, "-MNFO:");
                if (err_pos == NULL)
                    err_pos = strstr(response_buf, "-MERR:");
                if (err_pos == NULL) {
                    return_code = MIOTYATCLIENT_RETURN_CODE_ERR;
                    break;
                }
                err_pos += 6;
                return_code = atoi(err_pos);
                break;
            } else if (strstr(response_buf, "\r\n2\r\n")) {
                char * err_pos = strstr(response_buf, "AT!ERR:");
                if (err_pos == NULL) {
                    return_code = MIOTYATCLIENT_RETURN_CODE_ATErr;
                    break;
                }
                err_pos += 7;
                return_code = atoi(err_pos) + MIOTYATCLIENT_RETURN_CODE_ATErr;
                break;
            }
        } else {
            return MIOTYATCLIENT_RETURN_CODE_ATReadFailed;
        }
    }
    return return_code;
}

static miotyAtClient_returnCode get_string_AtResponse(char * AT_cmd, uint8_t sizeCmd, uint8_t * buffer, uint8_t * sizeBuf, char * response_buf) {
    uint8_t pos=0;
    miotyAtClient_returnCode return_code = MIOTYATCLIENT_RETURN_CODE_ERR;
    while(1) {
        uint8_t buf[30];
        uint8_t len = 30;
        if(miotyAtClientRead(buf, &len)) {
            for (uint8_t i=0; i<len; i++) {
                response_buf[i+pos] = buf[i];
            }
            pos += len;
            response_buf[pos+1] = '\0';
            if (strstr(response_buf, "\r\n0\r\n") || strstr(response_buf, "0\r\n")==response_buf) {
                return_code = MIOTYATCLIENT_RETURN_CODE_OK;
                char * string_pos = strstr(response_buf, AT_cmd+2);
                string_pos += (sizeCmd - 2);
                string_pos++;  // ':' 
                char * end_pos = strstr(string_pos, "\r");
                if (end_pos != 0){
                    uint8_t len_data = end_pos - string_pos;
                    *sizeBuf = len_data;
                    memcpy(buffer, string_pos, len_data);
                }
                break;
            } else if (strstr(response_buf, "\r\n1\r\n")) {
                char * err_pos = strstr(response_buf, "-MNFO:");
                if (err_pos == NULL)
                    err_pos = strstr(response_buf, "-MERR:");
                if (err_pos == NULL) {
                    return_code = MIOTYATCLIENT_RETURN_CODE_ERR;
                    break;
                }
                err_pos += 6;
                return_code = atoi(err_pos);
                break;
            } else if (strstr(response_buf, "\r\n2\r\n")) {
                char * err_pos = strstr(response_buf, "AT!ERR:");
                if (err_pos == NULL) {
                    return_code = MIOTYATCLIENT_RETURN_CODE_ATErr;
                    break;
                }
                err_pos += 7;
                return_code = atoi(err_pos) + MIOTYATCLIENT_RETURN_CODE_ATErr;
                break;
            }
        } else {
            return MIOTYATCLIENT_RETURN_CODE_ATReadFailed;
        }
    }
    return return_code;
}

static miotyAtClient_returnCode check_AtResponse(char * response_buf) {
    uint8_t pos=0;
    miotyAtClient_returnCode return_code = MIOTYATCLIENT_RETURN_CODE_ERR;
    while(1) {
        uint8_t buf[30];
        uint8_t len = 30;
        if(miotyAtClientRead(buf, &len)) {
            for (uint8_t i=0; i<len; i++) {
                if(isalpha(buf[i]))
                    buf[i] = toupper(buf[i]);
                response_buf[i+pos] = buf[i];
            }
            pos += len;
            response_buf[pos+1] = '\0';
            if (strstr(response_buf, "\r\n0\r\n") || strstr(response_buf, "0\r\n")==response_buf) {
                return_code = MIOTYATCLIENT_RETURN_CODE_OK;
                break;
            } else if (strstr(response_buf, "\r\n1\r\n")) {
                char * err_pos = strstr(response_buf, "-MNFO:");
                if (err_pos == NULL)
                    err_pos = strstr(response_buf, "-MERR:");
                if (err_pos == NULL) {
                    return_code = MIOTYATCLIENT_RETURN_CODE_ERR;
                    break;
                }
                err_pos += 6;
                return_code = atoi(err_pos);
                break;
            } else if (strstr(response_buf, "\r\n2\r\n")) {
                char * err_pos = strstr(response_buf, "AT!ERR:");
                if (err_pos == NULL) {
                    return_code = MIOTYATCLIENT_RETURN_CODE_ATErr;
                    break;
                }
                err_pos += 7;
                return_code = atoi(err_pos) + MIOTYATCLIENT_RETURN_CODE_ATErr;
                break;
            }
        } else {
            return MIOTYATCLIENT_RETURN_CODE_ATReadFailed;
        }
    }
    return return_code;
}
