
## HowTo compile esp-device
1. Set-up build environment like [described in Wiki](https://github.com/Necktschnagge/wired-watering/wiki/Toolchain-SETUP-(ESP8266)-(on-Windows)).
2. start your shell `C:\msys32\mingw32.exe`.
3. `git clone <this_repository>`
4. `cd <path_to_repository>/esp-device`
5. `make -j17 all`
   * You might get propmted with so called _MENUCONFIG_.
   * You can just exit it.
   * If you want to flash a device you should set the port where to talk to your device at `Serial flasher config ---> Default serial port`.
   * You can also do it later running `make menuconfig` to get back to this menu.


## to be replaced: Simple HTTPD Server Example

The Example consists of HTTPD server demo with demostration of URI handling :
    1. URI \hello for GET command returns "Hello World!" message
    2. URI \echo for POST command echoes back the POSTed message

* Configure the project using "make menuconfig" and goto :
    * Example Configuration ->
        1. WIFI SSID: WIFI network to which your PC is also connected to.
        2. WIFI Password: WIFI password

* In order to test the HTTPD server persistent sockets demo :
    1. compile and burn the firmware "make flash"
    2. run "make monitor" and note down the IP assigned to your ESP module. The default port is 80
    3. test the example :
        * run the test script : "python2 scripts/client.py \<IP\> \<port\> \<MSG\>"
            * the provided test script first does a GET \hello and displays the response
            * the script does a POST to \echo with the user input \<MSG\> and displays the response
        * or use curl (asssuming IP is 192.168.43.130):
            1. "curl 192.168.43.130:80/hello"  - tests the GET "\hello" handler
            2. "curl -X POST --data-binary @anyfile 192.168.43.130:80/echo > tmpfile"
                * "anyfile" is the file being sent as request body and "tmpfile" is where the body of the response is saved
                * since the server echoes back the request body, the two files should be same, as can be confirmed using : "cmp anyfile tmpfile"
            3. "curl -X PUT -d "0" 192.168.43.130:80/ctrl" - disable /hello and /echo handlers
            4. "curl -X PUT -d "1" 192.168.43.130:80/ctrl" -  enable /hello and /echo handlers

See the README.md file in the upper level 'examples' directory for more information about examples.
