object SaveMem: TSaveMem
  Left = 266
  Top = 134
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = ' Save memory'
  ClientHeight = 178
  ClientWidth = 378
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Bevel1: TBevel
    Left = 8
    Top = 8
    Width = 361
    Height = 121
  end
  object Label1: TLabel
    Left = 24
    Top = 28
    Width = 41
    Height = 13
    Caption = 'Address:'
  end
  object Label2: TLabel
    Left = 24
    Top = 60
    Width = 36
    Height = 13
    Caption = 'Length:'
  end
  object Label3: TLabel
    Left = 24
    Top = 92
    Width = 45
    Height = 13
    Caption = 'Filename:'
  end
  object eAddress: TEdit
    Left = 88
    Top = 24
    Width = 105
    Height = 21
    TabOrder = 0
    Text = '0xA0000000'
  end
  object eLength: TEdit
    Left = 88
    Top = 56
    Width = 105
    Height = 21
    TabOrder = 1
    Text = '0x00100000'
  end
  object Button1: TButton
    Left = 112
    Top = 144
    Width = 75
    Height = 25
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 2
  end
  object Button2: TButton
    Left = 200
    Top = 144
    Width = 75
    Height = 25
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 3
  end
  object eFilename: TEdit
    Left = 88
    Top = 88
    Width = 241
    Height = 21
    TabOrder = 4
    OnChange = eFilenameChange
  end
  object Button3: TButton
    Left = 328
    Top = 88
    Width = 25
    Height = 22
    Caption = '...'
    TabOrder = 5
    OnClick = Button3Click
  end
  object SaveMemoryDlg: TSaveDialog
    DefaultExt = 'bin'
    Filter = 'Binary files (.bin)|*.bin|All files (*.*)|*.*'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofEnableSizing]
    Left = 344
    Top = 144
  end
end
