const { app, BrowserWindow, ipcMain } = require('electron')
const nodecpp = require('../src/build/Release/nodecpp.node')
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
ipcMain.handle('set-callback', (_) => {
    nodecpp.setCallback((un, ip, port) => {
        mainWindow.webContents.send('net-link', un, ip, port)
    }, (ip, port, msg) => {
        mainWindow.webContents.send('show-info', ip, port, msg)
    })
})
ipcMain.handle('get-cpp-ips', (_) => nodecpp.getIps())
ipcMain.handle('stop-network', (_) => nodecpp.stopNetwork())
ipcMain.handle('start-network', (_, ...args) => {
    nodecpp.startNetwork(...args)
})
ipcMain.handle('send-client', (_, ...args) => {
    nodecpp.sendClient(...args)
})
