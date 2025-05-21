function connected(ip, port, msg) {
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
    const btn = document.querySelector('button')
    btn.addEventListener('click', () => {
        const port = document.getElementById('port')
        const radios = document.getElementsByName('network')
        let type
        for (let i in radios) {
            if (radios[i].checked == true) {
                type = radios[i].value
                break
            }
        }
        const res = electronApi.startNetwork(select.value, Number(port.value), type)
        if (res != null && res !== '')
            alert(res)
    })
    electronApi.onConnected(connected)
})
