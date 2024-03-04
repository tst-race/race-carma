When moving to a new preview branch from Stealth, do the following:
```
git rebase -i upstream/<preview-branch-name>
```

Then set to 'drop' the initial commit (which is Stealth's), but retain all the other commits (which are this fork's)