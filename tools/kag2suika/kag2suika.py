#!/bin/python3

import sys
import os
import shutil
import re

base_dir = ''

chara_pos_dict = {}
chara_file_dict = {}

def convert_ks_to_txt(file_name):
    lines = ''
    with open(file_name, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    f = open('suika/txt/' + os.path.basename(file_name), 'w', encoding='utf-8')
    character_name = ''
    serif_body = ''
    message = ''
    print(':start', file=f)
    for line in lines:
        if line == '':
            continue
        elif line.startswith('[tb_title '):
            m = re.compile('name="(.+?)"').search(line)
            title = m.group(1)
            print('@chapter "' + title + '"', file=f)
        elif line.startswith('*'):
            m = re.compile('^\*(.+?)').search(line)
            label = m.group(1)
            print(':' + label, file=f)
        elif line.startswith('[bg '):
            m = re.compile('storage="(.+?)"').search(line)
            file = m.group(1)
            m = re.compile('time="(\d+?)"').search(line)
            if m:
                time = str(float(m.group(1)) / 1000)
            else:
                time = '0'
            print('@bg ' + file + ' ' + time, file=f)
        elif line.startswith('[playbgm '):
            m = re.compile('volume="(\d+?)"').search(line)
            vol = float(m.group(1)) / 100
            if vol < 0:
                vol = '0'
            elif vol > 1:
                vol = '1'
            else:
                vol = str(vol)
            m = re.compile('time="(\d+?)"').search(line)
            time = str(float(m.group(1)) / 1000)
            print('@vol bgm ' + vol + ' ' + time, file=f)
            m = re.compile('storage="(.+?)"').search(line)
            file = m.group(1)
            print('@bgm ' + file, file=f)
        elif line.startswith('[stopbgm '):
            m = re.compile('time="(\d+?)"').search(line)
            time = str(float(m.group(1)) / 1000)
            print('@vol bgm 0.0 ' + time, file=f)
            print('@wait ' + time, file=f)
            print('@bgm stop', file=f)
        elif line.startswith('[bg '):
            m = re.compile('storage="(.+?)"').search(line)
            file = m.group(1)
            m = re.compile('time="(\d+?)"').search(line)
            time = str(float(m.group(1)) / 1000)
            print('@bg ' + file + ' ' + time, file=f)
        elif line.startswith('[playse '):
            m = re.compile('storage="(.+?)"').search(line)
            if m:
                file = m.group(1)
            else:
                file = "stop"
            m = re.compile('time="(\d+?)"').search(line)
            vol = str(float(m.group(1)) / 100)
            print('@se ' + file, file=f)
        elif line.startswith('[chara_show '):
            m = re.compile('name="(.+?)"').search(line)
            name = m.group(1)
            m = re.compile('storage="(.+?)"').search(line)
            if m:
                file = m.group(1)
            else:
                file = chara_file_dict[name]
            m = re.compile('time="(\d+?)"').search(line)
            time = str(float(m.group(1)) / 1000)
            # m = re.compile('left=""').search(line)
            # if m:
            #     pos = 'left'
            # else:
            #     m = re.compile('right=""').search(line)
            #     if m:
            #         pos = 'right'
            #     else:
            #         pos = 'center'
            pos = 'center'
            chara_pos_dict[name] = pos
            chara_file_dict[name] = file
            print('@ch ' + pos + ' ' + file + ' ' + time, file=f)
        elif line.startswith('[chara_mod '):
            m = re.compile('name="(.+?)"').search(line)
            name = m.group(1)
            m = re.compile('storage="(.+?)"').search(line)
            file = m.group(1)
            m = re.compile('time="(\d+?)"').search(line)
            time = str(float(m.group(1)) / 1000)
            print('@ch ' + chara_pos_dict[name] + ' ' + file + ' ' + time, file=f)
            chara_file_dict[name] = file
        elif line.startswith('[chara_hide '):
            m = re.compile('name="(.+?)"').search(line)
            name = m.group(1)
            m = re.compile('time="(\d+?)"').search(line)
            time = str(float(m.group(1)) / 1000)
            print('@ch ' + chara_pos_dict[name] + ' none ' + time, file=f)
            chara_pos_dict[name] = ''
        elif line.startswith('[quake '):
            m = re.compile('time="(\d+?)"').search(line)
            time = str(float(m.group(1)) / 1000)
            m = re.compile('count="(\d+?)"').search(line)
            count = m.group(1)
            print('@shake horizontal ' + time + ' ' + count + ' 100', file=f)
        elif line.startswith('[jump '):
            m = re.compile('storage="(.+?)"').search(line)
            if m:
                file = m.group(1)
                m = re.compile('target="\*(.+?)"').search(line)
                if m:
                    label = m.group(1)
                    print('@load ' + file + ' ' + label, file=f)
                else:
                    print('@load ' + file, file=f)
            else:
                m = re.compile('target="\*(.+?)"').search(line)
                label = m.group(1)
                print('@goto ' + label, file=f)
        elif line.startswith('#'):
            m = re.compile('#(.+)').search(line)
            if m:
                character_name = m.group(1)
            else:
                m = ''
        elif line.startswith('['):
             print('#ignore ' + line, file=f)
        elif line.startswith(';'):
             print('# ' + line, file=f)
        elif line == '\n':
            print('', file=f)
        else:
            if character_name == '':
                message = message + line.replace('[l]', '').replace('[r]', '').replace('[p]', '').replace('\n', '')
                if message != '' and line.find('[p]') > 0:
                    print(message, file=f)
            else:
                serif_body = serif_body + line.replace('[l]', '').replace('[r]', '').replace('[p]', '').replace('\n', '')
                if serif_body != '' and line.find('[p]') > 0:
                    print('*' + character_name + '*' + serif_body, file=f)
                    serif_body = ''

if __name__ == '__main__':
    if len(sys.argv) == 1:
        print('Usage: kag2suika [directory]')
        exit(1)

    base_dir = sys.argv[1]

    os.mkdir('suika')
    os.mkdir('suika/anime')
    os.mkdir('suika/cv')
    os.mkdir('suika/rule')
    os.mkdir('suika/wms')

    shutil.copytree(base_dir + '/data/bgimage', 'suika/bg')
    shutil.copytree(base_dir + '/data/fgimage', 'suika/ch')
    shutil.copytree(base_dir + '/data/bgm', 'suika/bgm')
    shutil.copytree(base_dir + '/data/sound', 'suika/se')
    shutil.copytree(base_dir + '/data/scenario', 'suika/txt')

    for f in os.listdir(base_dir + '/data/scenario'):
        if f.endswith('.ks'):
            convert_ks_to_txt(base_dir + '/data/scenario/' + f)
