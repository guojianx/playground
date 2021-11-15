const HelloVueApp = {
	data() {
		return {
			message: 'Hello Vue!!'
		}
	}
}

function hi(name) {
	let phrase = `Hello, ${name}`;
	Vue.createApp(HelloVueApp).mount('#hello-vue');
}