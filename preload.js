const { contextBridge, ipcRenderer } = require('electron')
contextBridge.exposeInMainWorld('electronApi', {
    getIps: async () => {
        return await ipcRenderer.invoke('getCppIps')
    },
    startNetwork: (ip, port, type) => {
        ipcRenderer.invoke('startCppNetwork', ip, port, type)
    }
})