#ifndef PAGE_INDEX_H
#define PAGE_INDEX_H

const char page_index[] =
"<!DOCTYPE html>\n"
"<html>\n"
"<head>\n"
    "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n"
    "<title>ESP32 Management AP</title>\n"
    "<style>\n"
        "body {\n"
            "width: fit-content;\n"
        "}\n"
        "table, tr, th {\n"
            "border: 1px solid;\n"
            "border-collapse: collapse;\n"
            "text-align: center;\n"
        "}\n"
        "th, td {\n"
            "padding:10px;\n"
        "}\n"
        "tr:hover, tr.selected {\n"
            "background-color: lightblue;\n"
            "cursor:pointer;\n"
        "}\n"
    "</style>\n"
"</head>\n"
"<body onLoad=\"getStatus()\">\n"
    "<h1>ESP32 Wi-Fi Penetration Tool</h1>\n"
    "<section id=\"errors\"></section>\n"
    "<section id=\"loading\">Loading... Please wait</section>\n"
    "<section id=\"ready\" style=\"display: none;\">\n"
        "<h2>Attack configuration</h2>\n"
        "<form onSubmit=\"runAttack(); return false;\">\n"
            "<fieldset>\n"
                "<legend>Select target</legend>\n"
                "<table id=\"ap-list\"></table>\n"
                "<p>\n"
                    "<button type=\"button\" onClick=\"refreshAps()\">Refresh</button>\n"
                "</p>\n"
            "</fieldset>\n"
            "<fieldset>\n"
                "<legend>Attack configuration</legend>\n"
                "<p>\n"
                    "<label for=\"attack_type\">Attack type:</label>\n"
                    "<select id=\"attack_type\" onChange=\"updateConfigurableFields(this)\" required>\n"
                        "<option value=\"0\" title=\"This type is not implemented yet.\" disabled>ATTACK_TYPE_PASSIVE</option>\n"
                        "<option value=\"1\">ATTACK_TYPE_HANDSHAKE</option>\n"
                        "<option value=\"2\" selected>ATTACK_TYPE_PMKID</option>\n"
                        "<option value=\"3\">ATTACK_TYPE_DOS</option>\n"
                    "</select>\n"
                "</p>\n"
                "<p>\n"
                    "<label for=\"attack_method\">Attack method:</label>\n"
                    "<select id=\"attack_method\" required disabled>\n"
                        "<option value=\"\" selected disabled hidden>NOT AVAILABLE</option>\n"
                    "</select>\n"
                "</p>\n"
                "<p>\n"
                    "<label for=\"attack_timeout\">Attack timeout (seconds):</label>\n"
                    "<input type=\"number\" min=\"0\" max=\"255\" id=\"attack_timeout\" value=\"\" required/>\n"
                "</p>\n"
                "<p>\n"
                    "<button>Attack</button>\n"
                "</p>\n"
            "</fieldset>\n"
        "</form>\n"
    "</section>\n"
    "<section id=\"running\" style=\"display: none;\">\n"
        "Time elapsed: <span id=\"running-progress\"></span>\n"
    "</section>\n"
    "<section id=\"result\" style=\"display: none;\">\n"
        "<div id=\"result-meta\">Loading result.. Please wait</div>\n"
        "<div id=\"result-content\"></div>\n"
        "<button type=\"button\" onClick=\"resetAttack()\">New attack</button>\n"
    "</section>\n"
    "<script>\n"
    "var AttackStateEnum = { READY: 0, RUNNING: 1, FINISHED: 2, TIMEOUT: 3};\n"
    "var AttackTypeEnum = { ATTACK_TYPE_PASSIVE: 0, ATTACK_TYPE_HANDSHAKE: 1, ATTACK_TYPE_PMKID: 2, ATTACK_TYPE_DOS: 3};\n"
    "var selectedApElement = -1;\n"
    "var poll;\n"
    "var poll_interval = 1000;\n"
    "var running_poll;\n"
    "var running_poll_interval = 1000;\n"
    "var attack_timeout = 0;\n"
    "var time_elapsed = 0;\n"
    "var defaultResultContent = document.getElementById(\"result\").innerHTML;\n"
    "var defaultAttackMethods = document.getElementById(\"attack_method\").outerHTML;\n"
    "function getStatus() {\n"
        "var oReq = new XMLHttpRequest();\n"
        "oReq.onload = function() {\n"
            "var arrayBuffer = oReq.response;\n"
            "if(arrayBuffer) {\n"
                "var attack_state = parseInt(new Uint8Array(arrayBuffer, 0, 1));\n"
                "var attack_type = parseInt(new Uint8Array(arrayBuffer, 1, 1));\n"
                "var attack_content_size = parseInt(new Uint16Array(arrayBuffer, 2, 1));\n"
                "var attack_content = new Uint8Array(arrayBuffer, 4);\n"
                "console.log(\"attack_state=\" + attack_state + \"; attack_type=\" + attack_type + \"; attack_count_size=\" + attack_content_size);\n"
                "var status = \"ERROR: Cannot parse attack state.\";\n"
                "hideAllSections();\n"
                "switch(attack_state) {\n"
                    "case AttackStateEnum.READY:\n"
                        "showAttackConfig();\n"
                        "break;\n"
                    "case AttackStateEnum.RUNNING:\n"
                        "showRunning();\n"
                        "console.log(\"Poll\");\n"
                        "setTimeout(getStatus, poll_interval);\n"
                        "break;\n"
                    "case AttackStateEnum.FINISHED:\n"
                        "showResult(\"FINISHED\", attack_type, attack_content_size, attack_content);\n"
                        "break;\n"
                    "case AttackStateEnum.TIMEOUT:\n"
                        "showResult(\"TIMEOUT\", attack_type, attack_content_size, attack_content);\n"
                        "break;\n"
                    "default:\n"
                        "document.getElementById(\"errors\").innerHTML = \"Error loading attack status! Unknown state.\";\n"
                "}\n"
                "return;\n"
                "\n"
            "}\n"
        "};\n"
        "oReq.onerror = function() {\n"
            "console.log(\"Request error\");\n"
            "document.getElementById(\"errors\").innerHTML = \"Cannot reach ESP32. Check that you are connected to management AP. You might get disconnected during attack.\";\n"
            "getStatus();\n"
        "};\n"
        "oReq.ontimeout = function() {\n"
            "console.log(\"Request timeout\");\n"
            "getStatus();  \n"
        "};\n"
        "oReq.open(\"GET\", \"status\", true);\n"
        "oReq.responseType = \"arraybuffer\";\n"
        "oReq.send();\n"
    "}\n"
    "function hideAllSections(){\n"
        "for(let section of document.getElementsByTagName(\"section\")){\n"
            "section.style.display = \"none\";\n"
        "};\n"
    "}\n"
    "function showRunning(){\n"
        "hideAllSections();\n"
        "document.getElementById(\"running\").style.display = \"block\";\n"
    "}\n"
    "function countProgress(){\n"
        "if(time_elapsed >= attack_timeout){\n"
            "document.getElementById(\"errors\").innerHTML = \"Please reconnect to management AP\";\n"
            "document.getElementById(\"errors\").style.display = \"block\";\n"
            "clearInterval(running_poll);\n"
        "}\n"
        "document.getElementById(\"running-progress\").innerHTML = time_elapsed + \"/\" + attack_timeout + \"s\";\n"
        "time_elapsed++;\n"
    "}\n"
    "function showAttackConfig(){\n"
        "document.getElementById(\"ready\").style.display = \"block\";\n"
        "refreshAps();\n"
    "}\n"
    "function showResult(status, attack_type, attack_content_size, attack_content){\n"
        "hideAllSections();\n"
        "clearInterval(poll);\n"
        "document.getElementById(\"result\").innerHTML = defaultResultContent;\n"
        "document.getElementById(\"result\").style.display = \"block\";\n"
        "document.getElementById(\"result-meta\").innerHTML = status + \"<br>\";\n"
        "type = \"ERROR: Cannot parse attack type.\";\n"
        "switch(attack_type) {\n"
            "case AttackTypeEnum.ATTACK_TYPE_PASSIVE:\n"
                "type = \"ATTACK_TYPE_PASSIVE\";\n"
                "break;\n"
            "case AttackTypeEnum.ATTACK_TYPE_HANDSHAKE:\n"
                "type = \"ATTACK_TYPE_HANDSHAKE\";\n"
                "resultHandshake(attack_content, attack_content_size);\n"
                "break;\n"
            "case AttackTypeEnum.ATTACK_TYPE_PMKID:\n"
                "type = \"ATTACK_TYPE_PMKID\";\n"
                "resultPmkid(attack_content, attack_content_size);\n"
                "break;\n"
            "case AttackTypeEnum.ATTACK_TYPE_DOS:\n"
                "type = \"ATTACK_TYPE_DOS\";\n"
                "break;\n"
            "default:\n"
                "type = \"UNKNOWN\";\n"
        "}\n"
        "document.getElementById(\"result-meta\").innerHTML += type + \"<br>\";\n"
    "}\n"
    "function refreshAps() {\n"
        "document.getElementById(\"ap-list\").innerHTML = \"Loading (this may take a while)...\";\n"
        "var oReq = new XMLHttpRequest();\n"
        "oReq.onload = function() {\n"
            "document.getElementById(\"ap-list\").innerHTML = \"<th>SSID</th><th>BSSID</th><th>RSSI</th>\";\n"
            "var arrayBuffer = oReq.response;\n"
            "if(arrayBuffer) {\n"
                "var byteArray = new Uint8Array(arrayBuffer);\n"
                "for  (let i = 0; i < byteArray.byteLength; i = i + 40) {\n"
                    "var tr = document.createElement('tr');\n"
                    "tr.setAttribute(\"id\", i / 40);\n"
                    "tr.setAttribute(\"onClick\", \"selectAp(this)\");\n"
                    "var td_ssid = document.createElement('td');\n"
                    "var td_rssi = document.createElement('td');\n"
                    "var td_bssid = document.createElement('td');\n"
                    "td_ssid.innerHTML = new TextDecoder(\"utf-8\").decode(byteArray.subarray(i + 0, i + 32));\n"
                    "tr.appendChild(td_ssid);\n"
                    "for(let j = 0; j < 6; j++){\n"
                        "td_bssid.innerHTML += uint8ToHex(byteArray[i + 33 + j]) + \":\";\n"
                    "}\n"
                    "tr.appendChild(td_bssid);\n"
                    "td_rssi.innerHTML = byteArray[i + 39] - 255;\n"
                    "tr.appendChild(td_rssi);\n"
                    "document.getElementById(\"ap-list\").appendChild(tr);\n"
                "}\n"
            "}\n"
        "};\n"
        "oReq.onerror = function() {\n"
            "document.getElementById(\"ap-list\").innerHTML = \"ERROR\";\n"
        "};\n"
        "oReq.open(\"GET\", \"ap-list\", true);\n"
        "oReq.responseType = \"arraybuffer\";\n"
        "oReq.send();\n"
    "}\n"
    "function selectAp(el) {\n"
        "console.log(el.id);\n"
        "if(selectedApElement != -1){\n"
            "selectedApElement.classList.remove(\"selected\")\n"
        "}\n"
        "selectedApElement=el;\n"
        "el.classList.add(\"selected\");\n"
    "}\n"
    "function runAttack() {\n"
        "if(selectedApElement == -1){\n"
            "console.log(\"No AP selected. Attack not started.\");\n"
            "document.getElementById(\"errors\").innerHTML = \"No AP selected. Attack not started.\";\n"
            "return;\n"
        "}\n"
        "hideAllSections();\n"
        "document.getElementById(\"running\").style.display = \"block\";\n"
        "var arrayBuffer = new ArrayBuffer(4);\n"
        "var uint8Array = new Uint8Array(arrayBuffer);\n"
        "uint8Array[0] = parseInt(selectedApElement.id);\n"
        "uint8Array[1] = parseInt(document.getElementById(\"attack_type\").value);\n"
        "uint8Array[2] = parseInt(document.getElementById(\"attack_method\").value);\n"
        "uint8Array[3] = parseInt(document.getElementById(\"attack_timeout\").value);\n"
        "var oReq = new XMLHttpRequest();\n"
        "oReq.open(\"POST\", \"run-attack\", true);\n"
        "oReq.send(arrayBuffer);\n"
        "getStatus();\n"
        "attack_timeout = parseInt(document.getElementById(\"attack_timeout\").value);\n"
        "time_elapsed = 0;\n"
        "running_poll = setInterval(countProgress, running_poll_interval);\n"
    "}\n"
    "function resetAttack(){\n"
        "hideAllSections();\n"
        "showAttackConfig();\n"
        "var oReq = new XMLHttpRequest();\n"
        "oReq.open(\"HEAD\", \"reset\", true);\n"
        "oReq.send();\n"
    "}\n"
    "function resultPmkid(attack_content, attack_content_size){\n"
        "var mac_ap = \"\";\n"
        "var mac_sta = \"\";\n"
        "var ssid = \"\";\n"
        "var ssid_text = \"\";\n"
        "var pmkid = \"\";\n"
        "var index = 0;\n"
        "for(let i = 0; i < 6; i = i + 1) {\n"
            "mac_ap += uint8ToHex(attack_content[index + i]);\n"
        "}\n"
        "index = index + 6;\n"
        "for(let i = 0; i < 6; i = i + 1) {\n"
            "mac_sta += uint8ToHex(attack_content[index + i]);\n"
        "}\n"
        "index = index + 6;\n"
        "for(let i = 0; i < attack_content[index]; i = i + 1) {\n"
            "ssid += uint8ToHex(attack_content[index + 1 + i]);\n"
            "ssid_text += String.fromCharCode(attack_content[index + 1 + i]);\n"
        "}\n"
        "index = index + attack_content[index] + 1;\n"
        "var pmkid_cnt = 0;\n"
        "for(let i = 0; i < attack_content_size - index; i = i + 1) {\n"
            "if((i % 16) == 0){\n"
                "pmkid += \"<br>\";\n"
                "pmkid += \"</code>PMKID #\" + pmkid_cnt + \": <code>\";\n"
                "pmkid_cnt += 1;\n"
            "}\n"
            "pmkid += uint8ToHex(attack_content[index + i]);\n"
        "}\n"
        "document.getElementById(\"result-content\").innerHTML = \"\";\n"
        "document.getElementById(\"result-content\").innerHTML += \"MAC AP: <code>\" + mac_ap + \"</code><br>\";\n"
        "document.getElementById(\"result-content\").innerHTML += \"MAC STA: <code>\" + mac_sta + \"</code><br>\";\n"
        "document.getElementById(\"result-content\").innerHTML += \"(E)SSID: <code>\" + ssid + \"</code> (\" + ssid_text + \")\";\n"
        "document.getElementById(\"result-content\").innerHTML += \"<code>\" + pmkid + \"</code><br>\";\n"
        "document.getElementById(\"result-content\").innerHTML += \"<br>Hashcat ready format:\"\n"
        "document.getElementById(\"result-content\").innerHTML += \"<code>\" + pmkid + \"*\" + mac_ap + \"*\" + mac_sta  + \"*\" + ssid  + \"</code><br>\";\n"
    "}\n"
    "function resultHandshake(attack_content, attack_content_size){\n"
        "document.getElementById(\"result-content\").innerHTML = \"\";\n"
        "var pcap_link = document.createElement(\"a\");\n"
        "pcap_link.setAttribute(\"href\", \"capture.pcap\");\n"
        "pcap_link.text = \"Download PCAP file\";\n"
        "var hccapx_link = document.createElement(\"a\");\n"
        "hccapx_link.setAttribute(\"href\", \"capture.hccapx\");\n"
        "hccapx_link.text = \"Download HCCAPX file\";\n"
        "document.getElementById(\"result-content\").innerHTML += \"<p>\" + pcap_link.outerHTML + \"</p>\";\n"
        "document.getElementById(\"result-content\").innerHTML += \"<p>\" + hccapx_link.outerHTML + \"</p>\";\n"
        "var handshakes = \"\";\n"
        "for(let i = 0; i < attack_content_size; i = i + 1) {\n"
            "handshakes += uint8ToHex(attack_content[i]);\n"
            "if(i % 50 == 49) {\n"
                "handshakes += \"\\n\";\n"
            "}\n"
        "}\n"
        "document.getElementById(\"result-content\").innerHTML += \"<pre><code>\" + handshakes + \"</code></pre>\";\n"
    "}\n"
    "function uint8ToHex(uint8){\n"
        "return (\"00\" + uint8.toString(16)).slice(-2);\n"
    "}\n"
    "function updateConfigurableFields(el){\n"
        "document.getElementById(\"attack_method\").outerHTML = defaultAttackMethods;\n"
        "switch(parseInt(el.value)){\n"
            "case AttackTypeEnum.ATTACK_TYPE_PASSIVE:\n"
                "console.log(\"PASSIVE configuration\");\n"
                "break;\n"
            "case AttackTypeEnum.ATTACK_TYPE_HANDSHAKE:\n"
                "console.log(\"HANDSHAKE configuration\");\n"
                "document.getElementById(\"attack_timeout\").value = 60;\n"
                "setAttackMethods([\"DEAUTH_ROGUE_AP (PASSIVE)\", \"DEAUTH_BROADCAST (ACTIVE)\", \"CAPTURE_ONLY (PASSIVE)\"]);\n"
                "break;\n"
            "case AttackTypeEnum.ATTACK_TYPE_PMKID:\n"
                "console.log(\"PMKID configuration\");\n"
                "document.getElementById(\"attack_timeout\").value = 5;\n"
                "break;\n"
            "case AttackTypeEnum.ATTACK_TYPE_DOS:\n"
                "console.log(\"DOS configuration\");\n"
                "document.getElementById(\"attack_timeout\").value = 120;\n"
                "setAttackMethods([\"DEAUTH_ROGUE_AP (PASSIVE)\", \"DEAUTH_BROADCAST (ACTIVE)\"]);\n"
                "break;\n"
            "default:\n"
                "console.log(\"Unknown attack type\");\n"
                "break;\n"
        "}\n"
    "}\n"
    "function setAttackMethods(attackMethodsArray){\n"
        "document.getElementById(\"attack_method\").removeAttribute(\"disabled\");\n"
        "attackMethodsArray.forEach(function(method, index){\n"
            "let option = document.createElement(\"option\");\n"
            "option.value = index;\n"
            "option.text = method;\n"
            "option.selected = true;\n"
            "document.getElementById(\"attack_method\").appendChild(option);\n"
        "});\n"
    "}\n"
    "</script>\n"
"</body>\n"
"</html>\n"
;
#endif

