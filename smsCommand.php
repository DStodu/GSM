<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<link rel="stylesheet" type="text/css" href="style.css">

<?php

$command = "";
$commandErr = "";
error_reporting(E_ALL);

if ($_SERVER["REQUEST_METHOD"] == "POST") {
   if (empty($_POST["command"])) {
     $commandErr = "Zadejte prikaz";
   } 
   else 
   {
	$command = $_POST["command"];
      
    //  exec("mode COM6 BAUD=9600 PARITY=N data=8 stop=1 xon=off");
	exec("/bin/stty -F /dev/ttyUSB0 19200 sane raw cs8 hupcl cread clocal -echo -onlcr ");
      
  	$fp=fopen("/dev/ttyUSB0","c+");
  	if(!$fp) die("Can't open device");
      
  	stream_set_blocking($fp,1);
      
    $bytes = fwrite($fp,$command."\r");
      
    usleep(1000000);
      
    stream_set_blocking($fp,0);
  
    fclose($fp);
   }
}


?>
  

</head>
<body>

<div id="header">
<h1>SMS</h1>
</div>

<?php
  require "nav.html";
?>

<div id="section">
  <p>
    <strong>Odeslání samostatného příkazu</strong>
  </p>
  <p>
    <form method="post" action="<?php echo htmlspecialchars($_SERVER["PHP_SELF"]);?>">
       Prikaz: <input type="text" name="command"> (samostatny prikaz)
       <span class="error">* <?php echo $commandErr;?></span>
       <br><br>
       <input type="submit" name="submit" value="Odeslat">
     </form>  
  </p>
</div>

<div id="footer">

</div>

</body>
</html>
