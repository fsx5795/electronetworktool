const { app, BrowserWindow, ipcMain } = require('electron')
const nodecpp = require('./build/Release/nodecpp.node')
app.whenReady().then(() => {
    const path = require('node:path')
    const mainWindow = new BrowserWindow({
        width: 800,
        height: 600,
        webPreferences: {
            preload: path.join(__dirname, 'preload.js')
        },
        autoHideMenuBar: true
    })
    mainWindow.loadFile('index.html')
    mainWindow.webContents.openDevTools()
})
app.on('window-all-closed', () => {
    app.quit()
})
ipcMain.handle('getCppIps', (_) => {
    return nodecpp.getIps()
})
ipcMain.handle('startCppNetwork', (_, ...args) => {
    nodecpp.startNetwork(...args)
})