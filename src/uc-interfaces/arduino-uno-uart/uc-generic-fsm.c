/*
 * uC Interface - A GUI for Programming & Interfacing with Microcontrollers
 * Copyright (C) 2018  Mitchell Oleson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "uc-generic-fsm.h"

/*
 * Replace the following section with the desired checksum.
 * This will need to be input as part of the INI file under each
 * GUI section (gets broken apart based on the GUI tab type).
 * Multiple instances of the same tab type must have the same checksum.
 * Default checksum is CRC-8 POLY.
*/
#include "crc-8-lut.h"

// GUI checksums
checksum_struct io_checksum = {get_crc_8_LUT_size, get_crc_8_LUT, check_crc_8_LUT};
checksum_struct data_transfer_checksum = {get_crc_8_LUT_size, get_crc_8_LUT, check_crc_8_LUT};
checksum_struct programmer_checksum = {get_crc_8_LUT_size, get_crc_8_LUT, check_crc_8_LUT};

// Default checksum (for acks, errors, and resets);
checksum_struct default_checksum = {get_crc_8_LUT_size, get_crc_8_LUT, check_crc_8_LUT};

// Recv buffers
uint8_t *fsm_buffer;
uint8_t *fsm_checksum_buffer;
uint8_t *fsm_checksum_cmp_buffer;
uint8_t *fsm_ack_buffer;

// Key & packet holders
uint8_t num_s2_bytes;
uint8_t major_key, curr_packet_stage;
uint32_t checksum_max_size;

// Buffer pointer holder
uint8_t* fsm_buffer_ptr;

// Function prototypes (local access only)
void fsm_ack(uint8_t ack_key);
bool fsm_read_next(uint8_t* data_array, uint32_t num_bytes, uint32_t timeout);
bool fsm_check_checksum(uint8_t* data, uint32_t data_len, uint8_t* checksum_cmp);
checksum_struct* fsm_get_checksum_struct();

void fsm_setup(uint32_t buffer_len)
{
    // Initialize variables
    major_key = MAJOR_KEY_ERROR;
    curr_packet_stage = 1;

    // Select largest checksum for array creation
    checksum_max_size = default_checksum.get_checksum_size();
    if (checksum_max_size < io_checksum.get_checksum_size())
        checksum_max_size = io_checksum.get_checksum_size();
    if (checksum_max_size < data_transfer_checksum.get_checksum_size())
        checksum_max_size = data_transfer_checksum.get_checksum_size();
    if (checksum_max_size < programmer_checksum.get_checksum_size())
        checksum_max_size = programmer_checksum.get_checksum_size();

    // Malloc buffers
    fsm_buffer = malloc(buffer_len*sizeof(fsm_buffer));
    fsm_ack_buffer = malloc(num_s1_bytes*sizeof(fsm_ack_buffer));
    fsm_checksum_buffer = malloc(checksum_max_size*sizeof(fsm_checksum_buffer));
    fsm_checksum_cmp_buffer = malloc(checksum_max_size*sizeof(fsm_checksum_cmp_buffer));

    // Reset to start defaults
    uc_reset();
}

void fsm_destroy()
{
    // Free buffers
    free(fsm_buffer);
    free(fsm_ack_buffer);
    free(fsm_checksum_buffer);
    free(fsm_checksum_cmp_buffer);
}

void fsm_poll()
{
    // Loop forever
    while (true)
    {
        // Reset buffer pointer
        fsm_buffer_ptr = fsm_buffer;

        // Read first stage or loop after 1 second
        if (!fsm_read_next(fsm_buffer_ptr, num_s1_bytes, packet_timeout)) continue;
        fsm_buffer_ptr += num_s1_bytes;

        // Store first stage info
        major_key = fsm_buffer[s1_major_key_loc];
        num_s2_bytes = fsm_buffer[s1_num_s2_bytes_loc];

        // Read Second stage or ACK failed after 1 second
        // Reset uc buffers and return to first stage if failed
        if (!fsm_read_next(fsm_buffer_ptr, num_s2_bytes, packet_timeout))
        {
            fsm_ack(MAJOR_KEY_ERROR);
            uc_reset_buffers();
            continue;
        }

        // Read Checksum
        uint32_t checksum_size = fsm_get_checksum_struct()->get_checksum_size();
        if (!fsm_read_next(fsm_checksum_buffer, checksum_size, packet_timeout))
        {
            fsm_ack(MAJOR_KEY_ERROR);
            uc_reset_buffers();
            continue;
        }

        // Check Checksum
        if (!fsm_check_checksum(fsm_buffer, num_s1_bytes+num_s2_bytes, fsm_checksum_buffer))
        {
            fsm_ack(MAJOR_KEY_ERROR);
            uc_reset_buffers();
            continue;
        }

        // Send Packet Ack
        fsm_ack(major_key);

        // Run FSM
        fsm_run();
    }
}

bool fsm_isr()
{
    // Select which packet stage we are receiving
    if (curr_packet_stage == 1)
    {
        // Only read if enough values present to not block
        if (uc_bytes_available() == num_s1_bytes)
        {
            // Reset fsm_buffer_ptr for first read
            fsm_buffer_ptr = fsm_buffer;

            // Read with 0 timeout
            if (fsm_read_next(fsm_buffer_ptr, num_s1_bytes, 0))
            {
                // Increment buffer pointer
                fsm_buffer_ptr += num_s1_bytes;

                // Parse info for first stage
                major_key = fsm_buffer[s1_major_key_loc];
                num_s2_bytes = fsm_buffer[s1_num_s2_bytes_loc];

                // If no second stage go to stage 4
                if (num_s2_bytes == 0)
                {
                    // Move to third stage
                    curr_packet_stage = 3;
                } else
                {
                    // Move to second stage
                    curr_packet_stage = 2;
                }
            }
        }
    } else if (curr_packet_stage == 2)
    {
        // Only read if enough values present to not block
        if (uc_bytes_available() == num_s2_bytes)
        {
            // Read second stage with 0 timeout
            if (fsm_read_next(fsm_buffer_ptr, num_s2_bytes, 0))
            {
                // Move to third stage
                curr_packet_stage = 3;
            }
        }
    } else if (curr_packet_stage == 3)
    {
        // Only read if enough values present to not block
        uint32_t checksum_size = fsm_get_checksum_struct()->get_checksum_size();
        if (uc_bytes_available() == checksum_size)
        {
            // Read Checksum with 0 timeout
            if (fsm_read_next(fsm_checksum_buffer, checksum_size, 0))
            {
                // Check Checksum
                if (!fsm_check_checksum(fsm_buffer, num_s1_bytes+num_s2_bytes, fsm_checksum_buffer))
                {
                    fsm_ack(MAJOR_KEY_ERROR);
                    uc_reset_buffers();
                    return false;
                }

                // Send Packet Ack
                fsm_ack(major_key);

                // Return to first stage on next call
                curr_packet_stage = 1;

                // Ready for fsm call
                return true;
            }
        }
    } else
    {
        // Encountered error, reset everything
        curr_packet_stage = 1;
        uc_reset_buffers();
    }

    // Not ready for fsm call
    return false;
}

void fsm_run()
{
    // Remove fsm stage 1 info before passing to function
    // fsm_buffer_ptr = fsm_buffer+num_s1_bytes;

    // Parse and act on major key
    switch (major_key)
    {
        case GUI_TYPE_IO:
            uc_io(fsm_buffer_ptr, num_s2_bytes);
            break;
        case GUI_TYPE_DATA_TRANSMIT:
            uc_data_transmit(fsm_buffer_ptr, num_s2_bytes);
            break;
        case GUI_TYPE_PROGRAMMER:
            uc_programmer(fsm_buffer_ptr, num_s2_bytes);
            break;
        default: // Will fall threw for MAJOR_KEY_ERROR, MAJOR_KEY_RESET
            uc_reset();
            break;
    }
}

void fsm_ack(uint8_t ack_key)
{
    // Fill buffer
    fsm_ack_buffer[s1_major_key_loc] = MAJOR_KEY_ACK;
    fsm_ack_buffer[s1_num_s2_bytes_loc] = ack_key;

    // Send buffer (fsm send transparently adds checksum)
    fsm_send(fsm_ack_buffer, num_s1_bytes);
}

void fsm_send(uint8_t* data, uint32_t data_len)
{
    checksum_struct* check = fsm_get_checksum_struct();
    uint32_t checksum_size = check->get_checksum_size();
    uint8_t checksum_start[checksum_size];
    memset(checksum_start, 0, checksum_size);
    check->get_checksum(data, data_len, checksum_start, fsm_checksum_buffer);

    uc_send(data, data_len);
    uc_send(fsm_checksum_buffer, checksum_size);
}

bool fsm_read_next(uint8_t* data_array, uint32_t num_bytes, uint32_t timeout)
{
    // Set control variables
    uint32_t check_delay = 10; // ms
    uint32_t wait_time = 0;

    // Wait for num_bytes to be received
    while (uc_bytes_available() < num_bytes)
    {
        uc_delay(check_delay);
        wait_time += check_delay;
        if (timeout < wait_time) return false;
    }

    // Read bytes into array
    for (uint32_t i = 0; i < num_bytes; i++)
    {
        data_array[i] = uc_getch();
    }
    return true;
}

bool fsm_check_checksum(uint8_t* data, uint32_t data_len, uint8_t* checksum_cmp)
{
    checksum_struct* check = fsm_get_checksum_struct();
    uint32_t checksum_size = check->get_checksum_size();
    uint8_t checksum_start[checksum_size];
    memset(checksum_start, 0, checksum_size);
    check->get_checksum(data, data_len, checksum_start, fsm_checksum_cmp_buffer);
    return check->check_checksum(checksum_cmp, fsm_checksum_cmp_buffer);
}

checksum_struct* fsm_get_checksum_struct()
{
    switch (major_key)
    {
        case GUI_TYPE_IO:
            return &io_checksum;
        case GUI_TYPE_DATA_TRANSMIT:
            return &data_transfer_checksum;
        case GUI_TYPE_PROGRAMMER:
            return &programmer_checksum;
        default:
            return &default_checksum;
    }

}