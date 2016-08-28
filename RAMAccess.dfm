object AccessAnalyzer: TAccessAnalyzer
  Left = 316
  Top = 157
  BorderStyle = bsSingle
  Caption = ' RAM Access Analyzer'
  ClientHeight = 200
  ClientWidth = 372
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object State: TLabel
    Left = 96
    Top = 176
    Width = 3
    Height = 13
    Caption = ' '
  end
  object bInit: TButton
    Left = 8
    Top = 104
    Width = 75
    Height = 25
    Caption = 'Initialize'
    TabOrder = 0
    OnClick = bInitClick
  end
  object StaticText1: TStaticText
    Left = 8
    Top = 8
    Width = 201
    Height = 89
    AutoSize = False
    TabOrder = 1
  end
  object bAnalyze: TButton
    Left = 8
    Top = 136
    Width = 75
    Height = 25
    Caption = 'Analyze'
    TabOrder = 2
    OnClick = bAnalyzeClick
  end
  object bClose: TButton
    Left = 8
    Top = 168
    Width = 75
    Height = 25
    Cancel = True
    Caption = 'Close'
    TabOrder = 3
    OnClick = bCloseClick
  end
  object PageList2: TMemo
    Left = 219
    Top = 0
    Width = 153
    Height = 200
    Align = alRight
    ReadOnly = True
    ScrollBars = ssVertical
    TabOrder = 4
  end
end
