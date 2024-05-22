<html>
<head>
<title>Submission form</title>
</head>
<body>
<?php
echo "<h1>\$_SERVER</h1>";
echo "There are " . count($_SERVER) . " entries<br>";
foreach($_SERVER as $x => $x_value) {
    echo "\$_SERVER[<strong>" . $x . "</strong>]: " . $x_value . "<br>";
}

echo "<h1>\$_REQUEST</h1>";
echo "There are " . count($_REQUEST) . " entries<br>";
foreach($_REQUEST as $x => $x_value) {
    echo "\$_REQUEST[<strong>" . $x . "</strong>]: " . $x_value . "<br>";
}

echo "<h1>\$_POST</h1>";
echo "There are " . count($_POST) . " entries<br>";
foreach($_POST as $x => $x_value) {
    echo "\$_POST[<strong>" . $x . "</strong>]: " . $x_value . "<br>";
}

echo "<h1>\$_GET</h1>";
echo "There are " . count($_GET) . " entries<br>";
foreach($_GET as $x => $x_value) {
    echo "\$_GET[<strong>" . $x . "</strong>]: " . $x_value . "<br>";
}


echo "<h1>\$_FILES</h1>";
echo "There are " . count($_FILES) . " entries<br>";
foreach($_FILES as $x => $x_value) {
    echo "\$_FILES[<strong>" . $x . "</strong>]: " . $x_value . "<br>";
}

echo "<h1>\$_ENV</h1>";
echo "There are " . count($_ENV) . " entries<br>";
foreach($_ENV as $x => $x_value) {
    echo "\$_ENV[<strong>" . $x . "</strong>]: " . $x_value . "<br>";
}

echo "<h1>\$_COOKIE</h1>";
echo "There are " . count($_COOKIE) . " entries<br>";
foreach($_COOKIE as $x => $x_value) {
    echo "\$_COOKIE[<strong>" . $x . "</strong>]: " . $x_value . "<br>";
}

echo "<h1>\$_SESSION</h1>";
echo "There are " . count($_SESSION) . " entries<br>";
foreach($_SESSION as $x => $x_value) {
    echo "\$_SESSION[<strong>" . $x . "</strong>]: " . $x_value . "<br>";
}

?>
</body>
</html>

