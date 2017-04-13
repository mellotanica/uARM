/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uARM
 *
 * Copyright (C) 2013 Marco Melletti
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "qarm/qarm.h"
#include <QApplication>
#include "armProc/machine_config.h"
#include <QFile>
#include <string>

QString MONOSPACE_FONT_SIZE = "MONOSPACE_FONT_SIZE=";
QString MONOSPACE_FONT_FACE = "MONOSPACE_FONT_FACE=";

void readConfigs(){
    QFile *def = new QFile("/etc/default/uarm");
    int fsize = 10;
    std::string fface = "Courrier";
    if (def->open(QFile::ReadOnly)) {
        char buf[1024];
        QString line;
        for(qint64 lineLength = def->readLine(buf, sizeof(buf)); lineLength != -1; lineLength = def->readLine(buf, sizeof(buf))){
            if(lineLength == 0 || buf[0] == '\n')         //empty line
                continue;
            line = QString::fromLocal8Bit(buf, lineLength);
            line = line.trimmed();
            if(line.startsWith('#'))    //comment line
                continue;
            if(line.contains(MONOSPACE_FONT_SIZE)){
                int optStart = line.indexOf('\'') + 1;
                int optLen = line.indexOf('\'', optStart) - optStart;
                QString opt = line.mid(optStart, optLen);
                fsize = opt.toUInt();
            }
            if(line.contains(MONOSPACE_FONT_FACE)){
                int optStart = line.indexOf('\'') + 1;
                int optLen = line.indexOf('\'', optStart) - optStart;
                fface = line.mid(optStart, optLen).toStdString();
            }
        }
    }
    MC_Holder::getInstance()->mainConfigs.monofont_size = fsize;
    MC_Holder::getInstance()->mainConfigs.monofont_face = fface;
}

int main(int argn, char **argv){
    int i;
    QFile *configFile = NULL;
    bool autorun = false, runandexit = false, gui = true;
    QApplication app(argn, argv);
    readConfigs();
    //FIXME: ugly debug dump
    MC_Holder::getInstance()->dumpExecution = false;
    if(argn > 1){
        for(i = 1; i < argn; i++){
            if(!strcmp(argv[i], "--dumpExec") && i < (argn - 1)){
                MC_Holder::getInstance()->dumpExecution = true;
                MC_Holder::getInstance()->dumpFilename = strdup(argv[i+1]);
                FILE*f = fopen(argv[i+1], "w");
                fprintf(f, "EXEC start:\n\n");
                fclose(f);
                i++;
            }
            if(!strcmp(argv[i], "-c") && i < (argn - 1)){
                configFile = new QFile(argv[i+1]);
                i++;
            }
            if(!strcmp(argv[i], "-e")){
                autorun = true;
            }
            if(!strcmp(argv[i], "-x")){
                runandexit = true;
            }
            if(!strcmp(argv[i], "--no-gui")){
                gui = false;
            }
        }
    }
    app.setFont(monoLabel::getMonospaceFont(), "procDisplay");
    qarm *wid = new qarm(&app, configFile, autorun, runandexit, gui);
    if(gui){
        wid->show();
    } else {
        app.setQuitOnLastWindowClosed(false);
    }
    return app.exec();
}
