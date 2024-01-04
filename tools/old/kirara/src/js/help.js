//
// リンク | Links
//

window.addEventListener("load", async() => {
    document.getElementById("web-docs").addEventListener("click", () => {
      window.api.openURL("https://github.com/suika2engine/suika2/wiki");
    });
})

//
// ドキュメント | Document
//

// Jump to 'pagew' (Home) page when opened or refreshed.
document.location.hash = "#pagew";
