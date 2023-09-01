#!/bin/python3

import tkinter as tk
import tkinter.messagebox as messagebox
import sys
import re

class PathFinder:
    def __init__(self):
        self.file_stack = ['init.txt']
        self.line_stack = [0]
        self.choose_stack = []
        self.var_stack = [{}]
        self.goal = []
        self.search_space = 0
        self.too_many_options = False
        try:
            self.file = open('txt/init.txt', 'r', encoding='utf-8')
        except OSError as e:
            messagebox.showinfo('Error', 'Cannot open txt/init.txt')
        self.jump_to_label('STORY_START')

    def run(self):
        while True:
            line = self.file.readline()
            if not line:
                break
            line = line.replace('\n', '')

            #print(line)

            if line.startswith('@'):
                self.parse_line(line)
                if line.startswith('@choose ') or line.startswith('@選択肢 '):
                    return
            elif line.startswith(':GOAL_'):
                self.reach_goal(line)
                return

            self.line_stack.append(self.line_stack.pop() + 1)

            #print(self.file_stack[-1] + ':' + str(self.line_stack[-1]))

    def parse_line(self, line):
        if line.startswith('@choose ') or line.startswith('@選択肢 '):
            self.eval_choose_command(self.split_params(line))
        elif line.startswith('@set ') or line.startswith('@フラグをセット'):
            self.eval_set_command(line.split(' '))
        elif line.startswith('@if ') or line.startswith('@フラグでジャンプ '):
            self.eval_if_command(line.split(' '))
        elif line.startswith('@goto ') or line.startswith('@ジャンプ '):
            self.eval_goto_command(self.split_params(line))
        elif line.startswith('@load ') or line.startswith('@シナリオ '):
            self.eval_load_command(self.split_params(line))

    def split_params(self, line):
        params = []
        for p in re.split(r' \s*(?=(?:[^"]*"[^"]*")*[^"]*$)', line):
            if p.find('=') != -1:
                p = p.split('=')[1]
            params.append(p)
        return params

    def eval_choose_command(self, params):
        #print(params)

        # Error or warning by search space.
        param_count = (len(params) - 1) / 2
        self.search_space = self.search_space + param_count
        if self.search_space >= 100000:
            print(self.search_space)
            self.abort('Too many options.')
        if self.search_space >= 1000:
            if not self.too_many_options:
                messagebox.showinfo('Warning', 'Search will take a while.')
                self.too_many_options = True

        for i in range(8):
            if 1 + i * 2 >= len(params):
                break

            label = params[1 + i * 2]
            option = params[1 + i * 2 + 1]

            #print(option)

            if self.choose_stack.count(option) >= 2:
                #print('[loop detected] "' + option + '"')
                #print('stack:' + str(self.choose_stack))
                return

            # Restart search.
            self.push_context(option)
            self.jump_to_label(label)
            self.run()
            self.pop_context()

    def eval_set_command(self, params):
        lhs = params[1]
        op = params[2]
        rhs = params[3]

        assert lhs.startswith('$')
        lhs_index = int(lhs[1:])

        if rhs.startswith('$'):
            rhs_index = int(rhs.substring(1))
            if self.var_stack[-1].haskey(rhs_index):
                rhs_value = self.var_stack[-1][rhs_index]
            else:
                rhs_value = 0
        else:
            rhs_value = int(rhs)

        if op == '=':
            self.var_stack[-1][lhs_index] = rhs_value
        elif op == '+=':
            self.var_stack[-1][lhs_index] += rhs_value
        else:
            self.abort('Unknown op "'+ op + '".')

    def eval_if_command(self, params):
        lhs = params[1]
        op = params[2]
        rhs = params[3]
        label = params[4]

        assert lhs.startswith('$')
        lhs_index = int(lhs[1])
        if lhs_index in self.var_stack[-1]:
            lhs_value = self.var_stack[-1][lhs_index]
        else:
            lhs_value = 0

        if rhs.startswith('$'):
            rhs_index = int(rhs[1])
            if rhs_index in self.var_stack[-1]:
                rhs_value = self.var_stack[-1][rhs_index]
            else:
                rhs_value = 0
        else:
            rhs_value = int(rhs)

        if op == '==':
            if lhs_value == rhs_value:
                self.jump_to_label(label)
        elif op == '!=':
            if lhs_value != rhs_value:
                self.jump_to_label(label)
        elif op == '>':
            if lhs_value > rhs_value:
                self.jump_to_label(label)
        elif op == '>':
            if lhs_value >= rhs_value:
                self.jump_to_label(label)
        else:
            self.abort('Unknown op "'+ op + '".')

    def jump_to_label(self, label):
        label_line = ':' + label
        self.file.seek(0)
        line_num = 0
        while True:
            line = self.file.readline()
            if not line:
                self.abort('Label "' + label + '" not found.')
            line = line.replace('\n', '')
            if line == label_line:
                break
            line_num = line_num + 1

        self.line_stack.pop()
        self.line_stack.append(line_num)

    def eval_goto_command(self, params):
        label = params[1]
        if not label.startswith('$'):
            self.jump_to_label(label)

    def eval_load_command(self, params):
        txt = params[1]
        try:
            self.file.close()
            self.file = open('txt/' + txt, 'r', encoding='utf-8')
        except OSError as e:
            self.abort('Cannot open "txt/' + txt + '".')

        self.file_stack.pop()
        self.file_stack.append(txt)
        self.line_stack.pop()
        self.line_stack.append(0)

    def reach_goal(self, line):
        goal = line[6:]
        self.goal.append({"name": goal, "options": self.choose_stack.copy()})

    def push_context(self, option):
        self.file_stack.append(self.file_stack[-1])
        self.line_stack.append(self.line_stack[-1])
        self.choose_stack.append(option)
        self.var_stack.append(self.var_stack[-1].copy())

    def pop_context(self):
        self.file_stack.pop()
        self.line_stack.pop()
        self.choose_stack.pop()
        self.var_stack.pop()
        self.file.close()
        self.file = open('txt/' + self.file_stack[-1], 'r', encoding='utf-8')
        for i in range(self.line_stack[-1]):
            self.file.readline()

    def abort(self, msg):
        messagebox.showinfo('Error', self.file_stack[-1] + '(' + str(self.line_stack[-1]) + '): ' + msg)
        sys.exit(1)

    def write_result(self):
        sorted_goal = {}
        for g in self.goal:
            if not g['name'] in sorted_goal:
                sorted_goal[g['name']] = []
            sorted_goal[g['name']].append(g['options'].copy())

        messagebox.showinfo('Success', str(len(sorted_goal)) + ' routes found')

        f = open('routes.csv', 'w', encoding='shift-jis')
        print('GOAL,OPTION', file=f)
        for name in sorted_goal:
            for opt_list in sorted_goal[name]:
                print(name, end='', file=f)
                for opt in opt_list:
                    print(',' + opt, file=f)
        f.close()

if __name__ == '__main__':
    pf = PathFinder()
    pf.run()
    pf.write_result()
