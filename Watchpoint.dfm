object WptForm: TWptForm
  Left = 159
  Top = 314
  ActiveControl = RtEdit
  BorderStyle = bsDialog
  Caption = ' Snappoint'
  ClientHeight = 324
  ClientWidth = 345
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
  object Bevel1: TBevel
    Left = 8
    Top = 288
    Width = 331
    Height = 2
  end
  object Label1: TLabel
    Left = 8
    Top = 8
    Width = 24
    Height = 13
    Caption = 'Text:'
  end
  object Label2: TLabel
    Left = 8
    Top = 120
    Width = 47
    Height = 13
    Caption = 'Condition:'
  end
  object RtEdit: TRichEdit
    Left = 8
    Top = 24
    Width = 329
    Height = 89
    ScrollBars = ssVertical
    TabOrder = 0
    OnKeyPress = RtEditKeyPress
  end
  object bOk: TButton
    Left = 104
    Top = 296
    Width = 75
    Height = 25
    Caption = 'Set'
    Default = True
    ModalResult = 1
    ParentShowHint = False
    ShowHint = False
    TabOrder = 2
  end
  object bCancel: TButton
    Left = 264
    Top = 296
    Width = 75
    Height = 25
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 4
  end
  object Button1: TButton
    Left = 184
    Top = 296
    Width = 75
    Height = 25
    Caption = 'Remove'
    ModalResult = 7
    TabOrder = 3
  end
  object Memo4: TMemo
    Left = 8
    Top = 168
    Width = 329
    Height = 113
    TabStop = False
    Color = clInfoBk
    Lines.Strings = (
      'Enter text that will be stored to log on this snappoint.'
      'You can use special symbols (replace % with $ to display'
      'the value in decimal):'
      ''
      '%A     address of snappoint'
      '%C     CPSR flags'
      '%RR  all registers (R0-R15)'
      '%Rn   register n'
      '%S     stack (top 8 entries)'
      '%T     time of execution, msec'
      '%[addr:t], %[Rn:t], %[Rn+off:t], %[*Rn+off:t]'
      '          memory at addr or pointed by Rn or Rn+off, or double '
      '          pointer *(Rn+off), where t is:'
      '          B -byte'
      '          W - word'
      '          S - ASCII string'
      '          U - Unicode string'
      '          E - Unicode string, 1st two bytes are length'
      '          4..64  dump up to 64 bytes'
      '@x equals to %[x:64]'
      '&x equals to %[x:S]'
      ''
      'Format of the condition field:'
      'Left side - Rn, [Rn], [Rn:B], [Rn+off], [Rn+off:B]'
      '(Rn - register R0..R12, off - offset (hex), B means byte value)'
      'Right side - constant value'
      'Condition - one of ==, !=, <, >, <=, >='
      'Examples:'
      '  R0 >= 5'
      '  [R2+1c] == 0x12345678'
      '  [R7+20:b] < 0xff')
    ReadOnly = True
    ScrollBars = ssVertical
    TabOrder = 5
  end
  object eCond: TEdit
    Left = 8
    Top = 136
    Width = 329
    Height = 21
    MaxLength = 63
    TabOrder = 1
  end
end
