	<!DOCTYPE html>
	<html>
	<head>
	<meta charset="UTF-8">
	<link rel="stylesheet" type="text/css" href="style.css">

	<?php

	$pin = "";
	$pinErr = "";
	error_reporting(E_ALL);

	if ($_SERVER["REQUEST_METHOD"] == "POST") {
	   if (empty($_POST["pin"])) {
		 $pinErr = "Zadejte pin";
	   } 
	   else 
	   {
		  $pin = $_POST["pin"];
		  
		  exec("/bin/stty -F /dev/ttyUSB0 19200 sane raw cs8 hupcl cread clocal -echo -onlcr ");
		  
	  	  $fp=fopen("/dev/ttyUSB0","c+");
	  	  if(!$fp) die("Can't open device");
		  
	  	  stream_set_blocking($fp,1);
		  fwrite($fp,"AT+CPIN=\"$pin\"\r");
	  	  usleep(1000000);
		  
		  stream_set_blocking($fp,0);
	  
	  	  fclose($fp);
	   }
	   if (!empty ($_POST["reset"]))
	   {
		  exec("/bin/stty -F /dev/ttyUSB0 19200 sane raw cs8 hupcl cread clocal -echo -onlcr ");
		  
	  	  $fp=fopen("/dev/ttyUSB0","c+");
	  	  if(!$fp) die("Can't open device");
		  
	  	  stream_set_blocking($fp,1);
		  fwrite($fp,"AT+CFUN=1,1\r");
	  	  usleep(2000000);
		  
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
		<strong>Status modemu a základní nastavení</strong>
	  </p>
	  <p>
		<form method="post" action="<?php echo htmlspecialchars($_SERVER["PHP_SELF"]);?>">
		   PIN: <input type="password" name="pin"> (stejne jako na mobilu 3 pokusy, pak PUK)
		   <span class="error">* <?php echo $pinErr;?></span>
		   <br><br>
		   <input type="submit" name="submit" value="Submit">
		 </form>  
	  </p>
	  <p> 
		<form method="post" action="<?php echo htmlspecialchars($_SERVER["PHP_SELF"]);?>">
		    Reset : <input type="hidden" name="reset" value="ok"> software GSM reset
		   <br><br>
		   <input type="submit" name="submit" value="Restart">
		 </form> 
	  </p>
	</div>

	<div id="footer">
	
	</div>

	</body>
	</html>
