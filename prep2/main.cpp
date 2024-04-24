#include <iostream>
#include <cstddef>
#include <cstdint>

void print_rds(uint8_t* t_status, uint32_t t_len) {
    int num = 0;

    printf("-------------------------/0x--/0b--------/num----\n");
    printf("| [0]      Status        = %.2x %.8b\n", t_status[0], t_status[0]);
    num = (t_status[1] & 0b00100000) >> 5;    
    printf("| [1.5]    RDSNewBlockB  = %.2x ; %.8b ; %d\t\n", num, num, num);
    num = (t_status[1] & 0b00010000) >> 4;
    printf("| [1.4]    RDSNewBlockA  = %.2x ; %.8b ; %d\t\n", num, num, num);
    num = (t_status[1] & 0b00000100) >> 2;
    printf("| [1.2]    RDSSyncFound  = %.2x ; %.8b ; %d\t\n", num, num, num);
    num = (t_status[1] & 0b00000010) >> 1;
    printf("| [1.1]    RDSSyncLost   = %.2x ; %.8b ; %d\t\n", num, num, num);
    num = (t_status[1] & 0b00000001) >> 0;
    printf("| [1.0]    RDSRecv       = %.2x ; %.8b ; %d\t\n", num, num, num);
    num = (t_status[2] & 0b00000100) >> 2;
    printf("| [2.2]    GrpLost       = %.2x ; %.8b ; %d\t\n", num, num, num);
    num = t_status[2] & 0x01;
    printf("| [2.0]    RDSSync       = %.2x ; %.8b ; %d\t\n", num, num, num);
    num = t_status[2];
    printf("| [3.7:0]  RDSfifoUsed   = %.2x ; %.8b ; %d\t\n", num, num, num);
    num = t_status[4];
    printf("| [4.7:0]  BlockA (15:8) = %.2x ; %.8b ; %d\t\n", num, num, num);
    num = t_status[5];
    printf("| [5.7:0]  BlockA (7:0)  = %.2x ; %.8b ; %d\t\n", num, num, num);
    num = t_status[6];
    printf("| [6.7:0]  BlockB (15:8) = %.2x ; %.8b ; %d\t\n", num, num, num);
    num = t_status[7];
    printf("| [7.7:0]  BlockB (7:0)  = %.2x ; %.8b ; %d\t\n", num, num, num);
    num = t_status[8];
    printf("| [8.7:0]  BlockC (15:8) = %.2x ; %.8b ; %d\t\n", num, num, num);
    num = t_status[9];
    printf("| [9.7:0]  BlockC (7:0)  = %.2x ; %.8b ; %d\t\n", num, num, num);
    num = t_status[10];
    printf("| [10.7:0] BlockD (15:8) = %.2x ; %.8b ; %d\t\n", num, num, num);
    num = t_status[11];
    printf("| [11.7:0] BlockD (7:0)  = %.2x ; %.8b ; %d\t\n", num, num, num);
    num = (t_status[12] & 0xC0) >> 6;
    printf("| [12.7:6] BleA[1:0]     = %.2x ; %.8b ; %d\t\n", num, num, num);
    num = (t_status[12] & 0x30) >> 4;
    printf("| [12.5:4] BleB[1:0]     = %.2x ; %.8b ; %d\t\n", num, num, num);
    num = (t_status[12] & 0x0C) >> 2;
    printf("| [12.3:2] BleC[1:0]     = %.2x ; %.8b ; %d\t\n", num, num, num);
    num = (t_status[12] & 0x03) >> 0;
    printf("| [12.1:0] BleD[1:0]     = %.2x ; %.8b ; %d\t\n", num, num, num);
    printf("-------------------------------------------------\n");
}

#if 0

void rds_init() {
    i2c_start();
    i2c_output(SI4735_ADDRESS | W);
    i2c_output(0x12); // Set property
    i2c_output(0x00); // Property group 0x00 (RDS)
    i2c_output(0x12); // RDS configuration
    i2c_output(0x01); // Enable RDS
    i2c_output(0x00); // Reserved
    i2c_stop();
}

uint8_t rds_status(char* t_str, int t_len) {
    i2c_start();

    uint8_t l_ack = 0;

    size_t l_len = t_len / 4;

    for(size_t i = 0; i < l_len; ++i) {
        l_ack = i2c_output();
        l_ack = i2c_input()
    }

    i2c_stop();
}

char si4735_get_radiotext(char* radiotext) {
    char l_ack = 0;

    i2c_start();

    // l_ack |= i2c_output(SI4735_ADDRESS | W); // redundant
    l_ack |= i2c_output(0x24); // FM_RDS_STATUS
    l_ack |= i2c_output(0x00); // ARG1

    i2c_start();

    // l_ack |= i2c_output(SI4735_ADDRESS | R); // redundant

    for (int i = 0; i < 64; ++i) {
        radiotext[i] = i2c_input();
        i2c_ack();
    }
    i2c_nack();

    i2c_stop();

    return (l_ack != 0) ? 1 : 0;
}

void monitor_rds_data(void) {
    char radiotext[65];
    while (1) {
        if (si4735_get_radiotext(radiotext) == 0)
            printf("Radiotext: %s\n", radiotext);
        else printf("Error retrieving Radiotext.\n");

        wait_ms(1000);
    }
}

#endif


int main() {
    srand(time(nullptr));
    uint8_t l_status[13];
    for(size_t i = 0; i < 13; ++i) {
        l_status[i] = rand();
    }
    print_rds(l_status, 13);

    

    return 0;
}
