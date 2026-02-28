main: {
    const self = document.currentScript;
    const data_source = self.dataset.src;
    fetch(data_source)
        .then(response => response.text())
        .then((data) => {
            const code_element = document.createElement ("code");
            code_element.textContent = data;
            self.parentNode.insertBefore (code_element, self);
    })
}