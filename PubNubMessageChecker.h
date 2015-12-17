class PubNubMessageChecker{
  String TimeToken;    
  QueueList <String> Messages;
  public:
    PubNubMessageChecker(){
      TimeToken = "0";
    }
    String operator()(String username){      
      if(!Messages.isEmpty ()){
        PRINT_LN("pop");
        return Messages.pop();
      }
      HttpClient client;
      PRINT_LN("username: " + username);
      String url = "http://pubsub.pubnub.com/subscribe/" + String(PUBNUB_SUBSCRIBE_KEY) + "/" + username + String(EMBEDDED_CHANNEL) + "/0/" + String(TimeToken);
      
      char next_char;
      String thejson;
        
      client.getAsynchronously(url);
      
      // Wait for the http request to complete
      while (!client.ready()) {}
    
      while (client.available()) {
        next_char = client.read();
        if(String(next_char) == '\0') {
        break;
        } else {
          //PRINT_LN(next_char);
          thejson += next_char;
        }
      }
      StaticJsonBuffer<200> jsonBuffer;             
      JsonArray& rootMessage = jsonBuffer.parseArray(thejson);      
      //Messages from the rest api look like this [["test"],"14445945018090212"]
      //The first element is the list of messages, the second is a timetoken specific to the rest api.
      TimeToken = rootMessage[1].asString();  
      PRINT_LN("TIME TOKEN: " + TimeToken);
      JsonArray& messages = rootMessage[0].asArray();
      
      int i = 0;      
      for(JsonArray::iterator messageIt = messages.begin(); i < 5 && messageIt != messages.end(); ++messageIt, ++i){  //because of low memory we'll only check for five missed messages        
        const char * s = *messageIt;
        PRINT_LN("loop");        
          Messages.push(s);
          PRINT_LN("push");
      }      
      SERIAL_FLUSH();
      if(!Messages.isEmpty ()){
        PRINT_LN("pop");
        return Messages.pop();
      }
      else{
        return "";
      }
    }
};
