object AutoCGSNform: TAutoCGSNform
  Left = 655
  Top = 118
  BorderStyle = bsSingle
  Caption = ' Generate CGSN patch'
  ClientHeight = 85
  ClientWidth = 377
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 11
    Width = 57
    Height = 13
    Caption = 'Fullflash file:'
  end
  object Label2: TLabel
    Left = 8
    Top = 37
    Width = 55
    Height = 13
    Caption = 'Flash Base:'
  end
  object Label3: TLabel
    Left = 8
    Top = 61
    Width = 71
    Height = 13
    Caption = 'Patch address:'
  end
  object eFile: TEdit
    Left = 88
    Top = 8
    Width = 257
    Height = 21
    TabOrder = 0
  end
  object bSearch: TButton
    Left = 344
    Top = 8
    Width = 25
    Height = 21
    Caption = '...'
    TabOrder = 1
    OnClick = bSearchClick
  end
  object eBase: TEdit
    Left = 88
    Top = 32
    Width = 73
    Height = 21
    MaxLength = 8
    TabOrder = 2
    Text = 'A0000000'
  end
  object bGenerate: TBitBtn
    Left = 272
    Top = 48
    Width = 97
    Height = 31
    Caption = 'Generate'
    Default = True
    TabOrder = 3
    OnClick = bGenerateClick
  end
  object ePA: TEdit
    Left = 88
    Top = 56
    Width = 73
    Height = 21
    MaxLength = 8
    TabOrder = 4
    Text = '(Auto)'
  end
  object FlashOpenDlg: TOpenDialog
    DefaultExt = 'bin'
    Filter = 'Binary files (*.bin)|*.bin|All files (*.*)|*.*'
    Title = 'Fullflash File'
    Left = 200
    Top = 38
  end
  object SaveVkpDlg: TSaveDialog
    DefaultExt = 'vkp'
    Filter = 'V-Klay patch files (*.vkp)|*.vkp'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofEnableSizing]
    Title = 'Save VKP to'
    Left = 232
    Top = 38
  end
end
