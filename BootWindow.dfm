object BootForm: TBootForm
  Left = 268
  Top = 316
  Width = 612
  Height = 345
  BorderIcons = [biSystemMenu]
  Caption = ' Boot'
  Color = clBtnFace
  Constraints.MinHeight = 345
  Constraints.MinWidth = 255
  ParentFont = True
  OldCreateOrder = False
  Position = poMainFormCenter
  OnCloseQuery = FormCloseQuery
  DesignSize = (
    604
    318)
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 441
    Top = 6
    Width = 61
    Height = 13
    Anchors = [akTop, akRight]
    Caption = 'Commands'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object bClose: TButton
    Left = 549
    Top = 267
    Width = 49
    Height = 25
    Anchors = [akRight, akBottom]
    Cancel = True
    Caption = 'Close'
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    TabOrder = 13
    OnClick = bCloseClick
  end
  object bSave: TButton
    Left = 496
    Top = 267
    Width = 49
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'Save'
    TabOrder = 12
    OnClick = bSaveClick
  end
  object StatusBar1: TStatusBar
    Left = 0
    Top = 299
    Width = 604
    Height = 19
    Panels = <>
    SimplePanel = True
  end
  object bReset: TButton
    Left = 443
    Top = 267
    Width = 49
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'Reset'
    TabOrder = 11
    OnClick = bResetClick
  end
  object BootCmd0: TEdit
    Left = 442
    Top = 24
    Width = 135
    Height = 21
    Anchors = [akTop, akRight]
    TabOrder = 1
    OnKeyPress = BootCmd0KeyPress
  end
  object ExecCmd0: TButton
    Left = 581
    Top = 24
    Width = 19
    Height = 20
    Anchors = [akTop, akRight]
    Caption = '>'
    TabOrder = 2
    OnClick = ExecCmd0Click
  end
  object BootCmd1: TEdit
    Tag = 1
    Left = 442
    Top = 48
    Width = 135
    Height = 21
    Anchors = [akTop, akRight]
    TabOrder = 3
  end
  object ExecCmd1: TButton
    Tag = 1
    Left = 581
    Top = 48
    Width = 19
    Height = 20
    Anchors = [akTop, akRight]
    Caption = '>'
    TabOrder = 4
    OnClick = ExecCmd0Click
  end
  object BootCmd2: TEdit
    Tag = 2
    Left = 442
    Top = 72
    Width = 135
    Height = 21
    Anchors = [akTop, akRight]
    TabOrder = 5
  end
  object ExecCmd2: TButton
    Tag = 2
    Left = 581
    Top = 72
    Width = 19
    Height = 20
    Anchors = [akTop, akRight]
    Caption = '>'
    TabOrder = 6
    OnClick = ExecCmd0Click
  end
  object BootCmd3: TEdit
    Tag = 3
    Left = 442
    Top = 96
    Width = 135
    Height = 21
    Anchors = [akTop, akRight]
    TabOrder = 7
  end
  object ExecCmd3: TButton
    Tag = 3
    Left = 581
    Top = 96
    Width = 19
    Height = 20
    Anchors = [akTop, akRight]
    Caption = '>'
    TabOrder = 8
    OnClick = ExecCmd0Click
  end
  object BootCmd4: TEdit
    Tag = 4
    Left = 442
    Top = 120
    Width = 135
    Height = 21
    Anchors = [akTop, akRight]
    TabOrder = 9
  end
  object ExecCmd4: TButton
    Tag = 4
    Left = 581
    Top = 120
    Width = 19
    Height = 20
    Anchors = [akTop, akRight]
    Caption = '>'
    TabOrder = 10
    OnClick = ExecCmd0Click
  end
  object Memo1: TMemo
    Left = 445
    Top = 152
    Width = 145
    Height = 97
    Anchors = [akTop, akRight]
    BevelInner = bvNone
    BevelOuter = bvNone
    BorderStyle = bsNone
    Lines.Strings = (
      'You can use hexadecimal '
      'values and "strings" in '
      'commands. Example:'
      '  "at" 0123 "\n"'
      ''
      'Press [>] to send command '
      'to mobile.')
    ParentColor = True
    ReadOnly = True
    TabOrder = 15
  end
  object Answer: TDrawGrid
    Left = 0
    Top = 0
    Width = 433
    Height = 299
    Align = alLeft
    Anchors = [akLeft, akTop, akRight, akBottom]
    ColCount = 33
    DefaultColWidth = 20
    DefaultRowHeight = 16
    DefaultDrawing = False
    FixedCols = 0
    FixedRows = 0
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Courier New'
    Font.Style = [fsBold]
    Options = [goFixedVertLine, goFixedHorzLine, goRangeSelect]
    ParentFont = False
    ScrollBars = ssVertical
    TabOrder = 0
    OnDrawCell = AnswerDrawCell
  end
  object Timer1: TTimer
    Enabled = False
    Interval = 1
    OnTimer = Timer1Timer
    Left = 336
    Top = 256
  end
  object Timer2: TTimer
    Enabled = False
    Interval = 1
    OnTimer = Timer2Timer
    Left = 368
    Top = 256
  end
  object BootSave: TSaveDialog
    DefaultExt = 'txt'
    Filter = 'Text files (*.txt)|*.txt|All files (*.*)|*.*'
    Options = [ofOverwritePrompt, ofEnableSizing]
    Left = 400
    Top = 256
  end
end
