
var MongoClient = require('mongodb').MongoClient;
var url = "mongodb://localhost:27017/RNAmute";
var send_email=require('./email');

module.exports=
{

/*insert new object*/
Insert:function(new_user,res)
{

    MongoClient.connect(url, function(err, db) 
    {
        if (err) throw err;

        var dbo = db.db("RNAmute");
        var myobj = { Username:new_user.Username, Password:new_user.Password ,Email:new_user.Email};
        dbo.collection("User").insertOne(myobj, function(err, result) 
        {

            if (err) throw err;
            db.close();
            if(result)
            {
                console.log("1 document inserted");
                res.send(200,true);
            }
            else
            {
              res.send(200,false);
            }


            db.close();
          });
        });
},


/*find some object*/
Find:function find(U_name,res)
{


    MongoClient.connect(url, function(err, db) 
    {
        if (err) throw err;

        var dbo = db.db("RNAmute");
        dbo.collection("User").findOne({"Username":U_name}, function(err, result) 
        {
          if (err) throw err;
          db.close();
          if(result)
          {
              res.send(200,true);
          }
          else
          {
            res.send(200,false);
          }
         
         });
    });
},


Findemail:function findemail(email,res)
{

    MongoClient.connect(url, function(err, db) 
    {
        if (err) throw err;

        var dbo = db.db("RNAmute");
        dbo.collection("User").findOne({"Email":email}, function(err, result) 
        {
          if (err) throw err;
          db.close();
          if(result)
          {
            res.send(200,true);
          }
          else
          {
            res.send(200,false);
          }
         
         });
    });
},

Findtologin:function findtologin(req,login,res)
{

    MongoClient.connect(url, function(err, db) 
    {
        if (err) throw err;

        var dbo = db.db("RNAmute");
        dbo.collection("User").findOne({"Username":login.Username, "Password":login.Password}, function(err, result) 
        {
          if (err) throw err;
          db.close();
          if(result)
          {
              console.log("login");
              res.send(200,true);
          }
          else
          {
              res.send(200,false);
          }
         
         });
    });
},


/*delets and existing object*/
Delete:function Delete ()
{

    MongoClient.connect(url, function(err, db) 
    {
        if (err) throw err;

        var dbo = db.db("RNAmute");
        var myquery = { username:"Company Inc" };
        dbo.collection("User").deleteOne(myquery, function(err, obj) 
        {
            if (err) throw err;
            console.log("1 document deleted");
            db.close();
            });
        });
},


Recover:function recover(email,res)
{

    MongoClient.connect(url, function(err, db) 
    {
        if (err) throw err;
        
        var dbo = db.db("RNAmute");
        dbo.collection("User").findOne({"Email":email}, function(err, result) 
        {
          if (err) throw err;
          db.close();
          if(result)
          {
            send_email.Recover(result,res);
            res.send(200,true);
          }
          else
          {
            res.send(200,false);
          }
         
         });
    });
},

Insertcalc:function insertcalc(calc,res)
{

    MongoClient.connect(url, function(err, db) 
    {
        if (err) throw err;

        var dbo = db.db("RNAmute");
        //var myobj = { Username:new_user.Username, Password:new_user.Password ,Email:new_user.Email};
        dbo.collection("Calculations").insertOne(calc, function(err, res) 
        {
            if (err) throw err;
            console.log("1 document inserted");

        
            db.close();
          });
        });
},

/*sorting the object in db*/
Sort:function Sort()
{
     
    MongoClient.connect(url, function(err, db) 
    {
        if (err) throw err;

        var dbo = db.db("RNAmute");
        var mysort = { User: 1 };
        dbo.collection("Calculations").find().sort(mysort).toArray(function(err, result) 
        {
            if (err) throw err;
            console.log(result);
            db.close();
        });
    });
},


Findcalcs:function findcalcs(User,res)
{

    MongoClient.connect(url, function(err, db) 
    {
        if (err) throw err;
        var dbo = db.db("RNAmute");
        dbo.collection("Calculations").find({"User":"Artur"}).toArray(function(err, result) 
        {
            if (err) throw err;
            db.close();
            if(result)
            {
                console.log(result);
                res.json(200,result);
            }
      });
}
    )},





  

Findcalcbyid:function findcalcbyid(ID,req,res)
{

    MongoClient.connect(url, function(err, db) 
    {
        if (err) throw err;
        var dbo = db.db("RNAmute");
        dbo.collection("Calculations").find({"Date":ID}).toArray(function(err, result) 
        {
            if (err) throw err;
            db.close();
            if(result)
            {
                console.log(result);
                res.json(200,result);
            }
      });
}
    )},


////////////////////////////////////////////////////////////User_History/////////////////////////////
InsertHistory:function InsertHistory(Cal,res)
{

    MongoClient.connect(url, function(err, db) 
    {
        if (err) throw err;

        var dbo = db.db("RNAmute");
        
        var myobj = { User:Cal.User, ID:Cal.Date};

        dbo.collection("History").insertOne(myobj, function(err, res) 
        {
            if (err) throw err;
            console.log("1 document inserted");

        
            db.close();
          });
        });
},



FindHistory:function FindHistory(User,req,res)
{

    MongoClient.connect(url, function(err, db) 
    {
        if (err) throw err;
        var dbo = db.db("RNAmute");
        dbo.collection("History").find({"User":User}).toArray(function(err, result) 
        {
            if (err) throw err;
            db.close();
            if(result)
            {
                console.log(result);
                res.json(200,result);
            }
      });   
}
)}

};