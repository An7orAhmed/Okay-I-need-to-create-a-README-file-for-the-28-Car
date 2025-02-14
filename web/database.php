<?php
// database.php
$host = 'localhost';  
$dbname = 'esinebdc_iot-car-accident';  
$username = 'esinebdc_projects'; 
$password = 'QyO2P7h{e;DBW)o!7)Of'; 

try {
    $pdo = new PDO("mysql:host=$host;dbname=$dbname", $username, $password);
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
} catch (PDOException $e) {
    die("Connection failed: " . $e->getMessage());
}
?>
