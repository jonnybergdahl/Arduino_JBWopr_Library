#!/usr/bin/env zsh

# Check for uncommitted changes
#if [ -n "$(git status --porcelain)" ]; then
#  echo "There are uncommitted changes in the Git repository. Aborting."
#  exit 1
#fi

start_path=$(pwd)
/opt/homebrew/bin/doxygen Doxyfile
rm -r ~/Developer/jonnybergdahl/jonnybergdahl.github.io/jbwopr/*
cp -r docs/* ~/Developer/jonnybergdahl/jonnybergdahl.github.io/jbwopr

# Commit and push docs
cd ~/Developer/jonnybergdahl/jonnybergdahl.github.io
git checkout main
git add jbwopr
if [ -n "$(git status --porcelain)" ]; then
  git commit -m "Update JBWOPR docs"
  git push
else
   echo "Branch main is up to date,nothing to do."
fi
cd "$start_path"
