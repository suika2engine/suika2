var config = {};

let typingTimer;
let typeInterval = 500;
let searchInput;

function getConfigValue(key, defaultValue) {
    if(typeof config[key] === "undefined") {
        return defaultValue;
    }
    return config[key];
}

function intToBool(val) {
    if(val === "0") {
        return false;
    }
    return true;
}

function boolToInt(val) {
    if(val) {
        return "1";
    }
    return "0";
}

function liveSearch() {
    var cards = document.querySelectorAll(".card");
    var searchQuery = document.getElementById("searchbox").value;
    for(let card of cards) {
        if(searchQuery === "") {
            card.classList.remove("is-hidden");
        } else if(card.innerText.toLowerCase().includes(searchQuery.toLowerCase())) {
            card.classList.remove("is-hidden");
        } else {
            card.classList.add("is-hidden");
        }
    }

    var h3s = document.querySelectorAll("h3");
    for(let elem of h3s) {
        if(searchQuery === "") {
            elem.classList.remove("highlight");
        } else if(elem.textContent.toLowerCase().includes(searchQuery.toLowerCase())) {
            elem.classList.add("highlight");
        } else {
            elem.classList.remove("highlight");
        }
    }
}

window.addEventListener("load", async() => {
    // Load from config.txt.
    config = await window.api.loadConfig();

    // Set properties.
    document.getElementById("window-title").value = getConfigValue("window.title", "Suika");
    document.getElementById("window-width").value = getConfigValue("window.width", "1280");
    document.getElementById("window-height").value = getConfigValue("window.height", "720");
    document.getElementById("window-white").checked = intToBool(getConfigValue("window.white", "1"));
    document.getElementById("window-menubar").checked = intToBool(getConfigValue("window.menubar", "0"));
    document.getElementById("font-file").value = getConfigValue("font.file", "yasashisa.ttf");
    document.getElementById("font-size").value = getConfigValue("font.size", "38");
    document.getElementById("font-color-r").value = getConfigValue("font.color.r", "255");
    document.getElementById("font-color-g").value = getConfigValue("font.color.g", "255");
    document.getElementById("font-color-b").value = getConfigValue("font.color.b", "255");
    document.getElementById("font-outline-color-r").value = getConfigValue("font.outline.color.r", "128");
    document.getElementById("font-outline-color-g").value = getConfigValue("font.outline.color.g", "128");
    document.getElementById("font-outline-color-b").value = getConfigValue("font.outline.color.b", "128");
    document.getElementById("font-outline-remove").checked = intToBool(getConfigValue("font.outline.remove", "0"));
    document.getElementById("namebox-file").value = getConfigValue("namebox.file", "namebox.png");
    document.getElementById("namebox-x").value = getConfigValue("namebox.x", "95");
    document.getElementById("namebox-y").value = getConfigValue("namebox.y", "480");
    document.getElementById("namebox-margin-top").value = getConfigValue("namebox.margin.top", "6");
    document.getElementById("namebox-centering-no").checked = intToBool(getConfigValue("namebox.centering.no", "0"));
    document.getElementById("namebox-margin-left").value = getConfigValue("namebox.margin.left", "6");
    document.getElementById("namebox-hidden").checked = intToBool(getConfigValue("namebox.hidden", "0"));
    document.getElementById("msgbox-bg-file").value = getConfigValue("msgbox.bg.file", "msbox-bg.png");
    document.getElementById("msgbox-fg-file").value = getConfigValue("msgbox.fg.file", "msbox-fg.png");
    document.getElementById("msgbox-x").value = getConfigValue("msgbox.x", "43");
    document.getElementById("msgbox-y").value = getConfigValue("msgbox.y", "503");
    document.getElementById("msgbox-margin-left").value = getConfigValue("msgbox.margin.left", "80");
    document.getElementById("msgbox-margin-top").value = getConfigValue("msgbox.margin.top", "50");
    document.getElementById("msgbox-margin-right").value = getConfigValue("msgbox.margin.right", "80");
    document.getElementById("msgbox-margin-line").value = getConfigValue("msgbox.margin.line", "55");
    document.getElementById("msgbox-speed").value = getConfigValue("msgbox.speed", "22.0");
    document.getElementById("msgbox-btn-hide-x").value = getConfigValue("msgbox.btn.hide.x", "1146");
    document.getElementById("msgbox-btn-hide-y").value = getConfigValue("msgbox.btn.hide.y", "16");
    document.getElementById("msgbox-btn-hide-width").value = getConfigValue("msgbox.btn.hide.width", "29");
    document.getElementById("msgbox-btn-hide-height").value = getConfigValue("msgbox.btn.hide.height", "29");
    document.getElementById("msgbox-btn-qsave-x").value = getConfigValue("msgbox.btn.qsave.x", "0");
    document.getElementById("msgbox-btn-qsave-y").value = getConfigValue("msgbox.btn.qsave.y", "0");
    document.getElementById("msgbox-btn-qsave-width").value = getConfigValue("msgbox.btn.qsave.width", "0");
    document.getElementById("msgbox-btn-qsave-height").value = getConfigValue("msgbox.btn.qsave.height", "0");
    document.getElementById("msgbox-btn-qload-x").value = getConfigValue("msgbox.btn.qload.x", "0");
    document.getElementById("msgbox-btn-qload-y").value = getConfigValue("msgbox.btn.qload.y", "0");
    document.getElementById("msgbox-btn-qload-width").value = getConfigValue("msgbox.btn.qload.width", "0");
    document.getElementById("msgbox-btn-qload-height").value = getConfigValue("msgbox.btn.qload.height", "0");
    document.getElementById("msgbox-btn-save-x").value = getConfigValue("msgbox.btn.save.x", "0");
    document.getElementById("msgbox-btn-save-y").value = getConfigValue("msgbox.btn.save.y", "0");
    document.getElementById("msgbox-btn-save-width").value = getConfigValue("msgbox.btn.save.width", "0");
    document.getElementById("msgbox-btn-save-height").value = getConfigValue("msgbox.btn.save.height", "0");
    document.getElementById("msgbox-btn-load-x").value = getConfigValue("msgbox.btn.load.x", "0");
    document.getElementById("msgbox-btn-load-y").value = getConfigValue("msgbox.btn.load.y", "0");
    document.getElementById("msgbox-btn-load-width").value = getConfigValue("msgbox.btn.load.width", "0");
    document.getElementById("msgbox-btn-load-height").value = getConfigValue("msgbox.btn.load.height", "0");
    document.getElementById("msgbox-btn-skip-x").value = getConfigValue("msgbox.btn.skip.x", "0");
    document.getElementById("msgbox-btn-skip-y").value = getConfigValue("msgbox.btn.skip.y", "0");
    document.getElementById("msgbox-btn-skip-width").value = getConfigValue("msgbox.btn.skip.width", "0");
    document.getElementById("msgbox-btn-skip-height").value = getConfigValue("msgbox.btn.skip.height", "0");
    document.getElementById("msgbox-btn-history-x").value = getConfigValue("msgbox.btn.history.x", "0");
    document.getElementById("msgbox-btn-history-y").value = getConfigValue("msgbox.btn.history.y", "0");
    document.getElementById("msgbox-btn-history-width").value = getConfigValue("msgbox.btn.history.width", "0");
    document.getElementById("msgbox-btn-history-height").value = getConfigValue("msgbox.btn.history.height", "0");
    document.getElementById("msgbox-btn-config-x").value = getConfigValue("msgbox.btn.config.x", "0");
    document.getElementById("msgbox-btn-config-y").value = getConfigValue("msgbox.btn.config.y", "0");
    document.getElementById("msgbox-btn-config-width").value = getConfigValue("msgbox.btn.config.width", "0");
    document.getElementById("msgbox-btn-config-height").value = getConfigValue("msgbox.btn.config.height", "0");
    document.getElementById("msgbox-btn-change-se").value = getConfigValue("msgbox.btn.change.se", "btn-change.ogg");
    document.getElementById("msgbox-history-se").value = getConfigValue("msgbox.history.se", "click.ogg");
    document.getElementById("msgbox-config-se").value = getConfigValue("msgbox.config.se", "click.ogg");
    document.getElementById("msgbox-hide-se").value = getConfigValue("msgbox.hide.se", "click.ogg");
    document.getElementById("msgbox-show-se").value = getConfigValue("msgbox.show.se", "click.ogg");
    document.getElementById("msgbox-auto-cancel-se").value = getConfigValue("msgbox.auto.cancel.se", "click.ogg");
    document.getElementById("msgbox-skip-cancel-se").value = getConfigValue("msgbox.skip.cancel.se", "click.ogg");
    document.getElementById("msgbox-skip-unseen").checked = intToBool(getConfigValue("msgbox.skip.unseen", "0"));
    document.getElementById("msgbox-dim").checked = intToBool(getConfigValue("msgbox.dim", "0"));
    document.getElementById("msgbox-dim-color-r").value = getConfigValue("msgbox.dim.color.r", "80");
    document.getElementById("msgbox-dim-color-g").value = getConfigValue("msgbox.dim.color.g", "80");
    document.getElementById("msgbox-dim-color-b").value = getConfigValue("msgbox.dim.color.b", "80");
    document.getElementById("msgbox-dim-color-outline-r").value = getConfigValue("msgbox.dim.color.outline.r", "40");
    document.getElementById("msgbox-dim-color-outline-g").value = getConfigValue("msgbox.dim.color.outline.g", "40");
    document.getElementById("msgbox-dim-color-outline-b").value = getConfigValue("msgbox.dim.color.outline.b", "40");
    document.getElementById("click-x").value = getConfigValue("click.x", "1170");
    document.getElementById("click-y").value = getConfigValue("click.y", "660");
    document.getElementById("click-move").checked = intToBool(getConfigValue("click.move", "0"));
    document.getElementById("click-file1").value = getConfigValue("click.file1", "click1.png");
    document.getElementById("click-file2").value = getConfigValue("click.file2", "click2.png");
    document.getElementById("click-file3").value = getConfigValue("click.file3", "click3.png");
    document.getElementById("click-file4").value = getConfigValue("click.file4", "click4.png");
    document.getElementById("click-file5").value = getConfigValue("click.file5", "click5.png");
    document.getElementById("click-file6").value = getConfigValue("click.file6", "click6.png");
    document.getElementById("click-interval").value = getConfigValue("click.interval", "1.0");
    document.getElementById("click-disable").checked = intToBool(getConfigValue("click.disable", "0"));
    document.getElementById("switch-bg-file").value = getConfigValue("switch.bg.file", "switch-bg.png");
    document.getElementById("switch-fg-file").value = getConfigValue("switch.fg.file", "switch-fg.png");
    document.getElementById("switch-x").value = getConfigValue("switch.x", "409");
    document.getElementById("switch-y").value = getConfigValue("switch.y", "129");
    document.getElementById("switch-margin-y").value = getConfigValue("switch.margin.y", "20");
    document.getElementById("switch-text-margin-y").value = getConfigValue("switch.text.margin.y", "20");
    document.getElementById("switch-color-active").checked = intToBool(getConfigValue("switch.color.active", "0"));
    document.getElementById("switch-color-active-body-r").value = getConfigValue("switch.color.active.body.r", "255");
    document.getElementById("switch-color-active-body-g").value = getConfigValue("switch.color.active.body.g", "0");
    document.getElementById("switch-color-active-body-b").value = getConfigValue("switch.color.active.body.b", "0");
    document.getElementById("switch-color-active-outline-r").value = getConfigValue("switch.color.active.outline.r", "128");
    document.getElementById("switch-color-active-outline-g").value = getConfigValue("switch.color.active.outline.g", "128");
    document.getElementById("switch-color-active-outline-b").value = getConfigValue("switch.color.active.outline.b", "128");
    document.getElementById("switch-parent-click-se-file").value = getConfigValue("switch.parent.click.se.file", "click.ogg");
    document.getElementById("switch-change-se").value = getConfigValue("switch.change.se", "click.ogg");
    document.getElementById("save-data-thumb-width").value = getConfigValue("save.data.thumb.width", "213");
    document.getElementById("save-data-thumb-height").value = getConfigValue("save.data.thumb.height", "120");
    document.getElementById("sysmenu-x").value = getConfigValue("sysmenu.x", "731");
    document.getElementById("sysmenu-y").value = getConfigValue("sysmenu.y", "29");
    document.getElementById("sysmenu-idle-file").value = getConfigValue("sysmenu.idle.file", "sysmenu-idle.png");
    document.getElementById("sysmenu-hover-file").value = getConfigValue("sysmenu.hover.file", "sysmenu-hover.png");
    document.getElementById("sysmenu-disable-file").value = getConfigValue("sysmenu.disable.file", "sysmenu-disable.png");
    document.getElementById("sysmenu-qsave-x").value = getConfigValue("sysmenu.qsave.x", "62");
    document.getElementById("sysmenu-qsave-y").value = getConfigValue("sysmenu.qsave.y", "7");
    document.getElementById("sysmenu-qsave-width").value = getConfigValue("sysmenu.qsave.width", "60");
    document.getElementById("sysmenu-qsave-height").value = getConfigValue("sysmenu.qsave.height", "58");
    document.getElementById("sysmenu-qload-x").value = getConfigValue("sysmenu.qload.x", "123");
    document.getElementById("sysmenu-qload-y").value = getConfigValue("sysmenu.qload.y", "7");
    document.getElementById("sysmenu-qload-width").value = getConfigValue("sysmenu.qload.width", "60");
    document.getElementById("sysmenu-qload-height").value = getConfigValue("sysmenu.qload.height", "58");
    document.getElementById("sysmenu-save-x").value = getConfigValue("sysmenu.save.x", "184");
    document.getElementById("sysmenu-save-y").value = getConfigValue("sysmenu.save.y", "7");
    document.getElementById("sysmenu-save-width").value = getConfigValue("sysmenu.save.width", "60");
    document.getElementById("sysmenu-save-height").value = getConfigValue("sysmenu.save.height", "58");
    document.getElementById("sysmenu-load-x").value = getConfigValue("sysmenu.load.x", "245");
    document.getElementById("sysmenu-load-y").value = getConfigValue("sysmenu.load.y", "7");
    document.getElementById("sysmenu-load-width").value = getConfigValue("sysmenu.load.width", "60");
    document.getElementById("sysmenu-load-height").value = getConfigValue("sysmenu.load.height", "58");
    document.getElementById("sysmenu-auto-x").value = getConfigValue("sysmenu.auto.x", "306");
    document.getElementById("sysmenu-auto-y").value = getConfigValue("sysmenu.auto.y", "7");
    document.getElementById("sysmenu-auto-width").value = getConfigValue("sysmenu.auto.width", "60");
    document.getElementById("sysmenu-auto-height").value = getConfigValue("sysmenu.auto.height", "58");
    document.getElementById("sysmenu-skip-x").value = getConfigValue("sysmenu.skip.x", "367");
    document.getElementById("sysmenu-skip-y").value = getConfigValue("sysmenu.skip.y", "7");
    document.getElementById("sysmenu-skip-width").value = getConfigValue("sysmenu.skip.width", "60");
    document.getElementById("sysmenu-skip-height").value = getConfigValue("sysmenu.skip.height", "58");
    document.getElementById("sysmenu-history-x").value = getConfigValue("sysmenu.history.x", "428");
    document.getElementById("sysmenu-history-y").value = getConfigValue("sysmenu.history.y", "7");
    document.getElementById("sysmenu-history-width").value = getConfigValue("sysmenu.history.width", "60");
    document.getElementById("sysmenu-history-height").value = getConfigValue("sysmenu.history.height", "58");
    document.getElementById("sysmenu-config-x").value = getConfigValue("sysmenu.config.x", "489");
    document.getElementById("sysmenu-config-y").value = getConfigValue("sysmenu.config.y", "7");
    document.getElementById("sysmenu-config-width").value = getConfigValue("sysmenu.config.width", "60");
    document.getElementById("sysmenu-config-height").value = getConfigValue("sysmenu.config.height", "58");
    document.getElementById("sysmenu-enter-se").value = getConfigValue("sysmenu.enter.se", "click.ogg");
    document.getElementById("sysmenu-leave-se").value = getConfigValue("sysmenu.leave.se", "click.ogg");
    document.getElementById("sysmenu-change-se").value = getConfigValue("sysmenu.change.se", "btn-change.ogg");
    document.getElementById("sysmenu-qsave-se").value = getConfigValue("sysmenu.qsave.se", "click.ogg");
    document.getElementById("sysmenu-qload-se").value = getConfigValue("sysmenu.qload.se", "click.ogg");
    document.getElementById("sysmenu-save-se").value = getConfigValue("sysmenu.save.se", "click.ogg");
    document.getElementById("sysmenu-load-se").value = getConfigValue("sysmenu.load.se", "click.ogg");
    document.getElementById("sysmenu-auto-se").value = getConfigValue("sysmenu.auto.se", "click.ogg");
    document.getElementById("sysmenu-skip-se").value = getConfigValue("sysmenu.skip.se", "click.ogg");
    document.getElementById("sysmenu-history-se").value = getConfigValue("sysmenu.history.se", "click.ogg");
    document.getElementById("sysmenu-config-se").value = getConfigValue("sysmenu.config.se", "click.ogg");
    document.getElementById("sysmenu-collapsed-x").value = getConfigValue("sysmenu.collapsed.x", "1219");
    document.getElementById("sysmenu-collapsed-y").value = getConfigValue("sysmenu.collapsed.y", "29");
    document.getElementById("sysmenu-collapsed-idle-file").value = getConfigValue("sysmenu.collapsed.idle.file", "sysmenu-collapsed-idle.png");
    document.getElementById("sysmenu-collapsed-hover-file").value = getConfigValue("sysmenu.collapsed.hover.file", "sysmenu-collapsed-hover.png");
    document.getElementById("sysmenu-collapsed-se").value = getConfigValue("sysmenu.collapsed.se", "btn-change.ogg");
    document.getElementById("sysmenu-hidden").checked = intToBool(getConfigValue("sysmenu.hidden", "0"));
    document.getElementById("automode-banner-file").value = getConfigValue("automode.banner.file", "auto.png");
    document.getElementById("automode-banner-x").value = getConfigValue("automode.banner.x", "0");
    document.getElementById("automode-banner-y").value = getConfigValue("automode.banner.y", "126");
    document.getElementById("automode-speed").value = getConfigValue("automode.speed", "0.15");
    document.getElementById("skipmode-banner-file").value = getConfigValue("skipmode.banner.file", "skip.png");
    document.getElementById("skipmode-banner-x").value = getConfigValue("skipmode.banner.x", "0");
    document.getElementById("skipmode-banner-y").value = getConfigValue("skipmode.banner.y", "186");
    document.getElementById("sound-vol-bgm").value = getConfigValue("sound.vol.bgm", "1.0");
    document.getElementById("sound-vol-voice").value = getConfigValue("sound.vol.voice", "1.0");
    document.getElementById("sound-vol-se").value = getConfigValue("sound.vol.se", "1.0");
    document.getElementById("sound-vol-character").value = getConfigValue("sound.vol.character", "1.0");
    document.getElementById("sound-character-name1").value = getConfigValue("sound.character.name1", "");
    document.getElementById("sound-character-name2").value = getConfigValue("sound.character.name2", "");
    document.getElementById("sound-character-name3").value = getConfigValue("sound.charactername3", "");
    document.getElementById("sound-character-name4").value = getConfigValue("sound.character.name4", "");
    document.getElementById("sound-character-name5").value = getConfigValue("sound.character.name5", "");
    document.getElementById("sound-character-name6").value = getConfigValue("sound.character.name6", "");
    document.getElementById("sound-character-name7").value = getConfigValue("sound.character.name7", "");
    document.getElementById("sound-character-name8").value = getConfigValue("sound.character.name8", "");
    document.getElementById("sound-character-name9").value = getConfigValue("sound.character.name9", "");
    document.getElementById("sound-character-name10").value = getConfigValue("sound.character.name10", "");
    document.getElementById("sound-character-name11").value = getConfigValue("sound.character.name11", "");
    document.getElementById("sound-character-name12").value = getConfigValue("sound.character.name12", "");
    document.getElementById("sound-character-name13").value = getConfigValue("sound.character.name13", "");
    document.getElementById("sound-character-name14").value = getConfigValue("sound.character.name14", "");
    document.getElementById("sound-character-name15").value = getConfigValue("sound.character.name15", "");
    document.getElementById("serif-color1-name").value = getConfigValue("serif.color1.name", "");
    document.getElementById("serif-color1-r").value = getConfigValue("serif.color1.r", "0");
    document.getElementById("serif-color1-g").value = getConfigValue("serif.color1.g", "0");
    document.getElementById("serif-color1-b").value = getConfigValue("serif.color1.b", "0");
    document.getElementById("serif-color1-outline-r").value = getConfigValue("serif.color1.outline.r", "0");
    document.getElementById("serif-color1-outline-g").value = getConfigValue("serif.color1.outline.g", "0");
    document.getElementById("serif-color1-outline-b").value = getConfigValue("serif.color1.outline.b", "0");
    document.getElementById("serif-color2-name").value = getConfigValue("serif.color2.name", "");
    document.getElementById("serif-color2-r").value = getConfigValue("serif.color2.r", "0");
    document.getElementById("serif-color2-g").value = getConfigValue("serif.color2.g", "0");
    document.getElementById("serif-color2-b").value = getConfigValue("serif.color2.b", "0");
    document.getElementById("serif-color2-outline-r").value = getConfigValue("serif.color2.outline.r", "0");
    document.getElementById("serif-color2-outline-g").value = getConfigValue("serif.color2.outline.g", "0");
    document.getElementById("serif-color2-outline-b").value = getConfigValue("serif.color2.outline.b", "0");
    document.getElementById("serif-color3-name").value = getConfigValue("serif.color3.name", "");
    document.getElementById("serif-color3-r").value = getConfigValue("serif.color3.r", "0");
    document.getElementById("serif-color3-g").value = getConfigValue("serif.color3.g", "0");
    document.getElementById("serif-color3-b").value = getConfigValue("serif.color3.b", "0");
    document.getElementById("serif-color3-outline-r").value = getConfigValue("serif.color3.outline.r", "0");
    document.getElementById("serif-color3-outline-g").value = getConfigValue("serif.color3.outline.g", "0");
    document.getElementById("serif-color3-outline-b").value = getConfigValue("serif.color3.outline.b", "0");
    document.getElementById("serif-color4-name").value = getConfigValue("serif.color4.name", "");
    document.getElementById("serif-color4-r").value = getConfigValue("serif.color4.r", "0");
    document.getElementById("serif-color4-g").value = getConfigValue("serif.color4.g", "0");
    document.getElementById("serif-color4-b").value = getConfigValue("serif.color4.b", "0");
    document.getElementById("serif-color4-outline-r").value = getConfigValue("serif.color4.outline.r", "0");
    document.getElementById("serif-color4-outline-g").value = getConfigValue("serif.color4.outline.g", "0");
    document.getElementById("serif-color4-outline-b").value = getConfigValue("serif.color4.outline.b", "0");
    document.getElementById("serif-color5-name").value = getConfigValue("serif.color5.name", "");
    document.getElementById("serif-color5-r").value = getConfigValue("serif.color5.r", "0");
    document.getElementById("serif-color5-g").value = getConfigValue("serif.color5.g", "0");
    document.getElementById("serif-color5-b").value = getConfigValue("serif.color5.b", "0");
    document.getElementById("serif-color5-outline-r").value = getConfigValue("serif.color5.outline.r", "0");
    document.getElementById("serif-color5-outline-g").value = getConfigValue("serif.color5.outline.g", "0");
    document.getElementById("serif-color5-outline-b").value = getConfigValue("serif.color5.outline.b", "0");
    document.getElementById("voice-stop-off").checked = intToBool(getConfigValue("voice.stop.off", "0"));
    document.getElementById("window-fullscreen-disable").checked = intToBool(getConfigValue("window.fullscreen.disable", "0"));
    document.getElementById("window-maximize-disable").checked = intToBool(getConfigValue("window.maximize.disable", "0"));
    document.getElementById("window-title-separator").value = getConfigValue("window.title.separator", " ");
    document.getElementById("msgbox-show-on-ch").checked = intToBool(getConfigValue("msgbox.show.on.ch", "0"));
    document.getElementById("msgbox-show-on-bg").checked = intToBool(getConfigValue("msgbox.show.on.bg", "0"));

    // Show slider values.
    var rangeArray = [].slice.call(document.querySelectorAll("input[type='range']"));
    for(let r of rangeArray) {
        if(typeof r.id !== "undefined" && r.id !== null) {
            var num = document.getElementById(r.id + "-num");
            if(num !== null) {
                num.textContent = r.value;
                r.linkedRange = num;
                r.addEventListener("input", (e) => {
                    e.target.linkedRange.textContent = e.target.value;
                });
            }
        }
    }

    // Add live search.
    searchInput = document.getElementById('searchbox');
    searchInput.addEventListener('keyup', () => {
        clearTimeout(typingTimer);
        typingTimer = setTimeout(liveSearch, typeInterval);
    });
})

window.addEventListener("beforeunload", async () => {
    // Get properties.
    config["window.title"] = document.getElementById("window-title").value;
    config["window.width"] = document.getElementById("window-width").value;
    config["window.height"] = document.getElementById("window-height").value;
    config["window.white"] = boolToInt(document.getElementById("window-white").checked);
    config["window.menubar"] = boolToInt(document.getElementById("window-menubar").checked)
    config["font.file"] = document.getElementById("font-file").value;
    config["font.size"] = document.getElementById("font-size").value;
    config["font.color.r"] = document.getElementById("font-color-r").value;
    config["font.color.g"] = document.getElementById("font-color-g").value;
    config["font.color.b"] = document.getElementById("font-color-b").value;
    config["font.outline.color.r"] = document.getElementById("font-outline-color-r").value;
    config["font.outline.color.g"] = document.getElementById("font-outline-color-g").value;
    config["font.outline.color.b"] = document.getElementById("font-outline-color-b").value;
    config["font.outline.remove"] = boolToInt(document.getElementById("font-outline-remove").checked);
    config["namebox.file"] = document.getElementById("namebox-file").value;
    config["namebox.x"] = document.getElementById("namebox-x").value;
    config["namebox.y"] = document.getElementById("namebox-y").value;
    config["namebox.margin.top"] = document.getElementById("namebox-margin-top").value;
    config["namebox.centering.no"] = boolToInt(document.getElementById("namebox-centering-no").checked);
    config["namebox.margin.left"] = document.getElementById("namebox-margin-left").value;
    config["namebox.hidden"] = boolToInt(document.getElementById("namebox-hidden").checked);
    config["msgbox.bg.file"] = document.getElementById("msgbox-bg-file").value;
    config["msgbox.fg.file"] = document.getElementById("msgbox-fg-file").value;
    config["msgbox.x"] = document.getElementById("msgbox-x").value;
    config["msgbox.y"] = document.getElementById("msgbox-y").value;
    config["msgbox.margin.left"] = document.getElementById("msgbox-margin-left").value;
    config["msgbox.margin.top"] = document.getElementById("msgbox-margin-top").value;
    config["msgbox.margin.right"] = document.getElementById("msgbox-margin-right").value;
    config["msgbox.margin.line"] = document.getElementById("msgbox-margin-line").value;
    config["msgbox.speed"] = document.getElementById("msgbox-speed").value;
    config["msgbox.btn.hide.x"] = document.getElementById("msgbox-btn-hide-x").value;
    config["msgbox.btn.hide.y"] = document.getElementById("msgbox-btn-hide-y").value;
    config["msgbox.btn.hide.width"] = document.getElementById("msgbox-btn-hide-width").value;
    config["msgbox.btn.hide.height"] = document.getElementById("msgbox-btn-hide-height").value;
    config["msgbox.btn.qsave.x"] = document.getElementById("msgbox-btn-qsave-x").value;
    config["msgbox.btn.qsave.y"] = document.getElementById("msgbox-btn-qsave-y").value;
    config["msgbox.btn.qsave.width"] = document.getElementById("msgbox-btn-qsave-width").value;
    config["msgbox.btn.qsave.height"] = document.getElementById("msgbox-btn-qsave-height").value;
    config["msgbox.btn.qload.x"] = document.getElementById("msgbox-btn-qload-x").value;
    config["msgbox.btn.qload.y"] = document.getElementById("msgbox-btn-qload-y").value;
    config["msgbox.btn.qload.width"] = document.getElementById("msgbox-btn-qload-width").value;
    config["msgbox.btn.qload.height"] = document.getElementById("msgbox-btn-qload-height").value;
    config["msgbox.btn.save.x"] = document.getElementById("msgbox-btn-save-x").value;
    config["msgbox.btn.save.y"] = document.getElementById("msgbox-btn-save-y").value;
    config["msgbox.btn.save.width"] = document.getElementById("msgbox-btn-save-width").value;
    config["msgbox.btn.save.height"] = document.getElementById("msgbox-btn-save-height").value;
    config["msgbox.btn.load.x"] = document.getElementById("msgbox-btn-load-x").value;
    config["msgbox.btn.load.y"] = document.getElementById("msgbox-btn-load-y").value;
    config["msgbox.btn.load.width"] = document.getElementById("msgbox-btn-load-width").value;
    config["msgbox.btn.load.height"] = document.getElementById("msgbox-btn-load-height").value;
    config["msgbox.btn.skip.x"] = document.getElementById("msgbox-btn-skip-x").value;
    config["msgbox.btn.skip.y"] = document.getElementById("msgbox-btn-skip-y").value;
    config["msgbox.btn.skip.width"] = document.getElementById("msgbox-btn-skip-width").value;
    config["msgbox.btn.skip.height"] = document.getElementById("msgbox-btn-skip-height").value;
    config["msgbox.btn.history.x"] = document.getElementById("msgbox-btn-history-x").value;
    config["msgbox.btn.history.y"] = document.getElementById("msgbox-btn-history-y").value;
    config["msgbox.btn.history.width"] = document.getElementById("msgbox-btn-history-width").value;
    config["msgbox.btn.history.height"] = document.getElementById("msgbox-btn-history-height").value;
    config["msgbox.btn.config.x"] = document.getElementById("msgbox-btn-config-x").value;
    config["msgbox.btn.config.y"] = document.getElementById("msgbox-btn-config-y").value;
    config["msgbox.btn.config.width"] = document.getElementById("msgbox-btn-config-width").value;
    config["msgbox.btn.config.height"] = document.getElementById("msgbox-btn-config-height").value;
    config["msgbox.btn.change.se"] = document.getElementById("msgbox-btn-change-se").value;
    config["msgbox.history.se"] = document.getElementById("msgbox-history-se").value;
    config["msgbox.config.se"] = document.getElementById("msgbox-config-se").value;
    config["msgbox.hide.se"] = document.getElementById("msgbox-hide-se").value;
    config["msgbox.show.se"] = document.getElementById("msgbox-show-se").value;
    config["msgbox.auto.cancel.se"] = document.getElementById("msgbox-auto-cancel-se").value;
    config["msgbox.skip.cancel.se"] = document.getElementById("msgbox-skip-cancel-se").value;
    config["msgbox.skip.unseen"] = boolToInt(document.getElementById("msgbox-skip-unseen").checked);
    config["msgbox.dim"] = boolToInt(document.getElementById("msgbox-dim").checked);
    config["msgbox.dim.color.r"] = document.getElementById("msgbox-dim-color-r").value;
    config["msgbox.dim.color.g"] = document.getElementById("msgbox-dim-color-g").value;
    config["msgbox.dim.color.b"] = document.getElementById("msgbox-dim-color-b").value;
    config["msgbox.dim.color.outline.r"] = document.getElementById("msgbox-dim-color-outline-r").value;
    config["msgbox.dim.color.outline.g"] = document.getElementById("msgbox-dim-color-outline-g").value;
    config["msgbox.dim.color.outline.b"] = document.getElementById("msgbox-dim-color-outline-b").value;
    config["click.x"] = document.getElementById("click-x").value;
    config["click.y"] = document.getElementById("click-y").value;
    config["click.move"] = boolToInt(document.getElementById("click-move").checked);
    config["click.file1"] = document.getElementById("click-file1").value;
    config["click.file2"] = document.getElementById("click-file2").value;
    config["click.file3"] = document.getElementById("click-file3").value;
    config["click.file4"] = document.getElementById("click-file4").value;
    config["click.file5"] = document.getElementById("click-file5").value;
    config["click.file6"] = document.getElementById("click-file6").value;
    config["click.interval"] = document.getElementById("click-interval").value;
    config["click.disable"] = boolToInt(document.getElementById("click-disable").checked);
    config["switch.bg.file"] = document.getElementById("switch-bg-file").value;
    config["switch.fg.file"] = document.getElementById("switch-fg-file").value;
    config["switch.x"] = document.getElementById("switch-x").value;
    config["switch.y"] = document.getElementById("switch-y").value;
    config["switch.margin.y"] = document.getElementById("switch-margin-y").value;
    config["switch.text.margin.y"] = document.getElementById("switch-text-margin-y").value;
    config["switch.color.active"] = boolToInt(document.getElementById("switch-color-active").checked);
    config["switch.color.active.body.r"] = document.getElementById("switch-color-active-body-r").value;
    config["switch.color.active.body.g"] = document.getElementById("switch-color-active-body-g").value;
    config["switch.color.active.body.b"] = document.getElementById("switch-color-active-body-b").value;
    config["switch.color.active.outline.r"] = document.getElementById("switch-color-active-outline-r").value;
    config["switch.color.active.outline.g"] = document.getElementById("switch-color-active-outline-g").value;
    config["switch.color.active.outline.b"] = document.getElementById("switch-color-active-outline-b").value;
    config["switch.parent.click.se.file"] = document.getElementById("switch-parent-click-se-file").value;
    config["switch.change.se"] = document.getElementById("switch-change-se").value;
    config["save.data.thumb.width"] = document.getElementById("save-data-thumb-width").value;
    config["save.data.thumb.height"] = document.getElementById("save-data-thumb-height").value;
    config["sysmenu.x"] = document.getElementById("sysmenu-x").value;
    config["sysmenu.y"] = document.getElementById("sysmenu-y").value;
    config["sysmenu.idle.file"] = document.getElementById("sysmenu-idle-file").value;
    config["sysmenu.hover.file"] = document.getElementById("sysmenu-hover-file").value;
    config["sysmenu.disable.file"] = document.getElementById("sysmenu-disable-file").value;
    config["sysmenu.qsave.x"] = document.getElementById("sysmenu-qsave-x").value;
    config["sysmenu.qsave.y"] = document.getElementById("sysmenu-qsave-y").value;
    config["sysmenu.qsave.width"] = document.getElementById("sysmenu-qsave-width").value;
    config["sysmenu.qsave.height"] = document.getElementById("sysmenu-qsave-height").value;
    config["sysmenu.qload.x"] = document.getElementById("sysmenu-qload-x").value;
    config["sysmenu.qload.y"] = document.getElementById("sysmenu-qload-y").value;
    config["sysmenu.qload.width"] = document.getElementById("sysmenu-qload-width").value;
    config["sysmenu.qload.height"] = document.getElementById("sysmenu-qload-height").value;
    config["sysmenu.save.x"] = document.getElementById("sysmenu-save-x").value = getConfigValue("sysmenu.save.x", "184");
    config["sysmenu.save.y"] = document.getElementById("sysmenu-save-y").value = getConfigValue("sysmenu.save.y", "7");
    config["sysmenu.save.width"] = document.getElementById("sysmenu-save-width").value;
    config["sysmenu.save.height"] = document.getElementById("sysmenu-save-height").value;
    config["sysmenu.load.x"] = document.getElementById("sysmenu-load-x").value;
    config["sysmenu.load.y"] = document.getElementById("sysmenu-load-y").value;
    config["sysmenu.load.width"] = document.getElementById("sysmenu-load-width").value;
    config["sysmenu.load.height"] = document.getElementById("sysmenu-load-height").value;
    config["sysmenu.auto.x"] = document.getElementById("sysmenu-auto-x").value;
    config["sysmenu.auto.y"] = document.getElementById("sysmenu-auto-y").value;
    config["sysmenu.auto.width"] = document.getElementById("sysmenu-auto-width").value;
    config["sysmenu.auto.height"] = document.getElementById("sysmenu-auto-height").value;
    config["sysmenu.skip.x"] = document.getElementById("sysmenu-skip-x").value;
    config["sysmenu.skip.y"] = document.getElementById("sysmenu-skip-y").value;
    config["sysmenu.skip.width"] = document.getElementById("sysmenu-skip-width").value;
    config["sysmenu.skip.height"] = document.getElementById("sysmenu-skip-height").value;
    config["sysmenu.history.x"] = document.getElementById("sysmenu-history-x").value;
    config["sysmenu.history.y"] = document.getElementById("sysmenu-history-y").value;
    config["sysmenu.history.width"] = document.getElementById("sysmenu-history-width").value;
    config["sysmenu.history.height"] = document.getElementById("sysmenu-history-height").value;
    config["sysmenu.config.x"] = document.getElementById("sysmenu-config-x").value;
    config["sysmenu.config.y"] = document.getElementById("sysmenu-config-y").value;
    config["sysmenu.config.width"] = document.getElementById("sysmenu-config-width").value;
    config["sysmenu.config.height"] = document.getElementById("sysmenu-config-height").value;
    config["sysmenu.enter.se"] = document.getElementById("sysmenu-enter-se").value;
    config["sysmenu.leave.se"] = document.getElementById("sysmenu-leave-se").value;
    config["sysmenu.change.se"] = document.getElementById("sysmenu-change-se").value;
    config["sysmenu.qsave.se"] = document.getElementById("sysmenu-qsave-se").value;
    config["sysmenu.qload.se"] = document.getElementById("sysmenu-qload-se").value;
    config["sysmenu.save.se"] = document.getElementById("sysmenu-save-se").value;
    config["sysmenu.load.se"] = document.getElementById("sysmenu-load-se").value;
    config["sysmenu.auto.se"] = document.getElementById("sysmenu-auto-se").value;
    config["sysmenu.skip.se"] = document.getElementById("sysmenu-skip-se").value;
    config["sysmenu.history.se"] = document.getElementById("sysmenu-history-se").value;
    config["sysmenu.config.se"] = document.getElementById("sysmenu-config-se").value;
    config["sysmenu.collapsed.x"] = document.getElementById("sysmenu-collapsed-x").value;
    config["sysmenu.collapsed.y"] = document.getElementById("sysmenu-collapsed-y").value;
    config["sysmenu.collapsed.idle.file"] = document.getElementById("sysmenu-collapsed-idle-file").value;
    config["sysmenu.collapsed.idle.file"] = document.getElementById("sysmenu-collapsed-hover-file").value;
    config["sysmenu.collapsed.se"] = document.getElementById("sysmenu-collapsed-se").value;
    config["sysmenu.hidden"] = boolToInt(document.getElementById("sysmenu-hidden").checked);
    config["automode.banner.file"] = document.getElementById("automode-banner-file").value;
    config["automode.banner.x"] = document.getElementById("automode-banner-x").value;
    config["automode.banner.y"] = document.getElementById("automode-banner-y").value;
    config["automode.speed"] = document.getElementById("automode-speed").value;
    config["skipmode.banner.file"] = document.getElementById("skipmode-banner-file").value;
    config["skipmode.banner.x"] = document.getElementById("skipmode-banner-x").value;
    config["skipmode.banner.y"] = document.getElementById("skipmode-banner-y").value;
    config["sound.vol.bgm"] = document.getElementById("sound-vol-bgm").value;
    config["sound.vol.voice"] = document.getElementById("sound-vol-voice").value;
    config["sound.vol.se"] = document.getElementById("sound-vol-se").value;
    config["sound.vol.character"] = document.getElementById("sound-vol-character").value;
    config["sound.character.name1"] = document.getElementById("sound-character-name1").value;
    config["sound.character.name2"] = document.getElementById("sound-character-name2").value;
    config["sound.character.name3"] = document.getElementById("sound-character-name3").value;
    config["sound.character.name4"] = document.getElementById("sound-character-name4").value;
    config["sound.character.name5"] = document.getElementById("sound-character-name5").value;
    config["sound.character.name6"] = document.getElementById("sound-character-name6").value;
    config["sound.character.name7"] = document.getElementById("sound-character-name7").value;
    config["sound.character.name8"] = document.getElementById("sound-character-name8").value;
    config["sound.character.name9"] = document.getElementById("sound-character-name9").value;
    config["sound.character.name10"] = document.getElementById("sound-character-name10").value;
    config["sound.character.name11"] = document.getElementById("sound-character-name11").value;
    config["sound.character.name12"] = document.getElementById("sound-character-name12").value;
    config["sound.character.name13"] = document.getElementById("sound-character-name13").value;
    config["sound.character.name14"] = document.getElementById("sound-character-name14").value;
    config["sound.character.name15"] = document.getElementById("sound-character-name15").value;
    config["serif.color1.name"] = document.getElementById("serif-color1-name").value;
    config["serif.color1.r"] = document.getElementById("serif-color1-r").value;
    config["serif.color1.g"] = document.getElementById("serif-color1-g").value;
    config["serif.color1.b"] = document.getElementById("serif-color1-b").value;
    config["serif.color1.outline.r"] = document.getElementById("serif-color1-outline-r").value;
    config["serif.color1.outline.g"] = document.getElementById("serif-color1-outline-g").value;
    config["serif.color1.outline.b"] = document.getElementById("serif-color1-outline-b").value;
    config["serif.color2.name"] = document.getElementById("serif-color2-name").value;
    config["serif.color2.r"] = document.getElementById("serif-color2-r").value;
    config["serif.color2.g"] = document.getElementById("serif-color2-g").value;
    config["serif.color2.b"] = document.getElementById("serif-color2-b").value;
    config["serif.color2.outline.r"] = document.getElementById("serif-color2-outline-r").value;
    config["serif.color2.outline.g"] = document.getElementById("serif-color2-outline-g").value;
    config["serif.color2.outline.b"] = document.getElementById("serif-color2-outline-b").value;
    config["serif.color3.name"] = document.getElementById("serif-color3-name").value;
    config["serif.color3.r"] = document.getElementById("serif-color3-r").value;
    config["serif.color3.g"] = document.getElementById("serif-color3-g").value;
    config["serif.color3.b"] = document.getElementById("serif-color3-b").value;
    config["serif.color3.outline.r"] = document.getElementById("serif-color3-outline-r").value;
    config["serif.color3.outline.g"] = document.getElementById("serif-color3-outline-g").value;
    config["serif.color3.outline.b"] = document.getElementById("serif-color3-outline-b").value;
    config["serif.color4.name"] = document.getElementById("serif-color4-name").value;
    config["serif.color4.r"] = document.getElementById("serif-color4-r").value;
    config["serif.color4.g"] = document.getElementById("serif-color4-g").value;
    config["serif.color4.b"] = document.getElementById("serif-color4-b").value;
    config["serif.color4.outline.r"] = document.getElementById("serif-color4-outline-r").value;
    config["serif.color4.outline.g"] = document.getElementById("serif-color4-outline-g").value;
    config["serif.color4.outline.b"] = document.getElementById("serif-color4-outline-b").value;
    config["serif.color5.name"] = document.getElementById("serif-color5-name").value;
    config["serif.color5.r"] = document.getElementById("serif-color5-r").value;
    config["serif.color5.g"] = document.getElementById("serif-color5-g").value;
    config["serif.color5.b"] = document.getElementById("serif-color5-b").value;
    config["serif.color5.outline.r"] = document.getElementById("serif-color5-outline-r").value;
    config["serif.color5.outline.g"] = document.getElementById("serif-color5-outline-g").value;
    config["serif.color5.outline.b"] = document.getElementById("serif-color5-outline-b").value;
    config["voice.stop.off"] = boolToInt(document.getElementById("voice-stop-off").checked);
    config["window.fullscreen.disable"] = boolToInt(document.getElementById("window-fullscreen-disable").checked);
    config["window.maximize.disable"] = boolToInt(document.getElementById("window-maximize-disable").checked);
    config["window.title.separator"] = document.getElementById("window-title-separator").value;
    config["msgbox.show.on.ch"] = boolToInt(document.getElementById("msgbox-show-on-ch").checked);
    config["msgbox.show.on.bg"] = boolToInt(document.getElementById("msgbox-show-on-bg").checked);

    // Validate properties.
    config["sound.character.name1"] = config["sound.character.name1"] === "" ? "_" : config["sound.character.name1"];
    config["sound.character.name2"] = config["sound.character.name2"] === "" ? "_" : config["sound.character.name2"];
    config["sound.character.name3"] = config["sound.character.name3"] === "" ? "_" : config["sound.character.name3"];
    config["sound.character.name4"] = config["sound.character.name4"] === "" ? "_" : config["sound.character.name4"];
    config["sound.character.name5"] = config["sound.character.name5"] === "" ? "_" : config["sound.character.name5"];
    config["sound.character.name6"] = config["sound.character.name6"] === "" ? "_" : config["sound.character.name6"];
    config["sound.character.name7"] = config["sound.character.name7"] === "" ? "_" : config["sound.character.name7"];
    config["sound.character.name8"] = config["sound.character.name8"] === "" ? "_" : config["sound.character.name8"];
    config["sound.character.name9"] = config["sound.character.name9"] === "" ? "_" : config["sound.character.name9"];
    config["sound.character.name10"] = config["sound.character.name10"] === "" ? "_" : config["sound.character.name10"];
    config["sound.character.name11"] = config["sound.character.name11"] === "" ? "_" : config["sound.character.name11"];
    config["sound.character.name12"] = config["sound.character.name12"] === "" ? "_" : config["sound.character.name12"];
    config["sound.character.name13"] = config["sound.character.name13"] === "" ? "_" : config["sound.character.name13"];
    config["sound.character.name14"] = config["sound.character.name14"] === "" ? "_" : config["sound.character.name14"];
    config["sound.character.name15"] = config["sound.character.name15"] === "" ? "_" : config["sound.character.name15"];
    config["serif.color1.name"] = config["serif.color1.name"] === "" ? "_" : config["serif.color1.name"];
    config["serif.color2.name"] = config["serif.color2.name"] === "" ? "_" : config["serif.color2.name"];
    config["serif.color3.name"] = config["serif.color3.name"] === "" ? "_" : config["serif.color3.name"];
    config["serif.color4.name"] = config["serif.color4.name"] === "" ? "_" : config["serif.color4.name"];
    config["serif.color5.name"] = config["serif.color5.name"] === "" ? "-" : config["serif.color5.name"];

    // Store to config.txt.
    await window.api.storeConfig(config);    
})
