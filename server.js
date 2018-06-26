
var express = require('express');   /*opening express module*/
var app = express();                /*use express under "app" variable*/
var path = require('path');            /*use path module*/
var Mongo=require('./node.js/mongo');
var session = require('express-session');
var email=require('./node.js/email');
var myParser = require("body-parser");
var url = require( "url" );
var queryString = require( "querystring" );

var mut;

var Algo=require('./Algo/main');
app.timeout=3600000;

var calculation_id;
const csv=require("csvtojson/v2");
/*data parser for json and non-json */
app.use(myParser.json());
app.use(myParser.urlencoded({extended: true}));
/*Session*/
/*like "createserver" on node.js its creating the server and the port he will listen*/
app.listen(8888, function () 
{
  console.log('Example app listening on port 8888!');

}).timeout=3600000;



 
/*Get accses to Home page */
app.get('/',function(req, res) 
{
    res.sendFile(__dirname+"/Html/Home.html");

});

app.get('/Home.html',function(req, res) 
{
    res.sendFile(__dirname+"/Html/Home.html");

});

app.get('/About.html', function(req, res) 
{
    res.sendFile(__dirname+"/Html/About.html");
});

app.get('/Help.html', function(req, res) 
{
    res.sendFile(__dirname+"/Html/Help.html");
});

app.get('/Calculation.html', function(req, res) 
{
    res.sendFile(__dirname+"/Html/Calculation.html");
});

app.get('/Contact.html', function(req, res) 
{
    res.sendFile(__dirname+"/Html/Contact.html");
});

app.get('/Userpage.html', function(req, res) 
{
    res.sendFile(__dirname+"/Html/Userpage.html");
  });

app.get('/Table.html', function(req, res) 
{
    res.sendFile(__dirname+"/Html/Table.html");
    
  });

  app.get('/Loading.html', function(req, res) 
{
    res.sendFile(__dirname+"/Html/Loading.html");
    
  });

app.get('/Table/get', function(req, res) 
{
    /*const csvFilePath='results.txt';
    const csv=require('csvtojson')
    

    csv().fromFile(csvFilePath).then((jsonObj)=>
    {
      res.json(200,JSON.stringify(jsonObj));
      })*/
    
  });
 
  


app.post('/calculation/Userpage', function(req, res) 
{
    Mongo.FindHistory(req.body.User,req,res);
  });


app.post('/send', function(req, res) 
{
  console.log("inside");
  email.contact(req, res);
});



app.post('/check', function(req, res) 
{
  Mongo.Find(req.body.username,res);
});


app.post('/login', function(req, res) 
{
  var login_user = 
    {
      Username: req.body.username,
      Password: req.body.password,
    }
  Mongo.Findtologin(req,login_user,res);
});


app.post('/register', function(req, res) 
{
    var new_user = 
    {
      Username: req.body.username,
      Password: req.body.password,
      Email:req.body.email
    }
    Mongo.Insert(new_user,res);

});

app.post('/calculation/solution',function(req,res)
{


  //ADD an array of results to existing calculation class//
    var Cal=req.body;

    //swrite the Cal to file and send to algorithm//./////////
    //Running Algorith//.//////////////
    Algo.Algoritm(Cal);
    const timeout = setInterval(function() {

      var fs = require('fs');  
      const file ='./Algo/end.txt';
      const fileExists = fs.existsSync(file);

      //console.log('Checking for: ', file);
      //console.log('Exists: ', fileExists);

      if (fileExists) 
      {
          clearInterval(timeout);

          var im = require ('imagemagick');
          im.convert(['./Algo/rna.ps','-flatten','./Algo/rna.jpg'],function(err,stdout){
              if(err) throw err;
              console.log('stdout:',stdout);
          });
      
          //const csvFilePath='Algo/results.txt';
          const csvFilePath='Algo/results.txt';
          const csv=require('csvtojson')
          
          csv().fromFile(csvFilePath).then((jsonObj)=>
          {
            console.log(jsonObj);
            Cal['Result'] = jsonObj;
            console.log(Cal);
            
      
            })
          
            Mongo.Insertcalc(Cal,res);
            Mongo.InsertHistory(Cal,res);
      
            if(email!="")
            {
              email.Calculation(Cal.Email,"http://localhost:8888/getcalc/byid/"+Cal.Date);
            }
      

            var filePath1 = './Algo/end.txt'; 
            fs.unlinkSync(filePath1);
      
        res.send(200,'/getcalc/byid/'+Cal.Date);
      
      }
  }, 1000);



    
});

app.post('/emailvalid',function(req,res)
{
    Mongo.Findemail(req.body.email,res);
});

app.post('/Recover', function(req, res) 
{
 
  Mongo.Recover(req.body.email,res);
});


//generate different links//

app.get('/getcalc/byid/:calcid', function(req, res) 
{
  console.log(req.params.calcid);
  calculation_id=JSON.parse(JSON.stringify(req.params.calcid));
  //calculation_id=req.params.calcid;
  res.redirect('/Table.html');  

  //Mongo.Findcalcbyid(req.params.calcid,req,res);
  //Mongo.Findcalcbyid(req.params.calcid,req,res);



});

//Temp for trying to get a calculation object//
app.get('/Temp', function(req, res) 
{
  Mongo.Findcalcbyid(calculation_id,req,res);

  

});













/**************************Get the images************************/
app.post('/getmutimage', function(req, res) 
{
  var fs = require('fs');  
  var logger = fs.createWriteStream('./Convert_ps/seq.fa',
  {
       flags: 'w' // 'a' means appending (old data will be preserved)
     });
     
  logger.write(req.body.Sequence+"\n");
  logger.end( );
  
  
  
  
  var nrc = require('node-run-cmd');
  var options = { cwd: './Convert_ps' };
  nrc.run('RNAfold --noLP seq.fa ', options).then(function(exitCodes) 
  {
  console.log("file exist!\n");
  }, function(err) {
     console.log('Command failed to run with error: ', err);
  });
  
  var im = require ('imagemagick');
  im.convert(['./Convert_ps/rna.ps','-flatten','./Convert_ps/'+ req.body.Mutation+'.jpg'],function(err,stdout){
     if(err) throw err;
     console.log('stdout:',stdout);
  });

   mut=''+req.body.Mutation+'.jpg';

    const timeout = setInterval(function() {
      const file = './Convert_ps/'+ req.body.Mutation+'.jpg';
      const fileExists = fs.existsSync(file);

      console.log('Checking for: ', file);
      console.log('Exists: ', fileExists);

      if (fileExists) 
      {
          clearInterval(timeout);
          res.send(200);
          
      }
  }, 500);


});


app.get('/deletepicture', function(req, res) 
{
  var t = require('fs');
  var filePath2 = './Convert_ps/'+mut; 
  console.log(filePath2);
  t.unlinkSync(filePath2); 
  var u = require('fs');
  var filePath3 = './Convert_ps/seq.fa'; 
  t.unlinkSync(filePath3);
  res.send(200);
 

});




/***************************************************************************** */

 /*Get accses to all the static files */
app.use(express.static(__dirname+"/css files/"));
app.use(express.static(__dirname));



