<?php
   // the gist of the algorithm
   //    if the request method is 'SPECIAL' then user is sending data to the server
   //       we take everything on the request body and dump it to data.txt
   //    else user is retrieving data
   //       we read data.txt and transfer that over

   $filename = "data.txt";

   //----[saving data]--------------------------------------------------
   if ($_SERVER['REQUEST_METHOD'] === 'SPECIAL') { 

      if (file_exists($filename)) { // rename file to avoid loosing it on error
         if(!rename( $filename, $filename.".bak")) {
            http_response_code(500);
            die();
         }
      }

      if ($fp = fopen('data.txt', 'w')) {
         fwrite($fp, file_get_contents('php://input') );
         fclose($fp);
         echo "Last save: ".date("Ymd H:i:s")."\0";  // on success we repond with a time stamp
      } else {
         http_response_code(500);
         die();
      }

      if (file_exists($filename.".bak")) { // delete old file
         unlink($filename.".bak");
      }

      die();
   }


   //----[loading data]--------------------------------------------------
   if (file_exists($filename)) {
      header('Content-Description: File Transfer');
      header('Content-Type: application/octet-stream');
      header('Content-Disposition: attachment; filename="'.basename($filename).'"');
      header('Expires: 0');
      header('Cache-Control: must-revalidate');
      header('Pragma: public');
      header('Content-Length: ' . filesize($filename));
      flush(); // Flush system output buffer
      readfile($filename);
      die();
   } else {
      http_response_code(404);
	    die();
   }
?>
