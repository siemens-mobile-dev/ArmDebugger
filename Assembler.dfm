object AssemblerForm: TAssemblerForm
  Left = 198
  Top = 103
  BorderIcons = [biSystemMenu]
  BorderStyle = bsToolWindow
  Caption = ' Assemble'
  ClientHeight = 42
  ClientWidth = 249
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  DesignSize = (
    249
    42)
  PixelsPerInch = 96
  TextHeight = 13
  object EditLine: TEdit
    Left = 0
    Top = 1
    Width = 249
    Height = 21
    Anchors = [akLeft, akTop, akRight]
    MaxLength = 64
    TabOrder = 0
  end
  object Status: TStaticText
    Left = 0
    Top = 24
    Width = 249
    Height = 18
    AutoSize = False
    BevelInner = bvNone
    BevelKind = bkSoft
    Color = clBtnFace
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clMaroon
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentColor = False
    ParentFont = False
    TabOrder = 1
  end
  object Button1: TButton
    Left = 88
    Top = 48
    Width = 75
    Height = 25
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 2
  end
  object Button2: TButton
    Left = 168
    Top = 48
    Width = 75
    Height = 25
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 3
  end
end
