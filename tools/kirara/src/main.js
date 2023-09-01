'use strict';

const {app, dialog, shell, BrowserWindow, ipcMain} = require("electron");

let mainWindow;
let splash;

function createWindow () {
    mainWindow = new BrowserWindow({width: 1280, height: 720, show: false, webPreferences: {
        nodeIntegration: false,
        contextIsolation: true,
        preload: __dirname + '/preload.js'
    }});
    mainWindow.setMenuBarVisibility(false);
    mainWindow.loadURL('file://' + __dirname + (getLocale() == "ja" ? '/html/boot.html' : '/html/boot_en.html'));

    splash = new BrowserWindow({
        width: 640,
        height: 380,
        transparent: true,
        frame: false,
        resizable: false,
        movable: false, // doesn't seem to work, uses the CSS 'pointer-events' modifier instead.
        alwaysOnTop: true
    });

    splash.setMenuBarVisibility(false);
    splash.loadURL('file://' + __dirname + '/html/splash.html');
    setTimeout(function () {
        splash.close();
        mainWindow.center();
        mainWindow.show();
    }, 1000);
}

function getLocale() {
    var code = app.getLocale();
    if(code === "ja") {
        return "ja";
//      return "en";
    }
    return "en";
}

app.on('ready', function() {
    createWindow();

    // デバッグウィンドウ
//  mainWindow.webContents.openDevTools();
});

app.on('window-all-closed', () => {
    app.quit()
});

//
// モデル操作
//

const fs = require("fs");
const path = require("path");

class Model {
    static dir = "";
    static txt = [];
    static bg = [];
    static ch = [];
    static bgm = [];
    static se = [];
    static mov = [];
    static flags = {};
    static scenarioFile = "";
    static scenarioData = [""];
}

//
// ゲームの一覧を取得する
//
ipcMain.handle('getGameList', (event) => {
    // ユーザのドキュメントフォルダにSuika2フォルダがない場合
    var path = app.getPath("documents") + "/Suika2";
    if(!fs.existsSync(path)) {
        return [];
    }

    // ゲームフォルダの一覧を返す
    var ret = [];
    fs.readdirSync(path).forEach(function(file) {
        if(ret !== ".DS_Store") {
            ret.push(file);
        }
    });
    return ret;
})

//
// ゲームを作成する
//
ipcMain.handle('createGame', (event, dir) => {
    // ゲームフォルダを作成する
    var path = app.getPath("documents") + "/Suika2/" + dir;
    if(!fs.mkdirSync(path, { recursive: true })) {
        return false;
    }

    // テンプレートをコピーする
    copyTemplateFiles("template", path, true);

    // 実行ファイルをコピーする
    if(process.platform === "win32") {
        fs.writeFileSync(path + "/suika.exe", fs.readFileSync(app.getAppPath() + "/apps/suika.exe"));
        fs.writeFileSync(path + "/suika-pro.exe", fs.readFileSync(app.getAppPath() + "/apps/suika-pro.exe"));
    } else if(process.platform === "darwin") {
        fs.writeFileSync(path + "/mac.zip", fs.readFileSync(app.getAppPath() + "/apps/mac.zip"));
        exec("cd " + path + " && unzip " + path + "/mac.zip");
    }

    return true;
})

function copyTemplateFiles(src, dst, top) {
    if(!fs.existsSync(app.getAppPath() + "/" + src)) {
        return;
    }
    if(top) {
        fs.readdirSync(app.getAppPath() + "/" + src).forEach(function (fname) {
            copyTemplateFiles(src + "/" + fname, dst + "/" + fname, false);
        });
        return;
    }
    if(fs.statSync(app.getAppPath() + "/" + src).isFile()) {
        let file = dst;
        let dir = path.dirname(file);
        if(!fs.existsSync(dir)) {
            fs.mkdirSync(dir, { recursive: true });
        }
        fs.writeFileSync(file, fs.readFileSync(app.getAppPath() + "/" + src));
    } else if (fs.statSync(app.getAppPath() + "/" + src).isDirectory()) {
        fs.readdirSync(app.getAppPath() + "/" + src).forEach(function (fname) {
            copyTemplateFiles(src + "/" + fname, dst + "/" + fname, false);
        });
    }
}

//
// ゲームを開く
//
ipcMain.handle('openGame', (event, dir) => {
    var path = app.getPath("documents") + "/Suika2/" + dir;
    if(!fs.existsSync(path + "/txt/init.txt")) {
        return false;
    }

    Model.dir = path;
    refreshFiles("txt", Model.txt, ['.txt']);
    refreshFiles("bg", Model.bg, ['.png', '.jpg', '.jpeg']);
    refreshFiles("ch", Model.ch, ['.png', '.jpg', '.jpeg']);
    refreshFiles("bgm", Model.bgm, ['.ogg']);
    refreshFiles("se", Model.se, ['.ogg']);
    loadFlags();
})

function refreshFiles(subDir, list, allowExts) {
    var dirPath = Model.dir + "/" + subDir;
    if(!fs.existsSync(dirPath)) {
        return;
    }

    list.length = 0;
    fs.readdirSync(dirPath).forEach(function (file) {
        var ext = path.extname(file).toLowerCase();
        if(allowExts.includes(ext)) {
            list.push(file);
        }
    });
    list.sort();
}

//
// シナリオを開く
//
ipcMain.handle('openScenario', (event, file) => {
    var filePath = path.normalize(Model.dir + "/txt/" + file);
    if(!fs.existsSync(filePath)) {
        return;
    }

    var rawData = fs.readFileSync(filePath, { encoding: 'utf8' });

    Model.scenarioFile = file;
    Model.scenarioData = rawData.split(/\r\n|\n/);

    if(Model.scenarioData.length > 0) {
        if(Model.scenarioData[Model.scenarioData.length - 1] === "") {
            Model.scenarioData.pop();
        }
    }
})

//
// シナリオファイル名を取得する
//
ipcMain.handle('getScenarioName', (event) => {
    return Model.scenarioFile;
})

//
// シナリオデータを取得する
//
ipcMain.handle('getScenarioData', (event) => {
    return Model.scenarioData;
})

//
// シナリオデータを保存する
//
ipcMain.handle('setScenarioData', (event, data) => {
    Model.scenarioData = data;

    var filePath = path.normalize(Model.dir + "/txt/" + Model.scenarioFile);
    fs.writeFileSync(filePath, data.join("\n"), "utf8");
})

//
// ゲームフォルダのURLを取得する
//
ipcMain.handle('getBaseUrl', (event) => {
    return "file:///" + Model.dir + "/";
})

//
// txtフォルダのファイルのリストを取得する
//
ipcMain.handle('getTxtList', (event) => {
    refreshFiles("txt", Model.txt, ['.txt']);
    return Model.txt;
})

//
// bgフォルダのファイルのリストを取得する
//
ipcMain.handle('getBgList', (event) => {
    refreshFiles("bg", Model.bg, ['.png', '.jpg', '.jpeg']);
    return Model.bg;
})

//
// chフォルダのファイルのリストを取得する
//
ipcMain.handle('getChList', (event) => {
    refreshFiles("ch", Model.ch, ['.png', '.jpg', '.jpeg']);
    return Model.ch;
})

//
// bgmフォルダのファイルのリストを取得する
//
ipcMain.handle('getBgmList', (event) => {
    refreshFiles("bgm", Model.bgm, ['.ogg']);
    return Model.bgm;
})

//
// seフォルダのファイルのリストを取得する
//
ipcMain.handle('getSeList', (event) => {
    refreshFiles("se", Model.se, ['.ogg']);
    return Model.se;
})

//
// movフォルダのファイルのリストを取得する
//
ipcMain.handle('getMovList', (event) => {
    refreshFiles("mov", Model.mov, [".wmv", ".mp4"]);
    return Model.mov;
})

//
// txtファイルを追加する
//
ipcMain.handle('addTxtFile', (event, srcFilePath) => {
    return copyAsset(srcFilePath, ['.txt'], "txt");
})

function copyAsset(srcFilePath, allowExts, subDir) {
    if(!allowExts.includes(path.extname(srcFilePath).toLowerCase())) {
        return false;
    }

    var srcFileName = path.basename(srcFilePath);
    var dstFileName = srcFileName.replace(/[^\x00-\x7F]/g, "_").replace(/ /g, "_");
    var dstDir = Model.dir + "/" + subDir;
    var dstPath = dstDir + "/" + dstFileName;

    if(!fs.existsSync(dstDir)) {
        fs.mkdirSync(dstDir);
    }

    fs.copyFileSync(srcFilePath, dstPath);
    return true;
}

//
// bgファイルを追加する
//
ipcMain.handle('addBgFile', (event, srcFilePath) => {
    return copyAsset(srcFilePath, ['.png', '.jpg', '.jpeg'], "bg");
})

//
// chファイルを追加する
//
ipcMain.handle('addChFile', (event, srcFilePath) => {
    return copyAsset(srcFilePath, ['.png', '.jpg', '.jpeg'], "ch");
})

//
// bgmファイルを追加する
//
ipcMain.handle('addBgmFile', (event, srcFilePath) => {
    return copyAsset(srcFilePath, ['.ogg'], "bgm");
})

//
// seファイルを追加する
//
ipcMain.handle('addSeFile', (event, srcFilePath) => {
    return copyAsset(srcFilePath, ['.ogg'], "se");
})

//
// movファイルを追加する
//
ipcMain.handle('addMovFile', (event, srcFilePath) => {
    return copyAsset(srcFilePath, [".wmv", ".mp4"], "mov");
})

//
// txtファイルを削除する
//
ipcMain.handle('removeTxtFile', (event, file) => {
    removeAsset("txt", file);
})

function removeAsset(subDir, file) {
    var filePath = Model.dir + "/" + subDir + "/" + file;
    if(fs.existsSync(filePath)) {
        fs.rmSync(filePath);
    }
}

//
// bgファイルを削除する
//
ipcMain.handle('removeBgFile', (event, file) => {
    removeAsset("bg", file);
})

//
// chファイルを削除する
//
ipcMain.handle('removeChFile', (event, file) => {
    removeAsset("ch", file);
})

//
// bgmファイルを削除する
//
ipcMain.handle('removeBgmFile', (event, file) => {
    removeAsset("bgm", file);
})

//
// seファイルを削除する
//
ipcMain.handle('removeSeFile', (event, file) => {
    removeAsset("se", file);
})

//
// movファイルを削除する
//
ipcMain.handle('removeMovFile', (event, file) => {
    removeAsset("mov", file);
})

//
// txtフォルダを開く
//
ipcMain.handle('openTxtFolder', (event) => {
    shell.showItemInFolder(path.normalize(Model.dir + "/txt"));
})

//
// bgフォルダを開く
//
ipcMain.handle('openBgFolder', (event) => {
    shell.showItemInFolder(path.normalize(Model.dir + "/bg"));
})

//
// chフォルダを開く
//
ipcMain.handle('openChFolder', (event) => {
    shell.showItemInFolder(path.normalize(Model.dir + "/ch"));
})

//
// bgmフォルダを開く
//
ipcMain.handle('openBgmFolder', (event) => {
    shell.showItemInFolder(path.normalize(Model.dir + "/bgm"));
})

//
// seフォルダを開く
//
ipcMain.handle('openSeFolder', (event) => {
    shell.showItemInFolder(path.normalize(Model.dir + "/se"));
})

//
// movフォルダを開く
//
ipcMain.handle('openMovFolder', (event) => {
    shell.showItemInFolder(path.normalize(Model.dir + "/mov"));
})


//
// ゲーム実行
//

const exec = require('child_process').exec;
const execSync = require('child_process').execSync;

ipcMain.handle('playGame', (event) => {
    // セーブデータをクリアする
    fs.rmSync(Model.dir + "/sav", { recursive: true, force: true });

    // プレーヤを実行する
    if(process.platform === "win32") {
        var command = "cd " + Model.dir + " && " + "suika.exe";
        exec(command);
    } else if(process.platform === "darwin") {
        var command = "open " + Model.dir + "/suika.app";
        exec(command);
    }
})

ipcMain.handle('debugGame', (event, lineIndex) => {
    // セーブデータをクリアする
    fs.rmSync(Model.dir + "/sav", { recursive: true, force: true });

    // デバッガを実行する
    if(process.platform === "win32") {
        var command = "cd " + Model.dir + " && suika-pro.exe " + Model.scenarioFile + " " + lineIndex;
        exec(command);
    } else if(process.platform === "darwin") {
        var command = "open " + Model.dir + "/suika-pro.app --args scenario-file=" + Model.scenarioFile + " scenario-line=" + lineIndex;
        exec(command);
    }
})

//
// エクスポート
//

ipcMain.handle('exportForWindows', (event) => {
    doDesktopExport(false);
})

ipcMain.handle('exportForWinMac', (event) => {
    doDesktopExport(true);
})

async function doDesktopExport(withMac) {
    // フォルダ選択ダイアログでパスを取得する
    var files = dialog.showOpenDialogSync({
        properties: ['openDirectory']
    });
    if(typeof files === "undefined") {
        return "canceled";
    }

    // 選択されたフォルダ直下にフォルダを作成する
    var dstPathRoot = files[0] + "/" + path.basename(Model.dir);
    if(!fs.existsSync(dstPathRoot)) {
        fs.mkdirSync(dstPathRoot);
    } else {
        // TODO: 上書きします。よろしいですか？
    }

    // パッケージを作成する
    if(process.platform === "win32") {
        fs.writeFileSync(Model.dir + "/pack.exe", fs.readFileSync(app.getAppPath() + "/apps/pack.exe"));
        execSync("cd " + Model.dir + " && pack.exe");
    } else if(process.platform === "darwin") {
        fs.writeFileSync(Model.dir + "/pack", fs.readFileSync(app.getAppPath() + "/apps/pack.mac"));
        execSync("cd " + Model.dir + " && chmod +x ./pack && ./pack");
    }
    if(!fs.existsSync(Model.dir + "/data01.arc")) {
        return "failed";
    }

    // ファイルをコピーする
    fs.copyFileSync(Model.dir + "/data01.arc", dstPathRoot + "/data01.arc");
    fs.writeFileSync(dstPathRoot + "/game.exe", fs.readFileSync(app.getAppPath() + "/apps/suika.exe"));
    if(withMac) {
        fs.writeFileSync(dstPathRoot + "/game.dmg", fs.readFileSync(app.getAppPath() + "/apps/mac.dmg"));
    }
    if(fs.existsSync(Model.dir + "/mov")) {
        if(!fs.existsSync(dstPathRoot + "/mov")) {
            fs.mkdirSync(dstPathRoot + "/mov");
        }
        fs.readdirSync(Model.dir + "/mov").forEach(function (file) {
            fs.copyFileSync(file, dstPathRoot + "/mov/" + path.basename(file));
        });
    }

    // フォルダを開く
    shell.showItemInFolder(path.normalize(dstPathRoot));
}

ipcMain.handle('exportForWeb', (event) => {
    // フォルダ選択ダイアログでパスを取得する
    var files = dialog.showOpenDialogSync({
        properties: ['openDirectory']
    });
    if(typeof files === "undefined") {
        return "canceled";
    }

    // 選択されたフォルダ直下にフォルダを作成する
    var dstPathRoot = files[0] + "/" + path.basename(Model.dir);
    if(!fs.existsSync(dstPathRoot)) {
        fs.mkdirSync(dstPathRoot);
    } else {
        // TODO: 上書きします。よろしいですか？
    }

    // パッケージを作成する
    if(process.platform === "win32") {
        fs.writeFileSync(Model.dir + "/pack.exe", fs.readFileSync(app.getAppPath() + "/apps/pack.exe"));
        execSync("cd " + Model.dir + " && pack.exe");
    } else if(process.platform === "darwin") {
        fs.writeFileSync(Model.dir + "/pack", fs.readFileSync(app.getAppPath() + "/apps/pack.mac"));
        execSync("cd " + Model.dir + " && chmod +x ./pack && ./pack");
    }
    if(!fs.existsSync(Model.dir + "/data01.arc")) {
        return "failed";
    }

    // ファイルをコピーする
    fs.copyFileSync(Model.dir + "/data01.arc", dstPathRoot + "/data01.arc");
    fs.writeFileSync(dstPathRoot + "/index.html", fs.readFileSync(app.getAppPath() + "/apps/index.html"));
    fs.writeFileSync(dstPathRoot + "/index.js", fs.readFileSync(app.getAppPath() + "/apps/index.js"));
    fs.writeFileSync(dstPathRoot + "/index.wasm", fs.readFileSync(app.getAppPath() + "/apps/index.wasm"));
    if(fs.existsSync(Model.dir + "/mov")) {
        if(!fs.existsSync(dstPathRoot + "/mov")) {
            fs.mkdirSync(dstPathRoot + "/mov");
        }
        fs.readdirSync(Model.dir + "/mov").forEach(function (file) {
            fs.copyFileSync(file, dstPathRoot + "/mov/" + path.basename(file));
        });
    }

    // フォルダを開く
    shell.showItemInFolder(path.normalize(dstPathRoot));
})

//
// コンフィグ
//

ipcMain.handle('loadConfig', (event) => {
    var config = {};
    var lines = fs.readFileSync(Model.dir + "/conf/config.txt", "utf8").replace(/\r/g, "").split("\n");
    for(let line of lines) {
        if(!line.startsWith("#")) {
            // Split line before and after the first "=".
            var [key, ...value] = line.split("=");

            // Add to the dictionary.
            if(key !== "" && value !== "") {
                config[key] = value.join("=");
            }
        }
    }
    return config;
})

ipcMain.handle('storeConfig', (event, config) => {
    var outputString = "";
    for(let key of Object.keys(config)) {
        outputString = outputString + key + "=" + config[key] + "\n";
    }
    fs.writeFileSync(Model.dir + "/conf/config.txt", outputString, "utf8");
})

//
// フラグ
//

ipcMain.handle('getFlagList', (event) => {
    return Model.flags;
})

ipcMain.handle('addFlag', (event, name, index) => {
    Model.flags[index] = name;
    storeFlags();
})

ipcMain.handle('removeFlag', (event, name) => {
    if(Model.flags.hasOwnProperty(name)) {
        delete Model.flags['lastName'];
    }
    storeFlags();
})

function loadFlags() {
    var filePath = Model.dir + "/flags.txt";
    if(!fs.existsSync(filePath)) {
        return;
    }

    Model.flags.length = 0;
    var lines = fs.readFileSync(Model.dir + "/flags.txt", "utf8").replace(/\r/g, "").split("\n");
    for(let line of lines) {
        // Split line before and after the first "=".
        var [index, ...name] = line.split("=");

        // Add to the dictionary.
        Model.flags[index] = name.join("=");
    }
    return Model.flags;
}

function storeFlags() {
    var outputString = "";
    for(let index of Object.keys(Model.flags)) {
        outputString = outputString + index + "=" + Model.flags[index] + "\n";
    }

    var filePath = Model.dir + "/flags.txt";
    fs.writeFileSync(filePath, outputString, "utf8");
}

//
// その他
//

ipcMain.handle('getLocale', (event) => {
    return getLocale();
})

ipcMain.handle('openURL', (event, url) => {
    shell.openExternal(url);
})
