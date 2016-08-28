object ConfigForm: TConfigForm
  Left = 273
  Top = 161
  BorderStyle = bsDialog
  Caption = ' Configuration'
  ClientHeight = 343
  ClientWidth = 521
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  DesignSize = (
    521
    343)
  PixelsPerInch = 96
  TextHeight = 13
  object bOk: TButton
    Left = 358
    Top = 313
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 0
  end
  object bCancel: TButton
    Left = 438
    Top = 313
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object GroupBox1: TGroupBox
    Left = 8
    Top = 8
    Width = 249
    Height = 161
    Caption = ' Link '
    TabOrder = 2
    object Label2: TLabel
      Left = 16
      Top = 28
      Width = 73
      Height = 13
      Caption = 'Access method'
    end
    object Label1: TLabel
      Left = 16
      Top = 52
      Width = 52
      Height = 13
      Caption = 'Link speed'
    end
    object Label4: TLabel
      Left = 16
      Top = 91
      Width = 72
      Height = 13
      Caption = 'Monitor interval'
    end
    object Label3: TLabel
      Left = 208
      Top = 91
      Width = 25
      Height = 13
      Caption = 'msec'
    end
    object Label11: TLabel
      Left = 16
      Top = 115
      Width = 65
      Height = 13
      Caption = 'Exec Timeout'
    end
    object Label12: TLabel
      Left = 208
      Top = 115
      Width = 25
      Height = 13
      Caption = 'msec'
    end
    object cbMethod: TComboBox
      Left = 96
      Top = 24
      Width = 137
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 0
      Text = 'CGSN patch'
      Items.Strings = (
        'CGSN patch'
        'Java midlet')
    end
    object cbSpeed: TComboBox
      Left = 96
      Top = 48
      Width = 137
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 1
      Text = '115200'
      Items.Strings = (
        '115200'
        '230400'
        '460800')
    end
    object eMonIntv: TEdit
      Left = 96
      Top = 88
      Width = 97
      Height = 21
      TabOrder = 2
    end
    object eExecTm: TEdit
      Left = 96
      Top = 112
      Width = 97
      Height = 21
      TabOrder = 3
    end
  end
  object GroupBox2: TGroupBox
    Left = 264
    Top = 8
    Width = 249
    Height = 81
    Caption = ' Fonts '
    TabOrder = 3
    object Label5: TLabel
      Left = 16
      Top = 25
      Width = 48
      Height = 13
      Caption = 'Editor font'
    end
    object Label6: TLabel
      Left = 16
      Top = 49
      Width = 68
      Height = 13
      Caption = 'Debugger font'
    end
    object bEditFont: TButton
      Left = 96
      Top = 21
      Width = 137
      Height = 20
      TabOrder = 0
      OnClick = bEditFontClick
    end
    object bDbgFont: TButton
      Left = 96
      Top = 45
      Width = 137
      Height = 20
      TabOrder = 1
      OnClick = bDbgFontClick
    end
  end
  object GroupBox3: TGroupBox
    Left = 264
    Top = 96
    Width = 249
    Height = 73
    Caption = ' Trace level '
    TabOrder = 4
    object ckEvents: TCheckBox
      Left = 40
      Top = 24
      Width = 65
      Height = 17
      Caption = 'Events'
      TabOrder = 0
    end
    object ckDebug: TCheckBox
      Left = 40
      Top = 40
      Width = 65
      Height = 17
      Caption = 'Debug'
      TabOrder = 1
    end
    object ckWire: TCheckBox
      Left = 144
      Top = 24
      Width = 57
      Height = 17
      Caption = 'Wire'
      TabOrder = 2
    end
    object ckComm: TCheckBox
      Left = 144
      Top = 40
      Width = 57
      Height = 17
      Caption = 'Comm'
      TabOrder = 3
    end
  end
  object GroupBox4: TGroupBox
    Left = 8
    Top = 176
    Width = 505
    Height = 129
    Caption = ' Environment '
    TabOrder = 5
    object Label7: TLabel
      Left = 16
      Top = 27
      Width = 81
      Height = 13
      Caption = 'Path to Keil ARM'
    end
    object Label8: TLabel
      Left = 16
      Top = 51
      Width = 100
      Height = 13
      Caption = 'Assembler arguments'
    end
    object Label9: TLabel
      Left = 16
      Top = 75
      Width = 92
      Height = 13
      Caption = 'Compiler arguments'
    end
    object Label10: TLabel
      Left = 16
      Top = 99
      Width = 81
      Height = 13
      Caption = 'Linker arguments'
    end
    object eArmPath: TEdit
      Left = 128
      Top = 24
      Width = 361
      Height = 21
      TabOrder = 0
    end
    object eAsmArgs: TEdit
      Left = 128
      Top = 48
      Width = 361
      Height = 21
      TabOrder = 1
    end
    object eCompArgs: TEdit
      Left = 128
      Top = 72
      Width = 361
      Height = 21
      TabOrder = 2
    end
    object eLinkArgs: TEdit
      Left = 128
      Top = 96
      Width = 361
      Height = 21
      TabOrder = 3
    end
  end
  object EditorFontDlg: TFontDialog
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    MinFontSize = 0
    MaxFontSize = 0
    Options = [fdEffects, fdForceFontExist]
    Left = 424
    Top = 64
  end
  object DebuggerFontDlg: TFontDialog
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    MinFontSize = 0
    MaxFontSize = 0
    Options = [fdEffects, fdForceFontExist]
    Left = 456
    Top = 64
  end
end
