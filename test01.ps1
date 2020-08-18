Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

function CreateForm([int]$w
    , [int]$h
    , [string]$title) {
    $form = New-Object System.Windows.Forms.Form
    $form.Text = $title
    $form.Size = New-Object System.Drawing.Size($w, $h)
    Write-Output $form
}

function CreateButton([int]$x
    , [int]$y
    , [int]$w
    , [int]$h
    , [string]$text
    , [string]$result = "") {
    $button = New-Object System.Windows.Forms.Button
    $button.Location = New-Object System.Drawing.Point($x, $y)
    $button.Size = New-Object System.Drawing.Size($w, $h)
    $button.Text = $text
    if ($result -eq "") {
        $button.DialogResult = $text
    }
    else {
        $button.DialogResult = $result
    }
    Write-Output $button
}

function CreateLabel([int]$x
    , [int]$y
    , [int]$w
    , [int]$h
    , [string]$text) {
    $label = New-Object System.Windows.Forms.Label
    $label.Location = New-Object System.Drawing.Point($x, $y)
    $label.Size = New-Object System.Drawing.Size($w, $h)
    $label.Text = $text
    Write-Output $label
}

function CreateTextBox([int]$x
    , [int]$y
    , [int]$w
    , [int]$h
    , [string]$text = "") {
    $textBox = New-Object System.Windows.Forms.TextBox
    $textBox.Location = New-Object System.Drawing.Point($x, $y)
    $textBox.Size = New-Object System.Drawing.Size($w, $h)
    $textBox.Text = $text
    Write-Output $textBox
}

# $form = New-Object System.Windows.Forms.Form
# $form.Text = "Input"
# $form.Size = New-Object System.Drawing.Size(260, 180)

# $OKButton = New-Object System.Windows.Forms.Button
# $OKButton.Location = New-Object System.Drawing.Point(40, 100)
# $OKButton.Size = New-Object System.Drawing.Size(75, 30)
# $OKButton.Text = "OK"
# $OKButton.DialogResult = "OK"

# $CancelButton = New-Object System.Windows.Forms.Button
# $CancelButton.Location = New-Object System.Drawing.Point(130, 100)
# $CancelButton.Size = New-Object System.Drawing.Size(75, 30)
# $CancelButton.Text = "Cancel"
# $CancelButton.DialogResult = "Cancel"

# $label = New-Object System.Windows.Forms.Label
# $label.Location = New-Object System.Drawing.Point(10, 30)
# $label.Size = New-Object System.Drawing.Size(250, 20)
# $label.Text = "Message"

# $textBox = New-Object System.Windows.Forms.TextBox
# $textBox.Location = New-Object System.Drawing.Point(10, 70)
# $textBox.Size = New-Object System.Drawing.Size(225, 50)

$form = CreateForm 260 180 "Input"
$OKButton = CreateButton 40 100 75 30 "OK"
$CancelButton = CreateButton 130 100 75 30 "Cancel"
$label = CreateLabel 10 30 250 20 "LabelText"
$textBox = CreateTextBox 10 70 225 50 "Default"

$form.AcceptButton = $OKButton
$form.CancelButton = $CancelButton

$form.Controls.Add($OKButton)
$form.Controls.Add($CancelButton)
$form.Controls.Add($label)
$form.Controls.Add($textBox)

$result = $form.ShowDialog()

if ( $result -eq "OK") {
    $x = $textBox.Text
    $x
}
