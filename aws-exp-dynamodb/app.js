import { fromCognitoIdentityPool } from "@aws-sdk/credential-providers";
import { DynamoDBClient, CreateTableCommand } from "@aws-sdk/client-dynamodb";
import { createServer } from "http";
import { request } from "https";

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

function helloDynamo(param = null) {

    if (!param) {
        console.log("Hello Dynamo with invalid parameter.")
        return;
    }

    console.log("Hello Dynamo, ID Token:", param);
    let loginData = { [COGNITO_ID]: param, };

    const ddbClient = new DynamoDBClient({
        region: REGION,
        credentials: fromCognitoIdentityPool({
            clientConfig: {region: REGION},
            identityPoolId: 'us-west-1:1802a9c7-7061-451e-8f3a-f21ba70f52f6',
            logins: loginData,
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
}

const options = {
    hostname: "hello.auth.us-west-1.amazoncognito.com",
    port: 443,
    path: '/oauth2/token',
    method: 'POST',
    headers: {
        'Content-Type': 'application/x-www-form-urlencoded',
        'Authorization': 'Basic MmxldHIwcGFidG9iaTM1MXEwaDFrYXRwdTM6MWFoZWg5cDRoZjlkcGdzbGFmbWtuYzZrbDZhZ2hiaW04Mm9vNWdsMGtyY28yN3Y1dGwwMA=='
    }
};

const httpsReq = request(options, res => {
    console.log("statusCode:", res.statusCode);

    res.on("data", d => {
        const data = JSON.parse(d.toString());
        console.log(data);
        helloDynamo(data.id_token);
    });
});

httpsReq.on("error", error => {
    console.log("HTTPS POST on Error:", error);
});


const server = createServer((req, res) => {
    let u = new URL(req.url, `http://${req.headers.host}`);
    let authCode = u.searchParams.get('code');

    if (authCode) {
        let data = `grant_type=authorization_code&client_id=2letr0pabtobi351q0h1katpu3&code=${authCode}&redirect_uri=http://localhost:3000/`
        console.log(`raw-data: ${data}`);
        httpsReq.write(data);
        httpsReq.end();
   

        res.write("Hello Node.");
        res.end();

    }
});

server.listen(3000);
console.log("Listen to the port 3000");
