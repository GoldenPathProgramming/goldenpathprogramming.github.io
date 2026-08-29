document.currentScript.insertAdjacentHTML('beforebegin',`
<title>The Golden Path of Game Programming</title>
<link href="/style.css" rel="stylesheet">
<link rel="icon" href="/favicon.ico">
`);

window.goatcounter = {
    endpoint: 'https://ultiman3rd.goatcounter.com/count',
};

const gc = document.createElement('script');
gc.async = true;
gc.src = '//gc.zgo.at/count.js';
document.head.appendChild(gc);