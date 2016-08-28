object PatchesForm: TPatchesForm
  Left = 750
  Top = 19
  Width = 385
  Height = 267
  Caption = ' Patches'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poDefault
  PixelsPerInch = 96
  TextHeight = 13
  object List: TStringGrid
    Left = 0
    Top = 0
    Width = 377
    Height = 240
    Align = alClient
    ColCount = 4
    DefaultColWidth = 90
    DefaultRowHeight = 16
    FixedCols = 0
    RowCount = 2
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goRangeSelect, goColSizing, goRowSelect, goThumbTracking]
    PopupMenu = PatchesPopup
    TabOrder = 0
    OnDblClick = ListDblClick
  end
  object PatchesPopup: TPopupMenu
    Left = 304
    Top = 48
    object Goto1: TMenuItem
      Caption = 'Goto'
      ShortCut = 13
      OnClick = Goto1Click
    end
    object Edit1: TMenuItem
      Caption = 'Edit'
      OnClick = Edit1Click
    end
    object Undo1: TMenuItem
      Caption = 'Undo'
      ShortCut = 46
      OnClick = Undo1Click
    end
    object Decreaseorder1: TMenuItem
      Caption = 'Decrease order'
      ShortCut = 16422
      OnClick = Decreaseorder1Click
    end
    object Increaseorder1: TMenuItem
      Caption = 'Increase order'
      ShortCut = 16424
      OnClick = Increaseorder1Click
    end
    object Enable1: TMenuItem
      Caption = 'Enable'
      OnClick = Enable1Click
    end
    object EnableAll1: TMenuItem
      Caption = 'Enable All'
      ShortCut = 16491
      OnClick = EnableAll1Click
    end
    object Disable1: TMenuItem
      Caption = 'Disable'
      OnClick = Disable1Click
    end
    object DisableAll1: TMenuItem
      Caption = 'Disable All'
      ShortCut = 16493
      OnClick = DisableAll1Click
    end
    object Saveasvkp1: TMenuItem
      Caption = 'Save as vkp'
      ShortCut = 16467
      OnClick = Saveasvkp1Click
    end
    object Saveallasvkp1: TMenuItem
      Caption = 'Save all as vkp'
      OnClick = Saveallasvkp1Click
    end
    object Close1: TMenuItem
      Caption = 'Close'
      ShortCut = 27
      Visible = False
      OnClick = Close1Click
    end
  end
  object SaveVkpDlg: TSaveDialog
    DefaultExt = 'vkp'
    Filter = 'Vkp file (*.vkp)|*.vkp'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofEnableSizing]
    Left = 336
    Top = 48
  end
end
