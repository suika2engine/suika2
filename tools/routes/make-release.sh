#!/bin/bash

set -eu

[ -e suika-routes.exe ]

zip -r suika-routes.zip suika-routes.exe txt
