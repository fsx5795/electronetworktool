const { app, BrowserWindow, ipcMain } = require('electron')
const path = require('node:path')
app.whenReady().then(() => {
    const mainWindow = new BrowserWindow({
        width: 800,
        height: 600,
        webPreferences: {
            preload: path.join(__dirname, 'preload.js')
        },
        autoHideMenuBar: true
    })
    mainWindow.loadFile('index.html')
})
app.on('window-all-closed', () => {
    app.quit()
})
ipcMain.handle('getCppIps', (_) => {
    const nodecpp = require('./build/Release/nodecpp.node')
    return nodecpp.getIps()
})