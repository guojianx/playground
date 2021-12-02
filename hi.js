

const HelloVueApp = {
	data() {
		return {
			message: 'Hello Vue!!'
		}
	}
}

export function hi(name) {
	let phrase = `Hello, ${name}`;
	Vue.createApp(HelloVueApp).mount('#hello-vue');

}
