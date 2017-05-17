<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<link rel="stylesheet" type="text/css" href="style.css">

</head>
<body>

<?php
$numberErr = $textErr = "";
$number = $text = $numbers = $textMultiple = "";
error_reporting(E_ALL);

if ($_SERVER["REQUEST_METHOD"] == "POST") {
   if (empty($_POST["number"])) {
     $numberErr = "Zadejte cislo";
   } else {
	   $number = $_POST["number"];
   }
  
   if (empty($_POST["text"])) {
     $textErr = "Zadejte text";
   } else {
	   $text = $_POST["text"];
   }
	
   if(!empty($number) && !empty($text))
   {

    exec("/bin/stty -F /dev/ttyUSB0 19200 sane raw cs8 hupcl cread clocal -echo -onlcr ");
      
  	$fp=fopen("/dev/ttyUSB0","c+");
  	if(!$fp) die("Can't open device");
  	
  	stream_set_blocking($fp,1);
  
    fwrite($fp,"AT+CMGF=1\r");
    usleep(1000000);
    
    fwrite($fp,"AT+CSMP=17,167,0,0\r");
    usleep(1000000);
    
    
    if(strpos($number,";") != false)
    {
      $smsNumbers =  explode(";",$number);
      foreach ($smsNumbers as $value) 
      {
        fwrite($fp,"AT+CMGS=\"$value\"\r");
  		  usleep(1000000);

  		  fwrite($fp,$text.chr(26));
        usleep(2000000);  

		    fwrite($fp,"AT\r");
		    usleep(2000000);
      }
    }
    else
    {
      fwrite($fp,"AT+CMGS=\"$number\"\r");
  	  usleep(1000000);

  	  fwrite($fp,$text.chr(26));
      usleep(1000000);
    }
    
  	stream_set_blocking($fp,0);
  
  	fclose($fp);
   }
   
   if (!empty($_POST["filesubmit"]))
   {
    $msgToSend = file_get_contents($_FILES['data']['tmp_name']);
    $pos = strrpos($msgToSend, ";");
    $textMultiple = substr(substr($msgToSend,$pos),1);
    $numbers = substr($msgToSend,0,$pos);
   }
}

?>

<div id="header">
<h1>SMS</h1>
</div>

<?php
  require "nav.html";
?>

<div id="section">
  <p>
    <strong>Odeslání více SMS se zvoleným textem</strong> 
  </p>
  <p>
    <br>
    <form action="<?php echo htmlspecialchars($_SERVER["PHP_SELF"]);?>" method="post" enctype="multipart/form-data">
    Zdrojový soubor pro poslání SMS:
    <input type="file" name="data" id="data"><br> <br>
    <input type="submit" value="Nahraj" name="filesubmit">
    </form>
    <br>
  </p>
  <p>
     <form method="post" action="<?php echo htmlspecialchars($_SERVER["PHP_SELF"]);?>">
       Cislo: <input type="text" name="number" value="<?php echo $numbers;?>"> Cislo ve tvaru +420xxxxxxxxx
       <span class="error">* <?php echo $numberErr;?></span>
       <br><br>
       Text: <input type="text" name="text" size="80" value="<?php echo $textMultiple;?>">
       <span class="error">* <?php echo $textErr;?></span>
       <br><br>
       <input type="submit" name="submit" value="Odeslat">
     </form> 
  </p>
</div>

<div id="footer">

</div>

</body>
</html>
