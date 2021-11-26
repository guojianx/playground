import { fromCognitoIdentityPool } from "@aws-sdk/credential-providers";
import { DynamoDBClient, CreateTableCommand } from "@aws-sdk/client-dynamodb";
import { createServer } from "http";

const REGION = "us-west-1";
const COGNITO_ID = 'cognito-idp.us-west-1.amazonaws.com/us-west-1_qe3U8Pht8';


// Set the parameters
const params = {
  AttributeDefinitions: [
    {
      AttributeName: "Season", //ATTRIBUTE_NAME_1
      AttributeType: "N", //ATTRIBUTE_TYPE
    },
    {
      AttributeName: "Episode", //ATTRIBUTE_NAME_2
      AttributeType: "N", //ATTRIBUTE_TYPE
    },
  ],
  KeySchema: [
    {
      AttributeName: "Season", //ATTRIBUTE_NAME_1
      KeyType: "HASH",
    },
    {
      AttributeName: "Episode", //ATTRIBUTE_NAME_2
      KeyType: "RANGE",
    },
  ],
  ProvisionedThroughput: {
    ReadCapacityUnits: 1,
    WriteCapacityUnits: 1,
  },
  TableName: "TEST_TABLE", //TABLE_NAME
  StreamSpecification: {
    StreamEnabled: false,
  },
};

/*

$ curl --location --request POST 'https://hello.auth.us-west-1.amazoncognito.com/oauth2/token' \
> --header 'Content-Type: application/x-www-form-urlencoded' \
> --header 'Authorization: Basic MmxldHIwcGFidG9iaTM1MXEwaDFrYXRwdTM6MWFoZWg5cDRoZjlkcGdzbGFmbWtuYzZrbDZhZ2hiaW04Mm9vNWdsMGtyY28yN3Y1dGwwMA==' \
> --data-raw 'grant_type=authorization_code&client_id=2letr0pabtobi351q0h1katpu3&code=29bdd471-6b71-4ba8-8162-a289b91f24ab&redirect_uri=http://localhost:3000/'
{"id_token":"eyJraWQiOiJqZitPYXNkeHlXMjV0OGlRWEJLUGZEcDczb3RIaHNkbFZCYThPc04xTU5ZPSIsImFsZyI6IlJTMjU2In0.eyJhdF9oYXNoIjoiNWZrcVhMRXNMNVdTYVlKcVg4UWo5ZyIsInN1YiI6ImM4N2Q4NDU3LWY2OWItNGI1Ni1iOTUxLWEyNzljNDFhNzgyMCIsImVtYWlsX3ZlcmlmaWVkIjp0cnVlLCJpc3MiOiJodHRwczpcL1wvY29nbml0by1pZHAudXMtd2VzdC0xLmFtYXpvbmF3cy5jb21cL3VzLXdlc3QtMV9xZTNVOFBodDgiLCJjb2duaXRvOnVzZXJuYW1lIjoic2ltdWx0YW5laW91c2x5Iiwib3JpZ2luX2p0aSI6IjBjODQ0NWRhLTc0YmItNDMzMi1iNTM1LThmN2NhZTQxZmVkYSIsImF1ZCI6IjJsZXRyMHBhYnRvYmkzNTFxMGgxa2F0cHUzIiwidG9rZW5fdXNlIjoiaWQiLCJhdXRoX3RpbWUiOjE2Mzc5MDExOTQsImV4cCI6MTYzNzkwNDc5NCwiaWF0IjoxNjM3OTAxMTk0LCJqdGkiOiIzMmY1MmI5OS1mODQyLTRjY2YtYTNiOS1mN2I3NmQ1YTBhMTYiLCJlbWFpbCI6Imphbi5neHhAaG90bWFpbC5jb20ifQ.KPdWgm1KJSyMWRXqI9Iv9IxcPfQJGgAkdXsscwzvHw1HJOuciDq_nZrxm3wbhMLmQhR7D-_5TAt6tYxEPYe3pwbVroid24wcxwE74LtI9ecoFP5FbhmQ3dmMZHaBAT2sEfz_Xsd-CYyOkfd0-ejuulxUgxJ8QAz0T1B5IpXnkKQa0cB2m2eD1OliKbJE3ZtG0Tsic3LhLK1G1wCA35gj-Hpi4t2owvt-G_TyC4GV23yGWBPtRnnDVKbJ2riyOrVl71YJqWu4u-W3mTY_0YNUg6BDu8KXA52uvklOiW2K27y5gEqA_Bxe6JU2-QgkxGNVH1ZwwkxUVZ2vIfUyYgulqQ","access_token":"eyJraWQiOiJZbFN5RnhjVmxPNk5HNkwxRmZaRGNGbm03ZmxtVzJGbzhmTnJ6WDFFQ25jPSIsImFsZyI6IlJTMjU2In0.eyJvcmlnaW5fanRpIjoiMGM4NDQ1ZGEtNzRiYi00MzMyLWI1MzUtOGY3Y2FlNDFmZWRhIiwic3ViIjoiYzg3ZDg0NTctZjY5Yi00YjU2LWI5NTEtYTI3OWM0MWE3ODIwIiwidG9rZW5fdXNlIjoiYWNjZXNzIiwic2NvcGUiOiJhd3MuY29nbml0by5zaWduaW4udXNlci5hZG1pbiBwaG9uZSBvcGVuaWQgcHJvZmlsZSBlbWFpbCIsImF1dGhfdGltZSI6MTYzNzkwMTE5NCwiaXNzIjoiaHR0cHM6XC9cL2NvZ25pdG8taWRwLnVzLXdlc3QtMS5hbWF6b25hd3MuY29tXC91cy13ZXN0LTFfcWUzVThQaHQ4IiwiZXhwIjoxNjM3OTA0Nzk0LCJpYXQiOjE2Mzc5MDExOTQsInZlcnNpb24iOjIsImp0aSI6IjI1YjdiNmY1LTcwMWItNGU5Zi04NjViLTgxMjU3NjA1Nzc1NSIsImNsaWVudF9pZCI6IjJsZXRyMHBhYnRvYmkzNTFxMGgxa2F0cHUzIiwidXNlcm5hbWUiOiJzaW11bHRhbmVpb3VzbHkifQ.eGmUhj81tNu5qrZP-54HjUnZzSvhQYeI8xA6rso5K07q31PgrW6DeJ-o5RzB0SBsQfO59julLsG23mT6ppabK0-NveH-yBRI4id1O4VJmRfdYbe8cdkN8GDCnmrHd_xF7cJJnjcwXG6U5Bjsc4PCkNpV-WIhTb1k4DJd8vTE88kU9fU9FBxqHWWJwZ0CH7BtFxYwzzxk_9lf3LDOdgdwEHhN1_Z8ijfzb61nwxP9RPTsmvC0UxSNWgO3nnxOPOKxHvHzAZKuHRK5qyd35sd2QAdQrt8WQXwhnnL6T9pbqVoiI_lHMvuewbqyKl6bnOx5sv2VG0VzCTTtTTDjqzs-ow","refresh_token":"eyJjdHkiOiJKV1QiLCJlbmMiOiJBMjU2R0NNIiwiYWxnIjoiUlNBLU9BRVAifQ.QERPT-831HN7Lq9qrQxk0T5n1NngADMSFexfyp7WK9g8sklqGPjDHsYVYLBvT_CU95H7jIFZqpWpCTwqzFCvA9Oxun1EfzCSwP2--Aut6uLxj1rncxvneMIN8CFxHOTeZH2Oj5QUwLyLXCO77dEnZ3eXXekea9byklasoZxrg89-Vt48GQsTOJhquPOuGkT3ahVyD5mM4cT_GM2FdA3PJIztOX4x5ZMLlcZjVQjHN0wFnGMQbD1ITktWiLeSsF3AGhNK__3yH5SoYB8dDLwv3KWOymurJJ3Z1JsWohCwEJ4kK-pfIuXplww5jTZxzhGtBBlCQDdbYQT9UhS9lTORhw.qXVfPVK7YUSXkQc1.7zw87DCuPBaeWg-NC3r0xtUjb6qP134oLeYYIEO6ZvSPS-KlRiXTwvfhBUAlLjeMvdLrulBGpZ0SYO_WgPmsWVFStSJgvoMYVy9_P_YoFaZZbKrC46pYWqGd1XEXtHep371J9eSAJsQG4KntfSKU7hldxQ8kitBHFO-3_MsjHwOrIutQoFZLqKI80bOA59mBQIJuy-gh9PitlHQLZTcG1SPVzbxdjN3K9DM-VLw5TXlsAeLLXhoIitRiXqhg8z8yTwTd60b0RYqy6yRDJOSqxk1CHGZoVX_TlcRjLIYjnMeSNH5pR_cSfMU5qEImIzA8RXTxNl0erIIWEpWw0zCTfmBto92r2ofINQReDcvhm8bzQqcY_zckyYLvRu72AZsDWP0peeqBwa7v6AErW5umTp1vzUBFe8YpyaXrAeOIjmiSiz0bVlxJWVSTRk3fwbjrXrSLhCGYQhfTMTlDFsXDKC3xxTx1Iuva0PXdZN5oBoSrgwkxoKjOEJecLm17x_0pwaoebsW9n2N9iCJwt_0lK3a70nhoyDu0qTCdC4UdvsfBjD1cV0_EggkxWERyOIfrOnO12bEBZA85emNdYo2XABgt13WhCxqpwY5eMibEihIVx-4-LCkX8swvca1Dbevdbh8K04O41tj2SUK92yx6gbxYl4_jekZvP8rXgMZzzizyy50Ze9BBr7aRv7HZcNoqEKLkBrMsJOislgZSYfI5trJ_8KeoUoNU2Dw048X77Aey6tA4TqhLV-_E1ykDI_NYv9bFrlnVLqvHD6nabz79v2zzD1gt7xStFRFWH9htSM6KKafOEC4n8wrhLFl2CJdDdUUAjGSzE6sEIGMhFwEUZ2RCeDhqoAk_EZNJ9PjQoDS_O-qPvjikmrW9lRBBdpAOvi4j08Hl84hValiH7kPmieeR-XtSqcLk2AKFe5ORE1x14D6UwwuQeGk8zVPv3YWbBstOE2UTtQMe2lSeyn6Y-krP3bT_IsHuYsMlzIgftfz69v8QZnd-2sHrpzE26VKXfaY5dNOX8jTzSogSnfJRW2RtGLrW3_ninZsNN_S4obQ64SFxVpOF72umQTau5NgeacC9eFGK8My64SHYiKw8L9HFR69G40Mf8A4KvkjbQ-g9mO5ltvof47g7gMDRQ51ZfMW9yS5RVOVtIvB3R4GGK8OquvqBS42RlCbAf2N_L8pFLr3uwsJuZQRDq3cGQbJOqVj1MSIfLg9ndf3le5EYHsPWxWXSzclNghaExdnS2tNACG78u1Z9BRe3oD57q3J7uw.Nl9EIEMTrlnLFEda0nkypQ","expires_in":3600,"token_type":"Bearer"}

*/


const server = createServer((req, res) => {
    let u = new URL(req.url, `http://${req.headers.host}`);
    let idtoken = u.searchParams.get('code');
    let loginData = { [COGNITO_ID]: idtoken, };
    
    if (idtoken) {
        console.log(`ID Token: ${idtoken}`);

        const ddbClient = new DynamoDBClient({
            region: REGION,
            credentials: fromCognitoIdentityPool({
                clientConfig: {region: REGION},
                identityPoolId: 'us-west-1:1802a9c7-7061-451e-8f3a-f21ba70f52f6',
                logins: {  },
            }),
        });

        const run = async() => {
            try {
                const data = await ddbClient.send(new CreateTableCommand(params));
                console.log("Table Created", data);
                return data
            }
            catch (err) {
                console.log("Error", err);
            }
        }

        run();

        res.write("Hello Node.");
        res.end();

    }
});

server.listen(3000);
console.log("Listen to the port 3000");
