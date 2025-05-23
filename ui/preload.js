const { contextBridge, ipcRenderer } = require('electron')
contextBridge.exposeInMainWorld('electronApi', {
    setCallback: () => ipcRenderer.invoke('set-callback'),
    getIps: async () => {
        return await ipcRenderer.invoke('get-cpp-ips')
    },
    startNetwork: (ip, port, type) => ipcRenderer.invoke('start-cpp-network', ip, port, type),
    sendClient: (ip, port, msg) => ipcRenderer.invoke('send-client', ip, port, msg),
    onConnect: (callback) => {
        ipcRenderer.on('connect', (_, un, ip, port) => {
            callback(un, ip, port)
        })
    },
    onShowInfo: (callback) => {
        ipcRenderer.on('show-info', (_, ip, port, msg) => {
            callback(ip, port, msg)
        })
    }
})