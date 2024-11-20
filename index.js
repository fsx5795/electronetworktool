document.addEventListener('DOMContentLoaded', () => {
    electronApi.ipsFunction('ips').then(value => {
        const strList = value.split(',')
        const select = document.querySelector('select')
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
        //webui.startNetwork(select.value, port.value, type)
    })
})
