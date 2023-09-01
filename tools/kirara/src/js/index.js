var locale = "ja";

/*
 * ゲームの構成
 */

// Base URL of the game. (includes "/" at the tails)
var baseUrl;

// List of the flags.
var flagList;

/*
 * シナリオビュー
 */

async function refreshScenario() {
    // ヘッダを更新する
    document.getElementById("scenario-header").innerHTML = await window.api.getScenarioName();

    // シナリオビューのアイテムをすべて削除する
    var element = document.getElementById("scenario");
    while (element.firstChild) {
        element.removeChild(element.firstChild);
    }

    // シナリオデータを取得してビューに追加する
    var arr = await window.api.getScenarioData();
    arr.forEach(line => {
        var elem = createCommandElement(line);
        elem.addEventListener("dragstart", onScenarioDragStart);
        elem.addEventListener("dragover", onScenarioDragOver);
        elem.addEventListener("dragleave", onScenarioDragLeave);
        elem.addEventListener("drop", onScenarioDrop);
        elem.addEventListener("dragend", onScenarioDragEnd);
        document.getElementById("scenario").appendChild(elem);
    });

    // 末尾に"ゲーム終了"のアイテムを挿入する
    var endElem = document.createElement("li");
    endElem.id = "end-mark";
    endElem.classList.add("drag-list-item");
    endElem.textContent = translate("ゲーム終了");
    endElem.addEventListener("dragover", onScenarioDragOver);
    endElem.addEventListener("dragleave", onScenarioDragLeave);
    endElem.addEventListener("drop", onScenarioDrop);
    endElem.classList.add("drag-list-item-end");
    endElem.style.cursor = "";
    document.getElementById("scenario").appendChild(endElem);
}

function createCommandElement(command) {
    // 要素を作成する
    var newElem = document.createElement("li");
    newElem.id = makeId();
    newElem.draggable = "true";
    newElem.cmd = command;
    newElem.classList.add("drag-list-item");
    newElem.addEventListener("click", async () => {
        Array.from(document.getElementById("scenario").childNodes).forEach(function (e) {
            if(e.classList != null && e.classList.contains("drag-list-item")) {
                if(e === event.srcElement) {
                    e.classList.add("drag-list-item-sel");
                } else {
                    e.classList.remove("drag-list-item-sel");
                }
            }
        });
        document.getElementById("thumbnail-picture").src = "";
        commitProps();
        changeElement(event.srcElement);
        showProps();
    });
    newElem.addEventListener("dblclick", async () => {
        // 保存する
        commitProps();
        saveScenario();

        // デバッグを開始する
        var scenarioArray = [].slice.call(document.querySelectorAll("#scenario li"));
        var lineIndex = scenarioArray.indexOf(elementInEdit);
        window.api.debugGame(lineIndex);
    });

    // コマンドの種類ごとに要素の設定を行う
    if (command.startsWith("@bg ") || command.startsWith("@背景 ")) {
        // @bg
        var cl = normalizeBg(command);
        newElem.textContent = translate("背景");
        newElem.classList.add("drag-list-item-bg");
        if(!cl[1].startsWith("#")) {
            newElem.style.backgroundImage = "url(\"" + baseUrl.replace(/\\/g, "\\\\") + "bg/" + cl[1] + "\")";
        } else if(cl[1] === "#ffffff") {
            newElem.style.background = "white";
            newElem.style.color = "black";
        } else if(cl[1] === "#000000") {
            newElem.style.background = "black";
            newElem.style.color = "white";
        }
        newElem.style.backgroundSize = "cover";
    } else if(command.startsWith("@ch ") || command.startsWith("@キャラ ")) {
        // @ch
        var cl = normalizeCh(command);
        newElem.textContent = translate("キャラ ") + cl[2];
        newElem.classList.add("drag-list-item-ch");
        if(cl[2] !== "none" && cl[2] !== "消去") {
            newElem.style.backgroundImage = "url(\"" + baseUrl.replace(/\\/g, "\\\\") + "ch/" + cl[2] + "\")";
            newElem.style.backgroundRepeat = "no-repeat";
            newElem.style.backgroundSize = "contain";
        }
    } else if(command.startsWith("@bgm ") || command.startsWith("@音楽 ")) {
        // @bgm
        var cl = normalizeBgm(command);
        newElem.textContent = translate("音楽 ") + cl[1];
        newElem.classList.add("drag-list-item-bgm");
    } else if(command.startsWith("@se ") || command.startsWith("@効果音 ")) {
        // @se
        var cl = normalizeSe(command);
        newElem.textContent = translate("効果音 ") + cl[1];
        newElem.classList.add("drag-list-item-se");
    } else if(command.startsWith("@vol ") || command.startsWith("@音量 ")) {
        // @vol
        newElem.textContent = translate("音量");
        newElem.classList.add("drag-list-item-vol");
    } else if(command.startsWith("@choose ") || command.startsWith("@選択肢 ")) {
        // @choose
        newElem.textContent = translate("選択肢");
        newElem.classList.add("drag-list-item-choose");
    } else if(command.startsWith("@cha ") || command.startsWith("@キャラ移動 ")) {
        // @cha
        var cl = normalizeCha(command);
        newElem.textContent = translate("キャラ移動 ") + translate(japanizeChPosition(cl[1]));
        newElem.classList.add("drag-list-item-cha");
    } else if(command.startsWith("@chs ") || command.startsWith("@場面転換 ")) {
        // @chs
        var cl = normalizeChs(command);
        newElem.textContent = translate("場面転換");
        newElem.classList.add("drag-list-item-chs");
    } else if(command.startsWith("@shake ") || command.startsWith("@振動 ")) {
        // @shake
        var cl = normalizeShake(command);
        newElem.textContent = translate("画面を揺らす");
        newElem.classList.add("drag-list-item-shake");
    } else if(command.startsWith("@click ") || command.startsWith("@クリック ")) {
        // @click
        newElem.textContent = translate("クリックを待つ");
        newElem.classList.add("drag-list-item-click");
    } else if(command.startsWith("@wait ") || command.startsWith("@時間待ち ")) {
        // @wait
        var cl = normalizeWait(command);
        newElem.textContent = translate("一定時間待つ ") + cl[1] + translate("秒");
        newElem.classList.add("drag-list-item-wait");
    } else if(command.startsWith("@skip ") || command.startsWith("@スキップ ")) {
        // @skip
        var cl = normalizeSkip(command);
        newElem.textContent = translate("スキップ") + (cl[1] === "enable" ? translate("許可") : translate("禁止"));
        newElem.classList.add("drag-list-item-skip");
    } else if(command.startsWith("@goto ") || command.startsWith("@ジャンプ ")) {
        // @goto
        var cl = normalizeGoto(command);
        newElem.textContent = translate("ジャンプ \"") + unquote(cl[1]) + translate("\"へ");
        newElem.classList.add("drag-list-item-goto");
    } else if(command.startsWith("@set ") || command.startsWith("@フラグをセット ")) {
        // @set
        var cl = normalizeSet(command);
        newElem.textContent = translate("フラグをセット");
        newElem.classList.add("drag-list-item-set");
    } else if(command.startsWith("@if ") || command.startsWith("@フラグでジャンプ ")) {
        // @if
        var cl = normalizeIf(command);
        newElem.textContent = translate("フラグでジャンプ");
        newElem.classList.add("drag-list-item-if");
    } else if(command.startsWith("@load ") || command.startsWith("@シナリオ ")) {
        // @load
        var cl = normalizeLoad(command);
        newElem.textContent = translate("シナリオへジャンプ");
        newElem.classList.add("drag-list-item-load");
    } else if(command.startsWith("@chapter ") || command.startsWith("@章 ")) {
        // @chapter
        var cl = normalizeChapter(command);
        newElem.textContent = translate("章のタイトル \"") + cl[1] + "\"";
        newElem.classList.add("drag-list-item-chapter");
    } else if(command.startsWith("@wms ") || command.startsWith("@スクリプト ")) {
        // @wms
        var cl = normalizeWms(command);
        newElem.textContent = translate("高機能スクリプトを呼び出す");
        newElem.classList.add("drag-list-item-wms");
    } else if(command.startsWith("@gui ") || command.startsWith("@メニュー ")) {
        // @gui
        var cl = normalizeGui(command);
        newElem.textContent = translate("高機能メニューを呼び出す");
        newElem.classList.add("drag-list-item-gui");
    } else if(command.startsWith("@video ") || command.startsWith("@動画 ")) {
        // @video
        var cl = normalizeVideo(command);
        newElem.textContent = translate("動画 ") + cl[1];
        newElem.classList.add("drag-list-item-video");
    } else if(command.startsWith("@")) {
        // Kiraraで未対応のコマンド
        newElem.textContent = command;
        newElem.classList.add("drag-list-item-etc");
    } else if(command.startsWith(":")) {
        newElem.textContent = translate("目印 \"") + command.substring(1) + "\"";
        newElem.classList.add("drag-list-item-label");
    } else if(command.match(/^\*[^\*]+\*[^\*]+$/)) {
        // セリフ(*キャラ名*セリフ)
        var sp = command.split("*");
        newElem.textContent = sp[1] + translate("「") + sp[2] + translate("」");
        newElem.classList.add("drag-list-item-serif");
    } else if(command.match(/^\*[^\*]+\*[^\*]+\*[^\*]+$/)) {
        // セリフ(*キャラ名*ボイス*セリフ)
        var sp = command.split("*");
        newElem.textContent = sp[1] + translate("「") + sp[3] + translate("」");
        newElem.classList.add("drag-list-item-serif");
    } else if(command.match(/^.+「.*」$/)) {
        // セリフ(「」)
        var sp = command.split("「");
        var name = sp[0];
        var msg = sp[1].substring(0, sp[1].length - "」".length);
        newElem.textContent = name + translate("「") + msg + translate("」");
        newElem.classList.add("drag-list-item-serif");
    } else if(command === "") {
        // 空行は空コマンドにする
        newElem.cmd = "";
        newElem.textContent = "　";
        newElem.classList.add("drag-list-item-comment");
    } else if(command.startsWith("#")) {
        // コメント
        newElem.textContent = command.substring(1);
        if(newElem.textContent === "") {
            newElem.textContent = " ";
        }
        newElem.classList.add("drag-list-item-comment");
    } else {
        // 上記に該当しなければメッセージ
        command = command.replace(/\n/g, "\\n");
        newElem.textContent = command;
        newElem.classList.add("drag-list-item-msg");
    }
    return newElem;
}

async function saveScenario() {
    var data = [];
    Array.from(document.getElementById("scenario").childNodes).forEach(function (e) {
        if(typeof e.cmd !== "undefined" && e.cmd !== null) {
            data.push(e.cmd);
        }
    });
    await window.api.setScenarioData(data);
}

function onScenarioDragStart(event) {
    event.dataTransfer.setData("text/plain", event.target.id);
    event.dataTransfer.setData("scenario", "");
    document.getElementById("thumbnail-picture").src = "../img/trash-close.png";
    return true;
}

function onScenarioDragOver(event) {
    event.preventDefault();
    this.style.borderTop = "5px solid blue";
    return false;
}

function onScenarioDragLeave(event) {
    this.style.borderTop = "";
}

function onScenarioDrop(event) {
    commitProps();

    event.preventDefault();
    this.style.borderTop = "";
    document.getElementById("thumbnail-picture").src = "";

    var id = event.dataTransfer.getData("text/plain");
    var elemDrag = document.getElementById(id);

    // シナリオ項目の移動
    if(typeof elemDrag.template === "undefined") {
        this.parentNode.insertBefore(elemDrag, this);
        return;
    }

    // パレット/素材の挿入
    var newElem = createCommandElement(elemDrag.cmd);
    newElem.addEventListener("dragstart", onScenarioDragStart);
    newElem.addEventListener("dragover", onScenarioDragOver);
    newElem.addEventListener("dragleave", onScenarioDragLeave);
    newElem.addEventListener("drop", onScenarioDrop);
    newElem.classList.add("drag-list-item-sel");
    this.parentNode.insertBefore(newElem, this);

    // プロパティを表示する
    changeElement(newElem);
    showProps();
}

function onScenarioDragEnd(event) {
    document.getElementById("thumbnail-picture").src = "";
}

/*
 * プロパティビュー
 */

var elementInEdit = null;

function changeElement(elem) {
    elementInEdit = elem;
}

function hidePropsForElement(elem) {
    if(elementInEdit == elem) {
        Array.from(document.getElementById("prop-container").childNodes).forEach(function (e) {
            if(e.style != null) {
                e.style.display = "none";
            }
        });
    }
}

// 編集を開始する
function showProps() {
    // すべてのプロパティペインを非表示にする
    Array.from(document.getElementById("prop-container").childNodes).forEach(function (e) {
        if(e.style != null) {
            e.style.display = "none";
        }
    });
    if(elementInEdit == null) {
        return;
    }

    // コマンドの種類ごとに、要素に値を入れ、ペインを表示する
    cmd = elementInEdit.cmd;
    if(cmd.startsWith("@bg ") || cmd.startsWith("@背景 ")) {
        // @bg編集開始
        var cl = normalizeBg(cmd);
        document.getElementById("prop-bg-file").value = cl[1];
        document.getElementById("prop-bg-duration").value = cl[2];
        document.getElementById("prop-bg-duration-num").textContent = cl[2];
        document.getElementById("prop-bg-effect").value = normalizeBgEffect(cl[3]);
        document.getElementById("prop-bg").style.display = "block";
        if(cl[1] === "#ffffff") {
            document.getElementById("thumbnail-picture").src = "../img/white.png";
        } else if(cl[1] === "#000000") {
            document.getElementById("thumbnail-picture").src = "../img/black.png";
        } else if(!cl[1].startsWith("#")) {
            document.getElementById("thumbnail-picture").src = baseUrl + "bg/" + cl[1];
        }
    } else if(cmd.startsWith("@ch ") || cmd.startsWith("@キャラ ")) {
        // @ch編集開始
        var cl = normalizeCh(cmd);
        document.getElementById("prop-ch-position").value = normalizeChPosition(cl[1]);
        document.getElementById("prop-ch-file").value = (locale === "ja") ? japanizeChFile(cl[2]) : cl[2];
        document.getElementById("prop-ch-duration").value = cl[3];
        document.getElementById("prop-ch-duration-num").textContent = cl[3];
        document.getElementById("prop-ch-effect").value = normalizeBgEffect(cl[4]);
        document.getElementById("prop-ch-xshift").value = cl[5];
        document.getElementById("prop-ch-xshift-num").textContent = cl[5];
        document.getElementById("prop-ch-yshift").value = cl[6];
        document.getElementById("prop-ch-yshift-num").textContent = cl[6];
        document.getElementById("prop-ch-alpha").value = cl[7];
        document.getElementById("prop-ch-alpha-num").textContent = cl[7];
        document.getElementById("prop-ch").style.display = "block";
        if(cl[2] !== "none" && cl[2] !== "消去") {
            document.getElementById("thumbnail-picture").src = baseUrl + "ch/" + cl[2];
        } else {
            document.getElementById("thumbnail-picture").src = "";
        }
    } else if(cmd.startsWith("@bgm ") || cmd.startsWith("@音楽 ")) {
        // @bgm編集開始
        var cl = normalizeBgm(cmd);
        document.getElementById("prop-bgm-file").value = cl[1];
        document.getElementById("prop-bgm-once").checked = (cl[2] === "once");
        document.getElementById("prop-bgm").style.display = "block";
    } else if(cmd.startsWith("@se ") || cmd.startsWith("@効果音 ")) {
        // @se編集開始
        var cl = normalizeSe(cmd);
        document.getElementById("prop-se-file").value = cl[1];
        document.getElementById("prop-se-loop").checked = (cl[2] === "loop");
        document.getElementById("prop-se-voice").checked = (cl[2] === "voice");
        document.getElementById("prop-se").style.display = "block";
    } else if(cmd.startsWith("@vol ") || cmd.startsWith("@音量 ")) {
        // @vol編集開始
        var cl = normalizeVol(cmd);
        document.getElementById("prop-vol-track").value = cl[1];
        document.getElementById("prop-vol-volume").value = cl[2];
        document.getElementById("prop-vol-volume-num").textContent = cl[2];
        document.getElementById("prop-vol-duration").value = cl[3];
        document.getElementById("prop-vol-duration-num").textContent = cl[3];
        document.getElementById("prop-vol").style.display = "block";
    } else if(cmd.startsWith("@choose ") || cmd.startsWith("@選択肢 ")) {
        // @choose編集開始
        var cl = normalizeChoose(cmd);
        for(let i = 1; i <= 8; i++) {
            if(cl.length >= i + 2 + 1) {
                document.getElementById("prop-choose-label" + i).value = unquote(cl[i * 2 - 1]);
                document.getElementById("prop-choose-text" + i).value = unquote(cl[i * 2]);
            } else {
                document.getElementById("prop-choose-label" + i).value = "";
                document.getElementById("prop-choose-text" + i).value = "";
            }
        }
        document.getElementById("prop-choose").style.display = "block";
    } else if(cmd.startsWith("@cha ") || cmd.startsWith("@キャラ移動 ")) {
        // @cha編集開始
        var cl = normalizeCha(cmd);
        document.getElementById("prop-cha-position").value = cl[1];
        document.getElementById("prop-cha-duration").value = cl[2];
        document.getElementById("prop-cha-duration-num").textContent = cl[2];
        document.getElementById("prop-cha-acceleration").value = cl[3];
        document.getElementById("prop-cha-xoffset").value = cl[4];
        document.getElementById("prop-cha-xoffset-num").textContent = cl[4];
        document.getElementById("prop-cha-yoffset").value = cl[5];
        document.getElementById("prop-cha-yoffset-num").textContent = cl[5];
        document.getElementById("prop-cha-alpha").value = cl[6];
        document.getElementById("prop-cha-alpha-num").textContent = cl[6];
        document.getElementById("prop-cha").style.display = "block";
    } else if(cmd.startsWith("@chs ") || cmd.startsWith("@場面転換 ")) {
        // @chs編集開始
        var cl = normalizeChs(cmd);
        createChsChList("prop-chs-center");
        createChsChList("prop-chs-right");
        createChsChList("prop-chs-left");
        createChsChList("prop-chs-back");
        createChsBgList();
        document.getElementById("prop-chs-center").value = cl[1];
        document.getElementById("prop-chs-right").value = cl[2];
        document.getElementById("prop-chs-left").value = cl[3];
        document.getElementById("prop-chs-back").value = cl[4];
        document.getElementById("prop-chs-duration").value = cl[5];
        document.getElementById("prop-chs-duration-num").textContent = cl[5];
        document.getElementById("prop-chs-background").value = cl[6];
        document.getElementById("prop-chs-effect").value = cl[7];
        document.getElementById("prop-chs").style.display = "block";
    } else if(cmd.startsWith("@shake ") || cmd.startsWith("@振動 ")) {
        // @shake編集開始
        var cl = normalizeShake(cmd);
        document.getElementById("prop-shake-direction").value = cl[1];
        document.getElementById("prop-shake-duration").value = cl[2];
        document.getElementById("prop-shake-duration-num").textContent = cl[2];
        document.getElementById("prop-shake-times").value = cl[3];
        document.getElementById("prop-shake-times-num").textContent = cl[3];
        document.getElementById("prop-shake-amplitude").value = cl[4];
        document.getElementById("prop-shake-amplitude-num").textContent = cl[4];
        document.getElementById("prop-shake").style.display = "block";
    } else if(cmd.startsWith("@wait ") || cmd.startsWith("@時間待ち ")) {
        // @wait編集開始
        var cl = normalizeWait(cmd);
        document.getElementById("prop-wait-duration").value = cl[1];
        document.getElementById("prop-wait-duration-num").textContent = cl[1];
        document.getElementById("prop-wait").style.display = "block";
    } else if(cmd.startsWith("@skip ") || cmd.startsWith("@スキップ ")) {
        // @skip編集開始
        var cl = normalizeSkip(cmd);
        document.getElementById("prop-skip-opt").checked = (cl[1] === "disable") ? true : false;
        document.getElementById("prop-skip").style.display = "block";
    } else if(cmd.startsWith("@goto ") || cmd.startsWith("@ジャンプ ")) {
        // 選択肢を作成する
        createLabelOptions("prop-goto-label");

        // @goto編集開始
        var cl = normalizeGoto(cmd);
        document.getElementById("prop-goto-label").value = unquote(cl[1]);
        document.getElementById("prop-goto").style.display = "block";
    } else if(cmd.startsWith("@set ") || cmd.startsWith("@フラグをセット ")) {
        // 選択肢を作成する
        createFlagOptions("prop-set-variable");

        // @set編集開始
        var cl = normalizeSet(cmd);
        document.getElementById("prop-set-variable").value = cl[1];
        document.getElementById("prop-set-operator").value = cl[2];
        document.getElementById("prop-set-value").value = cl[3];
        document.getElementById("prop-set").style.display = "block";
    } else if(cmd.startsWith("@if ") || cmd.startsWith("@フラグでジャンプ ")) {
        // 選択肢を作成する
        createFlagOptions("prop-if-variable");
        createLabelOptions("prop-if-label");

        // @if編集開始
        var cl = normalizeIf(cmd);
        document.getElementById("prop-if-variable").value = cl[1];
        document.getElementById("prop-if-operator").value = cl[2];
        document.getElementById("prop-if-value").value = cl[3];
        document.getElementById("prop-if-label").value = cl[4];
        document.getElementById("prop-if").style.display = "block";
    } else if(cmd.startsWith("@load ") || cmd.startsWith("@シナリオ ")) {
        // @load編集開始
        var cl = normalizeLoad(cmd);
        document.getElementById("prop-load-file").value = cl[1];
        document.getElementById("prop-load").style.display = "block";
    } else if(cmd.startsWith("@chapter ") || cmd.startsWith("@章 ")) {
        // @chapter編集開始
        var cl = normalizeChapter(cmd);
        document.getElementById("prop-chapter-title").value = cl[1];
        document.getElementById("prop-chapter").style.display = "block";
    } else if(cmd.startsWith("@wms ") || cmd.startsWith("@スクリプト ")) {
        // @wms編集開始
        var cl = normalizeWms(cmd);
        document.getElementById("prop-wms-file").value = cl[1];
        document.getElementById("prop-wms").style.display = "block";
    } else if(cmd.startsWith("@gui ") || cmd.startsWith("@メニュー ")) {
        // @gui編集開始
        var cl = normalizeGui(cmd);
        document.getElementById("prop-gui-file").value = cl[1];
        document.getElementById("prop-gui").style.display = "block";
    } else if(cmd.startsWith("@video ") || cmd.startsWith("@動画 ")) {
        // @video編集開始
        var cl = normalizeVideo(cmd);
        document.getElementById("prop-video-file").value = cl[1];
        document.getElementById("prop-video").style.display = "block";
    } else if(cmd.startsWith("@")) {
        // 未対応のコマンド編集開始
    } else if(cmd.startsWith(":")) {
        // ラベル編集開始
        var label = cmd.substring(1);
        document.getElementById("prop-label-name").value = label;
        document.getElementById("prop-label").style.display = "block";
    } else if(cmd.startsWith("#")) {
        // コメント編集開始
        document.getElementById("prop-comment-text").value = cmd.substring(1);
        document.getElementById("prop-comment").style.display = "block";
    } else if(cmd === "") {
        // 空行編集開始(FIXME:なにもしない)
    } else if(cmd.match(/^\*[^\*]+\*[^\*]+$/)) {
        // セリフ(ボイスなし)編集開始
        var sp = cmd.split("*");
        var name = sp[1];
        var text = sp[2];
        document.getElementById("prop-serif-name").value = name;
        document.getElementById("prop-serif-text").value = text;
        document.getElementById("prop-serif-voice").value = "";
        document.getElementById("prop-serif").style.display = "block";
    } else if(cmd.match(/^\*[^\*]+\*[^\*]+\*[^\*]+$/)) {
        // セリフ(ボイスあり)編集開始
        var sp = cmd.split("*");
        var name = sp[1];
        var voice = sp[2];
        var text = sp[3];
        document.getElementById("prop-serif-name").value = name;
        document.getElementById("prop-serif-text").value = text;
        document.getElementById("prop-serif-voice").value = voice;
        document.getElementById("prop-serif").style.display = "block";
    } else if(cmd.match(/^.+「.*」$/)) {
        // セリフ(かぎカッコ)編集開始
        var name = cmd.split("「")[0];
        var text = cmd.split("「")[1].split("」")[0];
        document.getElementById("prop-serif-name").value = name;
        document.getElementById("prop-serif-text").value = text;
        document.getElementById("prop-serif-voice").value = "";
        document.getElementById("prop-serif").style.display = "block";
    } else {
        // メッセージ編集開始
        document.getElementById("prop-msg-text").value = cmd;
        document.getElementById("prop-msg").style.display = "block";
    }
}

function createLabelOptions(selectId) {
    // 一度子要素を削除する
    var parent = document.getElementById(selectId);
    while (parent.firstChild) {
        parent.removeChild(parent.firstChild);
    }

    // シナリオからラベルを集めてoptionにする
    Array.from(document.getElementById("scenario").childNodes).forEach(function (e) {
        if(typeof e.cmd !== "undefined" && e.cmd != null && e.cmd.startsWith(":")) {
            var option = document.createElement("option");
            option.value = e.cmd.substring(1);
            option.textContent = option.value;
            document.getElementById(selectId).appendChild(option);
        }
    });
}

async function createFlagOptions(selectId) {
    // 一度子要素を削除する
    var parent = document.getElementById(selectId);
    while (parent.firstChild) {
        parent.removeChild(parent.firstChild);
    }

    // フラグをoptionにする
    for(let index of Object.keys(flagList)) {
        if(index === "" || index === "length" || flagList[index] === "") {
            continue;
        }

        var option = document.createElement("option");
        option.value = "$" + index;
        option.textContent = flagList[index];
        parent.appendChild(option);
    }
}

async function createChsChList(id) {
    var parentElem = document.getElementById(id);
    while (parentElem.firstChild) {
        parentElem.removeChild(parentElem.firstChild);
    }

    var stayElem = document.createElement('option');
    stayElem.value = "stay";
    stayElem.textContent = translate("変更なし");
    parentElem.appendChild(stayElem);

    var noneElem = document.createElement('option');
    noneElem.value = "none";
    noneElem.textContent = translate("消去");
    parentElem.appendChild(noneElem);

    var ch = await window.api.getChList();
    ch.forEach(function(file) {
        var fileElem = document.createElement('option');
        fileElem.value = file;
        fileElem.textContent = file;
        parentElem.appendChild(fileElem);
    });
}

async function createChsBgList(id) {
    var parentElem = document.getElementById("prop-chs-background");
    while (parentElem.firstChild) {
        parentElem.removeChild(parentElem.firstChild);
    }

    var stayElem = document.createElement('option');
    stayElem.value = "stay";
    stayElem.textContent = translate("変更なし");
    parentElem.appendChild(stayElem);

    var bg = await window.api.getBgList();
    bg.forEach(function(file) {
        var fileElem = document.createElement('option');
        fileElem.value = file;
        fileElem.textContent = file;
        parentElem.appendChild(fileElem);
    });
}

// 変更を保存する
function commitProps() {
    if(elementInEdit == null) {
        return;
    }

    // コマンドの種類ごとに、要素から値を出し、シナリオ要素に反映する
    cmd = elementInEdit.cmd;
    if(cmd.startsWith("@bg ") || cmd.startsWith("@背景 ")) {
        // @bg保存
        var file = document.getElementById("prop-bg-file").value;
        var duration = document.getElementById("prop-bg-duration").value;
        var effect = normalizeBgEffect(document.getElementById("prop-bg-effect").value);
        elementInEdit.cmd = "@bg " + quote(file) + " " + duration + " " + effect;
    } else if(cmd.startsWith("@ch ") || cmd.startsWith("@キャラ ")) {
        // @ch保存
        var position = normalizeChPosition(document.getElementById("prop-ch-position").value);
        var file = normalizeChFile(document.getElementById("prop-ch-file").value);
        var duration = document.getElementById("prop-ch-duration").value;
        var effect = normalizeBgEffect(document.getElementById("prop-ch-effect").value);
        var xshift = document.getElementById("prop-ch-xshift").value;
        var yshift = document.getElementById("prop-ch-yshift").value;
        var alpha = document.getElementById("prop-ch-alpha").value;
        elementInEdit.cmd = "@ch " + position + " " + quote(file) + " " + duration + " " + effect + " " + xshift + " " + yshift + " " + alpha;
    } else if(cmd.startsWith("@bgm ") || cmd.startsWith("@音楽 ")) {
        // @bgm保存
        var file = document.getElementById("prop-bgm-file").value;
        var once = document.getElementById("prop-bgm-once").checked;
        if(!once) {
            elementInEdit.cmd = "@bgm " + quote(file);
        } else {
            elementInEdit.cmd = "@bgm " + quote(file) + " once";
        }
    } else if(cmd.startsWith("@se ") || cmd.startsWith("@効果音 ")) {
        // @se保存
        var file = document.getElementById("prop-se-file").value;
        var loop = document.getElementById("prop-se-loop").checked;
        var voice = document.getElementById("prop-se-voice").checked;
        if(!loop && !voice) {
            elementInEdit.cmd = "@se " + quote(file);
        } else if(loop) {
            elementInEdit.cmd = "@se " + quote(file) + " loop";
        } else if(voice) {
            elementInEdit.cmd = "@se " + quote(file) + " voice";
        }
    } else if(cmd.startsWith("@vol ") || cmd.startsWith("@音量 ")) {
        // @vol保存
        var track = document.getElementById("prop-vol-track").value;
        var volume = document.getElementById("prop-vol-volume").value;
        var duration = document.getElementById("prop-vol-duration").value;
        elementInEdit.cmd = "@vol " + track + " " + volume + " " + duration;
    } else if(cmd.startsWith("@choose ") || cmd.startsWith("@選択肢 ")) {
        // @choose保存
        var label = [];
        var text = [];
        for(let i = 0; i < 8; i++) {
            label[i] = document.getElementById("prop-choose-label" + (i+1)).value;
            text[i] = document.getElementById("prop-choose-text" + (i+1)).value;
        }
        var c = "@choose";
        for(let i = 0; i < 8; i++) {
            if(label[i] === "" || text[i] === "") {
                break;
            }
            c = c + " " + quote(label[i]) + " " + quote(text[i]);
        }
        elementInEdit.cmd = c;
    } else if(cmd.startsWith("@cha ") || cmd.startsWith("@キャラ移動 ")) {
        // @cha保存
        var position = document.getElementById("prop-cha-position").value;
        var duration = document.getElementById("prop-cha-duration").value;
        var acceleration = document.getElementById("prop-cha-acceleration").value;
        var xoffset = document.getElementById("prop-cha-xoffset").value;
        var yoffset = document.getElementById("prop-cha-yoffset").value;
        var alpha = document.getElementById("prop-cha-alpha").value;
        elementInEdit.cmd = "@cha " + position + " " + duration + " " + acceleration + " " + xoffset + " " + yoffset + " " + alpha;
    } else if(cmd.startsWith("@chs ") || cmd.startsWith("@場面転換 ")) {
        // @chs保存
        var center = document.getElementById("prop-chs-center").value;
        var right = document.getElementById("prop-chs-right").value;
        var left = document.getElementById("prop-chs-left").value;
        var back = document.getElementById("prop-chs-back").value;
        var duration = document.getElementById("prop-chs-duration").value;
        var background = document.getElementById("prop-chs-background").value;
        var effect = document.getElementById("prop-chs-effect").value;
        elementInEdit.cmd = "@chs " + quote(center) + " " + quote(right) + " " + quote(left) + " " + quote(back) + " " + duration + " " + quote(background) + " " + effect;
    } else if(cmd.startsWith("@shake ") || cmd.startsWith("@振動 ")) {
        // @shake保存
        var direction = document.getElementById("prop-shake-direction").value;
        var duration = document.getElementById("prop-shake-duration").value;
        var times = document.getElementById("prop-shake-times").value;
        var amplitude = document.getElementById("prop-shake-amplitude").value;
        elementInEdit.cmd = "@shake " + direction + " " + duration + " " + times + " " + amplitude;
    } else if(cmd.startsWith("@wait ") || cmd.startsWith("@時間待ち ")) {
        // @wait保存
        var duration = document.getElementById("prop-wait-duration").value;
        elementInEdit.cmd = "@wait " + duration;
        elementInEdit.textContent = translate("一定時間待つ ") + duration + translate("秒");
    } else if(cmd.startsWith("@skip ") || cmd.startsWith("@スキップ ")) {
        // @skip保存
        var opt = document.getElementById("prop-skip-opt").checked ? "disable" : "enable";
        elementInEdit.cmd = "@skip " + opt;
        elementInEdit.textContent = translate("スキップ") + (opt === "disable" ? translate("禁止") : translate("許可"));
    } else if(cmd.startsWith("@goto ") || cmd.startsWith("@ジャンプ ")) {
        // @goto保存
        var label = document.getElementById("prop-goto-label").value;
        elementInEdit.cmd = "@goto " + quote(label);
        elementInEdit.textContent = translate("ジャンプ \"") + label + "\"へ";
    } else if(cmd.startsWith("@set ") || cmd.startsWith("@フラグをセット ")) {
        // @set保存
        var variable = document.getElementById("prop-set-variable").value;
        var operator = document.getElementById("prop-set-operator").value;
        var value = document.getElementById("prop-set-value").value;
        elementInEdit.cmd = "@set " + variable + " " + operator + " " + value;
    } else if(cmd.startsWith("@if ") || cmd.startsWith("@フラグでジャンプ ")) {
        // @if保存
        var variable = document.getElementById("prop-if-variable").value;
        var operator = document.getElementById("prop-if-operator").value;
        var value = document.getElementById("prop-if-value").value;
        var label = document.getElementById("prop-if-label").value;
        label = label === "" ? "ジャンプ先の目印を指定してください" : label;
        elementInEdit.cmd = "@if " + variable + " " + operator + " " + value + " " + quote(label);
    } else if(cmd.startsWith("@load ") || cmd.startsWith("@シナリオ ")) {
        // @load保存
        var file = document.getElementById("prop-load-file").value;
        elementInEdit.cmd = "@load " + quote(file);
    } else if(cmd.startsWith("@chapter ") || cmd.startsWith("@章 ")) {
        // @chapter保存
        var title = document.getElementById("prop-chapter-title").value;
        elementInEdit.cmd = "@chapter " + quote(title);
        elementInEdit.textContent = translate("章のタイトル \"") + title + "\"";
    } else if(cmd.startsWith("@wms ") || cmd.startsWith("@スクリプト ")) {
        // @wms保存
        var file = document.getElementById("prop-wms-file").value;
        elementInEdit.cmd = "@wms " + quote(file);
    } else if(cmd.startsWith("@gui ") || cmd.startsWith("@メニュー ")) {
        // @gui保存
        var file = document.getElementById("prop-gui-file").value;
        elementInEdit.cmd = "@gui " + quote(file);
    } else if(cmd.startsWith("@video ") || cmd.startsWith("@動画 ")) {
        // @video保存
        var file = document.getElementById("prop-video-file").value;
        elementInEdit.cmd = "@video " + quote(file);
    } else if (cmd.startsWith("@")) {
        // 未対応のコマンド保存
    } else if (cmd.startsWith(":")) {
        // ラベル保存
        var label = document.getElementById("prop-label-name").value;
        elementInEdit.cmd = ":" + label;
        elementInEdit.textContent = translate("目印 \"") + label + "\"";
    } else if(cmd.startsWith("#")) {
        // コメント保存
        var comment = document.getElementById("prop-comment-text").value;
        elementInEdit.cmd = "#" + comment;
        elementInEdit.textContent = comment;
        if(elementInEdit.textContent === "") {
            elementInEdit.textContent = "　";
        }
    } else if(cmd.length === 0) {
        // 空行保存
        elementInEdit.cmd = "";
        elementInEdit.textContent = "　";
    } else if(cmd.match(/^\*[^\*]+\*[^\*]+$/) || cmd.match(/^\*[^\*]+\*[^\*]+\*[^\*]+$/) || cmd.match(/^.+「.*」$/)) {
        // セリフ保存
        var name = document.getElementById("prop-serif-name").value;
        var text = document.getElementById("prop-serif-text").value;
        var voice = document.getElementById("prop-serif-voice").value;
        if(name === "") {
            name = translate("名前を入力してください");
        }
        if(text === "") {
            text = translate("セリフを入力してください");
        }
        if(voice === "") {
            elementInEdit.cmd = "*" + name + "*" + text;
        } else {
            elementInEdit.cmd = "*" + name + "*" + voice + "*" + text;
        }
        elementInEdit.textContent = name + translate("「") + text + translate("」");
    } else {
        // メッセージ保存
        var msg = document.getElementById("prop-msg-text").value;
        if(msg === "") {
            msg = translate("文章を入力してください");
        }
        msg = msg.replace(/\n/g, "\\n");
        elementInEdit.cmd = msg;
        elementInEdit.textContent = msg;
    }
}

function quote(s) {
    if(s.indexOf(" ") != -1) {
        return "\"" + s + "\"";
    }
    return s;
}

function unquote(s) {
    if(s.startsWith("\"") && s.endsWith("\"") && s.length >= 2) {
        return s.substring(1, s.length - 1);
    }
    return s;
}

/*
 * パレットビュー
 */

function setupPalette() {
    Array.from(document.getElementById("palette").childNodes).forEach(function (elem) {
        if(typeof elem.id === "undefined") {
            return;
        }
        switch(elem.id) {
        case "cmd-message": elem.cmd = translate("ここに文章を入力してください。"); break;
        case "cmd-serif": elem.cmd = translate("キャラ名「セリフを入力してください」"); break;
        case "cmd-choose": elem.cmd = translate("@choose 目印1 学校へ行く 目印2 海へ行く 目印3 公園へ行く"); break;
        case "cmd-chs": elem.cmd = "@chs stay stay stay stay 1.0 stay normal"; break;
        case "cmd-vol": elem.cmd = "@vol bgm 1.0 1.0"; break;
        case "cmd-cha": elem.cmd = "@cha center 1.0 move 100 0 show"; break;
        case "cmd-label": elem.cmd = translate(":名前をつけてください"); break;
        case "cmd-goto": elem.cmd = translate("@goto 目印を選んでください"); break;
        case "cmd-set": elem.cmd = "@set $0 = 1"; break;
        case "cmd-if": elem.cmd = translate("@if $0 == 1 目印を選んでください"); break;
        case "cmd-chapter": elem.cmd = translate("@chapter 章のタイトル"); break;
        case "cmd-click": elem.cmd = "@click"; break;
        case "cmd-wait": elem.cmd = "@wait 1.0"; break;
        case "cmd-shake": elem.cmd = "@shake horizontal 3 3 100"; break;
        case "cmd-skip": elem.cmd = "@skip enable"; break;
        case "cmd-wms": elem.cmd = translate("@wms ファイルを指定してください"); break;
        case "cmd-comment": elem.cmd = translate("#ここにメモを記入してください"); break;
        }
        elem.template = true;
        elem.addEventListener("dragstart", () => {
            event.dataTransfer.setData("text/plain", event.target.id);
            event.dataTransfer.setData("palette", "");
            return true;
        });
        elem.addEventListener("click", () => {
            Array.from(document.getElementById("palette").childNodes).forEach(function (c) {
                if(c.classList != null && (c.classList.contains("palette-list-item") || c.classList.contains("palette-list-item-sel"))) {
                    if(c === event.srcElement) {
                        c.classList.add("palette-list-item-sel");
                        c.classList.remove("palette-list-item");
                    } else {
                        c.classList.remove("palette-list-item-sel");
                        c.classList.add("palette-list-item");
                    }
                }
            });
            document.getElementById("thumbnail-picture").src = "";
        });
    });
}

/*
 * txtビュー
 */

async function refreshTxt() {
    var element = document.getElementById("txt-list");
    while (element.firstChild) {
        element.removeChild(element.firstChild);
    }

    const txt = await window.api.getTxtList();
    txt.forEach(function(file) {
        var elem = document.createElement('li');
        elem.id = makeId();
        elem.textContent = file;
        elem.draggable = "true";
        elem.className = "tab-list-item";
        elem.template = true;
        elem.cmd = "@load " + file;
        elem.addEventListener("click", () => {
            Array.from(document.getElementById("txt-list").childNodes).forEach(function (e) {
                if(e.classList != null && (e.classList.contains("tab-list-item") || e.classList.contains("tab-list-item-sel"))) {
                    if(e === event.srcElement) {
                        e.classList.add("tab-list-item-sel");
                        e.classList.remove("tab-list-item");
                    } else {
                        e.classList.remove("tab-list-item-sel");
                        e.classList.add("tab-list-item");
                    }
                }
            });
            document.getElementById("thumbnail-picture").src = "";
        });
        elem.addEventListener("dblclick", async () => {
            var cl = normalizeLoad(event.srcElement.cmd);
            commitProps();
            await saveScenario();
            await window.api.openScenario(cl[1]);
            refreshScenario();
            document.getElementById("thumbnail-picture").src = "";
        });
        elem.addEventListener("dragstart", () => {
            event.dataTransfer.setData("text/plain", event.target.id);
            document.getElementById("thumbnail-picture").src = "../img/trash-close.png";
            return true;
        });
        elem.addEventListener("dragend", () => {
            document.getElementById("thumbnail-picture").src = "";
        });
        document.getElementById("txt-list").appendChild(elem);
    });
}

function makeId() {
    return new Date().getTime().toString(16) + Math.floor(1000 * Math.random()).toString(16);
}

function setupTxt() {
    var btnTxt = document.getElementById("btn-txt");
    btnTxt.addEventListener("dblclick", async (e) => {
        window.api.openTxtFolder();
    });

    var txtPanel = document.getElementById("tab-panel-txt");
    txtPanel.addEventListener('dragover', (e) => {
        if(!event.dataTransfer.types.includes("scenario") &&
           !event.dataTransfer.types.includes("palette")) {
            e.preventDefault();
            e.stopPropagation();
            document.getElementById("tab-panel-txt").classList.add('dragover');
            return true;
        }
        return false;
    });
    txtPanel.addEventListener('dragleave', (e) => {
        e.preventDefault();
        e.stopPropagation();
        document.getElementById("tab-panel-txt").classList.remove('dragover');
    });
    txtPanel.addEventListener('drop', async (e) => {
        e.preventDefault();
        e.stopPropagation();
        document.getElementById("tab-panel-txt").classList.remove('dragover');
        document.getElementById("thumbnail-picture").src = "";
        for (const file of e.dataTransfer.files) {
            await window.api.addTxtFile(file.path);
        }
        refreshTxt();
    });
}

/*
 * bgビュー
 */

async function refreshBg() {
    var element = document.getElementById("bg-list");
    while (element.firstChild) {
        element.removeChild(element.firstChild);
    }

    const bg = await window.api.getBgList();
    bg.unshift("#ffffff");
    bg.unshift("#000000");
    bg.forEach(function(file) {
        var elem = document.createElement('li');
        elem.id = makeId();
        if(file === "#ffffff") {
            elem.textContent = translate("白");
        } else if(file === "#000000") {
            elem.textContent = translate("黒");
        } else {
            elem.textContent = file;
        }
        elem.draggable = "true";
        elem.className = "tab-list-item";
        elem.template = true;
        elem.cmd = "@bg " + file + " 1.0";
        if(file.startsWith("#")) {
            elem.donotremove = true;
        }
        elem.addEventListener("click", async () => {
            Array.from(document.getElementById("bg-list").childNodes).forEach(function (e) {
                if(e.classList != null && (e.classList.contains("tab-list-item") || e.classList.contains("tab-list-item-sel"))) {
                    if(e === event.srcElement) {
                        e.classList.add("tab-list-item-sel");
                        e.classList.remove("tab-list-item");
                    } else {
                        e.classList.remove("tab-list-item-sel");
                        e.classList.add("tab-list-item");
                    }
                }
            });
            if(!file.startsWith("#")) {
                document.getElementById("thumbnail-picture").src = baseUrl + "bg/" + file;
            } else if(file === "#ffffff") {
                document.getElementById("thumbnail-picture").src = "../img/white.png";
            } else if(file === "#000000") {
                document.getElementById("thumbnail-picture").src = "../img/black.png";
            }
        });
        elem.addEventListener("dragstart", () => {
            event.dataTransfer.setData("text/plain", event.target.id);
            if(typeof event.target.donotremove === "undefined") {
                document.getElementById("thumbnail-picture").src = "../img/trash-close.png";
            }
            return true;
        });
        elem.addEventListener("dragend", () => {
            document.getElementById("thumbnail-picture").src = "";
        });
        document.getElementById("bg-list").appendChild(elem);
    });
}

function setupBg() {
    var btnBg = document.getElementById("btn-bg");
    btnBg.addEventListener("dblclick", async (e) => {
        window.api.openBgFolder();
    });

    var bgPanel = document.getElementById("tab-panel-bg");
    bgPanel.addEventListener('dragover', (e) => {
        if(!event.dataTransfer.types.includes("scenario") &&
           !event.dataTransfer.types.includes("palette")) {
            e.preventDefault();
            e.stopPropagation();
            document.getElementById("tab-panel-bg").classList.add('dragover');
            return true;
        }
        return false;
    });
    bgPanel.addEventListener('dragleave', (e) => {
        e.preventDefault();
        e.stopPropagation();
        document.getElementById("tab-panel-bg").classList.remove('dragover');
    });
    bgPanel.addEventListener('drop', async (e) => {
        e.preventDefault();
        e.stopPropagation();
        document.getElementById("tab-panel-bg").classList.remove('dragover');
        document.getElementById("thumbnail-picture").src = "";
        for (const file of e.dataTransfer.files) {
            await window.api.addBgFile(file.path);
        }
        refreshBg();
    });
}

/*
 * chビュー
 */

async function refreshCh() {
    var element = document.getElementById("ch-list");
    while (element.firstChild) {
        element.removeChild(element.firstChild);
    }

    var ch = await window.api.getChList();
    ch.unshift(translate("消去"));
    ch.forEach(function(file) {
        var elem = document.createElement('li');
        elem.id = makeId();
        elem.textContent = file;
        elem.draggable = "true";
        elem.className = "tab-list-item";
        elem.template = true;
        elem.cmd = "@ch center " + file + " 1.0";
        elem.addEventListener("click", async () => {
            Array.from(document.getElementById("ch-list").childNodes).forEach(function (e) {
                if(e.classList != null && (e.classList.contains("tab-list-item") || e.classList.contains("tab-list-item-sel"))) {
                    if(e === event.srcElement) {
                        e.classList.add("tab-list-item-sel");
                        e.classList.remove("tab-list-item");
                    } else {
                        e.classList.remove("tab-list-item-sel");
                        e.classList.add("tab-list-item");
                    }
                }
            });
            if(file !== "none" && file !== "消去") {
                document.getElementById("thumbnail-picture").src = baseUrl + "ch/" + file;
            } else {
                document.getElementById("thumbnail-picture").src = "";
            }
        });
        elem.addEventListener("dragstart", () => {
            event.dataTransfer.setData("text/plain", event.target.id);
            document.getElementById("thumbnail-picture").src = "../img/trash-close.png";
            return true;
        });
        elem.addEventListener("dragend", () => {
            document.getElementById("thumbnail-picture").src = "";
        });
        document.getElementById("ch-list").appendChild(elem);
    });
}

function setupCh() {
    var btnCh = document.getElementById("btn-ch");
    btnCh.addEventListener("dblclick", async (e) => {
        window.api.openChFolder();
    });

    var chPanel = document.getElementById("tab-panel-ch");
    chPanel.addEventListener('dragover', (e) => {
        if(!event.dataTransfer.types.includes("scenario") &&
           !event.dataTransfer.types.includes("palette")) {
            e.preventDefault();
            e.stopPropagation();
            document.getElementById("tab-panel-ch").classList.add('dragover');
            return true;
        }
        return false;
    });
    chPanel.addEventListener('dragleave', (e) => {
        e.preventDefault();
        e.stopPropagation();
        document.getElementById("tab-panel-ch").classList.remove('dragover');
    });
    chPanel.addEventListener('drop', async (e) => {
        e.preventDefault();
        e.stopPropagation();
        document.getElementById("tab-panel-ch").classList.remove('dragover');
        document.getElementById("thumbnail-picture").src = "";
        for (const file of e.dataTransfer.files) {
            await window.api.addChFile(file.path);
        }
        refreshCh();
    });
}

/*
 * bgmビュー
 */

async function refreshBgm() {
    var element = document.getElementById("bgm-list");
    while (element.firstChild) {
        element.removeChild(element.firstChild);
    }

    const bgm = await window.api.getBgmList();
    bgm.unshift(translate("停止"));
    bgm.forEach(function(file) {
        var elem = document.createElement('li');
        elem.id = makeId();
        elem.textContent = file;
        elem.draggable = "true";
        elem.className = "tab-list-item";
        elem.template = true;
        elem.cmd = "@bgm " + file;
        elem.addEventListener("click", async () => {
            Array.from(document.getElementById("bgm-list").childNodes).forEach(function (e) {
                if(e.classList != null && (e.classList.contains("tab-list-item") || e.classList.contains("tab-list-item-sel"))) {
                    if(e === event.srcElement) {
                        e.classList.add("tab-list-item-sel");
                        e.classList.remove("tab-list-item");
                    } else {
                        e.classList.remove("tab-list-item-sel");
                        e.classList.add("tab-list-item");
                    }
                }
            });
            document.getElementById("thumbnail-picture").src = "";
        });
        elem.addEventListener("dragstart", () => {
            event.dataTransfer.setData("text/plain", event.target.id);
            document.getElementById("thumbnail-picture").src = "../img/trash-close.png";
            return true;
        });
        elem.addEventListener("dragend", () => {
            document.getElementById("thumbnail-picture").src = "";
        });
        document.getElementById("bgm-list").appendChild(elem);
    });
}

function setupBgm() {
    var btnBgm = document.getElementById("btn-bgm");
    btnBgm.addEventListener("dblclick", async (e) => {
        window.api.openBgmFolder();
    });

    var bgmPanel = document.getElementById("tab-panel-bgm");
    bgmPanel.addEventListener('dragover', (e) => {
        if(!event.dataTransfer.types.includes("scenario") &&
           !event.dataTransfer.types.includes("palette")) {
            e.preventDefault();
            e.stopPropagation();
            document.getElementById("tab-panel-bgm").classList.add('dragover');
            return true;
        }
        return false;
    });
    bgmPanel.addEventListener('dragleave', (e) => {
        e.preventDefault();
        e.stopPropagation();
        document.getElementById("tab-panel-bgm").classList.remove('dragover');
    });
    bgmPanel.addEventListener('drop', async (e) => {
        e.preventDefault();
        e.stopPropagation();
        document.getElementById("tab-panel-bgm").classList.remove('dragover');
        document.getElementById("thumbnail-picture").src = "";
        for (const file of e.dataTransfer.files) {
            await window.api.addBgmFile(file.path);
        }
        refreshBgm();
    });
}

/*
 * seビュー
 */

async function refreshSe() {
    var element = document.getElementById("se-list");
    while (element.firstChild) {
        element.removeChild(element.firstChild);
    }

    var se = await window.api.getSeList();
    se.unshift(translate("停止"));
    se.forEach(function(file) {
        var elem = document.createElement('li');
        elem.id = makeId();
        elem.textContent = file;
        elem.draggable = "true";
        elem.className = "tab-list-item";
        elem.template = true;
        elem.cmd = "@se " + file;
        elem.addEventListener("click", async () => {
            Array.from(document.getElementById("se-list").childNodes).forEach(function (e) {
                if(e.classList != null && (e.classList.contains("tab-list-item") || e.classList.contains("tab-list-item-sel"))) {
                    if(e === event.srcElement) {
                        e.classList.add("tab-list-item-sel");
                        e.classList.remove("tab-list-item");
                    } else {
                        e.classList.remove("tab-list-item-sel");
                        e.classList.add("tab-list-item");
                    }
                }
            });
            document.getElementById("thumbnail-picture").src = "";
        });
        elem.addEventListener("dragstart", () => {
            event.dataTransfer.setData("text/plain", event.target.id);
            document.getElementById("thumbnail-picture").src = "../img/trash-close.png";
            return true;
        });
        elem.addEventListener("dragend", () => {
            document.getElementById("thumbnail-picture").src = "";
        });
        document.getElementById("se-list").appendChild(elem);
    });
}

function setupSe() {
    var btnSe = document.getElementById("btn-se");
    btnSe.addEventListener("dblclick", async (e) => {
        window.api.openSeFolder();
    });

    var sePanel = document.getElementById("tab-panel-se");
    sePanel.addEventListener('dragover', (e) => {
        if(!event.dataTransfer.types.includes("scenario") &&
           !event.dataTransfer.types.includes("palette")) {
            e.preventDefault();
            e.stopPropagation();
            document.getElementById("tab-panel-se").classList.add('dragover');
            return true;
        }
        return false;
    });
    sePanel.addEventListener('dragleave', (e) => {
        e.preventDefault();
        e.stopPropagation();
        document.getElementById("tab-panel-se").classList.remove('dragover');
    });
    sePanel.addEventListener('drop', async (e) => {
        e.preventDefault();
        e.stopPropagation();
        document.getElementById("tab-panel-se").classList.remove('dragover');
        document.getElementById("thumbnail-picture").src = "";
        for (const file of e.dataTransfer.files) {
            await window.api.addSeFile(file.path);
        }
        refreshSe();
    });
}

/*
 * movビュー
 */

async function refreshMov() {
    var element = document.getElementById("mov-list");
    while (element.firstChild) {
        element.removeChild(element.firstChild);
    }

    const se = await window.api.getMovList();
    se.forEach(function(file) {
        var elem = document.createElement('li');
        elem.id = makeId();
        elem.textContent = file;
        elem.draggable = "true";
        elem.className = "tab-list-item";
        elem.template = true;
        elem.cmd = "@video " + file;
        elem.addEventListener("click", async () => {
            Array.from(document.getElementById("mov-list").childNodes).forEach(function (e) {
                if(e.classList != null && (e.classList.contains("tab-list-item") || e.classList.contains("tab-list-item-sel"))) {
                    if(e === event.srcElement) {
                        e.classList.add("tab-list-item-sel");
                        e.classList.remove("tab-list-item");
                    } else {
                        e.classList.remove("tab-list-item-sel");
                        e.classList.add("tab-list-item");
                    }
                }
            });
            document.getElementById("thumbnail-picture").src = "";
        });
        elem.addEventListener("dragstart", () => {
            event.dataTransfer.setData("text/plain", event.target.id);
            document.getElementById("thumbnail-picture").src = "../img/trash-close.png";
            return true;
        });
        elem.addEventListener("dragend", () => {
            document.getElementById("thumbnail-picture").src = "";
        });
        document.getElementById("mov-list").appendChild(elem);
    });
}

function setupMov() {
    var btnMov = document.getElementById("btn-mov");
    btnMov.addEventListener("dblclick", async (e) => {
        window.api.openMovFolder();
    });

    var movPanel = document.getElementById("tab-panel-mov");
    movPanel.addEventListener('dragover', (e) => {
        if(!event.dataTransfer.types.includes("scenario") &&
           !event.dataTransfer.types.includes("palette")) {
            e.preventDefault();
            e.stopPropagation();
            document.getElementById("tab-panel-mov").classList.add('dragover');
            return true;
        }
        return false;
    });
    movPanel.addEventListener('dragleave', (e) => {
        e.preventDefault();
        e.stopPropagation();
        document.getElementById("tab-panel-mov").classList.remove('dragover');
    });
    movPanel.addEventListener('drop', async (e) => {
        e.preventDefault();
        e.stopPropagation();
        document.getElementById("tab-panel-mov").classList.remove('dragover');
        document.getElementById("thumbnail-picture").src = "";
        for (const file of e.dataTransfer.files) {
            await window.api.addMovFile(file.path);
        }
        refreshMov();
    });
}

/*
 * コマンドの正規化
 *  - コマンドをトークナイズし、UIで使いやすいように変換して、配列を返す
 *  - コマンド名は英語に変換される
 *  - 引数の名前は省略される
 *  - UIのスライダーで調整される引数が省略された場合、省略可能でも数値に変換する
 *  - 引数として受容できない値が指定されていれば、エラー値かデフォルト値に変換される
 *  - 必須引数が指定されていなければエラー用の値が設定される
 */

const MSG_SPECIFY_FILE = "ファイルを指定してください";
const MSG_SPECIFY_LABEL = "行き先を指定してください";
const MSG_SPECIFY_OPTION = "選択肢を指定してください";

// @bg
function normalizeBg(command) {
    var op = "@bg";
    var file = "";
    var duration = "";
    var effect = "";

    // トークナイズする
    var tokens = command.match(/(".*?"|[^"\s]+)+(?=\s*|\s*$)/g);
    if(tokens.length >= 2) {
        file = normalizeParameter(tokens[1], ["file=", "ファイル="], MSG_SPECIFY_FILE);
    }
    if(tokens.length >= 3) {
        duration = normalizeParameter(tokens[2], ["duration=", "秒="], "0");
    }
    if(tokens.length >= 4) {
        effect = normalizeParameter(tokens[3], ["effect=", "エフェクト="], "normal");
        effect = normalizeBgEffect(effect);
    }

    // バリデーションする
    if(file === "") {
        file = translate(MSG_SPECIFY_FILE);
    }
    if(duration === "") {
        duration = "0.0";
    }
    if(effect === "") {
        effect = "normal";
    }

    return [op, file, duration, effect];
}

//
// 引数を正規化し、引数名の引数名のないフォーマットに変換する
//  - token: 正規化するトークン
//  - names: 受容する引数名の配列
//  - default: 引数名が指定されたが値が空白だった場合にフォールバックする値
//
function normalizeParameter(token, names, def) {
    for(const name of names) {
        var ret = "";
        if(token.startsWith(name)) {
            ret = token.substring(name.length);
            if(ret === "") {
                return def;
            } else {
                return ret;
            }
        } else {
            return token;
        }
    }
    return name;
}

function normalizeBgEffect(effect) {
    switch(effect) {
    case "標準":            return "normal";
    case "normal":          return "normal";
    case "n":               return "normal";
    case "右カーテン":      return "curtain-right";
    case "c":               return "curtain-right";
    case "curtain":         return "curtain-right";
    case "curtain-right":   return "curtain-right";
    case "左カーテン":      return "curtain-left";
    case "curtain-left":    return "curtain-left";
    case "上カーテン":      return "curtain-up";
    case "curtain-up":      return "curtain-up";
    case "下カーテン":      return "curtain-down";
    case "curtain-down":    return "curtain-down";
    default:                break;
    }
    return "normal";
}

function japanizeBgEffect(effect) {
    switch(effect) {
    case "標準":            return "標準";
    case "normal":          return "標準";
    case "n":               return "標準";
    case "右カーテン":      return "右カーテン";
    case "c":               return "右カーテン";
    case "curtain":         return "右カーテン";
    case "curtain-right":   return "右カーテン";
    case "左カーテン":      return "左カーテン";
    case "curtain-left":    return "左カーテン";
    case "上カーテン":      return "上カーテン";
    case "curtain-up":      return "上カーテン";
    case "下カーテン":      return "下カーテン";
    case "curtain-down":    return "下カーテン";
    default:                break;
    }
    return "標準";
}

// @ch
function normalizeCh(command) {
    var op = "@ch";
    var position = "";
    var file = "";
    var duration = "";
    var effect = "";
    var xshift = "";
    var yshift = "";
    var alpha = "";

    // トークナイズする
    var tokens = command.match(/(".*?"|[^"\s]+)+(?=\s*|\s*$)/g);
    if(tokens.length >= 2) {
        position = normalizeParameter(tokens[1], ["position=", "位置"], "center");
        position = normalizeChPosition(position);
    }
    if(tokens.length >= 3) {
        file = normalizeParameter(tokens[2], ["file=", "ファイル="], "ファイルを指定してください");
    }
    if(tokens.length >= 4) {
        duration = normalizeParameter(tokens[3], ["duration=", "秒="], "1.0");
    }
    if(tokens.length >= 5) {
        effect = normalizeParameter(tokens[4], ["effect=", "エフェクト="], "normal");
        effect = normalizeBgEffect(effect);
    }
    if(tokens.length >= 6) {
        xshift = normalizeParameter(tokens[5], ["right=", "右="], "");
    }
    if(tokens.length >= 7) {
        yshift = normalizeParameter(tokens[6], ["down=", "下="], "");
    }
    if(tokens.length >= 8) {
        alpha = normalizeParameter(tokens[7], ["alpha=", "アルファ="], "");
        alpha = normalizeChAlpha(alpha);
    }

    // バリデーションする
    if(position === "") {
        position = "center";
    }
    if(file === "") {
        file = translate(MSG_SPECIFY_FILE);
    }
    if(duration === "") {
        duration = "0.0";
    }
    if(effect === "") {
        effect = "normal";
    }
    if(xshift === "") {
        xshift = "0";
    }
    if(yshift === "") {
        yshift = "0";
    }
    if(alpha === "") {
        alpha = "255";
    }

    return [op, position, file, duration, effect, xshift, yshift, alpha];
}

function normalizeChPosition(pos) {
    switch(pos) {
    case "中央":            return "center";
    case "center":          return "center";
    case "右":              return "right";
    case "right":           return "right";
    case "左":              return "left";
    case "left":            return "left";
    case "中央背面":        return "back";
    case "back":            return "back";
    case "顔":              return "face";
    case "face":            return "face";
    default:                break;
    }
    return "center";
}

function japanizeChPosition(pos) {
    switch(pos) {
    case "中央":            return "中央";
    case "center":          return "中央";
    case "右":              return "右";
    case "right":           return "右";
    case "左":              return "左";
    case "left":            return "左";
    case "中央背面":        return "背面";
    case "back":            return "背面";
    case "顔":              return "顔";
    case "face":            return "顔";
    default:                break;
    }
    return "中央";
}

function normalizeChFile(file) {
    switch(file) {
    case "none":            return "none";
    case "消去":            return "none";
    default:                return file;
    }
}

function japanizeChFile(file) {
    switch(file) {
    case "none":            return "消去";
    case "消去":            return "消去";
    default:                return file;
    }
}

function normalizeChAlpha(alpha) {
    if(alpha === "hide") {
        return 0;
    }
    if(alpha === "show") {
        return 255;
    }
    if(alpha < 0) {
        return 0;
    }
    if(alpha > 255) {
        return 255;
    }
    return alpha;
}

// @bgm
function normalizeBgm(command) {
    var op = "@bgm";
    var file = "";
    var opt = "";

    // トークナイズする
    var tokens = command.match(/(".*?"|[^"\s]+)+(?=\s*|\s*$)/g);
    if(tokens.length >= 2) {
        file = normalizeParameter(tokens[1], ["file=", "ファイル="], MSG_SPECIFY_FILE);
    }
    if(tokens.length >= 3) {
        if(tokens[2] === "once") {
            opt = "once";
        }
    }

    // バリデーションする
    if(file === "") {
        file = translate(MSG_SPECIFY_FILE);
    }

    return [op, file, opt];
}

// @se
function normalizeSe(command) {
    var op = "@se";
    var file = "";
    var opt = "";

    // トークナイズする
    var tokens = command.match(/(".*?"|[^"\s]+)+(?=\s*|\s*$)/g);
    if(tokens.length >= 2) {
        file = normalizeParameter(tokens[1], ["file=", "ファイル="], MSG_SPECIFY_FILE);
    }
    if(tokens.length >= 3) {
        if(tokens[2] === "voice") {
            opt = "voice";
        } else if(tokens[2] === "loop") {
            opt = "loop";
        }
    }

    // バリデーションする
    if(file === "") {
        file = translate(MSG_SPECIFY_FILE);
    }

    return [op, file, opt];
}

// @vol
function normalizeVol(command) {
    var op = "@vol";
    var track = "";
    var volume = ""
    var duration = "";

    // トークナイズする
    var tokens = command.match(/(".*?"|[^"\s]+)+(?=\s*|\s*$)/g);
    if(tokens.length >= 2) {
        track = normalizeParameter(tokens[1], ["track=", "トラック"], "bgm");
        track = normalizeVolTrack(track);
    }
    if(tokens.length >= 3) {
        volume = normalizeParameter(tokens[2], ["volume=", "ボリューム="], "1.0");
        if(volume < 0)
            volume = 0;
        if(volume > 1)
            volume = 1;
    }
    if(tokens.length >= 4) {
        duration = normalizeParameter(tokens[3], ["duration=", "秒="], "0.0");
    }

    // バリデーションする
    if(track === "") {
        track = "bgm";
    }
    if(volume === "") {
        volume = "1.0";
    }
    if(duration === "") {
        duration = "0.0";
    }

    return [op, track, volume, duration];
}

function normalizeVolTrack(track) {
    switch(track) {
    case "bgm":    return "bgm";
    case "音楽":   return "bgm";
    case "voice":  return "voice";
    case "ボイス": return "voice";
    case "se":     return "se";
    case "効果音": return "se";
    case "BGM":    return "BGM";
    case "VOICE":  return "VOICE";
    case "SE":     return "SE";
    default:
        break;
    }
    return "bgm";
}

// @choose
function normalizeChoose(command) {
    var op = "@choose";
    var label = ["", "", "", "", "", "", "", ""];
    var text = ["", "", "", "", "", "", "", ""];

    var tokens = command.match(/(".*?"|[^"\s]+)+(?=\s*|\s*$)/g);
    for(let i = 0; i < 8; i++) {
        if(tokens.length < i * 2 + 2) {
            break;
        }
        label[i] = normalizeParameter(tokens[i * 2 + 1], ["destination" + (i+1) + "=", "行き先" + (i+1) + "="], MSG_SPECIFY_LABEL);
        text[i] = normalizeParameter(tokens[i * 2 + 2], ["option" + (i+1) + "=", "選択肢" + (i+1) + "="], MSG_SPECIFY_OPTION);
    }

    return [op, label[0], text[0], label[1], text[1], label[2], text[2], label[3], text[3], label[4], text[4], label[5], text[5], label[6], text[6], label[7], text[7]];
}

// @cha
function normalizeCha(command) {
    var op = "@cha";
    var position = "";
    var duration = ""
    var acceleration = "";
    var xoffset = "";
    var yoffset = "";
    var alpha = "";

    // トークナイズする
    var tokens = command.match(/(".*?"|[^"\s]+)+(?=\s*|\s*$)/g);
    if(tokens.length >= 2) {
        position = normalizeParameter(tokens[1], ["position=", "位置="], "center");
        position = normalizeChPosition(position);
    }
    if(tokens.length >= 3) {
        duration = normalizeParameter(tokens[2], ["duration=", "秒="], "0.0");
    }
    if(tokens.length >= 4) {
        acceleration = normalizeParameter(tokens[3], ["acceleration=", "加速="], "move");
        acceleration = normalizeChaAcceleration(acceleration);
    }
    if(tokens.length >= 5) {
        xoffset = normalizeParameter(tokens[4], ["x="], "0");
    }
    if(tokens.length >= 6) {
        yoffset = normalizeParameter(tokens[5], ["y="], "0");
    }
    if(tokens.length >= 7) {
        alpha = normalizeParameter(tokens[6], ["alpha=", "アルファ="], "255");
        alpha = normalizeChAlpha(alpha);
    }

    // バリデーションする
    if(position === "") {
        position = "center";
    }
    if(duration === "") {
        duration = "0.0";
    }
    if(acceleration === "") {
        acceleration = "move";
    }
    if(xoffset === "") {
        xoffset = "0";
    }
    if(yoffset === "") {
        yoffset = "0";
    }
    if(alpha === "") {
        alpha = "255";
    }

    return [op, position, duration, acceleration, xoffset, yoffset, alpha];
}

function normalizeChaAcceleration(acc) {
    switch(acc) {
    case "move":    return "move";
    case "なし":    return "move";
    case "accel":   return "accel";
    case "あり":    return "accel";
    case "brake":   return "brake";
    case "減速":    return "brake";
    default:
        break;
    }
    return "move";
}        

// @chs
function normalizeChs(command) {
    var op = "@chs";
    var center = "";
    var right = "";
    var left = "";
    var back = "";
    var duration = "";
    var background = "";
    var effect = "";

    // トークナイズする
    var tokens = command.match(/(".*?"|[^"\s]+)+(?=\s*|\s*$)/g);
    if(tokens.length >= 2) {
        center = normalizeParameter(tokens[1], ["center=", "中央="], MSG_SPECIFY_FILE);
        center = normalizeChsFile(center);
    }
    if(tokens.length >= 3) {
        right = normalizeParameter(tokens[2], ["right=", "右="], MSG_SPECIFY_FILE);
        right = normalizeChsFile(right);
    }
    if(tokens.length >= 4) {
        left = normalizeParameter(tokens[3], ["right=", "右="], MSG_SPECIFY_FILE);
        left = normalizeChsFile(left);
    }
    if(tokens.length >= 5) {
        back = normalizeParameter(tokens[4], ["back=", "背面="], MSG_SPECIFY_FILE);
        back = normalizeChsFile(back);
    }
    if(tokens.length >= 6) {
        duration = normalizeParameter(tokens[5], ["duration=", "秒="], "0.0");
    }
    if(tokens.length >= 7) {
        background = normalizeParameter(tokens[6], ["background=", "背景="], "stay");
        background = normalizeChsBackground(background);
    }
    if(tokens.length >= 8) {
        effect = normalizeParameter(tokens[7], ["effect=", "エフェクト="], "normal");
        effect = normalizeBgEffect(effect);
    }

    // バリデーション
    if(center === "") {
        center = "stay";
    }
    if(right === "") {
        right = "stay";
    }
    if(left === "") {
        left = "stay";
    }
    if(back === "") {
        back = "stay";
    }
    if(duration === "") {
        duration = "0.0";
    }
    if(background === "") {
        background = "stay";
    }
    if(effect === "") {
        effect = "normal";
    }

    return [op, center, right, left, back, duration, background, effect];
}

function normalizeChsFile(file) {
    switch(file) {
    case "none":     return "none";
    case "消去":     return "none";
    case "stay":     return "stay";
    case "変更なし": return "stay";
    case "":         return "stay";
    default:
        break;
    }
    return file;
}

function normalizeChsBackground(file) {
    switch(file) {
    case "stay":     return "stay";
    case "変更なし": return "stay";
    case "":         return "stay";
    default:
        break;
    }
    return file;
}

// @shake
function normalizeShake(command) {
    var op = "@shake";
    var direction = "";
    var duration = "";
    var times = "";
    var amplitude = "";

    // トークナイズ
    var tokens = command.match(/(".*?"|[^"\s]+)+(?=\s*|\s*$)/g);
    if(tokens.length >= 2) {
        direction = normalizeParameter(tokens[1], ["direction=", "方向="], "horizontal");
        direction = normalizeShakeDirection(direction);
    }
    if(tokens.length >= 3) {
        duration = normalizeParameter(tokens[2], ["duration=", "秒="], "1.0");
    }
    if(tokens.length >= 4) {
        times = normalizeParameter(tokens[3], ["times=", "回数="], "1");
    }
    if(tokens.length >= 5) {
        amplitude = normalizeParameter(tokens[4], ["amplitude=", "大きさ="], "100");
    }

    // バリデーション
    if(direction === "") {
        direction = "horizontal";
    }
    if(duration === "") {
        dration = "1.0";
    }
    if(times === "") {
        times = "1";
    }
    if(amplitude === "") {
        amplitude = "100";
    }

    return [op, direction, duration, times, amplitude];
}

function normalizeShakeDirection(dir) {
    switch(dir) {
    case "horizontal":  return "horizontal";
    case "h":           return "horizontal";
    case "横":          return "horizontal";
    case "vertical":    return "vertical";
    case "v":           return "vertical";
    case "縦":          return "vertical";
    default:
        break;
    }
    return "horizontal";
}

// @wait
function normalizeWait(command) {
    var op = "@wait";
    var duration = "";

    // トークナイズする
    var tokens = command.match(/(".*?"|[^"\s]+)+(?=\s*|\s*$)/g);
    if(tokens.length >= 2) {
        duration = normalizeParameter(tokens[1], ["duration=", "秒="], "1.0");
    }

    // バリデーションする
    if(duration === "") {
        duration = "1.0";
    }

    return [op, duration];
}

// @skip
function normalizeSkip(command) {
    var op = "@skip";
    var opt = "";

    // トークナイズする
    var tokens = command.match(/(".*?"|[^"\s]+)+(?=\s*|\s*$)/g);
    if(tokens.length >= 2) {
        if(tokens[1] === "enable" || tokens[1] === "許可") {
            opt = "enable";
        } else if(tokens[1] === "disable" || tokens[1] === "不許可") {
            opt = "disable";
        } else {
            opt = "enable";
        }
    }

    // バリデーションする
    if(opt === "") {
        opt = "enable";
    }

    return [op, opt];
}

// @goto
function normalizeGoto(command) {
    var op = "@goto";
    var destination = "";

    // トークナイズする
    var tokens = command.match(/(".*?"|[^"\s]+)+(?=\s*|\s*$)/g);
    if(tokens.length >= 2) {
        destination = normalizeParameter(tokens[1], ["destination=", "行き先="], MSG_SPECIFY_LABEL);
    }

    // バリデーションする
    if(destination === "") {
        destination = translate(MSG_SPECIFY_LABEL);
    }

    return [op, destination];
}

// @set
function normalizeSet(command) {
    var op = "@set";
    var variable = "";
    var operator = "";
    var value = "";

    // トークナイズする (引数名はない)
    var tokens = command.match(/(".*?"|[^"\s]+)+(?=\s*|\s*$)/g);
    if(tokens.length >= 2) {
        if(tokens[1].startsWith("$") && !isNaN(tokens[1].substring(1))) {
            variable = tokens[1];
        }
    }
    if(tokens.length >= 3) {
        if(tokens[2] === "=" || tokens[2] === "+=" || tokens[2] === "-=" || tokens[2] === "*=" || tokens[2] === "/=" || tokens[2] === "%=") {
            operator = tokens[2];
        }
    }
    if(tokens.length >= 4) {
        if(!isNaN(tokens[3])) {
            value = tokens[3];
        }
    }

    // バリデーションする
    if(variable === "") {
        variable = "$0";
    }
    if(operator === "") {
        operator = "=";
    }
    if(value === "") {
        value = "0";
    }

    return [op, variable, operator, value];
}

// @if
function normalizeIf(command) {
    var op = "@if";
    var variable = "";
    var operator = "";
    var value = "";
    var label = "";

    // トークナイズする (引数名はない)
    var tokens = command.match(/(".*?"|[^"\s]+)+(?=\s*|\s*$)/g);
    if(tokens.length >= 2) {
        if(tokens[1].startsWith("$") && !isNaN(tokens[1].substring(1))) {
            variable = tokens[1];
        }
    }
    if(tokens.length >= 3) {
        if(tokens[2] === "==" || tokens[2] === "!=" || tokens[2] === ">" || tokens[2] === ">=" || tokens[2] === "<" || tokens[2] === "<=") {
            operator = tokens[2];
        }
    }
    if(tokens.length >= 4) {
        if(!isNaN(tokens[3])) {
            value = tokens[3];
        }
    }
    if(tokens.length >= 5) {
        label = tokens[4];
    }

    // バリデーションする
    if(variable === "") {
        variable = "$0";
    }
    if(operator === "") {
        operator = "==";
    }
    if(value === "") {
        value = "0";
    }
    if(label === "") {
        label = translate(MSG_SPECIFY_LABEL);
    }

    return [op, variable, operator, value, label];
}

// @load
function normalizeLoad(command) {
    var op = "@load";
    var file = "";

    // トークナイズする
    var tokens = command.match(/(".*?"|[^"\s]+)+(?=\s*|\s*$)/g);
    if(tokens.length >= 2) {
        file = normalizeParameter(tokens[1], ["file=", "ファイル="], MSG_SPECIFY_FILE);
    }

    // バリデーションする
    if(file === "") {
        file = translate(MSG_SPECIFY_FILE);
    }

    return [op, file];
}

// @chapter
function normalizeChapter(command) {
    var op = "@chapter";
    var title = "";

    // トークナイズする
    var tokens = command.match(/(".*?"|[^"\s]+)+(?=\s*|\s*$)/g);
    if(tokens.length >= 2) {
        title = normalizeParameter(tokens[1], ["title=", "タイトル="], "");
    }

    // titleは""でもよい

    return [op, title];
}

// @gui
function normalizeGui(command) {
    var op = "@gui";
    var file = "";

    // トークナイズする
    var tokens = command.match(/(".*?"|[^"\s]+)+(?=\s*|\s*$)/g);
    if(tokens.length >= 2) {
        file = normalizeParameter(tokens[1], ["file=", "ファイル="], MSG_SPECIFY_FILE);
    }

    // バリデーションする
    if(file === "") {
        file = translate(MSG_SPECIFY_FILE);
    }

    return [op, file];
}

// @video
function normalizeVideo(command) {
    var op = "@video";
    var file = "";

    // トークナイズする
    var tokens = command.match(/(".*?"|[^"\s]+)+(?=\s*|\s*$)/g);
    if(tokens.length >= 2) {
        file = normalizeParameter(tokens[1], ["file=", "ファイル="], MSG_SPECIFY_FILE);
    }

    // バリデーションする
    if(file === "") {
        file = translate(MSG_SPECIFY_FILE);
    }

    return [op, file];
}

// @wms
function normalizeWms(command) {
    var op = "@wms";
    var file = "";

    // トークナイズする
    var tokens = command.match(/(".*?"|[^"\s]+)+(?=\s*|\s*$)/g);
    if(tokens.length >= 2) {
        file = normalizeParameter(tokens[1], ["file=", "ファイル="], MSG_SPECIFY_FILE);
    }

    // バリデーションする
    if(file === "") {
        file = translate(MSG_SPECIFY_FILE);
    }

    return [op, file];
}

/*
 * サムネイル
 */

function setupThumbnail() {
    var elem = document.getElementById("thumbnail-picture");
    elem.addEventListener("dragover", onThumbnailDragOver);
    elem.addEventListener("dragleave", onThumbnailDragLeave);
    elem.addEventListener("drop", onThumbnailDrop);
}

function onThumbnailDragOver(event) {
    event.preventDefault();

    var elem = document.getElementById("thumbnail-picture");
    if(elem.src.endsWith("trash-close.png")) {
        elem.src = "../img/trash-open.png";
    }
    return false;
}

function onThumbnailDragLeave(event) {
    var elem = document.getElementById("thumbnail-picture");
    if(elem.src.endsWith("trash-open.png")) {
        elem.src = "../img/trash-close.png";
    }
}

function onThumbnailDrop(event) {
    event.preventDefault();
    document.getElementById("thumbnail-picture").src = "";

    var id = event.dataTransfer.getData("text/plain");
    var elemDrag = document.getElementById(id);

    if(typeof elemDrag.donotremove !== "undefined") {
        return;
    }

    // シナリオ項目を削除する場合
    if(typeof elemDrag.template === "undefined") {
        hidePropsForElement(elemDrag);
        elemDrag.parentNode.removeChild(elemDrag);
        return;
    }

    // 素材を削除する場合
    if(elemDrag.cmd.startsWith("@load ")) {
        var cl = normalizeLoad(elemDrag.cmd);
        window.api.removeTxtFile(cl[1]);
        refreshTxt();
    } else if(elemDrag.cmd.startsWith("@bg ")) {
        var cl = normalizeBg(elemDrag.cmd);
        window.api.removeBgFile(cl[1]);
        refreshBg();
    } else if(elemDrag.cmd.startsWith("@ch ")) {
        var cl = normalizeCh(elemDrag.cmd);
        window.api.removeChFile(cl[2]);
        refreshCh();
    } else if(elemDrag.cmd.startsWith("@bgm ")) {
        var cl = normalizeBgm(elemDrag.cmd);
        window.api.removeBgmFile(cl[1]);
        refreshBgm();
    } else if(elemDrag.cmd.startsWith("@se ")) {
        var cl = normalizeSe(elemDrag.cmd);
        window.api.removeSeFile(cl[1]);
        refreshSe();
    } else if(elemDrag.cmd.startsWith("@video ")) {
        var cl = normalizeVideo(elemDrag.cmd);
        window.api.removeMovFile(cl[1]);
        refreshMov();
    }
}

/*
 * プロパティ
 */

function setupProp() {
    var rangeArray = [].slice.call(document.querySelectorAll("input[type='range']"));
    for(let r of rangeArray) {
        if(typeof r.id !== "undefined" && r.id !== null) {
            var num = document.getElementById(r.id + "-num");
            if(num !== null) {
                r.linkedRange = num;
                r.addEventListener("input", (e) => {
                    e.target.linkedRange.textContent = e.target.value;
                });
            }
        }
    }
}

/*
 * 翻訳
 */

function translate(msg) {
    if(locale === "ja") {
        return msg;
    }
    switch(msg) {
    case "ゲーム終了": return "End of the Game";
    case "背景": return "Background";
    case "キャラ ": return "Character ";
    case "音楽 ": return "Music ";
    case "効果音": return "Sound Effect";
    case "音量": return "Sound Volume";
    case "選択肢": return "Options";
    case "キャラ移動 ": return "Character Move ";
    case "場面転換": return "Stage Change";
    case "画面を揺らす": return "Shake the Screen";
    case "クリックを待つ": return "Wait for a Click";
    case "一定時間待つ ": return "Timed Wait ";
    case "秒": return "sec";
    case "スキップ": return "Skip";
    case "許可": return "Allow";
    case "禁止": return "Disallow";
    case "ジャンプ \"": return "Jump to \"";
    case "\"へ": return "\"";
    case "フラグをセット": return "Set a Flag";
    case "フラグでジャンプ": return "Jump by a Flag";
    case "シナリオへジャンプ": return "Jump to Scenario";
    case "章のタイトル \"": return "Chapter Title \"";
    case "高機能スクリプトを呼び出す": return "Call an Advanced Script";
    case "動画 ": return "Video ";
    case "目印 \"": return "Bookmark \"";
    case "「": return ": \"";
    case "」": return "\"";
    case "変更なし": return "No Change";
    case "消去する": return "Vanish";
    case "名前を入力してください": return "Enter the name";
    case "セリフを入力してください": return "Enter the message";
    case "文章を入力してください": return "Enter the message";
    case "ここに文章を入力してください。": return "Enter the text here.";
    case "キャラ名「セリフを入力してください」": return "*Name*Enter the text here.";
    case "@choose 目印1 学校へ行く 目印2 海へ行く 目印3 公園へ行く": return "@choose bookmark1 \"Go to school\" bookmark2 \"Go to coast\" bookmark3 \"Go to park\"";
    case ":名前をつけてください": return ":Name this";
    case "@goto 目印を選んでください": return "@goto Choose_the_bookmark";
    case "@if $0 == 1 目印を選んでください": return "@if $0 == 1 Choose_the_bookmark";
    case "@chapter 章のタイトル": return "@chapter Title";
    case "@wms ファイルを指定してください": return "@wms Specify_the_file";
    case "#ここにメモを記入してください": return "#This is a comment.";
    case "白": return "White";
    case "黒": return "Black";
    case "ファイルを指定してください": return "Specify a file.";
    case "行き先を指定してください": return "Specify a bookmark.";
    case "選択肢を指定してください": return "Specify a text.";
    case "標準": return "Normal";
    case "右カーテン": return "Right Curtain";
    case "左カーテン": return "Left Curtain";
    case "上カーテン": return "Up Curtain";
    case "下カーテン": return "Down Curtain";
    case "中央": return "Center";
    case "右": return "Right";
    case "左": return "Left";
    case "背面": return "Back";
    case "顔": return "Face";
    case "停止": return "stop";
    case "消去": return "none";
    default: return "(This message is not yet translated.)";
    }
}

/*
 * ロード時
 */

window.addEventListener("load", async() => {
    locale = await window.api.getLocale();

    // ゲームのベースURLを取得する
    baseUrl = await window.api.getBaseUrl();

    // Get the flag list.
    flagList = await window.api.getFlagList();

    // パレットの要素をセットアップする
    setupPalette();

    // txtタブの要素をセットアップする
    refreshTxt();
    setupTxt();

    // bgタブの要素をセットアップする
    refreshBg();
    setupBg();

    // chタブの要素をセットアップする
    refreshCh();
    setupCh();

    // bgmタブの要素をセットアップする
    refreshBgm();
    setupBgm();

    // seタブの要素をセットアップする
    refreshSe();
    setupSe();

    // movタブの要素をセットアップする
    refreshMov();
    setupMov();

    // シナリオの要素をセットアップする
    refreshScenario();

    // プロパティをセットアップする
    setupProp();

    // サムネイルをセットアップする
    setupThumbnail();

    // 再生ボタンをセットアップする
    document.getElementById("play").addEventListener("click", async () => {
        commitProps();
        await saveScenario();
        await window.api.playGame();
    });
    
    // フラグ管理ボタンをセットアップする
    document.getElementById("flags").addEventListener("click", async () => {
        commitProps();
        await saveScenario();
        window.location.href = locale === "ja" ? "flags.html" : "flags_en.html";
    });

    // コンフィグボタンをセットアップする
    document.getElementById("config").addEventListener("click", async () => {
        commitProps();
        await saveScenario();
        window.location.href = locale === "ja" ? "config.html" : "config_en.html";
    });

    // エクスポートボタンをセットアップする
    document.getElementById("export").addEventListener("click", async () => {
        commitProps();
        await saveScenario();
        window.location.href = locale === "ja" ? "export.html" : "export_en.html";
    });
})

window.addEventListener("beforeunload", async () => {
    // シナリオを保存する
    saveScenario();
})
