
    radix  DEC

    EXTERN _main

_reset code
    pagesel _main
    goto _main

startup code
    pagesel _main
    goto _main

    END
