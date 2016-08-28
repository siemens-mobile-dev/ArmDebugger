object VkpForm: TVkpForm
  Left = 368
  Top = 296
  Width = 466
  Height = 337
  Caption = ' Apply vkp patch'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  DesignSize = (
    458
    310)
  PixelsPerInch = 96
  TextHeight = 13
  object SpeedButton1: TSpeedButton
    Left = 28
    Top = 2
    Width = 27
    Height = 26
    Hint = 'Open'
    Anchors = [akTop, akRight]
    Glyph.Data = {
      F6000000424DF600000000000000760000002800000010000000100000000100
      0400000000008000000000000000000000001000000000000000000000000000
      8000008000000080800080000000800080008080000080808000C0C0C0000000
      FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00DDDDDDDDDDDD
      DDDDDDDDDDDDDDDDDDDD00000000000DDDDD003333333330DDDD0B0333333333
      0DDD0FB03333333330DD0BFB03333333330D0FBFB000000000000BFBFBFBFB0D
      DDDD0FBFBFBFBF0DDDDD0BFB0000000DDDDDD000DDDDDDDD000DDDDDDDDDDDDD
      D00DDDDDDDDD0DDD0D0DDDDDDDDDD000DDDDDDDDDDDDDDDDDDDD}
    ParentShowHint = False
    ShowHint = True
    OnClick = SpeedButton1Click
  end
  object SpeedButton2: TSpeedButton
    Tag = 1
    Left = 55
    Top = 2
    Width = 27
    Height = 26
    Hint = 'Append'
    Anchors = [akTop, akRight]
    Glyph.Data = {
      F6000000424DF600000000000000760000002800000010000000100000000100
      0400000000008000000000000000000000001000000000000000000000000000
      8000008000000080800080000000800080008080000080808000C0C0C0000000
      FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00DDDDDDDDDDDD
      DDDDDDDDDDDDDDDDDDDD00000000000DDDDD003333333330DDDD0B0333333333
      0DDD0FB03333333330DD0BFB03333333330D0FBFB000000000000BFBFBFBFB0D
      DDDD0FBFBFBFBF0DDDDD0BFB0000000D11DDD000DDDDDDDD11DDDDDDDDDDDD11
      1111DDDDDDDDDD111111DDDDDDDDDDDD11DDDDDDDDDDDDDD11DD}
    ParentShowHint = False
    ShowHint = True
    OnClick = SpeedButton1Click
  end
  object SpeedButton3: TSpeedButton
    Left = 1
    Top = 2
    Width = 27
    Height = 26
    Hint = 'Clear'
    Anchors = [akTop, akRight]
    Glyph.Data = {
      F6000000424DF600000000000000760000002800000010000000100000000100
      0400000000008000000000000000000000001000000000000000000000000000
      8000008000000080800080000000800080008080000080808000C0C0C0000000
      FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00DDDDDDDDDDDD
      DDDDDDDDDDDDDDDDDDDDDDD00000000000DDDDD0FFFFFFFFF0DDDDD0FFFFFFFF
      F0DDDDD0FFFFFFFFF0DDDDD0FFFFFFFFF0DDDDD0FFFFFFFFF0DDDDD0FFFFFFFF
      F0DDDDD0FFFFFFFFF0DDDDD0FFFFFFFFF0DDDDD0FFFFFF0000DDDDD0FFFFFF0F
      0DDDDDD0FFFFFF00DDDDDDD00000000DDDDDDDDDDDDDDDDDDDDD}
    ParentShowHint = False
    ShowHint = True
    OnClick = SpeedButton3Click
  end
  object VkpText: TMemo
    Left = 0
    Top = 32
    Width = 457
    Height = 240
    Anchors = [akLeft, akTop, akRight, akBottom]
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Courier New'
    Font.Style = []
    ParentFont = False
    ScrollBars = ssVertical
    TabOrder = 0
    WordWrap = False
  end
  object Button1: TButton
    Left = 296
    Top = 279
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 1
  end
  object Button2: TButton
    Left = 376
    Top = 279
    Width = 73
    Height = 25
    Anchors = [akRight, akBottom]
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 2
  end
  object OpenVkpDlg: TOpenDialog
    Filter = 'Vkp files (*.vkp)|*.vkp|All files (*.*)|*.*'
    Options = [ofHideReadOnly, ofFileMustExist, ofEnableSizing]
    Title = 'Vkp file'
    Left = 400
    Top = 40
  end
end
