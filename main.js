const { app, BrowserWindow, ipcMain } = require('electron')
const nodecpp = require('./build/Release/nodecpp.node')
let mainWindow
app.whenReady().then(() => {
    const path = require('node:path')
    mainWindow = new BrowserWindow({
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
ipcMain.handle('get-cpp-ips', (_) => {
    return nodecpp.getIps()
})
ipcMain.handle('start-cpp-network', (_, ...args) => {
    nodecpp.startNetwork(...args, (ip, port, msg) => {
        mainWindow.webContents.send('connected', ip, port, msg)
    })
})
ipcMain.handle('send-client', (_, ...args) => {
    nodecpp.sendClient(...args)
})