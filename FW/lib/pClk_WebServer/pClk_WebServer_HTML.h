#ifndef PCLK_WEBSERVER_HTML_H_
#define PCLK_WEBSERVER_HTML_H_

#include <Arduino.h>



const char WIFI_CONFIG_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Pixel Clock Setup</title>
    <style>
        body { font-family: sans-serif; background: #1a1a1a; color: white; text-align: center; margin: 0; padding: 0; }
        .container { display: inline-flex; flex-direction: column; width: 95%; max-width: 480px; height: 640px; background: #333; margin-top: 20px; border-radius: 15px; overflow: hidden; box-shadow: 0 10px 25px rgba(0,0,0,0.5); }
        .tab-bar { display: flex; background: #222; border-bottom: 2px solid #444; flex-shrink: 0; }
        .tab-btn { flex: 1; padding: 20px 0; cursor: pointer; border: none; background: none; color: #888; font-weight: bold; font-size: 14px; outline: none; }
        .tab-btn.active { color: #00adb5; border-bottom: 3px solid #00adb5; background: #2c2c2c; }
        .content { padding: 20px; flex: 1; min-height: 0; overflow-y: auto; background: #333; box-sizing: border-box; display: flex; flex-direction: column; }
        .tab-content { display: none; width: 100%; }
        
        /* 공통 입력 양식 */
        .input-group { text-align: left; position: relative; margin-bottom: 15px; }
        label { display: block; font-size: 13px; font-weight: bold; color: #00adb5; margin-bottom: 5px; margin-left: 2px; }
        input, select { padding: 12px; width: 100%; box-sizing: border-box; border-radius: 8px; border: 1px solid #444; background: #222; color: white; font-size: 15px; outline: none; }
        input:focus, select:focus { border-color: #00adb5; }

        /* Wi-Fi 스타일 */
        .scan-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 10px; padding: 0 5px; }
        .spinner { display: none; width: 14px; height: 14px; border: 2px solid rgba(0, 173, 181, 0.3); border-top: 2px solid #00adb5; border-radius: 50%; animation: spin 1s linear infinite; }
        @keyframes spin { 0% { transform: rotate(0deg); } 100% { transform: rotate(360deg); } }
        .wifi-list { flex: 1; min-height: 200px; max-height: 200px; background: #222; border-radius: 10px; border: 1px solid #444; overflow-y: auto; margin-bottom: 15px; padding: 5px; }
        .wifi-item { display: flex; justify-content: space-between; align-items: center; padding: 12px 15px; margin: 5px; background: #2c2c2c; border-radius: 8px; cursor: pointer; transition: 0.2s; }
        .sig-icon { display: flex; align-items: flex-end; width: 20px; height: 15px; gap: 2px; }
        .sig-bar { width: 3px; background: #555; border-radius: 1px; }
        .sig-bar.fill { background: #00adb5; }
        .b1 { height: 30%; } .b2 { height: 55%; } .b3 { height: 80%; } .b4 { height: 100%; }
        .pw-wrapper { position: relative; display: flex; align-items: center; }
        .pw-toggle { position: absolute; right: 15px; color: #00adb5; font-size: 11px; cursor: pointer; font-weight: bold; user-select: none; }

        /* 날씨 토글 및 비활성화 스타일 */
        .header-with-toggle { display: flex; justify-content: space-between; align-items: center; margin-bottom: 20px; border-bottom: 1px solid #444; padding-bottom: 10px; }
        .weather-toggle { position: relative; display: inline-block; width: 50px; height: 24px; }
        .weather-toggle input { opacity: 0; width: 0; height: 0; }
        .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #444; transition: .4s; border-radius: 24px; }
        .slider:before { position: absolute; content: ""; height: 18px; width: 18px; left: 3px; bottom: 3px; background-color: white; transition: .4s; border-radius: 50%; }
        input:checked + .slider { background-color: #00adb5; }
        input:checked + .slider:before { transform: translateX(26px); }
        .disabled-zone { opacity: 0.3; pointer-events: none; transition: 0.3s; }
        .unit-label { position: absolute; right: 12px; top: 35px; color: #888; font-size: 14px; }
        .info-text { font-size: 11px; color: #888; margin-top: 5px; line-height: 1.4; text-align: left; padding-left: 5px; }

        .footer { padding: 20px; background: #222; border-top: 1px solid #444; flex-shrink: 0; }
        .btn-save { background: #00adb5; color: white; cursor: pointer; font-weight: bold; width: 100%; padding: 15px; border: none; border-radius: 8px; font-size: 16px; }
        .btn-scan { background: #444; color: #ccc; border: none; padding: 6px 12px; border-radius: 5px; font-size: 11px; cursor: pointer; }

        /* 조명/알람 공통 */
        .time-group { display: flex; align-items: center; gap: 8px; }
        .time-group input { text-align: center; width: 70px; flex: none; }
        .time-sep { font-size: 20px; color: #666; }
        .time-format { font-size: 12px; color: #999; margin-left: 2px; }
        input[type="range"] { padding: 5px 0; cursor: pointer; accent-color: #00adb5; }
        .range-label { display: block; text-align: right; color: #00adb5; font-size: 13px; font-weight: bold; margin-top: 3px; }
        input[type="color"] { padding: 3px; height: 48px; cursor: pointer; }
        .day-picker { display: flex; gap: 5px; justify-content: space-between; }
        .day-label { cursor: pointer; }
        .day-label input[type="checkbox"] { position: absolute; opacity: 0; width: 0; height: 0; }
        .day-text { display: block; width: 36px; height: 36px; line-height: 36px; text-align: center; border-radius: 50%; background: #444; color: #888; font-size: 13px; font-weight: bold; transition: 0.2s; }
        .day-label input:checked + .day-text { background: #00adb5; color: white; }
        .calib-btn-group { display: flex; gap: 10px; margin-top: 5px; }
        .calib-btn { flex: 1; padding: 10px; background: #444; color: #ccc; border: 1px solid #555; border-radius: 8px; font-size: 14px; font-weight: bold; cursor: pointer; transition: 0.2s; }
        .calib-btn:hover:not(:disabled) { background: #00adb5; color: white; border-color: #00adb5; }
        .calib-btn:disabled { opacity: 0.3; cursor: default; }
        .calib-result { font-size: 12px; color: #888; margin-top: 6px; text-align: left; padding-left: 2px; min-height: 16px; }
    </style>
</head>
<body>
    <form action="/save" method="POST">
        <div class="container">
            <div class="tab-bar">
                <button type="button" class="tab-btn active" onclick="showTab('wifi', this)">Wi-Fi</button>
                <button type="button" class="tab-btn" onclick="showTab('weather', this)">날씨</button>
                <button type="button" class="tab-btn" onclick="showTab('light', this)">조명</button>
                <button type="button" class="tab-btn" onclick="showTab('alarm', this)">알람</button>
            </div>
            
            <div class="content">
                <div id="wifi" class="tab-content" style="display:block;">
                    <div class="scan-header">
                        <span style="font-size:14px; font-weight:bold; color:#00adb5;">주변 네트워크</span>
                        <div style="display:flex; align-items:center; gap:8px;">
                            <div id="scan-spinner" class="spinner"></div>
                            <button type="button" class="btn-scan" onclick="fetchScan()">스캔</button>
                        </div>
                    </div>
                    <div class="wifi-list" id="wifi-list">
                        <div style="padding:30px; color:#888; font-style:italic; font-size:14px;">검색 중...</div>
                    </div>
                    <div class="input-group">
                        <label>SSID</label>
                        <input type="text" id="ssid_input" name="ssid" value="{{SSID}}" required placeholder="선택 또는 직접 입력">
                    </div>
                    <div class="input-group">
                        <label>PW</label>
                        <div class="pw-wrapper">
                            <input type="password" id="pw_input" name="pw" value="{{PW}}" placeholder="비밀번호 입력">
                            <span class="pw-toggle" onclick="togglePW()">SHOW</span>
                        </div>
                    </div>
                </div>

                <div id="weather" class="tab-content">
                    <div class="header-with-toggle">
                        <span style="font-size:16px; font-weight:bold; color:#00adb5;">날씨</span>
                        <label class="weather-toggle">
                            <input type="checkbox" name="weather_en" id="weather_en" onchange="updateWeatherUI()" {{WEATHER_EN}}>
                            <span class="slider"></span>
                        </label>
                    </div>
                    
                    <div id="weather_fields" class="disabled-zone">
                        <div class="input-group" style="position:relative;">
                            <label>화면 전환 주기</label>
                            <input type="number" name="weather_cycle" value="{{WEATHER_CYCLE}}" min="1" placeholder="전환 주기 입력">
                            <span class="unit-label">sec</span>
                        </div>
                        <div class="input-group">
                            <label>지역 (시/도)</label>
                            <select id="city0" name="city0" onchange="updateCity1()">
                                <option value="">선택</option>
                            </select>
                        </div>
                        <div class="input-group">
                            <label>지역 (시/군/구)</label>
                            <select id="city1" name="city1" onchange="updateGrid()">
                                <option value="">선택</option>
                            </select>
                        </div>
                        <input type="hidden" id="grid_x" name="grid_x" value="{{GRID_X}}">
                        <input type="hidden" id="grid_y" name="grid_y" value="{{GRID_Y}}">
                        <div class="input-group">
                            <label>날씨 API KEY</label>
                            <input type="text" name="weather_key" value="{{WEATHER_KEY}}" placeholder="API Key 입력">
                            <div class="info-text">[공공데이터포털] 기상청_단기예보 ((구)_동네예보) 조회서비스</div>
                        </div>
                    </div>
                </div>

                <div id="light" class="tab-content">
                    <div class="header-with-toggle">
                        <span style="font-size:16px; font-weight:bold; color:#00adb5;">조명</span>
                    </div>

                    <div class="input-group">
                        <label>RGB 색상</label>
                        <input type="color" name="rgb_color0" id="rgb_color0" value="{{RGB_COLOR0}}">
                    </div>

                    <div class="input-group">
                        <label>주간 밝기</label>
                        <input type="range" name="rgb_lumi0_day" id="rgb_lumi0_day" min="0" max="255" value="{{RGB_LUMI0_DAY}}" oninput="updateSliderLabel('rgb_lumi0_day','lbl_lumi_day')">
                        <span class="range-label" id="lbl_lumi_day">{{RGB_LUMI0_DAY}}</span>
                    </div>

                    <div id="light_night_lumi">
                        <div class="input-group">
                            <label>야간 밝기</label>
                            <input type="range" name="rgb_lumi0_night" id="rgb_lumi0_night" min="0" max="255" value="{{RGB_LUMI0_NIGHT}}" oninput="updateSliderLabel('rgb_lumi0_night','lbl_lumi_night')">
                            <span class="range-label" id="lbl_lumi_night">{{RGB_LUMI0_NIGHT}}</span>
                        </div>
                    </div>

                    <div class="input-group">
                        <label>조도 캘리브레이션</label>
                        <div class="calib-btn-group">
                            <button type="button" class="calib-btn" onclick="doCalib('day')">주간</button>
                            <button type="button" class="calib-btn" onclick="doCalib('night')">야간</button>
                        </div>
                        <div class="calib-result" id="calib_result_day">주간: -</div>
                        <div class="calib-result" id="calib_result_night">야간: -</div>
                    </div>

                    <div class="header-with-toggle" style="margin-top:30px;">
                        <span style="font-size:16px; font-weight:bold; color:#00adb5;">절전모드</span>
                        <label class="weather-toggle">
                            <input type="checkbox" name="rgb_dsleep_en" id="rgb_dsleep_en" onchange="updateLightUI()" {{RGB_DSLEEP_EN}}>
                            <span class="slider"></span>
                        </label>
                    </div>

                    <div id="light_sleep_time" class="disabled-zone">
                        <div class="input-group">
                            <label>절전 시작</label>
                            <div class="time-group">
                                <input type="number" name="rgb_dsleep_str_hh" min="0" max="23" value="{{RGB_DSLEEP_STR_HH}}" placeholder="HH">
                                <span class="time-sep">:</span>
                                <input type="number" name="rgb_dsleep_str_mm" min="0" max="59" value="{{RGB_DSLEEP_STR_MM}}" placeholder="MM">
                                <span class="time-format">24H</span>
                            </div>
                        </div>
                        <div class="input-group">
                            <label>절전 종료</label>
                            <div class="time-group">
                                <input type="number" name="rgb_dsleep_end_hh" min="0" max="23" value="{{RGB_DSLEEP_END_HH}}" placeholder="HH">
                                <span class="time-sep">:</span>
                                <input type="number" name="rgb_dsleep_end_mm" min="0" max="59" value="{{RGB_DSLEEP_END_MM}}" placeholder="MM">
                                <span class="time-format">24H</span>
                            </div>
                        </div>
                    </div>
                </div>

                <div id="alarm" class="tab-content">
                    <div class="header-with-toggle">
                        <span style="font-size:16px; font-weight:bold; color:#00adb5;">알람</span>
                        <label class="weather-toggle">
                            <input type="checkbox" name="alarm_en" id="alarm_en" onchange="updateAlarmUI()" {{ALARM_EN}}>
                            <span class="slider"></span>
                        </label>
                    </div>

                    <div id="alarm_fields" class="disabled-zone">
                        <div class="input-group">
                            <label>알람 시간</label>
                            <div class="time-group">
                                <input type="number" name="alarm_hh" min="0" max="23" value="{{ALARM_HH}}" placeholder="HH">
                                <span class="time-sep">:</span>
                                <input type="number" name="alarm_mm" min="0" max="59" value="{{ALARM_MM}}" placeholder="MM">
                                <span class="time-format">24H</span>
                            </div>
                        </div>

                        <div class="input-group">
                            <label>요일</label>
                            <div class="day-picker">
                                <label class="day-label"><input type="checkbox" name="alarm_d_sun" {{ALARM_D_SUN}}><span class="day-text">일</span></label>
                                <label class="day-label"><input type="checkbox" name="alarm_d_mon" {{ALARM_D_MON}}><span class="day-text">월</span></label>
                                <label class="day-label"><input type="checkbox" name="alarm_d_tue" {{ALARM_D_TUE}}><span class="day-text">화</span></label>
                                <label class="day-label"><input type="checkbox" name="alarm_d_wed" {{ALARM_D_WED}}><span class="day-text">수</span></label>
                                <label class="day-label"><input type="checkbox" name="alarm_d_thu" {{ALARM_D_THU}}><span class="day-text">목</span></label>
                                <label class="day-label"><input type="checkbox" name="alarm_d_fri" {{ALARM_D_FRI}}><span class="day-text">금</span></label>
                                <label class="day-label"><input type="checkbox" name="alarm_d_sat" {{ALARM_D_SAT}}><span class="day-text">토</span></label>
                            </div>
                        </div>

                        <div class="input-group">
                            <label>사운드</label>
                            <select name="alarm_tone">
                                <option value="0" {{ALARM_TONE_SEL_0}}>Alarm 1</option>
                                <option value="1" {{ALARM_TONE_SEL_1}}>Alarm 2</option>
                                <option value="2" {{ALARM_TONE_SEL_2}}>Alarm 3</option>
                            </select>
                        </div>

                        <div class="input-group">
                            <label>볼륨</label>
                            <div style="display:flex; align-items:center; gap:8px;">
                                <input type="range" name="alarm_volume" id="alarm_volume" min="0" max="100" value="{{ALARM_VOL}}" oninput="updateSliderLabel('alarm_volume','lbl_alarm_vol')" style="flex:1; width:auto;">
                                <button type="button" class="calib-btn" style="flex:none; width:40px; padding:10px 0;"
                                        onclick="fetch('/ctrl?cmd=vol_play&val='+document.getElementById('alarm_volume').value+'&tone='+document.querySelector('select[name=alarm_tone]').value).catch(()=>{})">
                                    ▶
                                </button>
                            </div>
                            <span class="range-label" id="lbl_alarm_vol">{{ALARM_VOL}}</span>
                        </div>
                    </div>
                </div>
            </div>
            <div class="footer">
                <button type="submit" class="btn-save">Save & Exit</button>
            </div>
        </div>
    </form>

    <script>
        const regionData = {
            "서울특별시": {"종로구":[60,127],"중구":[60,127],"용산구":[60,126],"성동구":[61,127],"광진구":[62,126],"동대문구":[61,127],"중랑구":[62,128],"성북구":[61,127],"강북구":[61,128],"도봉구":[61,129],"노원구":[61,129],"은평구":[59,127],"서대문구":[59,127],"마포구":[59,127],"양천구":[58,126],"강서구":[58,126],"구로구":[58,125],"금천구":[59,124],"영등포구":[58,126],"동작구":[59,125],"관악구":[59,125],"서초구":[61,125],"강남구":[61,126],"송파구":[62,126],"강동구":[62,126]},
            "부산광역시": {"중구":[97,74],"서구":[97,74],"동구":[98,75],"영도구":[98,74],"부산진구":[97,75],"동래구":[98,76],"남구":[98,75],"북구":[96,76],"해운대구":[99,75],"사하구":[96,74],"금정구":[98,77],"강서구":[96,76],"연제구":[98,76],"수영구":[99,75],"사상구":[96,75],"기장군":[100,77]},
            "대구광역시": {"중구":[89,90],"동구":[90,91],"서구":[88,90],"남구":[89,90],"북구":[89,91],"수성구":[89,90],"달서구":[88,90],"달성군":[86,88],"군위군":[88,99]},
            "인천광역시": {"중구":[54,125],"동구":[54,125],"미추홀구":[54,124],"연수구":[55,123],"남동구":[56,124],"부평구":[55,125],"계양구":[56,126],"서구":[55,126],"강화군":[51,130],"옹진군":[54,124]},
            "광주광역시": {"동구":[60,74],"서구":[59,74],"남구":[59,73],"북구":[59,75],"광산구":[57,74]},
            "대전광역시": {"동구":[68,100],"중구":[68,100],"서구":[67,100],"유성구":[67,101],"대덕구":[68,100]},
            "울산광역시": {"중구":[102,84],"남구":[102,84],"동구":[104,83],"북구":[103,85],"울주군":[101,84]},
            "세종특별자치시": {"세종특별자치시":[66,103]},
            "경기도": {"수원시 장안구":[60,121],"수원시 권선구":[60,120],"수원시 팔달구":[61,121],"수원시 영통구":[61,120],"성남시 수정구":[63,124],"성남시 중원구":[63,124],"성남시 분당구":[62,123],"의정부시":[61,130],"안양시 만안구":[59,123],"안양시 동안구":[59,123],"부천시 원미구":[57,125],"부천시 소사구":[57,125],"부천시 오정구":[57,126],"광명시":[58,125],"평택시":[62,114],"동두천시":[61,134],"안산시 상록구":[58,121],"안산시 단원구":[57,121],"고양시 덕양구":[57,128],"고양시 일산동구":[56,129],"고양시 일산서구":[56,129],"과천시":[60,124],"구리시":[62,127],"남양주시":[64,128],"오산시":[62,118],"시흥시":[57,123],"군포시":[59,122],"의왕시":[60,122],"하남시":[64,126],"용인시 처인구":[64,119],"용인시 기흥구":[62,120],"용인시 수지구":[62,121],"파주시":[56,131],"이천시":[68,121],"안성시":[65,115],"김포시":[55,128],"화성시":[57,119],"광주시":[65,123],"양주시":[61,131],"포천시":[64,134],"여주시":[71,121],"연천군":[61,138],"가평군":[69,133],"양평군":[69,125]},
            "충청북도": {"청주시 상당구":[69,106],"청주시 서원구":[69,107],"청주시 흥덕구":[67,106],"청주시 청원구":[69,107],"충주시":[76,114],"제천시":[81,118],"보은군":[73,103],"옥천군":[71,99],"영동군":[74,97],"증평군":[71,110],"진천군":[68,111],"괴산군":[74,111],"음성군":[72,113],"단양군":[84,115]},
            "충청남도": {"천안시 동남구":[63,110],"천안시 서북구":[63,112],"공주시":[63,102],"보령시":[54,100],"아산시":[60,110],"서산시":[51,110],"논산시":[62,97],"계룡시":[65,99],"당진시":[54,112],"금산군":[69,95],"부여군":[59,99],"서천군":[55,94],"청양군":[57,103],"홍성군":[55,106],"예산군":[58,107],"태안군":[48,109]},
            "전라남도": {"목포시":[50,67],"여수시":[73,66],"순천시":[70,70],"나주시":[56,71],"광양시":[73,70],"담양군":[61,78],"곡성군":[66,77],"구례군":[69,75],"고흥군":[66,62],"보성군":[62,66],"화순군":[61,72],"장흥군":[59,64],"강진군":[57,63],"해남군":[54,61],"영암군":[56,66],"무안군":[52,71],"함평군":[52,72],"영광군":[52,77],"장성군":[57,77],"완도군":[57,56],"진도군":[48,59],"신안군":[50,66]},
            "경상북도": {"포항시 남구":[102,94],"포항시 북구":[102,95],"경주시":[100,91],"김천시":[80,96],"안동시":[91,106],"구미시":[84,96],"영주시":[89,111],"영천시":[95,93],"상주시":[81,102],"문경시":[81,106],"경산시":[91,90],"의성군":[90,101],"청송군":[96,103],"영양군":[97,108],"영덕군":[102,103],"청도군":[91,86],"고령군":[83,87],"성주군":[83,91],"칠곡군":[85,93],"예천군":[86,107],"봉화군":[90,113],"울진군":[102,115],"울릉군":[127,127]},
            "경상남도": {"창원시 의창구":[90,77],"창원시 성산구":[91,76],"창원시 마산합포구":[89,76],"창원시 마산회원구":[89,76],"창원시 진해구":[91,75],"진주시":[81,75],"통영시":[87,68],"사천시":[80,71],"김해시":[95,77],"밀양시":[92,83],"거제시":[90,69],"양산시":[97,79],"의령군":[83,78],"함안군":[86,77],"창녕군":[87,83],"고성군":[85,71],"남해군":[77,68],"하동군":[74,73],"산청군":[76,80],"함양군":[74,82],"거창군":[77,86],"합천군":[81,84]},
            "제주특별자치도": {"제주시":[53,38],"서귀포시":[53,33]},
            "강원특별자치도": {"춘천시":[73,134],"원주시":[76,122],"강릉시":[92,131],"동해시":[97,127],"태백시":[95,119],"속초시":[87,141],"삼척시":[98,125],"홍천군":[75,130],"횡성군":[77,125],"영월군":[86,119],"평창군":[84,123],"정선군":[89,123],"철원군":[65,139],"화천군":[72,139],"양구군":[77,139],"인제군":[80,138],"고성군":[85,145],"양양군":[88,138]},
            "전북특별자치도": {"전주시 완산구":[63,89],"전주시 덕진구":[63,89],"군산시":[56,92],"익산시":[60,91],"정읍시":[58,83],"남원시":[68,80],"김제시":[59,88],"완주군":[63,89],"진안군":[68,88],"무주군":[72,93],"장수군":[70,85],"임실군":[66,84],"순창군":[63,79],"고창군":[56,80],"부안군":[56,87]}
        };
        function showTab(id, btn) {
            document.querySelectorAll('.tab-content').forEach(c => c.style.display = 'none');
            document.querySelectorAll('.tab-btn').forEach(t => t.classList.remove('active'));
            document.getElementById(id).style.display = 'block';
            btn.classList.add('active');
        }
        function togglePW() {
            const input = document.getElementById('pw_input');
            const btn = document.querySelector('.pw-toggle');
            input.type = input.type === "password" ? "text" : "password";
            btn.innerText = input.type === "password" ? "SHOW" : "HIDE";
        }
        function updateWeatherUI() {
            const isEn = document.getElementById('weather_en').checked;
            document.getElementById('weather_fields').className = isEn ? "" : "disabled-zone";
            document.querySelectorAll('#weather_fields input, #weather_fields select').forEach(el => {
                el.disabled = !isEn;
            });
        }
        function initWeather() {
            const city0Select = document.getElementById('city0');
            for (let city in regionData) {
                let opt = document.createElement('option');
                opt.value = city;
                opt.innerText = city;
                city0Select.appendChild(opt);
            }
            updateWeatherUI();
        }
        function updateCity1() {
            const city0 = document.getElementById('city0').value;
            const city1Select = document.getElementById('city1');
            city1Select.innerHTML = '<option value="">선택</option>';
            if (city0 && regionData[city0]) {
                for (let sub in regionData[city0]) {
                    let opt = document.createElement('option');
                    opt.value = sub;
                    opt.innerText = sub;
                    city1Select.appendChild(opt);
                }
            }
        }
        function updateGrid() {
            const c0 = document.getElementById('city0').value;
            const c1 = document.getElementById('city1').value;
            if (c0 && c1 && regionData[c0][c1]) {
                const coords = regionData[c0][c1];
                document.getElementById('grid_x').value = coords[0];
                document.getElementById('grid_y').value = coords[1];
            }
        }
        function updateLightUI() {
            const isEn = document.getElementById('rgb_dsleep_en').checked;
            const zone = document.getElementById('light_sleep_time');
            zone.className = isEn ? '' : 'disabled-zone';
            zone.querySelectorAll('input').forEach(el => el.disabled = !isEn);
        }
        function updateAlarmUI() {
            const isEn = document.getElementById('alarm_en').checked;
            const zone = document.getElementById('alarm_fields');
            zone.className = isEn ? '' : 'disabled-zone';
            zone.querySelectorAll('input, select').forEach(el => el.disabled = !isEn);
        }
        function doCalib(type) {
            const el = document.getElementById('calib_result_' + type);
            el.innerText = (type === 'day' ? '주간' : '야간') + ': 측정 중...';
            el.style.color = '#888';
            fetch('/calib?type=' + type)
                .then(() => {
                    el.innerText = (type === 'day' ? '주간' : '야간') + ': 완료';
                    el.style.color = '#00adb5';
                })
                .catch(() => {
                    el.innerText = (type === 'day' ? '주간' : '야간') + ': 실패';
                    el.style.color = '#e05555';
                });
        }
        function updateSliderLabel(sliderId, labelId) {
            document.getElementById(labelId).innerText = document.getElementById(sliderId).value;
        }
        function fetchScan() {
            const list = document.getElementById('wifi-list');
            const spinner = document.getElementById('scan-spinner');
            spinner.style.display = 'block';
            list.innerHTML = '<div style="padding:30px; color:#888;">검색 중...</div>';
            fetch('/scan').then(r => r.json()).then(data => {
                spinner.style.display = 'none';
                list.innerHTML = '';
                data.list.forEach(item => {
                    let f = ["fill", "", "", ""];
                    if (item.r >= -80) f[1] = "fill";
                    if (item.r >= -70) f[2] = "fill";
                    if (item.r >= -55) f[3] = "fill";
                    let div = document.createElement('div');
                    div.className = 'wifi-item';
                    div.onclick = () => document.getElementById('ssid_input').value = item.s;
                    div.innerHTML = `<span>${item.s}</span>
                        <div class="sig-icon">
                            <div class="sig-bar b1 ${f[0]}"></div>
                            <div class="sig-bar b2 ${f[1]}"></div>
                            <div class="sig-bar b3 ${f[2]}"></div>
                            <div class="sig-bar b4 ${f[3]}"></div>
                        </div>`;
                    list.appendChild(div);
                });
            }).catch(() => { spinner.style.display = 'none'; });
        }
        window.onload = () => { 
            initWeather();
            if (window.savedCity && window.savedCity.c0) {
                const c0Sel = document.getElementById('city0');
                const c1Sel = document.getElementById('city1');
                
                c0Sel.value = window.savedCity.c0;
                updateCity1(); // 시/군/구 목록 생성
                
                if (window.savedCity.c1) {
                    c1Sel.value = window.savedCity.c1;
                    updateGrid(); // 격자 좌표 업데이트
                }
            }
            
            setTimeout(fetchScan, 500);
            updateLightUI();
            updateAlarmUI();

            // ── 실시간 제어 이벤트 등록 ──────────────────
            function debounce(fn, ms) {
                let t;
                return (...args) => { clearTimeout(t); t = setTimeout(() => fn(...args), ms); };
            }
            const sendCtrl = debounce((cmd, val) => {
                fetch('/ctrl?cmd=' + cmd + '&val=' + val).catch(() => {});
            }, 150);

            document.getElementById('rgb_color0').addEventListener('input', function() {
                sendCtrl('color', this.value.replace('#', ''));
            });
            document.getElementById('rgb_lumi0_day').addEventListener('input', function() {
                sendCtrl('lumi_day', this.value);
            });
            document.getElementById('rgb_lumi0_night').addEventListener('input', function() {
                sendCtrl('lumi_night', this.value);
            });
        };
        </script>
</body>
</html>
)rawliteral";



#endif