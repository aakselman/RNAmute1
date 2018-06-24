
/*exports.Convertion = function(Mutation)
{
    write_to_seq(Mutation);

    var nrc = require('node-run-cmd');
    var options = { cwd: './Convert_ps' };
    nrc.run('RNAfold --noLP seq.fa > opt_structure', options).then(function(exitCodes) 
    {
        //nrc.run('convert -flatten rna.ps -quality 92 rna.jpg');
        console.log("Succefully Completed!\n");
     }, function(err) {
           console.log('Command failed to run with error: ', err);
    });

}



function write_to_seq(Mutation)
{
    var fs = require('fs');
    fs.unlink('./Convert_ps/seq.fa', function (err) {
    if (err) throw err;
    console.log('Deletion sucessful.');//Clear file!
            });
    var logger = fs.createWriteStream('./Convert_ps/seq.fa',
    {
            flags: 'a' // 'a' means appending (old data will be preserved)
          });


logger.write(Mutation+"\n");
logger.end();

}

*/


/*var im = require ('imagemagick');
im.convert(['rna.ps','-flatten','rna.jpg'],function(err,stdout){
    if(err) throw err;
    console.log('stdout:',stdout);
}); */