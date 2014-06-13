Hi Laurent

1394API.cpp/1394API.h have some unrelated changes. Should be transparent
to you, but these are required.

HSS1394.cpp, HSS1394.h, HSS1394Types.h - all updated.

Test1394API.cpp has a snippet of code at the end of main that uses
the ::SendUserControl method.

--
Oxford:
NodeHSS1394.cpp, NodeHSS1394.h
Changes to API implemented.

Use of API shown in main.cpp (mixer) which installs a handler. The handler
does an echo back to the PC with a recognizable signature.

Hope this all makes sense.

Cheers
    Don

24Sep08
