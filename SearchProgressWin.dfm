object SearchProgress: TSearchProgress
  Left = 275
  Top = 117
  BorderIcons = [biSystemMenu]
  BorderStyle = bsToolWindow
  Caption = ' Search'
  ClientHeight = 88
  ClientWidth = 185
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  OnCloseQuery = FormCloseQuery
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 8
    Width = 57
    Height = 13
    Caption = 'Searching...'
  end
  object PBar: TProgressBar
    Left = 8
    Top = 27
    Width = 169
    Height = 16
    Min = 0
    Max = 100
    TabOrder = 0
  end
  object Button1: TButton
    Left = 56
    Top = 56
    Width = 75
    Height = 25
    Cancel = True
    Caption = 'Cancel'
    Default = True
    TabOrder = 1
    OnClick = Button1Click
  end
  object Timer1: TTimer
    Enabled = False
    Interval = 1
    OnTimer = Timer1Timer
    Left = 152
    Top = 56
  end
end
