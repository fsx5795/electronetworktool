let otherSelect
function connect(un, ip, port) {
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
            if (btn.textContent === '开始') {
                const port = document.getElementById('port')
                const radios = document.getElementsByName('network')
                let type
                for (let i in radios) {
                    if (radios[i].checked == true) {
                        type = radios[i].value
                        break
                    }
                }
                electronApi.startNetwork(localSelect.value, Number(port.value), type)
            } else {
                const input = document.getElementById('msg')
                const index = otherSelect.selectedIndex
                electronApi.sendClient(otherSelect.options[index].text, otherSelect.options[index].value, input.value)
            }
        })
    }
    electronApi.onShowInfo(showInfo)
    electronApi.onConnect(connect)
})