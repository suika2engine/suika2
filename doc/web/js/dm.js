window.addEventListener('load', () => {
    const theme = localStorage.getItem('theme');
    document.getElementById('theme').className = theme;
    console.log(`The current theme is ${theme}.`)
})

function changeTheme() {
    const theme = localStorage.getItem('theme')
    if (theme == 'dark') {
        document.getElementById('theme').className = 'light';
        localStorage.setItem('theme', 'light')
    } else {
        document.getElementById('theme').className = 'dark';
        localStorage.setItem('theme', 'dark')
    }
}

document.getElementById('btn_theme').addEventListener('click',  changeTheme);
document.getElementById('btn_theme_mobile').addEventListener('click',  changeTheme);