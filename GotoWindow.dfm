object GotoForm: TGotoForm
  Left = 467
  Top = 237
  BorderIcons = []
  BorderStyle = bsToolWindow
  Caption = 'Goto address'
  ClientHeight = 58
  ClientWidth = 137
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  OnHide = FormHide
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Button1: TButton
    Left = 8
    Top = 34
    Width = 57
    Height = 19
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 0
  end
  object Button2: TButton
    Left = 72
    Top = 34
    Width = 57
    Height = 19
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object AddressEdit: TComboBox
    Left = 8
    Top = 6
    Width = 121
    Height = 21
    CharCase = ecUpperCase
    ItemHeight = 13
    TabOrder = 2
  end
end
