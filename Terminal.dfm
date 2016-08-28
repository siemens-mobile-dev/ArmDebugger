object TerminalForm: TTerminalForm
  Left = 532
  Top = 260
  AutoScroll = False
  Caption = ' Terminal'
  ClientHeight = 364
  ClientWidth = 501
  Color = clBtnFace
  Font.Charset = RUSSIAN_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Fixedsys'
  Font.Style = []
  OldCreateOrder = False
  Position = poDefaultPosOnly
  OnCloseQuery = FormCloseQuery
  PixelsPerInch = 96
  TextHeight = 16
  object Win: TRichEdit
    Left = 1
    Top = 1
    Width = 499
    Height = 362
    Align = alCustom
    Anchors = [akLeft, akTop, akRight, akBottom]
    PlainText = True
    PopupMenu = PopupMenu1
    ReadOnly = True
    ScrollBars = ssVertical
    TabOrder = 0
    WantTabs = True
    OnKeyPress = WinKeyPress
  end
  object ReadTimer: TTimer
    Enabled = False
    Interval = 20
    OnTimer = ReadTimerTimer
    Left = 448
    Top = 16
  end
  object PopupMenu1: TPopupMenu
    Left = 448
    Top = 48
    object Copy1: TMenuItem
      Caption = 'Copy'
      ShortCut = 16451
      OnClick = Copy1Click
    end
    object Paste1: TMenuItem
      Caption = 'Paste'
      ShortCut = 16470
      OnClick = Paste1Click
    end
    object Clear1: TMenuItem
      Caption = 'Clear'
      OnClick = Clear1Click
    end
    object N1: TMenuItem
      Caption = '-'
    end
    object ParseSTKmessage1: TMenuItem
      Caption = 'Parse STK message'
      Visible = False
      OnClick = ParseSTKmessage1Click
    end
  end
end
