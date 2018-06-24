exports.contact = function(req, res)
{
  console.log("inside");

    var name = req.body.name;
    var email= req.body.email;
    var message = req.body.message;
    var nodemailer = require('nodemailer');

    var transporter = nodemailer.createTransport({
    service: 'gmail',
    secure: false, // use SSL
      port: 25, 
    auth: {
      user: 'rnamute.web@gmail.com',
      pass: 'rnamute2018'
    },
    tls: {
      rejectUnauthorized: false
    }
  });
  
  var mailOptions = 
  {
    from: 'rnamute.web@gmail.com',
    to: 'aakselman46@gmail.com',
    subject: name+" need's help in RNAmute tool",
    text: message+"\n\n<"+email+">"
  };
  
  transporter.sendMail(mailOptions, function(error, info)
  {
    if (error) 
    {
      console.log(error);
    } 
    else 
    {
      console.log('Email sent: ' + info.response);
      res.send(200,true);
  
    }
  });
}


exports.Recover = function(User, res)
{
    var nodemailer = require('nodemailer');
    var transporter = nodemailer.createTransport({
    service: 'gmail',
    secure: false, // use SSL
      port: 25, 
    auth: {
      user: 'rnamute.web@gmail.com',
      pass: 'rnamute2018'
    },
    tls: {
      rejectUnauthorized: false
    }
  });
  
  var mailOptions = 
  {
    from: 'rnamute.web@gmail.com',
    to: User.Email,
    subject: "Recover email with log in information",
    text: "Here is your log in information"+'\n'+"Username:     "+User.Username+'\n'+"Password     "+User.Password
  };
  
  transporter.sendMail(mailOptions, function(error, info)
  {
    if (error) 
    {
      console.log(error);
    } 
    else 
    {
      console.log('Email sent: ' + info.response);
  
    }
  });
}





exports.Calculation = function(Email,Link, res)
{
    var nodemailer = require('nodemailer');
    var transporter = nodemailer.createTransport({
    service: 'gmail',
    secure: false, // use SSL
      port: 25, 
    auth: {
      user: 'rnamute.web@gmail.com',
      pass: 'rnamute2018'
    },
    tls: {
      rejectUnauthorized: false
    }
  });
  
  var mailOptions = 
  {
    from: 'rnamute.web@gmail.com',
    to: Email,
    subject: "Link to your RNAmute calculation data",
    text: "Click here to see the calculation data: "+Link
  };
  
  transporter.sendMail(mailOptions, function(error, info)
  {
    if (error) 
    {
      console.log(error);
    } 
    else 
    {
      console.log('Email sent: ' + info.response);
  
    }
  });
}
