if(isset($_FILES['uploadedFile']) && $_FILES['uploadedFile']['error'] === UPLOAD_ERR_OK) {
  // Get the details of the uploaded file
  $fileTmpPath = $_FILES['uploadedFile']['tmp_name'];
  $fileName = $_FILES['uploadedFile']['name'];
  $fileSize = $_FILES['uploadedFile']['size'];
  $fileType = $_FILES['uploadedFile']['type'];
  $fileNameCmps = explode(".", $fileName);
  $fileExtension = strtolower(end($fileNameCmps));

  // sanitize file-name
  $newFileName = md5(time() . $fileName) . '.' . $fileExtension;

  // Check if the file has one of the following extensions
  $allowedfileExtensions = array('jpg', 'gif', 'png', 'zip', 'txt', 'xls', 'doc');
  if (in_array($fileExtension, $allowedfileExtensions)) {
    // directory in which the uploaded file will be moved
    $uploadFileDir = './uploadedFiles/';
    $dest_path = $uploadFileDir . $newFileName;

    if(move_uploaded_file($fileTmpPath, $dest_path)) {
      $message ='File is successfully uploaded.';
    } else {
      $message = 'There was some error moving the file to upload directory. Please make sure the upload directory is writable by web server.';
    }
  } else {
    $message = 'Upload failed. Allowed file types: ' . implode(',', $allowedfileExtensions);
  }
} else {
  $message = 'There is some error in the file upload. Please check the following error.<br>';
  $message .= 'Error:' . $_FILES['uploadedFile']['error'];
}