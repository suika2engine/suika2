suika2.com
==========
The contents of the `suika2.com` Web site excluding the downloads and the Wiki.

## Applying the header/footer templates

* Edit the Japanese header in `web/inc/header.html`
* Edit the Japanese footer in `web/inc/footer.html`
* Edit the English header in `web/inc/header_en.html`
* Edit the English footer in`web/inc/footer_en.html`
* Run `./web/templates.sh`

## Applying the release version number

* Edit `ChangeLog`
* Run `./we/version.sh`

## Upload

* Run `./web/upload.sh`.

You need a script named `ftp-upload.sh` in your path, e.g.,:
```
#!/bin/sh

scp $1 SSH_SERVER:/path/to/html/$2
```
