object CallForm: TCallForm
  Left = 217
  Top = 175
  Width = 238
  Height = 283
  HorzScrollBar.Visible = False
  VertScrollBar.Visible = False
  BorderStyle = bsSizeToolWin
  Caption = ' Call address'
  Color = clBtnFace
  Constraints.MaxWidth = 238
  Constraints.MinHeight = 200
  Constraints.MinWidth = 238
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  FormStyle = fsStayOnTop
  KeyPreview = True
  OldCreateOrder = False
  Position = poDefaultPosOnly
  OnKeyPress = FormKeyPress
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 10
    Top = 63
    Width = 17
    Height = 13
    Caption = 'R0'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label2: TLabel
    Left = 10
    Top = 78
    Width = 17
    Height = 13
    Caption = 'R1'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label3: TLabel
    Left = 10
    Top = 93
    Width = 17
    Height = 13
    Caption = 'R2'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label4: TLabel
    Left = 10
    Top = 108
    Width = 17
    Height = 13
    Caption = 'R3'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label9: TLabel
    Left = 7
    Top = 8
    Width = 46
    Height = 13
    Caption = 'Address'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label10: TLabel
    Left = 3
    Top = 130
    Width = 54
    Height = 13
    Caption = 'Last called:'
  end
  object Shape1: TShape
    Left = 34
    Top = 62
    Width = 64
    Height = 61
    Brush.Color = clBlack
  end
  object Shape3: TShape
    Left = 58
    Top = 7
    Width = 72
    Height = 17
    Brush.Color = clBlack
  end
  object Shape4: TShape
    Left = 154
    Top = 107
    Width = 64
    Height = 16
    Brush.Color = clBlack
  end
  object Label11: TLabel
    Left = 111
    Top = 108
    Width = 37
    Height = 13
    Caption = 'Result'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Shape5: TShape
    Left = 154
    Top = 62
    Width = 64
    Height = 31
    Brush.Color = clBlack
  end
  object Label12: TLabel
    Left = 110
    Top = 63
    Width = 25
    Height = 13
    Caption = '[SP]'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label13: TLabel
    Left = 110
    Top = 77
    Width = 39
    Height = 13
    Caption = '[SP+4]'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object eR0: TEdit
    Left = 35
    Top = 63
    Width = 62
    Height = 14
    AutoSize = False
    BorderStyle = bsNone
    CharCase = ecUpperCase
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    MaxLength = 8
    ParentFont = False
    TabOrder = 3
    Text = '0'
  end
  object eR1: TEdit
    Left = 35
    Top = 78
    Width = 62
    Height = 14
    AutoSize = False
    BorderStyle = bsNone
    CharCase = ecUpperCase
    MaxLength = 8
    TabOrder = 4
    Text = '0'
  end
  object eR2: TEdit
    Left = 35
    Top = 93
    Width = 62
    Height = 14
    AutoSize = False
    BorderStyle = bsNone
    CharCase = ecUpperCase
    MaxLength = 8
    TabOrder = 5
    Text = '0'
  end
  object eR3: TEdit
    Left = 35
    Top = 108
    Width = 62
    Height = 14
    AutoSize = False
    BorderStyle = bsNone
    CharCase = ecUpperCase
    MaxLength = 8
    TabOrder = 6
    Text = '0'
  end
  object bCall: TButton
    Left = 144
    Top = 5
    Width = 76
    Height = 36
    Caption = 'Call'
    Default = True
    TabOrder = 2
    OnClick = bCallClick
  end
  object eAddr: TEdit
    Left = 59
    Top = 8
    Width = 70
    Height = 15
    AutoSize = False
    BorderStyle = bsNone
    CharCase = ecUpperCase
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    MaxLength = 8
    ParentFont = False
    TabOrder = 0
    Text = '0'
  end
  object cbThumb: TCheckBox
    Left = 57
    Top = 32
    Width = 54
    Height = 17
    Caption = 'Thumb'
    Ctl3D = True
    ParentCtl3D = False
    TabOrder = 1
  end
  object LastCalled: TListBox
    Left = 0
    Top = 144
    Width = 230
    Height = 112
    Style = lbVirtual
    Align = alBottom
    Anchors = [akLeft, akTop, akRight, akBottom]
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clNavy
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ItemHeight = 16
    ParentFont = False
    TabOrder = 7
    OnClick = LastCalledClick
    OnData = LastCalledData
    OnDblClick = LastCalledDblClick
    OnMouseDown = LastCalledMouseDown
  end
  object eRes: TEdit
    Left = 155
    Top = 108
    Width = 62
    Height = 14
    TabStop = False
    AutoSize = False
    BorderStyle = bsNone
    CharCase = ecUpperCase
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clTeal
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    MaxLength = 8
    ParentFont = False
    ReadOnly = True
    TabOrder = 8
  end
  object eSP0: TEdit
    Left = 155
    Top = 63
    Width = 62
    Height = 14
    AutoSize = False
    BorderStyle = bsNone
    CharCase = ecUpperCase
    MaxLength = 8
    TabOrder = 9
    Text = '0'
  end
  object eSP4: TEdit
    Left = 155
    Top = 78
    Width = 62
    Height = 14
    AutoSize = False
    BorderStyle = bsNone
    CharCase = ecUpperCase
    MaxLength = 8
    TabOrder = 10
    Text = '0'
  end
end
