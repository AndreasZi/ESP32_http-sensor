#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "secrets.h" 

#define REFRESH_INTERVALL "1000"
#define VARIABLE_SPACE 7


String index_html; //MainPage can be eddited after all variables have been added
const String JS = 
    "<script onload='clearInterval(myInterval);'>"
    "const variables=[];"
    "const checkboxStates = {};"
    "let myInterval=setInterval(updateVariables, " REFRESH_INTERVALL ");"
    "async function requestVariableState (){const response = await fetch('data');return response.json()};"
    "function updateVariables (){requestVariableState().then(data=>{dt=data;for(let x in variables){document.getElementById(variables[x]).innerHTML=dt[variables[x]];};});};"
    "async function requestState (path){var x = await fetch(path);};"  
    "function updateCheckbox (checkboxId){if(checkboxStates[checkboxId]){checkboxStates[checkboxId]=0;}else{checkboxStates[checkboxId]=1;};requestState('data?'+checkboxId+'='+checkboxStates[checkboxId]);};"//
    "function updateSlider (sliderId){requestState('data?'+sliderId+'='+document.getElementById(sliderId).value);};"
    "</script>";


int variableCount = 0;
String variableId[VARIABLE_SPACE]; //All the variableId are saved here

//Pointer-arrays for useful Data-Types
int* intReferenceStack[VARIABLE_SPACE];
float* floatReferenceStack[VARIABLE_SPACE];
String* stringReferenceStack[VARIABLE_SPACE];


String addCheckbox(String id, int &variable){
    intReferenceStack[variableCount] = &variable;
    variableId[variableCount] = id;
    variableCount++;
    return "<input id='"+id+"' type='checkbox' onchange=\"updateCheckbox ('"+id+"');\"></input><script>checkboxStates[\""+id+"\"]="+String(*intReferenceStack[variableCount-1])+";</script>";
}


String addSlider(String id, int &variable){
    intReferenceStack[variableCount] = &variable;
    variableId[variableCount] = id;
    variableCount++;
    return "<input id='"+id+"' type='range' min='0' max='100' onchange=\"updateSlider ('"+id+"');\"></input>";
}

String addForm(String id, int &variable){
    intReferenceStack[variableCount] = &variable;
    variableId[variableCount] = id;
    variableCount++;
    return "<input id='"+id+"' type='number' oninput=\"updateSlider ('"+id+"');\"></input>";
}




//Adding a variable to the update-cycle
String addVariable(String id, int &variable){ 
    intReferenceStack[variableCount] = &variable; //Save the Reference to the corresponding array
    variableId[variableCount] = id; //Save the id to the variableId array
    variableCount++; //Stepusp variableCount to keep track of array-size
    return "<a id='"+id+"'>-</a></div><script>variables.push('"+id+"'); </script>";
}
//See Above but for floatReferenceStack
String addVariable(String id, float &variable){
    floatReferenceStack[variableCount] = &variable;
    variableId[variableCount] = id;
    variableCount++;
    return "<a id='"+id+"'>-</a></div><script>variables.push('"+id+"'); </script>";
}
//See Above but for StringReferenceStack
String addVariable(String id, String &variable){
    stringReferenceStack[variableCount] = &variable;
    variableId[variableCount] = id;
    variableCount++;
    return "<a id='"+id+"'>-</a></div><script>variables.push('"+id+"'); </script>";
}




// This is where the webserver is initialized by the standard of the ESP-Async library
AsyncWebServer server(80);

void Webinterface(){
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    }

    //main page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html",  JS + index_html);
    });

    //data-api for live-updates
    server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = "{";
        for(int i=0; i<variableCount; i++){

            if(request->hasParam(variableId[i])) {
                *intReferenceStack[i]=request->getParam(variableId[i])->value().toInt();
            }

            if (i != 0) json += ", ";
            if (intReferenceStack[i]){
                json += "\"" + variableId[i] + "\": " + String(*intReferenceStack[i]);
            }
            if (floatReferenceStack[i]){
                json += "\"" + variableId[i] + "\": " + String(*floatReferenceStack[i]);
            }
            if (stringReferenceStack[i]){
                json += "\"" + variableId[i] + "\": \"" + String(*stringReferenceStack[i])  + "\"";
            }
        }
        json += "}";
        request->send(200, "text/plain", json);
    });


    

    server.begin();
    
    Serial.begin(9600);
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}