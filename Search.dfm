object SearchForm: TSearchForm
  Left = 191
  Top = 132
  ActiveControl = What
  BorderStyle = bsDialog
  Caption = ' Search'
  ClientHeight = 135
  ClientWidth = 250
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  FormStyle = fsStayOnTop
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Bevel1: TBevel
    Left = 8
    Top = 8
    Width = 233
    Height = 89
  end
  object What: TEdit
    Left = 16
    Top = 16
    Width = 217
    Height = 21
    TabOrder = 0
  end
  object Button1: TButton
    Left = 48
    Top = 104
    Width = 75
    Height = 25
    Caption = 'Find'
    Default = True
    TabOrder = 1
    OnClick = NextSearch
  end
  object Button2: TButton
    Left = 128
    Top = 104
    Width = 75
    Height = 25
    Cancel = True
    Caption = 'Close'
    TabOrder = 2
    OnClick = Button2Click
  end
  object cbCase: TCheckBox
    Left = 32
    Top = 48
    Width = 89
    Height = 17
    Caption = 'Case sensitive'
    TabOrder = 3
  end
  object rbForward: TRadioButton
    Left = 144
    Top = 48
    Width = 89
    Height = 17
    Caption = 'Forward'
    Checked = True
    TabOrder = 4
    TabStop = True
  end
  object rbBackward: TRadioButton
    Left = 144
    Top = 64
    Width = 81
    Height = 17
    Caption = 'Backward'
    TabOrder = 5
  end
end
