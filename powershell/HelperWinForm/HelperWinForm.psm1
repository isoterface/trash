Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

# ----------
function MessageBoxShow([string]$text
    , [string]$caption = ""
    , [System.Windows.Forms.MessageBoxButtons]$buttons
        = [System.Windows.Forms.MessageBoxButtons]::OK
    , [System.Windows.Forms.MessageBoxIcon]$icon
        = [System.Windows.Forms.MessageBoxIcon]::None
    , [System.Windows.Forms.MessageBoxDefaultButton]$defaultButton
        = [System.Windows.Forms.MessageBoxDefaultButton]::Button1
) {
    $result = [System.Windows.Forms.MessageBox]::Show($text
                , $caption
                , $buttons
                , $icon
                , $defaultButton)
    return $result
}

# ----------
function MessageBoxShowTopMost([string]$text
    , [string]$caption = ""
    , [System.Windows.Forms.MessageBoxButtons]$buttons
        = [System.Windows.Forms.MessageBoxButtons]::OK
    , [System.Windows.Forms.MessageBoxIcon]$icon
        = [System.Windows.Forms.MessageBoxIcon]::None
    , [System.Windows.Forms.MessageBoxDefaultButton]$defaultButton
        = [System.Windows.Forms.MessageBoxDefaultButton]::Button1
) {
    $result = [System.Windows.Forms.MessageBox]::Show($text
                , $caption
                , $buttons
                , $icon
                , $defaultButton
                , [System.Windows.Forms.MessageBoxOptions]::DefaultDesktopOnly)
    return $result
}

#Test
function Test-MessageBoxShow() {
    $result = MessageBoxShowTopMost "Message Text" "Caption" "YesNo" "Warning" "Button1"
    if ($result -eq [System.Windows.Forms.DialogResult]::Yes) {
        Write-Host "Dialog Yes."
    }
    else {
        Write-Host "Dialog No."
    }
}

# CreateControl Functions
# ----------
function CreateForm([int]$w
    , [int]$h
    , [string]$title
) {
    $form = New-Object System.Windows.Forms.Form
    $form.Text = $title
    $form.Size = New-Object System.Drawing.Size($w, $h)
    # Write-Output $form
    return $form
}

# ----------
function CreateButton([int]$x
    , [int]$y
    , [int]$w
    , [int]$h
    , [string]$text
) {
    $button = New-Object System.Windows.Forms.Button
    $button.Location = New-Object System.Drawing.Point($x, $y)
    $button.Size = New-Object System.Drawing.Size($w, $h)
    $button.Text = $text
    # Write-Output $button
    return $button
}

# ----------
function CreateLabel([int]$x
    , [int]$y
    , [int]$w
    , [int]$h
    , [string]$text
) {
    $label = New-Object System.Windows.Forms.Label
    $label.Location = New-Object System.Drawing.Point($x, $y)
    $label.Size = New-Object System.Drawing.Size($w, $h)
    $label.Text = $text
    # Write-Output $label
    return $label
}

# ----------
function CreateTextBox([int]$x
    , [int]$y
    , [int]$w
    , [int]$h
    , [string]$text = ""
) {
    $textBox = New-Object System.Windows.Forms.TextBox
    $textBox.Location = New-Object System.Drawing.Point($x, $y)
    $textBox.Size = New-Object System.Drawing.Size($w, $h)
    $textBox.Text = $text
    # Write-Output $textBox
    return $textBox
}

# Test
function Test-WinForm() {
    # Form InitializeComponent
    $form = CreateForm 260 180 "Input"
    $btnOK = CreateButton 40 100 75 30 "OK"
    $btnCancel = CreateButton 130 100 75 30 "Cancel"
    $btnAbout = CreateButton 130 30 75 30 "About"
    $label = CreateLabel 10 30 250 20 "LabelText"
    $textBox = CreateTextBox 10 70 225 50 "Default"

    $form.Controls.Add($btnOK)
    $form.Controls.Add($btnCancel)
    $form.Controls.Add($btnAbout)
    $form.Controls.Add($label)
    $form.Controls.Add($textBox)

    $btnOK.DialogResult = "OK"
    $btnCancel.DialogResult = "Cancel"
    $btnAbout.Add_Click( { btnAbout_Click })

    $form.AcceptButton = $btnOK
    $form.CancelButton = $btnCancel

    # Main
    $result = $form.ShowDialog()
    if ($result -eq "OK") {
        $x = $textBox.Text
        Write-Host $x
    }
    else {
        Write-Host "Canceled."
    }
}
function btnAbout_Click() {
    $result = MessageBoxShow "Message Text" "Title" "OKCancel" "Information" "Button2"
    if ($result -eq [System.Windows.Forms.DialogResult]::OK) {
        Write-Host "Dialog OK."
    }
    else {
        Write-Host "Dialog Cancel."
    }
}
