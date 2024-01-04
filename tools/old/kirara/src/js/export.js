window.addEventListener("load", async() => {
    var btnWin = document.getElementById("export-windows");
    btnWin.addEventListener("click", async () => {
        exportForWindows();
    });

    var btnWeb = document.getElementById("export-web");
    btnWeb.addEventListener("click", async () => {
        exportForWeb();
    });

    var btnWinMac = document.getElementById("export-winmac");
    btnWinMac.addEventListener("click", async () => {
        exportForWinMac();
    });
})

async function exportForWindows() {
    window.api.exportForWindows();
}

function exportForWeb() {
    window.api.exportForWeb();
}

function exportForWinMac() {
    window.api.exportForWinMac();
}
