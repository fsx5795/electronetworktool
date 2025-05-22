let curIp, curPort
function connected(ip, port, msg) {
    curIp = ip
    curPort = port
    console.log(ip)
    console.log(port)
    console.log(msg)
}
document.addEventListener('DOMContentLoaded', () => {
    const select = document.querySelector('select')
    electronApi.getIps().then(value => {
        const strList = value.split(',')
        for (let i in strList)
            select.options.add(new Option(strList[i]))
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
                electronApi.startNetwork(select.value, Number(port.value), type)
            } else {
                const input = document.getElementById('msg')
                console.log(curIp)
                console.log(curPort)
                console.log(input.value)
                electronApi.sendClient(curIp, curPort, input.value)
            }
        })
    }
    electronApi.onConnected(connected)
})
