# SnapTrack

## Overview
SnapTrack is a VST3 plugin designed to integrate version control seamlessly into your digital audio workstation (DAW). It allows you to take snapshots of your project as you work, making it easy to experiment, undo changes, and manage different versions of your music. Whether you're new to version control or already familiar with Git, SnapTrack provides an intuitive interface tailored for music producers.

## Key Features
- **Automatic Git Integration:** SnapTrack sets up a Git repository in your project directory when you first add the plugin, making version control effortless.
- **Simple Controls:** 
  - **Checkout:** Revert to a previous snapshot of your project from any point in history.
  - **Create Branch:** Create a new version of your project to experiment with different ideas without affecting your main project.
  - **Delete Branch:** You tried something new, but it didn’t work out. No problem—delete the branch and go back to your original project.
  - **Merge**: If you like the changes you made in a branch, merge them back into your main project.
  - **Return:** Go back to the most recent snapshot of your project.
- **Auto Commit:** Automatically creates a snapshot of your project every time you save your DAW project file.
- **Visual History:** Navigate through your project’s history with a simple commit viewer, making it easy to track changes over time.

## Getting Started
1. **Install SnapTrack:** Load the plugin into your DAW as you would with any other VST3.
2. **Select Your Project Directory:** The first time you use SnapTrack for each project, press 'Browse' to select your project folder. SnapTrack will initialize a Git repository here. Note: Do not select the project file itself; choose the directory containing the project file.
3. **Start Creating:** As you work, SnapTrack will automatically take snapshots of your project whenever you save, allowing you to go back, branch out, or move forward as needed.

## Advanced Use
While SnapTrack simplifies version control, it’s built on the powerful Git system. If you want more control over your project's version history, consider spending some time learning Git. I highly recommend reading at least the first few chapters of [Pro Git](https://git-scm.com/book/en/v2) to get a deeper understanding Git.
