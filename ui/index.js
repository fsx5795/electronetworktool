let otherSelect
function netLink(un, ip, port) {
    if (un) {
        const options = Array.from(otherSelect.options)
        const rmOpt = options.find(opt => opt.text === ip)
        if (rmOpt)
            otherSelect.options.removeChild(rmOpt)
    } else {
        otherSelect.options.add(new Option(ip, port))
    }
}
function showInfo(ip, port, msg) {
    const textarea = document.querySelector('textarea')
    textarea.value += ip + ':' + port + '\n' + msg
}
document.addEventListener('DOMContentLoaded', () => {
    otherSelect = document.getElementById('other')
    electronApi.setCallback()
    const localSelect = document.getElementById('local')
    electronApi.getIps().then(ips => {
        for (let i in ips)
            localSelect.options.add(new Option(ips[i]))
    })
    const btns = document.getElementsByTagName('button')
    for (let btn of btns) {
        btn.addEventListener('click', () => {
            const port = document.getElementById('port')
            if (btn.textContent === '开始') {
                const radios = document.getElementsByName('network')
                let type
                for (let i in radios) {
                    if (radios[i].checked == true) {
                        type = radios[i].value
                        break
                    }
                }
                electronApi.startNetwork(localSelect.value, Number(port.value), type)
                btn.textContent = '停止'
            } else if (btn.textContent === '停止') {
                electronApi.stopNetwork()
            } else {
                const input = document.getElementById('msg')
                const index = otherSelect.selectedIndex
                if (electronApi.sendClient(otherSelect.options[index].text, Number(otherSelect.options[index].value), input.value)) {
                    showInfo(localSelect.value, Number(port.value), input.value)
                    input.value = ''
                }
            }
        })
    }
    electronApi.onShowInfo(showInfo)
    electronApi.onNetLink(netLink)
})
