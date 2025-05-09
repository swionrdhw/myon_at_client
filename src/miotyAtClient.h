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
 * \version     0.0.1
 * \brief       Client side of communication with a MIOTY™ modem via AT protocol v2.x.x
 */

#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#ifndef _AT_CLIENT_H
#define _AT_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum miotyAtClient_returnCode {
    MIOTYATCLIENT_RETURN_CODE_OK                         =   0, // ok
    MIOTYATCLIENT_RETURN_CODE_MacError                   =   1, // generic mac error
    MIOTYATCLIENT_RETURN_CODE_MacFramingError            =   2, // mac framing error
    MIOTYATCLIENT_RETURN_CODE_ArgumentSizeMismatch       =   3, // argument size mismatch
    MIOTYATCLIENT_RETURN_CODE_ArgumentOOR                =   4, // argument out of range
    MIOTYATCLIENT_RETURN_CODE_BufferSizeInsufficient     =   5, // buffer size insufficient
    MIOTYATCLIENT_RETURN_CODE_MacNodeNotAttached         =   6, // end-point not attached
    MIOTYATCLIENT_RETURN_CODE_MacNetworkKeyNotSet        =   7, // network key not set
    MIOTYATCLIENT_RETURN_CODE_MacAlreadyAttached         =   8, // already attached
    MIOTYATCLIENT_RETURN_CODE_ERR                        =   9, // generic error, not in protocol
    MIOTYATCLIENT_RETURN_CODE_MacDownlinkNotAvailable    =  10, // downlink not available
    MIOTYATCLIENT_RETURN_CODE_UplinkPackingErr           =  11, // uplink packing error
    MIOTYATCLIENT_RETURN_CODE_MacNoDownlinkReceived      =  12, // no downlink received of downlink acknowledge currupted
    MIOTYATCLIENT_RETURN_CODE_MacOptionNotAllowed        =  13, // option not allowed
    MIOTYATCLIENT_RETURN_CODE_MacDownlinkErr             =  14, // downlink crc error
    MIOTYATCLIENT_RETURN_CODE_MacDefaultsNotSet          =  15, // defaults not set
    MIOTYATCLIENT_RETURN_CODE_PreviousCommandNotFinished =  18, // previous command not finished
    MIOTYATCLIENT_RETURN_CODE_DownlinkDataCorrupted      =  22, // downlink acknowledge ok, downlink data currupted
    MIOTYATCLIENT_RETURN_CODE_FeatureNotSupported        = 100, // feature not supported
    MIOTYATCLIENT_RETURN_CODE_ATErr                      = 200, // generic AT error not found
    MIOTYATCLIENT_RETURN_CODE_ATgenericErr               = 201, // generic AT error
    MIOTYATCLIENT_RETURN_CODE_ATCommandNotKnown          = 202, // command not known,
    MIOTYATCLIENT_RETURN_CODE_ATParamOOB                 = 203, // parameter out of bounds
    MIOTYATCLIENT_RETURN_CODE_ATDataSizeMismatch         = 204, // data size mismatch
    MIOTYATCLIENT_RETURN_CODE_ATUnexpectedChar           = 206, // unexpected character
    MIOTYATCLIENT_RETURN_CODE_ATArgInvalid               = 207, // invalid argument
    MIOTYATCLIENT_RETURN_CODE_ATReadFailed               = 208, // reading data failed
} miotyAtClient_returnCode;


void miotyAtClientWrite(uint8_t *, uint16_t);
bool miotyAtClientRead(uint8_t *, uint8_t *);


/**
 * @brief Soft reset of the MIOTY™ modem. Persistent fields shall keep their current value.
 *
 * @return 
 */
miotyAtClient_returnCode miotyAtClient_reset(void); //AT-RST

/**
 * @brief Reset the MIOTY™ modem to its factory defaults
 *
 * @return 
 */
miotyAtClient_returnCode miotyAtClient_factoryReset(void); //ATZ

/**
 * @brief Restart device in bootloader (AT-SBTL)
 *
 * @return 
 */
miotyAtClient_returnCode miotyAtClient_startBootloader(void);

/**
 * @brief Shut down the modem. Wakeup only on TX_INH or RESET pin action. (AT-SHDN)
 *
 * @return 
 */
miotyAtClient_returnCode miotyAtClient_shutdown(void);

/**
 * @brief Send AT command to set the network key (AT-MNWK)
 *
 * @param[in]   nwKey           Pointer to a buffer containing the 16 byte long network key
 *
 * @return      miotyAtClient_returnCode    indicating success/error of AT_cmd execution

 */
miotyAtClient_returnCode miotyAtClient_setNetworkKey(uint8_t * nwKey);

/**
 * @brief Get/Set IPv6 subnet mask (AT-IPV6)
 *
 * @param[in,out]   ipv6            If set=true ipv6 contains the 8 byte value to be set as subnet mask, else ipv6 is a buffer where the value can be stored
 * @param[in]       set             If true the subnet mask will be set to ipv6, else it will be read from device and written to ipv6.
 *
 * @return          miotyAtClient_returnCode    indicating success/error of AT_cmd execution

 */
miotyAtClient_returnCode miotyAtClient_getOrSetIPv6SubnetMask(uint8_t * ipv6, bool set);

/**
 * @brief Get/Set EUI mask (AT-MEUI)
 *
 * @param[in,out]   eui             If set=true eui contains the 8 byte value to be set as EUI, else eui is a buffer where the value can be stored
 * @param[in]       set             If true the subnet mask will be set to eui, else it will be read from device and written to eui.
 *
 * @return          miotyAtClient_returnCode    indicating success/error of AT_cmd execution

 */
miotyAtClient_returnCode miotyAtClient_getOrSetEui(uint8_t * eui64, bool set);

/**
 * @brief Get/Set short Adress (AT-MSAD)
 *
 * @param[in,out]   shortAdress     Either a 2 byte array containing the new short Adress or a 2 byte buffer to store the current short Adress
 * @param[in]       set             If true short Adress will be set, otherwise it will be written to shortAdress
 *
 * @return 
 */
miotyAtClient_returnCode miotyAtClient_getOrSetShortAdress(uint8_t * shortAdress, bool set);

/**
 * @brief Send AT command to get the current packet counter (AT-MPCT)
 *
 * @param[out]   counter         Pointer to store the packet counter
 *
 * @return      miotyAtClient_returnCode    indicating success/error of AT_cmd execution

 */
miotyAtClient_returnCode miotyAtClient_getPacketCounter(uint32_t * counter);

/**
 * @brief Get/Set Uplink transmit power (AT-UTPL)
 *
 * @param[in,out]   txPower         Pointer to the transmit Power level needs to be in {10,...100}
 * @param[in]       set             If true transmit power will be set to *txPower otherwise it will be written to *txPower
 *
 * @return 
 */
miotyAtClient_returnCode miotyAtClient_getOrSetTransmitPower(uint32_t * txPower, bool set);

/**
 * @brief Get/Set uplink Mode (AT-UM)
 *
 * @param[in,out] ulMode    uplink Mode to set or get from the MIOTY modem
 * @param[in]     set       If true the uplink Mode of the MIOTY modem will be set to ulMode, else it will be read from the modem and written to ulMode
 *
 * @return      miotyAtClient_returnCode    indicating success/error of AT_cmd execution
 */
miotyAtClient_returnCode miotyAtClient_uplinkMode(uint32_t * ulMode, bool set);

/**
 * @brief Get/Set uplink Profile (AT-UP)
 *
 * @param[in,out] ulProfile    uplink Profile to set or get from the MIOTY modem
 * @param[in]     set          If true the uplink Profile of the MIOTY modem will be set to ulProfile, else it will be read from the modem and written to ulProfile
 *
 * @return      miotyAtClient_returnCode    indicating success/error of AT_cmd execution
 */
miotyAtClient_returnCode miotyAtClient_uplinkProfile(uint32_t * ulProfile, bool set);

/*!
 * \brief Send uni-directional message (AT-U)
 *
 * \param[in]       msg             Pointer to message to be send
 * \param[in]       sizemsg         Size of msg
 * \param[out]      packetCounter   packet Counter after successful transmission
 *
 * \return          miotyAtClient_returnCode    indicating success/error of AT_cmd execution
 */
miotyAtClient_returnCode miotyAtClient_sendMessageUni(uint8_t * msg, uint8_t sizeMsg, uint32_t * packetCounter);

/*!
 * \brief Send uni-directional message (AT-UMPF)
 *
 * \param[in]       msg             Pointer to message to be send (including MPF field)
 * \param[in]       sizemsg         Size of msg
 * \param[out]      packetCounter   packet Counter after successful transmission
 *
 * \return          miotyAtClient_returnCode    indicating success/error of AT_cmd execution
 */
miotyAtClient_returnCode miotyAtClient_sendMessageUniMPF(uint8_t * msg, uint8_t sizeMsg, uint32_t * packetCounter);

/*!
 * \brief Send bi-directional message (AT-B)
 *
 * \param[in]       msg             Pointer to message to be send
 * \param[in]       sizemsg         Size of msg
 * \param[out]      data            Pointer to a buffer, where data returned by AT_cmd will be stored
 * \param[out]      size_data       Size of the Buffer, will be set to size of data returned by AT_cmd
 * \param[out]      dl_mpf          Received downlink MPF field
 * \param[out]      packetCounter   packet Counter after successful transmission
 *
 * \return          miotyAtClient_returnCode    indicating success/error of AT_cmd execution
 */
miotyAtClient_returnCode miotyAtClient_sendMessageBidi(uint8_t * msg, uint8_t sizeMsg, uint8_t * data, uint8_t * size_data, uint8_t * dl_mpf, uint32_t * packetCounter);

/*!
 * \brief Send bi-directional message (AT-BMPF)
 *
 * \param[in]       msg             Pointer to message to be send (including MPF field)
 * \param[in]       sizemsg         Size of msg
 * \param[out]      data            Pointer to a buffer, where data returned by AT_cmd will be stored
 * \param[out]      size_data       Size of the Buffer, will be set to size of data returned by AT_cmd
 * \param[out]      dl_mpf          Received downlink MPF field
 * \param[out]      packetCounter   packet Counter after successful transmission
 *
 * \return          miotyAtClient_returnCode    indicating success/error of AT_cmd execution
 */
miotyAtClient_returnCode miotyAtClient_sendMessageBidiMPF(uint8_t * msg, uint8_t sizeMsg, uint8_t * data, uint8_t * size_data, uint8_t * dl_mpf, uint32_t * packetCounter);

/*!
 * \brief Send uni-directional message without MAC (AT-TU)
 *
 * \param[in]       msg             Pointer to message to be send
 * \param[in]       sizemsg         Size of msg
 * \param[out]      packetCounter   packet Counter after successful transmission
 *
 * \return          miotyAtClient_returnCode    indicating success/error of AT_cmd execution
 */
miotyAtClient_returnCode miotyAtClient_sendMessageUniTransparent(uint8_t * msg, uint8_t sizeMsg, uint32_t * packetCounter);

/*!
 * \brief Send bi-directional message without MAC (AT-TB)
 *
 * \param[in]       msg             Pointer to message to be send
 * \param[in]       sizemsg         Size of msg
 * \param[out]      data            Pointer to a buffer, where data returned by AT_cmd will be stored
 * \param[out]      size_data       Size of the Buffer, will be set to size of data returned by AT_cmd
 * \param[out]      packetCounter   packet Counter after successful transmission
 *
 * \return          miotyAtClient_returnCode    indicating success/error of AT_cmd execution
 */
miotyAtClient_returnCode miotyAtClient_sendMessageBidiTransparent(uint8_t * msg, uint8_t sizeMsg, uint8_t * data, uint8_t * size_data, uint32_t * packetCounter);

/*!
 * \brief Mac attach (AT-MAOA)
 *
 * \param[in]       msg     Pointer to NONCE data (length 4 required)
 * \param[out]      MSTA    MAC state as returned by the AT-Command
 *
 * \return          miotyAtClient_returnCode    indicating success/error of AT_cmd execution
 */
miotyAtClient_returnCode miotyAtClient_macAttach(uint8_t * data, uint8_t * MSTA);

/*!
 * \brief Mac dettach (AT-MDOA)
 *
 * \param[in]       data        Pointer to data that will be sent to base station
 * \param[in]       sizeData    Size of data
 * \param[out]      MSTA        MAC state as returned by the AT-Command
 *
 * \return          miotyAtClient_returnCode    indicating success/error of AT_cmd execution
 */
miotyAtClient_returnCode miotyAtClient_macDetach(uint8_t * data, uint8_t sizeData, uint8_t * MSTA);

/**
 * @brief Local Mac Attach (AT-MALO)
 *
 * @param[out]      MSTA            Mac State as returned by the MIOTY modem
 *
 * \return          miotyAtClient_returnCode    indicating success/error of AT_cmd execution
 */
miotyAtClient_returnCode miotyAtClient_macAttachLocal(uint8_t * MSTA);

/**
 * @brief Local Mac Detach (AT-MDLO)
 *
 * @param[out]      MSTA            Mac State as returned by the MIOTY modem
 *
 * \return          miotyAtClient_returnCode    indicating success/error of AT_cmd execution
 */
miotyAtClient_returnCode miotyAtClient_macDetachLocal(uint8_t * MSTA);

/**
 * @brief Get the attachment state
 *
 * @param[out]      attached_p            set to true if module is attached
 *
 * \return          miotyAtClient_returnCode    indicating success/error of AT_cmd execution
 */
miotyAtClient_returnCode miotyAtClient_getAttachment(bool * attached_p);

/**
 * @brief Get/Set downling request response flag for next uplink (AT-MRDR)
 *
 * @param[in,out] dl_flag   flag value to set or get from the MIOTY modem
 * @param[in]     set       If true the flag will be set to dl_flag, else it will be read from the modem and written to dl_flag
 *
 * @return      miotyAtClient_returnCode    indicating success/error of AT_cmd execution
 */
miotyAtClient_returnCode miotyAtClient_downlinkRequestResponseFlag(bool * dl_flag, bool set);

/*!
 * \brief Get end-point information (ATI)
 *
 * \param[out]      buffer          Pointer to a buffer, where data returned by AT_cmd will be stored
 * \param[out]      size_data       Size of the Buffer, will be set to size of data returned by AT_cmd
 *
 * \return          miotyAtClient_returnCode    indicating success/error of AT_cmd execution
 */
miotyAtClient_returnCode miotyAtClient_getEpInfo(uint8_t * buffer, uint8_t * sizeBuf);

/*!
 * \brief Get end-point core lib information (AT-LIBV)
 *
 * \param[out]      buffer          Pointer to a buffer, where data returned by AT_cmd will be stored
 * \param[out]      size_data       Size of the Buffer, will be set to size of data returned by AT_cmd
 *
 * \return          miotyAtClient_returnCode    indicating success/error of AT_cmd execution
 */
miotyAtClient_returnCode miotyAtClient_getCoreLibInfo(uint8_t * buffer, uint8_t * sizeBuf);

/**
 * @brief Get/Set TX inhibit function state (AT-TXINH)
 *
 * @param[in,out] enable    state value to set or get from the MIOTY modem
 * @param[in]     set       If true the state will be set to enable, else it will be read from the modem and written to enable
 *
 * @return      miotyAtClient_returnCode    indicating success/error of AT_cmd execution
 */
miotyAtClient_returnCode miotyArClient_txInhibit(bool * enable, bool set);

/**
 * @brief Get/Set TX active function state (AT-TXACT)
 *
 * @param[in,out] enable    state value to set or get from the MIOTY modem
 * @param[in]     set       If true the state will be set to enable, else it will be read from the modem and written to enable
 *
 * @return      miotyAtClient_returnCode    indicating success/error of AT_cmd execution
 */
miotyAtClient_returnCode miotyArClient_txActive(bool * enable, bool set);

/**
 * @brief Get/Set RX active function state (AT-RXACT)
 *
 * @param[in,out] enable    state value to set or get from the MIOTY modem
 * @param[in]     set       If true the state will be set to enable, else it will be read from the modem and written to enable
 *
 * @return      miotyAtClient_returnCode    indicating success/error of AT_cmd execution
 */
miotyAtClient_returnCode miotyArClient_rxActive(bool * enable, bool set);

/**
 * @brief Start sending an unmodulated carrier at the given frequency (AT$TXCU)
 *
 * @param[in]   frequency  Frequency in Hz to send the carrier at.
 *
 * @return      miotyAtClient_returnCode    indicating success/error of AT_cmd execution
 */
miotyAtClient_returnCode miotyAtClient_startTxContUnmodulated(uint32_t frequency);

/**
 * @brief Start sending a modulated carrier at the given frequency (AT$TXCLMP)
 *
 * @param[in]   frequency  Frequency in Hz to send the carrier at.
 *
 * @return      miotyAtClient_returnCode    indicating success/error of AT_cmd execution
 */
miotyAtClient_returnCode miotyAtClient_startTxContModulated(uint32_t frequency);

/**
 * @brief Stop sending a carrier (AT$TXOFF)
 *
 * @return      miotyAtClient_returnCode    indicating success/error of AT_cmd execution
 */
miotyAtClient_returnCode miotyAtClient_stopTxCont(void);

/**
 * @brief Start RX continuous mode at the given frequency (AT$TXCLMP)
 *
 * @param[in]   frequency  Frequency in Hz to start receiver at.
 *
 * @return      miotyAtClient_returnCode    indicating success/error of AT_cmd execution
 */
miotyAtClient_returnCode miotyAtClient_startRxCont(uint32_t frequency);

/**
 * @brief Stop RX continuous mode (AT$RXOFF)
 *
 * @return      miotyAtClient_returnCode    indicating success/error of AT_cmd execution
 */
miotyAtClient_returnCode miotyAtClient_stopRxCont(void);

#ifdef __cplusplus
}
#endif

#endif
