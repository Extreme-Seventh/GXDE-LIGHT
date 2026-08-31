#include <pgmspace.h>

    // ============================================================
    // WIFI HTML
    // ============================================================

    const char WIFI_HTML[] PROGMEM = R"rawliteral(
    <!DOCTYPE html>

    <html>

    <head>

        <meta charset="UTF-8">

        <meta name="viewport" content="width=device-width,initial-scale=1">

        <title>GXDE WiFi</title>

        <style>
            body {
                margin: 0;
                background: #020707;
                color: #00ffe1;
                font-family: Arial;
                display: flex;
                justify-content: center;
                align-items: center;
                min-height: 100vh;
                padding: 20px;
            }

            .panel {
                width: min(500px, 100%);
                padding: 25px;
                border: 1px solid #00ffe155;
                border-radius: 20px;
                background: #00ffe108;
            }

            h1 {
                text-align: center;
                letter-spacing: 5px;
            }

            label {
                display: block;
                margin-top: 15px;
                margin-bottom: 7px;
                font-size: 12px;
            }

            input,
            select {
                width: 100%;
                padding: 12px;
                box-sizing: border-box;
                background: #001514;
                color: #00ffe1;
                border: 1px solid #00ffe155;
                border-radius: 8px;
            }

            button {
                width: 100%;
                margin-top: 20px;
                padding: 14px;
                border: 0;
                border-radius: 10px;
                background: #00ff9c;
                font-weight: bold;
            }

            .info {
                text-align: center;
                opacity: .7;
                line-height: 1.6;
                font-size: 13px;
            }

            .passwordRow {
                display: flex;
                align-items: center;
                gap: 0;
                margin-top: 7px;
            }

            .passwordRow input {
                flex: 1;
                margin-top: 0;
                border-radius: 6px 0 0 6px;
            }

            .passwordToggle {
                width: 42px;
                height: 44px;
                min-height: 37px;
                margin-top: 0;
                padding: 0;

                border-left: none;
                border-radius: 0 6px 6px 0;

                display: flex;
                align-items: center;
                justify-content: center;
                cursor: pointer;
            }

            .passwordToggle svg {
                width: 20px;
                height: 20px;
            }
        </style>

    </head>

    <body>

        <div class="panel">

            <h1>GXDE</h1>

            <div class="info">
                WIFI CONFIGURATION
                <br><br>
                GXDE is currently in AP mode.
            </div>

            <form method="POST" action="/wifi/save">

                <label>WIFI NETWORK</label>

                <select id="ssid" name="ssid">

                    <option value="">
                        Select network
                    </option>

                </select>

                <label>SSID</label>

                <input id="manualssid" name="manualssid" placeholder="SSID">

                <label>PASSWORD</label>
                <div class="passwordRow">
                    <input type="password" name="password" id="password" placeholder="Wi-Fi password">

                    <button type="button" id="passwordButton" class="passwordToggle"
                        aria-label="Toggle password visibility" onclick="togglePassword()">
                        <!-- SVG Eye Icon (Visible by Default) -->
                        <svg id="eyeIcon" viewBox="0 0 24 24">
                            <path
                                d="M12 7c2.76 0 5 2.24 5 5 0 .65-.13 1.26-.36 1.82l2.92 2.92c1.51-1.26 2.7-2.89 3.44-4.74-1.73-4.39-6-7.5-11-7.5-1.4 0-2.74.25-3.98.7l2.16 2.16C10.74 7.13 11.35 7 12 7zM2 4.27l2.28 2.28.46.46C3.08 8.3 1.78 10.02 1 12c1.73 4.39 6 7.5 11 7.5 1.55 0 3.03-.3 4.38-.84l.42.42L19.73 22 21 20.73 3.27 3 2 4.27zM7.53 9.8l1.55 1.55c-.05.21-.08.43-.08.65 0 1.66 1.34 3 3 3 .22 0 .44-.03.65-.08l1.55 1.55c-.67.33-1.41.53-2.2.53-2.76 0-5-2.24-5-5 0-.79.2-1.53.53-2.2zm4.31-.78l3.15 3.15.01-.16c0-1.66-1.34-3-3-3l-.16.01z" />
                        </svg>
                    </button>
                </div>



                <button type="submit">

                    SAVE WIFI & RESTART

                </button>

            </form>

        </div>

        <script>

            /* ========================================================
                      PASSWORD SHOW / HIDE
                   ======================================================== */
            const eyeOpenPath = "M12 4.5C7 4.5 2.73 7.61 1 12c1.73 4.39 6 7.5 11 7.5s9.27-3.11 11-7.5c-1.73-4.39-6-7.5-11-7.5zM12 17c-2.76 0-5-2.24-5-5s2.24-5 5-5 5 2.24 5 5-2.24 5-5 5zm0-8c-1.66 0-3 1.34-3 3s1.34 3 3 3 3-1.34 3-3-1.34-3-3-3z";
            const eyeClosedPath = "M12 7c2.76 0 5 2.24 5 5 0 .65-.13 1.26-.36 1.82l2.92 2.92c1.51-1.26 2.7-2.89 3.44-4.74-1.73-4.39-6-7.5-11-7.5-1.4 0-2.74.25-3.98.7l2.16 2.16C10.74 7.13 11.35 7 12 7zM2 4.27l2.28 2.28.46.46C3.08 8.3 1.78 10.02 1 12c1.73 4.39 6 7.5 11 7.5 1.55 0 3.03-.3 4.38-.84l.42.42L19.73 22 21 20.73 3.27 3 2 4.27zM7.53 9.8l1.55 1.55c-.05.21-.08.43-.08.65 0 1.66 1.34 3 3 3 .22 0 .44-.03.65-.08l1.55 1.55c-.67.33-1.41.53-2.2.53-2.76 0-5-2.24-5-5 0-.79.2-1.53.53-2.2zm4.31-.78l3.15 3.15.01-.16c0-1.66-1.34-3-3-3l-.16.01z";

            function togglePassword() {

                const password =
                    document.getElementById(
                        'password'
                    );

                const button =
                    document.getElementById(
                        'passwordButton'
                    );

                if (
                    password.type === 'password'
                ) {

                    password.type =
                        'text';

                    // button.innerText ='HIDE';
                    document.querySelector('#eyeIcon path').setAttribute('d', eyeOpenPath);
                }

                else {

                    password.type =
                        'password';

                    // button.innerText = 'SHOW';
                    document.querySelector('#eyeIcon path').setAttribute('d', eyeClosedPath);
                }

            }
            async function scan() {

                try {

                    let r =
                        await fetch("/wifi/scan");

                    let networks =
                        await r.json();

                    let select =
                        document.getElementById("ssid");

                    networks.forEach(n => {

                        let o =
                            document.createElement("option");

                        o.value =
                            n.ssid;

                        o.text =
                            n.ssid +
                            " (" +
                            n.rssi +
                            " dBm)";

                        select.appendChild(o);

                    });

                    select.onchange =
                        function () {

                            document.getElementById(
                                "manualssid"
                            ).value =
                                this.value;

                        };

                } catch (e) {

                    console.log(e);

                }

            }

            scan();

        </script>

    </body>

    </html>

    )rawliteral";