#!/usr/bin/bash

# Setup note: All placeholder values contain FIXME. Search for FIXME and fix all the corresponding things. Make sure to
# remove the error messages as well.

echo "FIXME: Update CB_REMOTE. This must be a valid remote." && exit 1
# Note: this does not need to be a codeberg URL, nor an SSH URL. You can also add more remotes if you want.
CB_REMOTE="git@codeberg.org:FIXME/FIXME"

# Note: We avoid using BASH_SOURCE largely because it seems to be incompatible with worktrees, based on a similar
# problem I ran into at work. There's lots of ways to do this though.
git_root=$(git rev-parse --show-toplevel)
cd "$git_root"

echo "FIXME: Remove this cd if your page is directly under the git root. Otherwise, make sure it's correct." && exit 1
cd FIXME

rm -rf pages

time -- "violet" generate -d
if [[ $? != 0 ]]; then
    echo "Generation failed"
    exit 1
fi

cd pages
git init
git checkout -b pages
git remote add cb "$CB_REMOTE"
# Example: multiple push remotes
# git remote add gh "git@github.com:user/repo"

git add -A
git commit -m "Generate page"

git push -fu cb pages
# Example(cont.): multiple push remotes
# git push -fu gh pages
