#pragma once

static const char OTA_PAGE[] PROGMEM = R"html(

<!doctype html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <meta name="color-scheme" content="light dark">
    <title>ESP OTA Portal</title>

    <script>
        window.__espotaThemeQuery = matchMedia('(prefers-color-scheme: dark)');
        window.__espotaThemePref = localStorage.getItem('espota-theme') || (window.__espotaThemeQuery.matches ? 'dark' : 'light');
        document.documentElement.dataset.theme = window.__espotaThemePref;
    </script>

    <style>
        :root {
            color-scheme: light dark;
            --bg: #f5f5f7;
            --card: #fff;
            --ink: #1d1d1f;
            --muted: #6e6e73;
            --line: #d2d2d7;
            --blue: #0071e3;
            --blue-hover: #0077ed;
            --tabs: #e9e9ed;
            --tab: #fff;
            --field: #fff;
            --track: #e8e8ed;
            --card-shadow: 0 3px 12px #0000000a;
            --tab-shadow: 0 1px 3px #0002;
            --good: #1d8a4e;
        }

        html,
        body {
            background-color: var(--bg);
        }

        html[data-theme=dark] {
            color-scheme: dark;
            --bg: #000;
            --card: #1c1c1e;
            --ink: #f5f5f7;
            --muted: #98989d;
            --line: #38383a;
            --blue: #0a84ff;
            --blue-hover: #409cff;
            --tabs: #1c1c1e;
            --tab: #48484a;
            --field: #2c2c2e;
            --track: #2c2c2e;
            --card-shadow: none;
            --tab-shadow: none;
            --good: #30d158;
        }

        * {
            box-sizing: border-box;
        }

        body {
            margin: 0;
            background: var(--bg);
            font: 15px -apple-system, BlinkMacSystemFont, "SF Pro Text", "Segoe UI", sans-serif;
            color: var(--ink);
            transition: background .2s, color .2s;
        }

        main {
            width: min(620px, calc(100% - 32px));
            margin: 40px auto 64px;
        }

        .topbar {
            display: flex;
            justify-content: flex-end;
            margin-bottom: 12px;
        }

        .theme {
            border: 1px solid var(--line);
            border-radius: 999px;
            padding: 7px 13px;
            background: var(--card);
            color: var(--muted);
            font: inherit;
            font-size: 13px;
            font-weight: 600;
            cursor: pointer;
        }

        .theme:hover {
            color: var(--ink);
        }

        .hero {
            text-align: center;
            margin: 12px 0 28px;
        }

        .hero h1 {
            margin: 0;
            font-size: 32px;
            letter-spacing: -.04em;
        }

        .hero p {
            margin: 8px 0 0;
            color: var(--muted);
        }

        .identity {
            display: flex;
            justify-content: center;
            gap: 16px;
            margin-top: 13px;
            color: var(--muted);
            font-size: 13px;
        }

        .tabs {
            display: flex;
            padding: 4px;
            background: var(--tabs);
            border-radius: 12px;
            gap: 3px;
        }

        .tabs.hide {
            display: none;
        }

        .tab {
            flex: 1;
            border: 0;
            border-radius: 9px;
            padding: 9px;
            background: transparent;
            color: var(--muted);
            font: inherit;
            font-weight: 600;
            cursor: pointer;
        }

        .tab.active {
            background: var(--tab);
            color: var(--ink);
            box-shadow: var(--tab-shadow);
        }

        .panel {
            display: none;
            margin-top: 18px;
            padding: 24px;
            border: 1px solid var(--line);
            border-radius: 18px;
            background: var(--card);
            box-shadow: var(--card-shadow);
        }

        .panel.active {
            display: block;
        }

        .panel h2 {
            margin: 0;
            font-size: 21px;
            letter-spacing: -.02em;
        }

        .panel p {
            line-height: 1.45;
            color: var(--muted);
        }

        .usage {
            margin: 20px 0;
            padding: 14px 0;
            border-block: 1px solid var(--line);
        }

        .usage div {
            display: flex;
            justify-content: space-between;
            gap: 12px;
        }

        .usage b {
            font-weight: 600;
        }

        .usage small {
            display: block;
            margin-top: 5px;
            color: var(--muted);
        }

        button.upload {
            width: 100%;
            margin-top: 20px;
            padding: 12px;
            border: 0;
            border-radius: 10px;
            background: var(--blue);
            color: #fff;
            font: inherit;
            font-weight: 600;
            cursor: pointer;
        }

        button.upload:hover {
            background: var(--blue-hover);
        }

        button.upload:disabled {
            opacity: .55;
            cursor: default;
        }

        .file-drop {
            display: block;
            width: 100%;
            padding: 14px;
            border: 1px dashed var(--line);
            border-radius: 14px;
            background: var(--field);
            color: var(--muted);
            cursor: pointer;
            transition: border-color .15s, background .15s, color .15s, box-shadow .15s;
        }

        .file-drop:hover,
        .file-drop.dragover {
            border-color: var(--blue);
            color: var(--ink);
            box-shadow: 0 0 0 3px color-mix(in srgb, var(--blue) 15%, transparent);
        }

        .file-drop.has-file {
            border-style: solid;
            color: var(--ink);
        }

        .file-drop input[type=file] {
            position: absolute;
            width: 1px;
            height: 1px;
            padding: 0;
            margin: -1px;
            overflow: hidden;
            clip: rect(0, 0, 0, 0);
            white-space: nowrap;
            border: 0;
        }

        .file-prompt {
            display: flex;
            align-items: center;
            justify-content: center;
            min-height: 62px;
            text-align: center;
            font-weight: 600;
            font-size: 14px;
        }

        .file-selected {
            display: none;
            gap: 10px;
            align-items: center;
            justify-content: space-between;
        }

        .file-drop.has-file .file-prompt {
            display: none;
        }

        .file-drop.has-file .file-selected {
            display: flex;
        }

        .file-meta {
            min-width: 0;
        }

        .file-name {
            display: block;
            overflow: hidden;
            text-overflow: ellipsis;
            white-space: nowrap;
            color: var(--ink);
            font-weight: 600;
        }

        .file-size {
            display: block;
            margin-top: 4px;
            color: var(--muted);
            font-size: 13px;
        }

        .file-actions {
            display: flex;
            gap: 8px;
            flex-shrink: 0;
        }

        .file-action {
            border: 0;
            border-radius: 10px;
            padding: 9px 12px;
            background: var(--tabs);
            color: var(--ink);
            font: inherit;
            font-size: 13px;
            font-weight: 600;
            cursor: pointer;
        }

        .file-action:hover {
            background: var(--line);
        }

        .progress {
            display: none;
            margin-top: 18px;
        }

        .progress.show {
            display: block;
        }

        .progress-label {
            display: flex;
            justify-content: space-between;
            color: var(--muted);
            font-size: 13px;
        }

        .track {
            height: 7px;
            margin-top: 8px;
            overflow: hidden;
            border-radius: 8px;
            background: var(--track);
        }

        .bar {
            width: 0;
            height: 100%;
            border-radius: 8px;
            background: var(--blue);
            transition: width .1s;
        }

        #done {
            text-align: center;
        }

        #done .check {
            width: 54px;
            height: 54px;
            margin: 4px auto 18px;
            border-radius: 50%;
            background: var(--good);
            color: #fff;
            font-size: 28px;
            line-height: 54px;
        }

        #done h2 {
            margin-bottom: 8px;
        }

        #message {
            min-height: 21px;
            margin: 18px 0 0;
            text-align: center;
            color: var(--muted);
        }

        @media(max-width:480px) {
            main {
                margin: 24px auto 40px;
            }

            .identity {
                flex-direction: column;
                gap: 3px;
            }

            .panel {
                padding: 18px;
            }
        }
    </style>
</head>
<body>
    <main>
    <div class="topbar">
        <button
            class="theme"
            id="theme"
            type="button"
        >
            Light
        </button>
    </div>
    <header class="hero">
            <h1 id="name">
                ESP OTA Portal
            </h1>

            <p>
                Software updates, simply delivered.
            </p>
            <div class="identity">
                <span id="id"></span>
                <span id="mac"></span>
            </div>
        </header>
        <nav class="tabs" id="tabs">
            <button
                class="tab active"
                data-tab="firmware"
            >
                Firmware
            </button>
            <button
                class="tab"
                data-tab="filesystem"
            >
                SPIFFS/LittleFS
            </button>
        </nav>
        <section
            class="panel active"
            id="firmware"
        >
            <h2>
                Firmware update
            </h2>
            <p>
                Install a new application image to the inactive OTA slot.
            </p>
            <div class="usage">
                <div>
                    <span>
                        Current firmware
                    </span>

                    <b id="firmware-space"></b>
                </div>
                <small id="firmware-file">
                    Choose a .bin file to check its size.
                </small>
            </div>
            <form data-kind="firmware">
                <label
                    class="file-drop"
                    data-dropzone
                >
                    <input
                        type="file"
                        accept=".bin,application/octet-stream"
                        required
                    >
                    <div class="file-prompt">
                        Click to choose a .bin file or drag and drop one here
                    </div>
                    <div class="file-selected">
                        <div class="file-meta">
                            <span class="file-name" data-file-name>
                                No file selected
                            </span>
                            <span class="file-size" data-file-size>
                                Choose a .bin file to check its size.
                            </span>
                        </div>
                        <div class="file-actions">
                            <button
                                class="file-action"
                                data-clear-file
                                type="button"
                            >
                                Clear
                            </button>
                        </div>
                    </div>
                </label>
                <button
                    class="upload"
                >
                    Update firmware
                </button>
                <div class="progress">
                    <div class="progress-label">
                        <span>
                            Uploading
                        </span>
                        <span class="percent">
                            0%
                        </span>
                    </div>
                    <div class="track">
                        <div class="bar"></div>
                    </div>
                </div>
            </form>
        </section>
        <section
            class="panel"
            id="filesystem"
        >
            <h2>
                SPIFFS/LittleFS update
            </h2>
            <p>
                Replace the filesystem image used by this device.
            </p>
            <div class="usage">
                <div>
                    <span>
                        Filesystem in use
                    </span>
                    <b id="filesystem-space"></b>
                </div>
                <small id="filesystem-file">
                    Choose a .bin file to check its size.
                </small>
            </div>
            <form data-kind="filesystem">
                <label
                    class="file-drop"
                    data-dropzone
                >
                    <input
                        type="file"
                        accept=".bin,application/octet-stream"
                        required
                    >
                    <div class="file-prompt">
                        Click to choose a .bin file or drag and drop one here
                    </div>
                    <div class="file-selected">
                        <div class="file-meta">
                            <span class="file-name" data-file-name>
                                No file selected
                            </span>
                            <span class="file-size" data-file-size>
                                Choose a .bin file to check its size.
                            </span>
                        </div>
                        <div class="file-actions">
                            <button
                                class="file-action"
                                data-clear-file
                                type="button"
                            >
                                Clear
                            </button>
                        </div>
                    </div>
                </label>
                <button
                    class="upload"
                >
                    Update SPIFFS/LittleFS
                </button>
                <div class="progress">
                    <div class="progress-label">
                        <span>
                            Uploading
                        </span>
                        <span class="percent">
                            0%
                        </span>
                    </div>
                    <div class="track">
                        <div class="bar"></div>
                    </div>
                </div>
            </form>
        </section>
        <section
            class="panel"
            id="done"
        >
            <div class="check">
                &#10003;
            </div>
            <h2 id="done-title">
                Update complete
            </h2>
            <p id="done-text"></p>
            <button
                class="upload"
                id="done-back"
                type="button"
            >
                Back to the portal
            </button>
        </section>
        <p id="message"></p>
    </main>
    <script>
        let state;
        const $ = s => document.querySelector(s);
        const $$ = s => document.querySelectorAll(s);
        const msg = t => $('#message').textContent = t;
        const bytes = n =>
            n < 1048576
                ? (n / 1024).toFixed(1) + ' KiB'
                : (n / 1048576).toFixed(2) + ' MiB';
        const api = n =>
            location.pathname.replace(/\/$/, '') + '/api/' + n;
        const themeQuery = window.__espotaThemeQuery || matchMedia('(prefers-color-scheme: dark)');
        function applyTheme(mode, persist) {
            document.documentElement.dataset.theme = mode;
            window.__espotaThemePref = mode;
            $('#theme').textContent = (mode === 'dark' ? 'Dark' : 'Light');
            if (persist) {
                localStorage.setItem('espota-theme', mode);
            }
        }
        applyTheme(localStorage.getItem('espota-theme') || (themeQuery.matches ? 'dark' : 'light'));
        themeQuery.addEventListener('change', e => {
            localStorage.removeItem('espota-theme');
            applyTheme(e.matches ? 'dark' : 'light', false);
        });
        $('#theme').onclick = () => {
            const current = document.documentElement.dataset.theme === 'dark' ? 'dark' : 'light';
            const next = current === 'dark' ? 'light' : 'dark';
            applyTheme(next, true);
        };
        function render(d) {
            state = d;
            $('#name').textContent = d.name;
            $('#id').textContent = 'Device ID: ' + d.id;
            $('#mac').textContent = d.mac;
            for (let k of ['firmware', 'filesystem']) {
                $('#' + k + '-space').textContent =
                    bytes(d[k].used) +
                    ' of ' +
                    bytes(d[k].capacity);
            }
        }
        async function info() {
            render(
                await (
                    await fetch(api('info'))
                ).json()
            );
        }
        $$('.tab').forEach(t =>
            t.onclick = () => {
                $$('.tab,.panel').forEach(x =>
                    x.classList.remove('active')
                );
                t.classList.add('active');
                $('#' + t.dataset.tab)
                    .classList.add('active');
            }
        );
        $$('form[data-kind]').forEach(form => {
            let input = form.querySelector('input[type=file]');
            let dropzone = form.querySelector('[data-dropzone]');
            let name = form.querySelector('[data-file-name]');
            let size = form.querySelector('[data-file-size]');
            let clear = form.querySelector('[data-clear-file]');
            let kind = form.dataset.kind;
            let capacity = () => state && state[kind].capacity;
            let setSelected = file => {
                if (!file) {
                    input.value = '';
                    dropzone.classList.remove('has-file');
                    name.textContent = 'No file selected';
                    size.textContent = 'Choose a .bin file to check its size.';
                    return;
                }
                dropzone.classList.add('has-file');
                name.textContent = file.name;
                size.textContent =
                    bytes(file.size) +
                    ' · ' +
                    (
                        capacity() && file.size <= capacity()
                            ? 'Fits this partition'
                            : capacity()
                                ? 'Too large by ' +
                                  bytes(file.size - capacity())
                                : 'Ready to upload'
                    );
            };
            input.onchange = () => setSelected(input.files[0]);
            clear.onclick = e => {
                e.preventDefault();
                e.stopPropagation();
                setSelected(null);
            };
            dropzone.ondragover = e => {
                e.preventDefault();
                dropzone.classList.add('dragover');
            };
            dropzone.ondragleave = () => dropzone.classList.remove('dragover');
            dropzone.ondrop = e => {
                e.preventDefault();
                dropzone.classList.remove('dragover');
                if (e.dataTransfer.files && e.dataTransfer.files[0]) {
                    input.files = e.dataTransfer.files;
                    setSelected(input.files[0]);
                }
            };
            dropzone.onclick = e => {
                if (e.target === clear) {
                    return;
                }
            };
        });
        function finish(kind, restarting) {
            msg('');
            $('#tabs').classList.add('hide');
            $$('.panel').forEach(p =>
                p.classList.remove('active')
            );
            $('#done').classList.add('active');
            $('#done-title').textContent =
                (
                    kind == 'firmware'
                        ? 'Firmware'
                        : 'SPIFFS/LittleFS'
                ) +
                ' update complete';
            let back = $('#done-back');
            if (!restarting) {
                $('#done-text').textContent =
                    'The new image has been written. Restart the device to start using it.';

                return;
            }
            $('#done-text').textContent =
                'The new image has been written and the device is restarting. Give it a few seconds before returning.';
            let left = 8;
            back.disabled = true;
            (function tick() {
                if (!left) {
                    back.disabled = false;
                    back.textContent = 'Back to the portal';

                    return;
                }
                back.textContent =
                    'Restarting… ' +
                    left +
                    's';
                left--;
                setTimeout(tick, 1000);
            })();
        }
        $('#done-back').onclick = () =>
            location.href = location.pathname;
        $$('form').forEach(form =>
            form.onsubmit = e => {
                e.preventDefault();
                let file =
                    form.querySelector('input')
                        .files[0];
                let kind =
                    form.dataset.kind;
                let button =
                    form.querySelector('button');
                let progress =
                    form.querySelector('.progress');
                let bar =
                    form.querySelector('.bar');
                let percent =
                    form.querySelector('.percent');
                if (!file) {
                    return;
                }
                if (
                    state &&
                    file.size > state[kind].capacity
                ) {
                    msg(
                        'This file will not fit in the target partition.'
                    );
                    return;
                }
                let x =
                    new XMLHttpRequest();
                x.open(
                    'POST',
                    api(kind)
                );
                button.disabled = true;
                progress.classList.add('show');
                x.upload.onprogress = e => {
                    if (!e.lengthComputable) {
                        return;
                    }
                    let p =
                        Math.round(
                            e.loaded /
                            e.total *
                            100
                        );
                    bar.style.width =
                        p + '%';
                    percent.textContent =
                        p + '%';
                };
                x.onload = () => {
                    let r;
                    try {
                        r =
                            JSON.parse(
                                x.responseText
                            );
                    } catch (_) {}
                    if (r && r.ok) {
                        finish(
                            kind,
                            r.restarting !== false
                        );
                        return;
                    }
                    msg(
                        r && r.error
                            ? r.error
                            : 'Upload failed.'
                    );
                    button.disabled = false;
                    info();
                };
                x.onerror = () => {
                    msg(
                        'Network error during upload.'
                    );

                    button.disabled = false;
                };
                let data =
                    new FormData();
                data.append(
                    kind,
                    file
                );
                msg(
                    'Uploading ' +
                    file.name +
                    '…'
                );
                x.send(data);
            }
        );
        info()
            .catch(() =>
                msg(
                    'Could not load device information.'
                )
            );
    </script>
</body>
</html>
)html";
