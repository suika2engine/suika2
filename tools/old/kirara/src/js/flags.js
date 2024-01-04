var flags = {};

async function onAddButton() {
    var name = document.getElementById("flag-name").value;
    var isGlobal = document.getElementById("flag-global").checked;

    // Search for an unused index.
    var index = -1;
    if(!isGlobal) {
        for(var i = 0; i <= 9999; i++) {
            if(!flags.hasOwnProperty(i)) {
                index = i;
                break;
            }
        }
    } else {
        for(var i = 10000; i <= 10999; i++) {
            if(!flags.hasOwnProperty(i)) {
                index = i;
                break;
            }
        }
    }
    if(index === -1) {
        return;
    }

    // Add a flag.
    await window.api.addFlag(name, index);

    // Reload the page.
    var locale = await window.api.getLocale();
    window.location.href = locale === "ja" ? "flags.html" : "flags_en.html";
}

window.addEventListener("load", async () => {
    // Get the flag list.
    flags = await window.api.getFlagList();

    // Get "flag-list" element.
    var parent = document.getElementById("flag-list");
    for(let index of Object.keys(flags)) {
        if(index === "length" || index === "") {
            continue;
        }
        var elem = document.createElement("li");
        elem.classList.add("flag-list-item");
        elem.textContent = "$" + index + ": " + flags[index];
        parent.appendChild(elem);
    }

    // Add button listener for "Add flag".
    var btnAdd = document.getElementById("add");
    btnAdd.addEventListener("click", onAddButton);
})
