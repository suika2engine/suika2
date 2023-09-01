var locale = "ja";

function translate(msg) {
    if(locale === "ja") {
        return msg;
    }

    switch(msg) {
    case "名前を指定してください。": return "Please specify the name.";
    case "名前にスペースを含められません。": return "You can't include spaces in the game name.";
    case "ゲームフォルダの作成に失敗しました。": return "Failed to create the game folder.";
    default: return "(This message is not yet translated.)";
    }
}

window.addEventListener("load", async () => {
    locale = await window.api.getLocale();

    var gameList = await window.api.getGameList();
    gameList.forEach(game => {
        var e = document.createElement("li");
        e.textContent = game;
        e.classList.add("game-list-item");
        e.addEventListener("click", async () => {
            await window.api.openGame(event.srcElement.textContent);
            await window.api.openScenario("story001.txt");
            window.location.href = locale === "ja" ? "index.html" : "index_en.html";
        });
        document.getElementById("game-list").appendChild(e);
    });

    document.getElementById("new-game").addEventListener("click", async () => {
        var elem = document.getElementById("game-name");
        var gameName = elem.value;

        if(gameName === "") {
            document.getElementById("error-msg").textContent = translate("名前を指定してください。");
            return;
        }

        if(gameName.indexOf(" ") !== -1) {
            document.getElementById("error-msg").textContent = translate("名前にスペースを含められません。");
            return;
        }

        if(! await window.api.createGame(gameName)) {
            document.getElementById("error-msg").textContent = translate("ゲームフォルダの作成に失敗しました。");
            return;
        }

        await window.api.openGame(gameName);
        await window.api.openScenario("story001.txt");
        window.location.href = locale === "ja" ? "index.html" : "index_en.html";
    });
});
