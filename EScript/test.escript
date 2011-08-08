var a = new Object();
'Sending a message to a constant NSString'.log();
var n = 12;
n += 22;
ETTranscript.show(n);
ETTranscript.cr();

for (var i=0 ; i < 10 ; i++)
{
	ETTranscript.show(i);
	ETTranscript.cr();
}

function Test()
{
	ETTranscript.show('Running Test() function');
	ETTranscript.cr();
	this.log();
	this.print = 
		function()
		{
			ETTranscript.show('Running a method in an object constructed from EScript');
			ETTranscript.cr();
		};
}
var t = new Test();
'Test returned new object'.log();
t.log();
t.print();

var nsobj = new NSObject();
