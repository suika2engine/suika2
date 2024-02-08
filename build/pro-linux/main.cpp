/* -*- coding: utf-8; tab-width: 4; indent-tabs-mode: nil; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2024, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  - 2023-09-07 Created.
 */

#include "mainwindow.h"

extern "C" {
#include "suika.h"
#include "glrender.h"
};

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Create the main window and run app.
    MainWindow w;
    w.show();
    int ret = a.exec();

    return ret;
}
