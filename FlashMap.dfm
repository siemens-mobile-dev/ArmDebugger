object MapForm: TMapForm
  Left = 290
  Top = 94
  Width = 308
  Height = 247
  HorzScrollBar.Visible = False
  VertScrollBar.Visible = False
  ActiveControl = SB
  BorderStyle = bsSizeToolWin
  Caption = ' Flash Map'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poDefaultPosOnly
  DesignSize = (
    300
    220)
  PixelsPerInch = 96
  TextHeight = 13
  object SpeedButton1: TSpeedButton
    Left = 222
    Top = 1
    Width = 31
    Height = 16
    Anchors = [akTop, akRight]
    Caption = 'FF1'
    Flat = True
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Small Fonts'
    Font.Style = [fsBold]
    ParentFont = False
    OnClick = Button1Click
  end
  object SpeedButton2: TSpeedButton
    Left = 253
    Top = 1
    Width = 30
    Height = 16
    Anchors = [akTop, akRight]
    Caption = 'FF2'
    Flat = True
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Small Fonts'
    Font.Style = [fsBold]
    ParentFont = False
    OnClick = Button2Click
  end
  object SpeedButton3: TSpeedButton
    Left = 283
    Top = 1
    Width = 17
    Height = 16
    Anchors = [akTop, akRight]
    Caption = 'i'
    Flat = True
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Small Fonts'
    Font.Style = [fsBold]
    ParentFont = False
    OnClick = SpeedButton3Click
  end
  object Shape9: TShape
    Left = 9
    Top = 121
    Width = 11
    Height = 11
    Brush.Color = 15263976
  end
  object Label9: TLabel
    Left = 24
    Top = 120
    Width = 48
    Height = 13
    Caption = 'Filesystem'
  end
  object SB: TScrollBox
    Left = 0
    Top = 18
    Width = 302
    Height = 204
    HorzScrollBar.Tracking = True
    HorzScrollBar.Visible = False
    VertScrollBar.Tracking = True
    Anchors = [akLeft, akTop, akRight, akBottom]
    Color = clWhite
    ParentColor = False
    TabOrder = 0
    OnMouseWheelDown = SBMouseWheelDown
    OnMouseWheelUp = SBMouseWheelUp
    object Image: TImage
      Left = 0
      Top = 0
      Width = 277
      Height = 113
      PopupMenu = PopupMenu1
      OnDblClick = ImageDblClick
      OnMouseDown = ImageMouseDown
      OnMouseMove = ImageMouseMove
    end
  end
  object Info: TStaticText
    Left = 0
    Top = 0
    Width = 217
    Height = 17
    Anchors = [akLeft, akTop, akRight]
    AutoSize = False
    BorderStyle = sbsSunken
    Color = 14745599
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlack
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentColor = False
    ParentFont = False
    TabOrder = 1
  end
  object Legend: TPanel
    Left = 17
    Top = 31
    Width = 105
    Height = 159
    BevelInner = bvSpace
    BevelOuter = bvLowered
    Color = 14745599
    TabOrder = 2
    Visible = False
    OnMouseDown = LegendMouseDown
    object Shape1: TShape
      Left = 9
      Top = 9
      Width = 11
      Height = 11
      Brush.Color = 15263976
    end
    object Label1: TLabel
      Left = 24
      Top = 8
      Width = 53
      Height = 13
      Caption = 'Empty area'
    end
    object Label2: TLabel
      Left = 24
      Top = 24
      Width = 23
      Height = 13
      Caption = 'Data'
    end
    object Label3: TLabel
      Left = 24
      Top = 56
      Width = 50
      Height = 13
      Caption = 'ARM entry'
    end
    object Label4: TLabel
      Left = 24
      Top = 72
      Width = 51
      Height = 13
      Caption = 'ARM code'
    end
    object Label5: TLabel
      Left = 24
      Top = 88
      Width = 59
      Height = 13
      Caption = 'Thumb entry'
    end
    object Label6: TLabel
      Left = 24
      Top = 104
      Width = 60
      Height = 13
      Caption = 'Thumb code'
    end
    object Label7: TLabel
      Left = 24
      Top = 40
      Width = 27
      Height = 13
      Caption = 'String'
    end
    object Shape2: TShape
      Left = 9
      Top = 25
      Width = 11
      Height = 11
      Brush.Color = 11792838
    end
    object Shape3: TShape
      Left = 9
      Top = 41
      Width = 11
      Height = 11
      Brush.Color = 10547697
    end
    object Shape4: TShape
      Left = 9
      Top = 57
      Width = 11
      Height = 11
      Brush.Color = 233
    end
    object Shape5: TShape
      Left = 9
      Top = 73
      Width = 11
      Height = 11
      Brush.Color = 13160681
    end
    object Shape6: TShape
      Left = 9
      Top = 89
      Width = 11
      Height = 11
      Brush.Color = 15794176
    end
    object Shape7: TShape
      Left = 9
      Top = 105
      Width = 11
      Height = 11
      Brush.Color = 15849672
    end
    object Shape8: TShape
      Left = 9
      Top = 121
      Width = 11
      Height = 11
      Brush.Color = 7393521
    end
    object Label8: TLabel
      Left = 24
      Top = 120
      Width = 48
      Height = 13
      Caption = 'Filesystem'
    end
    object Shape10: TShape
      Left = 9
      Top = 137
      Width = 11
      Height = 11
      Brush.Color = 15794417
    end
    object Label10: TLabel
      Left = 24
      Top = 136
      Width = 46
      Height = 13
      Caption = 'EEPROM'
    end
  end
  object PopupMenu1: TPopupMenu
    Left = 168
    Top = 24
    object OpenasData1: TMenuItem
      Caption = 'Open as Data'
      ShortCut = 13
    end
    object OpenasCode1: TMenuItem
      Caption = 'Open as Code'
      ShortCut = 16397
    end
    object N1: TMenuItem
      Caption = '-'
    end
    object OpenFullFlash1: TMenuItem
      Caption = 'Open FullFlash...'
      OnClick = OpenFullFlash1Click
    end
    object OpenSecondFullFlash1: TMenuItem
      Caption = 'Open Second FullFlash...'
      OnClick = OpenSecondFullFlash1Click
    end
  end
  object OpenFlashDlg: TOpenDialog
    DefaultExt = 'bin'
    Filter = 'Binary files (*.bin)|*.bin|All files (*.*)|*.*'
    Options = [ofFileMustExist, ofNoNetworkButton, ofEnableSizing]
    Left = 200
    Top = 24
  end
  object Timer1: TTimer
    Enabled = False
    Interval = 100
    OnTimer = Timer1Timer
    Left = 232
    Top = 24
  end
end
