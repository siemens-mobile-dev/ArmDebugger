object EditAscForm: TEditAscForm
  Left = 214
  Top = 246
  ActiveControl = eLine
  BorderIcons = []
  BorderStyle = bsDialog
  Caption = 'EditAscForm'
  ClientHeight = 65
  ClientWidth = 233
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  PixelsPerInch = 96
  TextHeight = 13
  object eLine: TEdit
    Left = 0
    Top = 6
    Width = 233
    Height = 21
    TabOrder = 0
    OnChange = eLineChange
  end
  object bOk: TButton
    Left = 48
    Top = 35
    Width = 67
    Height = 25
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 1
  end
  object bCancel: TButton
    Left = 120
    Top = 35
    Width = 65
    Height = 25
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 2
  end
end
