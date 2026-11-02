#!/usr/bin/bash

CB_REMOTE="git@codeberg.org:LunarWatcher/magpie"

# Note: We avoid using BASH_SOURCE largely because it seems to be incompatible with worktrees, based on a similar
# problem I ran into at work. There's lots of ways to do this though.
git_root=$(git rev-parse --show-toplevel)
cd "$git_root"

cd docs

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
