exports.Algoritm = function(Cal)
{

    var nrc = require('node-run-cmd');
    var compile_run = require('compile-run');

    console.log(Cal);
    
    write_to_seq(Cal)
    write_to_log(Cal);

   compile_run.runFile('./Algo/Multi_Mut.c','',function(stdout,stderr,err){
    if(!err)
    {
        console.log(stdout);
        console.log("Wait until compute mutations...\n");
        var options = { cwd: 'Algo' };
        //nrc.run('./Multi_Mut');
        nrc.run('./Multi_Mut', options).then(function(stdout,exitCodes) 
        {
            console.log(stdout);
            console.log("Succefully Completed!\n");
        }, 
        function(err) 
        {
               console.log('Command failed to run with error: ', err);
        });
    }
    
    else{
        console.log(err);
        }
    
    });

}


function write_to_log(Cal)
{
    var fs = require('fs');
    /*if(fs.existsSync('./Algo/log.txt'))
    {
        fs.unlink('./Algo/log.txt', function (err) 
        {
            if (err) throw err;
                console.log('Deletion sucessful.');//Clear file!
                });
                var logger = fs.createWriteStream('./Algo/log.txt', {
                    //flags: 'a' // 'a' means appending (old data will be preserved)
                          });
    }   */
    var logger = fs.createWriteStream('./Algo/log.txt', {
    flags: 'w' // 'a' means appending (old data will be preserved)
          });


logger.write(Cal.dist_1+"\n");
logger.write(Cal.dist_2+"\n");
logger.write(Cal.e_range+"\n");

logger.write(Cal.Hamming_dist+"\n");
logger.write(Cal.Tree_edit_dist+"\n");
logger.write(Cal.Base_pair_dist+"\n");

logger.write(Cal.Num_of_mutations+"\n");
logger.write(Cal.Start_of_reading_frame+"\n");

logger.write(Cal.Type_of_distance+"\n");
logger.write(Cal.Method+"\n");

logger.end();
}



function write_to_seq(Cal)
{
    var fs = require('fs');
    /*if(fs.existsSync('./Algo/seq.fa'))
    {
        fs.unlink('./Algo/seq.fa', function (err) 
        {
            if (err) throw err;
                console.log('Deletion sucessful.');//Clear file!
                });
        }
    var fs = require('fs');*/
    var logger = fs.createWriteStream('./Algo/seq.fa', {
    flags: 'w' // 'a' means appending (old data will be preserved)
          });


logger.write(Cal.RNA_sequence+"\n");


logger.end();
}

