const { contextBridge, ipcRenderer } = require('electron')
contextBridge.exposeInMainWorld('electronApi', {
    ipsFunction: async type => {
        if (type === 'ips')
            return await ipcRenderer.invoke('getCppIps')
    }
})