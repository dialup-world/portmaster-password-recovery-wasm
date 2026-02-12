# portmaster-password-recovery-wasm
A Wasm implementation of [the PortMaster password recovery tool](http://home.eversberg.eu/public/portmaster.php) by [jolly](http://eversberg.eu/contact/). The [original C implementation](source/mz.c) was written by pmsac@toxyn.org, implementing an MD5 message-digest algorithm from Ron Rivest, written by Colin Plumb, with contributions by Ian Jackson and Stephen Early.

Livingston/Lucent PortMasters may have a password set if they are purchased used. There is a password recovery mechanism where the user is given a CHALLENGE code that they would have been able to send to customer support. The support agent would then provide a RESPONSE that the user could enter and reset the password. The RESPONSE is essentially just an MD5 digest composed of the CHALLENGE code and some extra bytes, and can be computed locally without the need to contact an external party. 

This should work for the PM2e, PM3, PM4, and similar devices.

## Usage

A live demo is available at https://dialup.world/portmaster-password/. It seems to work in FireFox, Chrome, and Safari.

Upon loading the page you can follow these instructions on your Portmaster:

1. At login prompt enter: `!root`
2. At password prompt enter: `override`
3. Write down the 16 character challenge string here: `<CHALLENGE_STRING>` and enter it into the webpage to receive a `<RESPONSE_STRING>`
4. At login prompt enter again: `!root`
5. At password prompt enter: `<RESPONSE_STRING>`
6. Be sure to change password `set password <pw>` and save it `save all`

## Why a Wasm Version?

The original tool is written in PHP and likely calls the underlying C code to get a response. If it doesn't do this, that means there is a PHP implementation that hasn't been made publically available. Either way, there should be a way to use this tool without needing to build/run a C program and/or host a PHP application if the original ever disappears.

Wasm seemed like the logical way to use the existing C code without a massive rewrite, and have the resulting tool available as a highly-portable application that can be copied into any webserver directory any *just work*.

## Run it Yourself

The necessary files to run this tool are `index.htm`, `mz_web.js`, and `mz_web.wasm` and can be dropped into any web server. Due to limitations with Wasm, this MUST be run via a web server, it cannot be loaded from the local filesystem.

## Build it Yourself

The original `mz.c` file was modified into `mz_web.c` with changes for Wasm. Notably, we cannot call a `main` function via Wasm, so this had to be modified.

[Emscripten](https://emscripten.org/) was used to build `mz_web`. The installation of Emscripten is out of scope for this guide, but after it is installed `mz_web.c` can be compiled via:

```
emcc mz_web.c -o mz_web.js \
  -sEXPORTED_FUNCTIONS='["_get_response", "_malloc", "_free"]' \
  -sEXPORTED_RUNTIME_METHODS='["cwrap", "UTF8ToString"]' \
  -sMODULARIZE -sENVIRONMENT=web
```

This generates `mz_web.js` and `mz_web.wasm`.
