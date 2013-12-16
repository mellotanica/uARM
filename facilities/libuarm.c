#define TRANSMITTED     5
#define READY           1
#define ACK             1
#define PRINTCHR        2
#define CHAROFFSET      8
#define BUSY            3
#define STATUSMASK      0xFF
#define TERM0ADDR       0x240
#define TRANSTATUS      2
#define TRANCOMMAND     3
#define DEVREGLEN       4

typedef long unsigned int devreg;

typedef long unsigned int memaddr;

void tprint(const char *s){
    memaddr * statusp;
    memaddr * commandp;
    devreg stat;
    devreg cmd;
    unsigned int error = 0;

    /* compute device register field addresses */
    statusp = (devreg *) (TERM0ADDR  + (TRANSTATUS * DEVREGLEN));
    commandp = (devreg *) (TERM0ADDR + (TRANCOMMAND * DEVREGLEN));

    /* test device status */
    stat = ((*statusp) & STATUSMASK);
    if (stat == READY || stat == TRANSMITTED)
    {
            /* device is available */

            /* print cycle */
            while (*s != '\0' && !error)
            {
                    cmd = (*s << CHAROFFSET) | PRINTCHR;
                    *commandp = cmd;


                    /* busy waiting */
                    stat = ((*statusp) & STATUSMASK);
                    while (stat == BUSY)
                             stat = ((*statusp) & STATUSMASK);

                    /* end of wait */
                    if (stat != TRANSMITTED)
                            error = 1;
                    else
                            /* move to next char */
                            s++;
            }
    }
}
