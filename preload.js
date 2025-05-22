const { contextBridge, ipcRenderer } = require('electron')
contextBridge.exposeInMainWorld('electronApi', {
    getIps: async () => {
        return await ipcRenderer.invoke('get-cpp-ips')
    },
    startNetwork: (ip, port, type) => ipcRenderer.invoke('start-cpp-network', ip, port, type),
    sendClient: (ip, port, msg) => ipcRenderer.invoke('send-client', ip, port, msg),
    onConnected: (callback) => {
        ipcRenderer.on('connected', (_, ip, port, msg) => {
            callback(ip, port, msg)
        })
    }
})