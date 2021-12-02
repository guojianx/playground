<script setup>
  import { Authenticator } from "@aws-amplify/ui-vue";
  import "@aws-amplify/ui-vue/styles.css";

  import Amplify from 'aws-amplify';
  import awsconfig from './aws-exports';

  Amplify.configure(awsconfig);
</script>

<template>
    <h1>Hello Vue</h1>
    <a>{{ count }}</a>
    
    <authenticator>
      <template v-slot="{ user, signOut }">
        <h1>Hello {{ user.username }}!</h1>
        <button @click="signOut">Sign Out</button>
      </template>
    </authenticator>
</template>

<script>
export default {
    name: 'app',
    data() {
        return {
            count: 12322
        };
    },

    created() {
        console.log("Here is created() XXXXX.");
        this.count = 99999;
    }
};
</script>