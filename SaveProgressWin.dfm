object SaveProgress: TSaveProgress
  Left = 361
  Top = 333
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsDialog
  Caption = ' Saving...'
  ClientHeight = 120
  ClientWidth = 266
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  FormStyle = fsStayOnTop
  OldCreateOrder = False
  Position = poMainFormCenter
  OnCloseQuery = FormCloseQuery
  PixelsPerInch = 96
  TextHeight = 13
  object Label: TLabel
    Left = 19
    Top = 16
    Width = 3
    Height = 13
    Caption = ' '
  end
  object PBar: TProgressBar
    Left = 16
    Top = 40
    Width = 233
    Height = 17
    Min = 0
    Max = 100
    TabOrder = 0
  end
  object Button1: TButton
    Left = 96
    Top = 80
    Width = 75
    Height = 25
    Cancel = True
    Caption = 'Cancel'
    TabOrder = 1
    OnClick = Button1Click
  end
  object Timer1: TTimer
    Enabled = False
    Interval = 1
    OnTimer = Timer1Timer
    Left = 232
    Top = 80
  end
end
