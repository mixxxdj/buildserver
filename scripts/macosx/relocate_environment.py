#!/usr/bin/env python

import os
import sys
import subprocess

dry_run = False

def run_command(args):
    p = subprocess.Popen(args,
                         stdin=subprocess.PIPE,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE)
    out, err = p.communicate()
    status = p.returncode
    if status != 0:
        raise Exception('Could not run: %s' % ' '.join(args))
    return out


def is_macho(path):
    out = run_command(['file', path])
    # If the file is a universal (lipo'd) file, it always has "Mach-O universal
    # binary" in the first line. Use the last line to check for Mach-O.
    return 'Mach-O' in out.strip().split('\n')[-1]


def dynamic_dependencies(path):
    out = run_command(['otool', '-L', path])

    for line in out.split('\n'):
        # Lines starting with tab characters indicate library dependencies.
        if not line.startswith('\t'):
            continue
        line = line.strip()
        if not line:
            continue
        # Strip after the first '('
        line = line[:line.index('(') - 1]
        yield line


def change_id(path, new_id):
    print('%s: setting id %s' % (os.path.basename(path), new_id))
    if dry_run:
        return
    run_command(['install_name_tool', path, '-id', new_id])


def change_install_name(path, old_install_name, new_install_name):
    print('%s: changing dependency %s -> %s' % (
        os.path.basename(path), old_install_name, new_install_name))
    if dry_run:
        return
    run_command(['install_name_tool', path, '-change', old_install_name, new_install_name])

def add_rpath(path, rpath):
    print('%s: adding rpath %s' % (os.path.basename(path), rpath))
    if dry_run:
        return
    run_command(['install_name_tool', path, '-add_rpath', rpath])

def relocate_environment(lib_dir):
    system_prefixes = ['/usr/', '/System/', '/Library']

    for root, dirs, files in os.walk(lib_dir):
        for d in dirs:
            # Don't traverse into frameworks or bundles.
            if d.endswith('.framework') or d.endswith('.app'):
                dirs.remove(d)

        for f in files:
            path = os.path.join(root, f)

            # Skip symlinks.
            if os.path.islink(path):
                continue

            if not is_macho(path):
                continue

            libraries = list(dynamic_dependencies(path))

            is_dso = path.endswith('.dylib')
            if is_dso:
                # For DSOs, otool -L lists the install name ("id") of the
                # library itself on the first line. It's not a dependency of the
                # library.
                libraries = libraries[1:]
                change_id(path, '@rpath/' + f)

            relpaths = set()
            for library in libraries:
                if library.startswith('@'):
                    print('Skipping already-relative dependency: ' + library)
                    continue
                if any(library.startswith(prefix) for prefix in system_prefixes):
                    print('Skipping system dependency: ' + library)
                    continue
                print('Making dependency relative: ' + library)
                relpath = os.path.relpath(library, os.path.dirname(path))
                relpaths.add(os.path.dirname(relpath))
                relative_path = '@rpath/' + os.path.basename(relpath)
                change_install_name(path, library, relative_path)

            if not is_dso:
                for relpath in relpaths:
                    add_rpath(path, '@loader_path/' + relpath)


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('Usage: %s <path-to-environment>' % sys.argv[0])
        sys.exit(1)
    relocate_environment(sys.argv[1])
