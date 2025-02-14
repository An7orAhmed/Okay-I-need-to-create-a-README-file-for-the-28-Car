<?php
// api.php
require 'database.php';

if (isset($_GET['la']) && isset($_GET['lo']) && isset($_GET['st'])) {
    $la = $_GET['la'];
    $lo = $_GET['lo'];
    $st = $_GET['st'];

    // Update the database with new values
    $stmt = $pdo->prepare("UPDATE data SET la = :la, lo = :lo, st = :st WHERE id = 1");
    $stmt->execute(['la' => $la, 'lo' => $lo, 'st' => $st]);
}

// Retrieve current values
$stmt = $pdo->prepare("SELECT la, lo, st FROM data WHERE id = 1");
$stmt->execute();
$data = $stmt->fetch(PDO::FETCH_ASSOC);

// Return data as JSON
header('Content-Type: application/json');
echo json_encode($data);
?>
