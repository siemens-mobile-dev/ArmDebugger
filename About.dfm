object AboutForm: TAboutForm
  Left = 222
  Top = 247
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'About ArmDebugger'
  ClientHeight = 145
  ClientWidth = 257
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
  object Label1: TLabel
    Left = 16
    Top = 16
    Width = 225
    Height = 13
    Alignment = taCenter
    AutoSize = False
    Caption = 'Debugger for Siemens ARM phones'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label2: TLabel
    Left = 16
    Top = 32
    Width = 225
    Height = 13
    Alignment = taCenter
    AutoSize = False
    Caption = 'Version 0.7.9'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label3: TLabel
    Left = 16
    Top = 72
    Width = 225
    Height = 13
    Alignment = taCenter
    AutoSize = False
    Caption = '(c) by Chaos, thx to SiNgle && ACiD[mrp]'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label4: TLabel
    Left = 8
    Top = 88
    Width = 241
    Height = 13
    Cursor = crHandPoint
    Alignment = taCenter
    AutoSize = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlue
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsUnderline]
    ParentFont = False
    OnClick = Label4Click
  end
  object Bevel1: TBevel
    Left = 8
    Top = 64
    Width = 241
    Height = 2
  end
  object Button1: TButton
    Left = 88
    Top = 112
    Width = 81
    Height = 25
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 0
  end
end
