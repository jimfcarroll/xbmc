/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "Control.h"
#include "LanguageHook.h"
#include "AddonUtils.h"

#include "guilib/GUILabel.h"
#include "guilib/GUIFontManager.h"
#include "guilib/GUILabelControl.h"
#include "guilib/GUIFadeLabelControl.h"
#include "guilib/GUITextBox.h"
#include "guilib/GUIButtonControl.h"
#include "guilib/GUICheckMarkControl.h"
#include "guilib/GUIImage.h"
#include "guilib/GUIListContainer.h"
#include "guilib/GUIProgressControl.h"
#include "guilib/GUISliderControl.h"
#include "guilib/GUIRadioButtonControl.h"
#include "GUIInfoManager.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/GUIEditControl.h"
#include "guilib/GUIControlFactory.h"

#include "utils/XBMCTinyXML.h"
#include "utils/StringUtils.h"

namespace XBMCAddon
{
  namespace xbmcgui
  {

    // ============================================================

    // ============================================================
    // ============================================================
    ControlFadeLabel::ControlFadeLabel(long x, long y, long width, long height, 
                                       const char* font, const char* _textColor, 
                                       long _alignment) : 
      Control("ControlFadeLabel"),
      strFont("font13"), textColor(0xffffffff), align(_alignment)
    {
      dwPosX = x;
      dwPosY = y;
      dwWidth = width;
      dwHeight = height;

      if (font)
        strFont = font;

      if (_textColor) 
        sscanf(_textColor, "%x", &textColor);

      pGUIControl = NULL;
    }

    // addLabel() Method
    /**
     * addLabel(label) -- Add a label to this control for scrolling.
     * 
     * label          : string or unicode - text string.
     * 
     * example:
     *   - self.fadelabel.addLabel('This is a line of text that can scroll.')
     */
    void ControlFadeLabel::addLabel(const String& label) throw (UnimplementedException)
    {
      CGUIMessage msg(GUI_MSG_LABEL_ADD, iParentId, iControlId);
      msg.SetLabel(label);

      g_windowManager.SendThreadMessage(msg, iParentId);
    }

    /**
     * reset() -- Clear this fade label.
     * 
     * example:
     *   - self.fadelabel.reset()\n
     */
    void ControlFadeLabel::reset() throw (UnimplementedException)
    {
      CGUIMessage msg(GUI_MSG_LABEL_RESET, iParentId, iControlId);

      vecLabels.clear();
      g_windowManager.SendThreadMessage(msg, iParentId);
    }

    CGUIControl* ControlFadeLabel::Create() throw (WindowException)
    {
      CLabelInfo label;
      label.font = g_fontManager.GetFont(strFont);
      label.textColor = label.focusedColor = textColor;
      label.align = align;
      pGUIControl = new CGUIFadeLabelControl(
        iParentId,
        iControlId,
        (float)dwPosX,
        (float)dwPosY,
        (float)dwWidth,
        (float)dwHeight,
        label,
        true,
        0,
        true);

      CGUIMessage msg(GUI_MSG_LABEL_RESET, iParentId, iControlId);
      pGUIControl->OnMessage(msg);

      return pGUIControl;
    }

    // ============================================================

    // ============================================================
    // ============================================================
    ControlTextBox::ControlTextBox(long x, long y, long width, long height, 
                                   const char* font, const char* _textColor) : 
      Control("ControlTextBox"),
      strFont("font13"), textColor(0xffffffff)
    {
      dwPosX = x;
      dwPosY = y;
      dwWidth = width;
      dwHeight = height;

      if (font)
        strFont = font;

      if (_textColor) 
        sscanf(_textColor, "%x", &textColor);
    }

    // SetText() Method
    /**
     * setText(text) -- Set's the text for this textbox.
     * 
     * text           : string or unicode - text string.
     * 
     * example:
     *   - self.textbox.setText('This is a line of text that can wrap.')
     */
    void ControlTextBox::setText(const String& text) throw(UnimplementedException)
    {
      // create message
      CGUIMessage msg(GUI_MSG_LABEL_SET, iParentId, iControlId);
      msg.SetLabel(text);

      // send message
      g_windowManager.SendThreadMessage(msg, iParentId);
    }

    // reset() Method
    /**
     * reset() -- Clear's this textbox.
     * 
     * example:
     *   - self.textbox.reset()\n
     */
    void ControlTextBox::reset() throw(UnimplementedException)
    {
      // create message
      CGUIMessage msg(GUI_MSG_LABEL_RESET, iParentId, iControlId);
      g_windowManager.SendThreadMessage(msg, iParentId);
    }

    // scroll() Method
    /**
     * scroll(position) -- Scrolls to the given position.
     * 
     * id           : integer - position to scroll to.
     * 
     * example:
     *   - self.textbox.scroll(10)
     */
    void ControlTextBox::scroll(long position) throw(UnimplementedException)
    {
      static_cast<CGUITextBox*>(pGUIControl)->Scroll((int)position);
    }

    CGUIControl* ControlTextBox::Create() throw (WindowException)
    {
      // create textbox
      CLabelInfo label;
      label.font = g_fontManager.GetFont(strFont);
      label.textColor = label.focusedColor = textColor;

      pGUIControl = new CGUITextBox(iParentId, iControlId,
           (float)dwPosX, (float)dwPosY, (float)dwWidth, (float)dwHeight,
           label);

      // reset textbox
      CGUIMessage msg(GUI_MSG_LABEL_RESET, iParentId, iControlId);
      pGUIControl->OnMessage(msg);

      return pGUIControl;
    }

    // ============================================================

    // ============================================================
    // ============================================================
    ControlButton::ControlButton(long x, long y, long width, long height, const String& label,
                                 const char* focusTexture, const char* noFocusTexture, 
                                 long _textOffsetX, long _textOffsetY, 
                                 long alignment, const char* font, const char* _textColor,
                                 const char* _disabledColor, long angle,
                                 const char* _shadowColor, const char* _focusedColor) :
      Control("ControlButton"), textOffsetX(_textOffsetX), textOffsetY(_textOffsetY),
      align(alignment), strFont("font13"), textColor(0xffffffff), disabledColor(0x60ffffff),
      iAngle(angle), shadowColor(0), focusedColor(0xffffffff)
    {
      dwPosX = x;
      dwPosY = y;
      dwWidth = width;
      dwHeight = height;

      strText = label;

      // if texture is supplied use it, else get default ones
      strTextureFocus = focusTexture ? focusTexture :
        XBMCAddonUtils::getDefaultImage((char*)"button", (char*)"texturefocus", (char*)"button-focus.png");
      strTextureNoFocus = noFocusTexture ? noFocusTexture :
        XBMCAddonUtils::getDefaultImage((char*)"button", (char*)"texturenofocus", (char*)"button-nofocus.jpg");
      
      if (font) strFont = font;
      if (_textColor) sscanf( _textColor, "%x", &textColor );
      if (_disabledColor) sscanf( _disabledColor, "%x", &disabledColor );
      if (_shadowColor) sscanf( _shadowColor, "%x", &shadowColor );
      if (_focusedColor) sscanf( _focusedColor, "%x", &focusedColor );
    }

    // setLabel() Method
    /**
     * setLabel([label, font, textColor, disabledColor, shadowColor, focusedColor]) -- Set's this buttons text attributes.
     * 
     * label          : [opt] string or unicode - text string.
     * font           : [opt] string - font used for label text. (e.g. 'font13')
     * textColor      : [opt] hexstring - color of enabled button's label. (e.g. '0xFFFFFFFF')
     * disabledColor  : [opt] hexstring - color of disabled button's label. (e.g. '0xFFFF3300')
     * shadowColor    : [opt] hexstring - color of button's label's shadow. (e.g. '0xFF000000')
     * focusedColor   : [opt] hexstring - color of focused button's label. (e.g. '0xFFFFFF00')
     * label2         : [opt] string or unicode - text string.
     * 
     * *Note, You can use the above as keywords for arguments and skip certain optional arguments.
     *        Once you use a keyword, all following arguments require the keyword.
     * 
     * example:
     *   - self.button.setLabel('Status', 'font14', '0xFFFFFFFF', '0xFFFF3300', '0xFF000000')
     */
    void ControlButton::setLabel(const String& label, 
                                 const char* font,
                                 const char* _textColor,
                                 const char* _disabledColor,
                                 const char* _shadowColor,
                                 const char* _focusedColor,
                                 const String& label2) throw (UnimplementedException)
    {
      if (isSet(label)) strText = label;
      if (isSet(label2)) strText2 = label2;
      if (font) strFont = font;
      if (_textColor) sscanf(_textColor, "%x", &textColor);
      if (_disabledColor) sscanf( _disabledColor, "%x", &disabledColor );
      if (_shadowColor) sscanf(_shadowColor, "%x", &shadowColor);
      if (_focusedColor) sscanf(_focusedColor, "%x", &focusedColor);

      if (pGUIControl)
      {
        LOCKGUI;
        ((CGUIButtonControl*)pGUIControl)->PythonSetLabel(strFont, strText, textColor, shadowColor, focusedColor);
        ((CGUIButtonControl*)pGUIControl)->SetLabel2(strText2);
        ((CGUIButtonControl*)pGUIControl)->PythonSetDisabledColor(disabledColor);
      }
    }

    // setDisabledColor() Method
    /**
     * setDisabledColor(disabledColor) -- Set's this buttons disabled color.
     * 
     * disabledColor  : hexstring - color of disabled button's label. (e.g. '0xFFFF3300')
     * 
     * example:
     *   - self.button.setDisabledColor('0xFFFF3300')
     */
    void ControlButton::setDisabledColor(const char* color) throw (UnimplementedException)
    { 
      if (color) sscanf(color, "%x", &disabledColor);

      if (pGUIControl)
      {
        LOCKGUI;
        ((CGUIButtonControl*)pGUIControl)->PythonSetDisabledColor(disabledColor);
      }
   }

    // getLabel() Method
    /**
     * getLabel() -- Returns the buttons label as a unicode string.
     * 
     * example:
     *   - label = self.button.getLabel()
     */
    String ControlButton::getLabel() throw (UnimplementedException)
    {
      if (!pGUIControl) return NULL;

      LOCKGUI;
      return ((CGUIButtonControl*) pGUIControl)->GetLabel();
    }

    // getLabel2() Method
    /**
     * getLabel2() -- Returns the buttons label2 as a unicode string.
     * 
     * example:
     *   - label = self.button.getLabel2()
     */
    String ControlButton::getLabel2() throw (UnimplementedException)
    {
      if (!pGUIControl) return NULL;

      LOCKGUI;
      return ((CGUIButtonControl*) pGUIControl)->GetLabel2();
    }

    CGUIControl* ControlButton::Create() throw (WindowException)
    {
      CLabelInfo label;
      label.font = g_fontManager.GetFont(strFont);
      label.textColor = textColor;
      label.disabledColor = disabledColor;
      label.shadowColor = shadowColor;
      label.focusedColor = focusedColor;
      label.align = align;
      label.offsetX = (float)textOffsetX;
      label.offsetY = (float)textOffsetY;
      label.angle = (float)-iAngle;
      pGUIControl = new CGUIButtonControl(
        iParentId,
        iControlId,
        (float)dwPosX,
        (float)dwPosY,
        (float)dwWidth,
        (float)dwHeight,
        (CStdString)strTextureFocus,
        (CStdString)strTextureNoFocus,
        label);

      CGUIButtonControl* pGuiButtonControl =
        (CGUIButtonControl*)pGUIControl;

      pGuiButtonControl->SetLabel(strText);
      pGuiButtonControl->SetLabel2(strText2);

      return pGUIControl;
    }

    // ============================================================

    // ============================================================
    // ============================================================
    ControlCheckMark::ControlCheckMark(long x, long y, long width, long height, const String& label,
                                       const char* focusTexture, const char* noFocusTexture, 
                                       long _checkWidth, long _checkHeight,
                                       long _alignment, const char* font, 
                                       const char* _textColor, const char* _disabledColor) :
      Control("ControlCheckMark"), strFont("font13"), checkWidth(_checkWidth), checkHeight(_checkHeight),
      align(_alignment), textColor(0xffffffff), disabledColor(0x60ffffff)
    {
      dwPosX = x;
      dwPosY = y;
      dwWidth = width;
      dwHeight = height;

      strText = label;
      if (font) strFont = font;
      if (_textColor) sscanf(_textColor, "%x", &textColor);
      if (_disabledColor) sscanf( _disabledColor, "%x", &disabledColor );

      strTextureFocus = focusTexture ?  focusTexture :
        XBMCAddonUtils::getDefaultImage((char*)"checkmark", (char*)"texturefocus", (char*)"check-box.png");
      strTextureNoFocus = noFocusTexture ? noFocusTexture :
        XBMCAddonUtils::getDefaultImage((char*)"checkmark", (char*)"texturenofocus", (char*)"check-boxNF.png");
    }

    // getSelected() Method
    /**
     * getSelected() -- Returns the selected status for this checkmark as a bool.
     * 
     * example:
     *   - selected = self.checkmark.getSelected()
     */
    bool ControlCheckMark::getSelected() throw (UnimplementedException)
    {
      bool isSelected = false;

      if (pGUIControl)
      {
        LOCKGUI;
        isSelected = ((CGUICheckMarkControl*)pGUIControl)->GetSelected();
      }

      return isSelected;
    }

    // setSelected() Method
    /**
     * setSelected(isOn) -- Sets this checkmark status to on or off.
     * 
     * isOn           : bool - True=selected (on) / False=not selected (off)
     * 
     * example:
     *   - self.checkmark.setSelected(True)
     */
    void ControlCheckMark::setSelected(bool selected) throw (UnimplementedException)
    {
      if (pGUIControl)
      {
        LOCKGUI;
        ((CGUICheckMarkControl*)pGUIControl)->SetSelected(selected);
      }
    }

    // setLabel() Method
    /**
     * setLabel(label[, font, textColor, disabledColor]) -- Set's this controls text attributes.
     * 
     * label          : string or unicode - text string.
     * font           : [opt] string - font used for label text. (e.g. 'font13')
     * textColor      : [opt] hexstring - color of enabled checkmark's label. (e.g. '0xFFFFFFFF')
     * disabledColor  : [opt] hexstring - color of disabled checkmark's label. (e.g. '0xFFFF3300')
     * 
     * example:
     *   - self.checkmark.setLabel('Status', 'font14', '0xFFFFFFFF', '0xFFFF3300')
     */
    void ControlCheckMark::setLabel(const String& label, 
                                    const char* font,
                                    const char* _textColor,
                                    const char* _disabledColor,
                                    const char* _shadowColor,
                                    const char* _focusedColor,
                                    const String& label2) throw (UnimplementedException)
    {

      if (font) strFont = font;
      if (_textColor) sscanf(_textColor, "%x", &textColor);
      if (_disabledColor) sscanf(_disabledColor, "%x", &disabledColor);

      if (pGUIControl)
      {
        LOCKGUI;
        ((CGUICheckMarkControl*)pGUIControl)->PythonSetLabel(strFont,strText,textColor);
        ((CGUICheckMarkControl*)pGUIControl)->PythonSetDisabledColor(disabledColor);
      }
    }

    // setDisabledColor() Method
    /**
     * setDisabledColor(disabledColor) -- Set's this controls disabled color.
     * 
     * disabledColor  : hexstring - color of disabled checkmark's label. (e.g. '0xFFFF3300')
     * 
     * example:
     *   - self.checkmark.setDisabledColor('0xFFFF3300')
     */
    void ControlCheckMark::setDisabledColor(const char* color) throw (UnimplementedException)
    {
      if (color) sscanf(color, "%x", &disabledColor);

      if (pGUIControl)
      {
        LOCKGUI;
        ((CGUICheckMarkControl*)pGUIControl)->PythonSetDisabledColor( disabledColor );
      }
    }

    CGUIControl* ControlCheckMark::Create() throw (WindowException)
    {
      CLabelInfo label;
      label.disabledColor = disabledColor;
      label.textColor = label.focusedColor = textColor;
      label.font = g_fontManager.GetFont(strFont);
      label.align = align;
      CTextureInfo imageFocus(strTextureFocus);
      CTextureInfo imageNoFocus(strTextureNoFocus);
      pGUIControl = new CGUICheckMarkControl(
        iParentId,
        iControlId,
        (float)dwPosX,
        (float)dwPosY,
        (float)dwWidth,
        (float)dwHeight,
        imageFocus, imageNoFocus,
        (float)checkWidth,
        (float)checkHeight,
        label );

      CGUICheckMarkControl* pGuiCheckMarkControl = (CGUICheckMarkControl*)pGUIControl;
      pGuiCheckMarkControl->SetLabel(strText);

      return pGUIControl;
    }

    // ============================================================

    // ============================================================
    // ============================================================
    ControlImage::ControlImage(long x, long y, long width, long height, 
                               const char* filename, long aspectRatio,
                               const char* _colorDiffuse):
      Control("ControlImage"), colorDiffuse(0)
    {
      dwPosX = x;
      dwPosY = y;
      dwWidth = width;
      dwHeight = height;

      // check if filename exists
      strFileName = filename;
      if (_colorDiffuse) 
        sscanf(_colorDiffuse, "%x", &colorDiffuse);
    }

    /**
     * setImage(filename) -- Changes the image.
     * 
     * filename       : string - image filename.
     * 
     * example:
     *   - self.image.setImage('special://home/scripts/test.png')
     */
    void ControlImage::setImage(const char* imageFilename) throw (UnimplementedException)
    {
      strFileName = imageFilename;

      LOCKGUI;
      if (pGUIControl)
        ((CGUIImage*)pGUIControl)->SetFileName(strFileName);
    }

    /**
     * setColorDiffuse(colorDiffuse) -- Changes the images color.
     * 
     * colorDiffuse   : hexString - (example, '0xC0FF0000' (red tint))
     * 
     * example:
     *   - self.image.setColorDiffuse('0xC0FF0000')
     */
    void ControlImage::setColorDiffuse(const char* cColorDiffuse) throw (UnimplementedException)
    {
      if (cColorDiffuse) sscanf(cColorDiffuse, "%x", &colorDiffuse);
      else colorDiffuse = 0;

      LOCKGUI;
      if (pGUIControl)
        ((CGUIImage *)pGUIControl)->SetColorDiffuse(colorDiffuse);
    }
    
    CGUIControl* ControlImage::Create() throw (WindowException)
    {
      pGUIControl = new CGUIImage(iParentId, iControlId,
            (float)dwPosX, (float)dwPosY, (float)dwWidth, (float)dwHeight,
            (CStdString)strFileName);

      if (pGUIControl && aspectRatio <= CAspectRatio::AR_KEEP)
        ((CGUIImage *)pGUIControl)->SetAspectRatio((CAspectRatio::ASPECT_RATIO)aspectRatio);

      if (pGUIControl && colorDiffuse)
        ((CGUIImage *)pGUIControl)->SetColorDiffuse(colorDiffuse);

      return pGUIControl;
    }

    // ============================================================

    // ============================================================
    // ============================================================
    ControlProgress::ControlProgress(long x, long y, long width, long height, 
                                     const char* texturebg,
                                     const char* textureleft,
                                     const char* texturemid,
                                     const char* textureright,
                                     const char* textureoverlay):
      Control("ControlProgress")
    {
      dwPosX = x;
      dwPosY = y;
      dwWidth = width;
      dwHeight = height;

      // if texture is supplied use it, else get default ones
      strTextureBg = texturebg ? texturebg : 
        XBMCAddonUtils::getDefaultImage((char*)"progress", (char*)"texturebg", (char*)"progress_back.png");
      strTextureLeft = textureleft ? textureleft : 
        XBMCAddonUtils::getDefaultImage((char*)"progress", (char*)"lefttexture", (char*)"progress_left.png");
      strTextureMid = texturemid ? texturemid : 
        XBMCAddonUtils::getDefaultImage((char*)"progress", (char*)"midtexture", (char*)"progress_mid.png");
      strTextureRight = textureright ? textureright : 
        XBMCAddonUtils::getDefaultImage((char*)"progress", (char*)"righttexture", (char*)"progress_right.png");
      strTextureOverlay = textureoverlay ? textureoverlay : 
        XBMCAddonUtils::getDefaultImage((char*)"progress", (char*)"overlaytexture", (char*)"progress_over.png");
    }

    /**
     * setPercent(percent) -- Sets the percentage of the progressbar to show.
     * 
     * percent       : float - percentage of the bar to show.
     * 
     * *Note, valid range for percent is 0-100
     * 
     * example:
     *   - self.progress.setPercent(60)
     */
    void ControlProgress::setPercent(float pct) throw (UnimplementedException)
    {
      if (pGUIControl)
        ((CGUIProgressControl*)pGUIControl)->SetPercentage(pct);
    }

    /**
     * getPercent() -- Returns a float of the percent of the progress.
     * 
     * example:
     *   - print self.progress.getValue()
     */
    float ControlProgress::getPercent() throw (UnimplementedException)
    {
      return (pGUIControl) ? ((CGUIProgressControl*)pGUIControl)->GetPercentage() : 0.0f;
    }

    CGUIControl* ControlProgress::Create() throw (WindowException)
    {
      pGUIControl = new CGUIProgressControl(iParentId, iControlId,
         (float)dwPosX, (float)dwPosY,
         (float)dwWidth,(float)dwHeight,
         (CStdString)strTextureBg,(CStdString)strTextureLeft,
         (CStdString)strTextureMid,(CStdString)strTextureRight,
         (CStdString)strTextureOverlay);

      if (pGUIControl && colorDiffuse)
        ((CGUIProgressControl *)pGUIControl)->SetColorDiffuse(colorDiffuse);

      return pGUIControl;
    }

    // ============================================================

    // ============================================================
    // ============================================================
    ControlSlider::ControlSlider(long x, long y, long width, long height, 
                                 const char* textureback, 
                                 const char* texture,
                                 const char* texturefocus) :
      Control("ControlSlider")
    {
      dwPosX = x;
      dwPosY = y;
      dwWidth = width;
      dwHeight = height;

      // if texture is supplied use it, else get default ones
      strTextureBack = textureback ? textureback : 
        XBMCAddonUtils::getDefaultImage((char*)"slider", (char*)"texturesliderbar", (char*)"osd_slider_bg_2.png");
      strTexture = texture ? texture : 
        XBMCAddonUtils::getDefaultImage((char*)"slider", (char*)"textureslidernib", (char*)"osd_slider_nibNF.png");
      strTextureFoc = texturefocus ? texturefocus : 
        XBMCAddonUtils::getDefaultImage((char*)"slider", (char*)"textureslidernibfocus", (char*)"osd_slider_nib.png");
    }

    /**
     * getPercent() -- Returns a float of the percent of the slider.
     * 
     * example:
     *   - print self.slider.getPercent()
     */
    float ControlSlider::getPercent() throw (UnimplementedException)
    {
      return (pGUIControl) ? (float)((CGUISliderControl*)pGUIControl)->GetPercentage() : 0.0f;
    }

    /**
     * setPercent(50) -- Sets the percent of the slider.
     * 
     * example:
     * self.slider.setPercent(50)
     */
    void ControlSlider::setPercent(float pct) throw (UnimplementedException)
    {
      if (pGUIControl)
        ((CGUISliderControl*)pGUIControl)->SetPercentage((int)pct);
    }

    CGUIControl* ControlSlider::Create () throw (WindowException)
    {
      pGUIControl = new CGUISliderControl(iParentId, iControlId,(float)dwPosX, (float)dwPosY,
                                          (float)dwWidth,(float)dwHeight,
                                          (CStdString)strTextureBack,(CStdString)strTexture,
                                          (CStdString)strTextureFoc,0);   
    
      return pGUIControl;
    }  

    // ============================================================

    // ============================================================
    // ============================================================
    ControlGroup::ControlGroup(long x, long y, long width, long height):
      Control("ControlCheckMark")
    {
      dwPosX = x;
      dwPosY = y;
      dwWidth = width;
      dwHeight = height;
    }

    CGUIControl* ControlGroup::Create() throw (WindowException)
    {
      pGUIControl = new CGUIControlGroup(iParentId,
                                         iControlId,
                                         (float) dwPosX,
                                         (float) dwPosY,
                                         (float) dwWidth,
                                         (float) dwHeight);
      return pGUIControl;
    }

    // ============================================================

    // ============================================================
    // ============================================================
    ControlRadioButton::ControlRadioButton(long x, long y, long width, long height, const String& label,
                                           const char* focusTexture, const char* noFocusTexture, 
                                           long _textOffsetX, long _textOffsetY, 
                                           long alignment, const char* font, const char* _textColor,
                                           const char* _disabledColor, long angle,
                                           const char* _shadowColor, const char* _focusedColor,
                                           const char* TextureRadioFocus, const char* TextureRadioNoFocus) :
      Control("ControlRadioButton"), strFont("font13"), textColor(0xffffffff), disabledColor(0x60ffffff), 
      textOffsetX(_textOffsetX), textOffsetY(_textOffsetY), align(alignment), iAngle(angle), 
      shadowColor(0), focusedColor(0xffffffff)
    {
      dwPosX = x;
      dwPosY = y;
      dwWidth = width;
      dwHeight = height;

      strText = label;

      // if texture is supplied use it, else get default ones
      strTextureFocus = focusTexture ? focusTexture :
        XBMCAddonUtils::getDefaultImage((char*)"button", (char*)"texturefocus", (char*)"button-focus.png");
      strTextureNoFocus = noFocusTexture ? noFocusTexture :
        XBMCAddonUtils::getDefaultImage((char*)"button", (char*)"texturenofocus", (char*)"button-nofocus.jpg");
      strTextureRadioFocus = TextureRadioFocus ? TextureRadioFocus :
        XBMCAddonUtils::getDefaultImage((char*)"radiobutton", (char*)"textureradiofocus", (char*)"radiobutton-focus.png");
      strTextureRadioNoFocus = TextureRadioNoFocus ? TextureRadioNoFocus :
        XBMCAddonUtils::getDefaultImage((char*)"radiobutton", (char*)"textureradionofocus", (char*)"radiobutton-nofocus.jpg");
      
      if (font) strFont = font;
      if (_textColor) sscanf( _textColor, "%x", &textColor );
      if (_disabledColor) sscanf( _disabledColor, "%x", &disabledColor );
      if (_shadowColor) sscanf( _shadowColor, "%x", &shadowColor );
      if (_focusedColor) sscanf( _focusedColor, "%x", &focusedColor );
    }

    // setSelected() Method
    /**
     * setSelected(selected) -- Sets the radio buttons's selected status.
     * 
     * selected            : bool - True=selected (on) / False=not selected (off)
     * 
     * *Note, You can use the above as keywords for arguments and skip certain optional arguments.
     *        Once you use a keyword, all following arguments require the keyword.
     * 
     * example:
     *   - self.radiobutton.setSelected(True)
     */
    void ControlRadioButton::setSelected(bool selected) throw (UnimplementedException)
    {
      if (pGUIControl)
      {
        LOCKGUI;
        ((CGUIRadioButtonControl*)pGUIControl)->SetSelected(selected);
      }
    }

    // isSelected() Method
    /**
     * isSelected() -- Returns the radio buttons's selected status.
     * 
     * example:
     *   - is = self.radiobutton.isSelected()\n
     */
    bool ControlRadioButton::isSelected() throw (UnimplementedException)
    {
      bool isSelected = false;

      if (pGUIControl)
      {
        LOCKGUI;
        isSelected = ((CGUIRadioButtonControl*)pGUIControl)->IsSelected();
      }
      return isSelected;
    }

    // setLabel() Method
    /**
     * setLabel(label[, font, textColor, disabledColor, shadowColor, focusedColor]) -- Set's the radio buttons text attributes.
     * 
     * label          : string or unicode - text string.
     * font           : [opt] string - font used for label text. (e.g. 'font13')
     * textColor      : [opt] hexstring - color of enabled radio button's label. (e.g. '0xFFFFFFFF')
     * disabledColor  : [opt] hexstring - color of disabled radio button's label. (e.g. '0xFFFF3300')
     * shadowColor    : [opt] hexstring - color of radio button's label's shadow. (e.g. '0xFF000000')
     * focusedColor   : [opt] hexstring - color of focused radio button's label. (e.g. '0xFFFFFF00')
     * 
     * *Note, You can use the above as keywords for arguments and skip certain optional arguments.
     *        Once you use a keyword, all following arguments require the keyword.
     * 
     * example:
     *   - self.radiobutton.setLabel('Status', 'font14', '0xFFFFFFFF', '0xFFFF3300', '0xFF000000')
     */
    void ControlRadioButton::setLabel(const String& label, 
                                      const char* font,
                                      const char* _textColor,
                                      const char* _disabledColor,
                                      const char* _shadowColor,
                                      const char* _focusedColor,
                                      const String& label2) throw (UnimplementedException)
    {
      if (isSet(label)) strText = label;
      if (font) strFont = font;
      if (_textColor) sscanf(_textColor, "%x", &textColor);
      if (_disabledColor) sscanf( _disabledColor, "%x", &disabledColor );
      if (_shadowColor) sscanf(_shadowColor, "%x", &shadowColor);
      if (_focusedColor) sscanf(_focusedColor, "%x", &focusedColor);

      if (pGUIControl)
      {
        LOCKGUI;
        ((CGUIRadioButtonControl*)pGUIControl)->PythonSetLabel(strFont, strText, textColor, shadowColor, focusedColor);
        ((CGUIRadioButtonControl*)pGUIControl)->PythonSetDisabledColor(disabledColor);
      }
    }

    // setRadioDimension() Method
    /**
     * setRadioDimension(x, y, width, height) -- Sets the radio buttons's radio texture's position and size.
     * 
     * x                   : integer - x coordinate of radio texture.
     * y                   : integer - y coordinate of radio texture.
     * width               : integer - width of radio texture.
     * height              : integer - height of radio texture.
     * 
     * *Note, You can use the above as keywords for arguments and skip certain optional arguments.
     *        Once you use a keyword, all following arguments require the keyword.
     * 
     * example:
     *   - self.radiobutton.setRadioDimension(x=100, y=5, width=20, height=20)
     */
    void ControlRadioButton::setRadioDimension(long x, long y, long width, long height) 
      throw (UnimplementedException)
    {
      dwPosX = x;
      dwPosY = y;
      dwWidth = width;
      dwHeight = height;
      if (pGUIControl)
      {
        LOCKGUI;
        ((CGUIRadioButtonControl*)pGUIControl)->SetRadioDimensions((float)dwPosX, (float)dwPosY, (float)dwWidth, (float)dwHeight);
      }
    }

    CGUIControl* ControlRadioButton::Create() throw (WindowException)
    {
      CLabelInfo label;
      label.font = g_fontManager.GetFont(strFont);
      label.textColor = textColor;
      label.disabledColor = disabledColor;
      label.shadowColor = shadowColor;
      label.focusedColor = focusedColor;
      label.align = align;
      label.offsetX = (float)textOffsetX;
      label.offsetY = (float)textOffsetY;
      label.angle = (float)-iAngle;
      pGUIControl = new CGUIRadioButtonControl(
        iParentId,
        iControlId,
        (float)dwPosX,
        (float)dwPosY,
        (float)dwWidth,
        (float)dwHeight,
        (CStdString)strTextureFocus,
        (CStdString)strTextureNoFocus,
        label,
        (CStdString)strTextureRadioFocus,
        (CStdString)strTextureRadioNoFocus);

      CGUIRadioButtonControl* pGuiButtonControl =
        (CGUIRadioButtonControl*)pGUIControl;

      pGuiButtonControl->SetLabel(strText);

      return pGUIControl;
    }

    // ============================================================

    // ============================================================
    // ============================================================
    Control::~Control() { deallocating(); }

    CGUIControl* Control::Create() throw (WindowException)
    {
      throw WindowException("Object is a Control, but can't be added to a window");
    }

    /**
     * getPosition() -- Returns the control's current position as a x,y integer tuple.
     * 
     * example:
     *   - pos = self.button.getPosition()
     */
    std::vector<int> Control::getPosition()
    {
      std::vector<int> ret(2);
      ret[0] = dwPosX;
      ret[1] = dwPosY;
      return ret;
    }

    // setEnabled() Method
    /**
     * setEnabled(enabled) -- Set's the control's enabled/disabled state.
     * 
     * enabled        : bool - True=enabled / False=disabled.
     * 
     * example:
     *   - self.button.setEnabled(False)\n
     */
    void Control::setEnabled(bool enabled)
    {
      DelayedCallGuard dcguard(languageHook);
      LOCKGUI;
      if (pGUIControl)
        pGUIControl->SetEnabled(enabled);
    }

    // setVisible() Method
    /**
     * setVisible(visible) -- Set's the control's visible/hidden state.
     * 
     * visible        : bool - True=visible / False=hidden.
     * 
     * example:
     *   - self.button.setVisible(False)
     */
    void Control::setVisible(bool visible)
    {
      DelayedCallGuard dcguard(languageHook);
      LOCKGUI;
      if (pGUIControl)
        pGUIControl->SetVisible(visible);
    }

    // setVisibleCondition() Method
    /**
     * setVisibleCondition(visible[,allowHiddenFocus]) -- Set's the control's visible condition.
     *     Allows XBMC to control the visible status of the control.
     * 
     * visible          : string - Visible condition.
     * allowHiddenFocus : bool - True=gains focus even if hidden.
     * 
     * List of Conditions - http://wiki.xbmc.org/index.php?title=List_of_Boolean_Conditions 
     * 
     * example:
     *   - self.button.setVisibleCondition('[Control.IsVisible(41) + !Control.IsVisible(12)]', True)\n
     */
    void Control::setVisibleCondition(const char* visible, bool allowHiddenFocus)
    {
      DelayedCallGuard dcguard(languageHook);
      LOCKGUI;

      if (pGUIControl)
        pGUIControl->SetVisibleCondition(visible, allowHiddenFocus ? "true" : "false");
    }

    // setEnableCondition() Method
    /**
     * setEnableCondition(enable) -- Set's the control's enabled condition.
     *     Allows XBMC to control the enabled status of the control.
     * 
     * enable           : string - Enable condition.
     * 
     * List of Conditions - http://wiki.xbmc.org/index.php?title=List_of_Boolean_Conditions 
     * 
     * example:
     *   - self.button.setEnableCondition('System.InternetState')
     */
    void Control::setEnableCondition(const char* enable)
    {
      DelayedCallGuard dcguard(languageHook);
      LOCKGUI;

      if (pGUIControl)
        pGUIControl->SetEnableCondition(enable);
    }


    void Control::setAnimations(const std::vector< std::vector<String> >& eventAttr) throw (WindowException)
    {
      CXBMCTinyXML xmlDoc;
      TiXmlElement xmlRootElement("control");
      TiXmlNode *pRoot = xmlDoc.InsertEndChild(xmlRootElement);
      if (!pRoot)
        throw WindowException("TiXmlNode creation error");

      std::vector<CAnimation> animations;

      for (unsigned int anim = 0; anim < eventAttr.size(); anim++)
      {
        const std::vector<String>& pTuple = eventAttr[anim];

        if (pTuple.size() != 2)
          throw WindowException("Error unpacking tuple found in list");

        const String& cAttr = pTuple[0];
        const String& cEvent = pTuple[1];

        TiXmlElement pNode("animation");
        CStdStringArray attrs;
        StringUtils::SplitString(cAttr.c_str(), " ", attrs);
        for (unsigned int i = 0; i < attrs.size(); i++)
        {
          CStdStringArray attrs2;
          StringUtils::SplitString(attrs[i], "=", attrs2);
          if (attrs2.size() == 2)
            pNode.SetAttribute(attrs2[0], attrs2[1]);
        }
        TiXmlText value(cEvent.c_str());
        pNode.InsertEndChild(value);
        pRoot->InsertEndChild(pNode);
      }

      const CRect animRect((float)dwPosX, (float)dwPosY, (float)dwPosX + dwWidth, (float)dwPosY + dwHeight);
      LOCKGUI;
      if (pGUIControl)
      {
        CGUIControlFactory::GetAnimations(pRoot, animRect, iParentId, animations);
        pGUIControl->SetAnimations(animations);
      }
    }

    // setPosition() Method
    /**
     * setPosition(x, y) -- Set's the controls position.
     * 
     * x              : integer - x coordinate of control.
     * y              : integer - y coordinate of control.
     * 
     * *Note, You may use negative integers. (e.g sliding a control into view)
     * 
     * example:
     *   - self.button.setPosition(100, 250)\n
     */
    void Control::setPosition(long x, long y)
    {
      DelayedCallGuard dcguard(languageHook);
      LOCKGUI;
      dwPosX = x;
      dwPosY = y;
      if (pGUIControl)
        pGUIControl->SetPosition((float)dwPosX, (float)dwPosY);
    }


    // setWidth() Method
    /**
     * setWidth(width) -- Set's the controls width.
     * 
     * width          : integer - width of control.
     * 
     * example:
     *   - self.image.setWidth(100)
     */
    void Control::setWidth(long width)
    {
      DelayedCallGuard dcguard(languageHook);
      LOCKGUI;
      dwWidth = width;
      if (pGUIControl) 
        pGUIControl->SetWidth((float)dwWidth);
    }


    // setHeight() Method
    /**
     * setHeight(height) -- Set's the controls height.
     * 
     * height         : integer - height of control.
     * 
     * example:
     *   - self.image.setHeight(100)
     */
    void Control::setHeight(long height)
    {
      DelayedCallGuard dcguard(languageHook);
      LOCKGUI;
      dwHeight = height;
      if (pGUIControl) 
        pGUIControl->SetHeight((float)dwHeight);
    }

    // setNavigation() Method
    /**
     * setNavigation(up, down, left, right) -- Set's the controls navigation.
     * 
     * up             : control object - control to navigate to on up.
     * down           : control object - control to navigate to on down.
     * left           : control object - control to navigate to on left.
     * right          : control object - control to navigate to on right.
     * 
     * *Note, Same as controlUp(), controlDown(), controlLeft(), controlRight().
     *        Set to self to disable navigation for that direction.
     * 
     * Throws: TypeError, if one of the supplied arguments is not a control type.
     *         ReferenceError, if one of the controls is not added to a window.
     * 
     * example:
     *   - self.button.setNavigation(self.button1, self.button2, self.button3, self.button4)
     */
    void Control::setNavigation(const Control* up, const Control* down,
                                const Control* left, const Control* right) 
      throw (WindowException)
    {
      if(iControlId == 0)
        throw WindowException("Control has to be added to a window first");

      iControlUp = up->iControlId;
      iControlDown = down->iControlId;
      iControlLeft = left->iControlId;
      iControlRight = right->iControlId;

      {
        LOCKGUI;
        if (pGUIControl)
          pGUIControl->SetNavigation(iControlUp,iControlDown,iControlLeft,iControlRight);
      }
    }

    // controlUp() Method
    /**
     * controlUp(control) -- Set's the controls up navigation.
     * 
     * control        : control object - control to navigate to on up.
     * 
     * *Note, You can also use setNavigation(). Set to self to disable navigation.
     * 
     * Throws: TypeError, if one of the supplied arguments is not a control type.
     *         ReferenceError, if one of the controls is not added to a window.
     * 
     * example:
     *   - self.button.controlUp(self.button1)
     */
    void Control::controlUp(const Control* control) throw (WindowException)
    {
      if(iControlId == 0)
        throw WindowException("Control has to be added to a window first");

      iControlUp = control->iControlId;
      {
        LOCKGUI;
        if (pGUIControl) 
          pGUIControl->SetNavigation(iControlUp,iControlDown,iControlLeft,iControlRight);
      }
    }

    // controlDown() Method
    /**
     * controlDown(control) -- Set's the controls down navigation.
     * 
     * control        : control object - control to navigate to on down.
     * 
     * *Note, You can also use setNavigation(). Set to self to disable navigation.
     * 
     * Throws: TypeError, if one of the supplied arguments is not a control type.
     *         ReferenceError, if one of the controls is not added to a window.
     * 
     * example:
     *   - self.button.controlDown(self.button1)
     */
    void Control::controlDown(const Control* control) throw (WindowException)
    {
      if(iControlId == 0)
        throw WindowException("Control has to be added to a window first");

      iControlDown = control->iControlId;
      {
        LOCKGUI;
        if (pGUIControl) 
          pGUIControl->SetNavigation(iControlUp,iControlDown,iControlLeft,iControlRight);
      }
    }

    // controlLeft() Method
    /**
     * controlLeft(control) -- Set's the controls left navigation.
     * 
     * control        : control object - control to navigate to on left.
     * 
     * *Note, You can also use setNavigation(). Set to self to disable navigation.
     * 
     * Throws: TypeError, if one of the supplied arguments is not a control type.
     *         ReferenceError, if one of the controls is not added to a window.
     * 
     * example:
     *   - self.button.controlLeft(self.button1)
     */
    void Control::controlLeft(const Control* control) throw (WindowException)
    {
      if(iControlId == 0)
        throw WindowException("Control has to be added to a window first");

      iControlLeft = control->iControlId;
      {
        LOCKGUI;
        if (pGUIControl) 
          pGUIControl->SetNavigation(iControlUp,iControlDown,iControlLeft,iControlRight);
      }
    }

    // controlRight() Method
    /**
     * controlRight(control) -- Set's the controls right navigation.
     * 
     * control        : control object - control to navigate to on right.
     * 
     * *Note, You can also use setNavigation(). Set to self to disable navigation.
     * 
     * Throws: TypeError, if one of the supplied arguments is not a control type.
     *         ReferenceError, if one of the controls is not added to a window.
     * 
     * example:
     *   - self.button.controlRight(self.button1)\n
     */
    void Control::controlRight(const Control* control) throw (WindowException)
    {
      if(iControlId == 0)
        throw WindowException("Control has to be added to a window first");

      iControlRight = control->iControlId;
      {
        LOCKGUI;
        if (pGUIControl) 
          pGUIControl->SetNavigation(iControlUp,iControlDown,iControlLeft,iControlRight);
      }
    }

    // ============================================================
    //  ControlSpin
    // ============================================================
    ControlSpin::ControlSpin() : Control("ControlSpin")
    {
      // default values for spin control
      color = 0xffffffff;
      dwPosX = 0;
      dwPosY = 0;
      dwWidth = 16;
      dwHeight = 16;

      // get default images
      strTextureUp = XBMCAddonUtils::getDefaultImage((char*)"listcontrol", (char*)"textureup", (char*)"scroll-up.png");
      strTextureDown = XBMCAddonUtils::getDefaultImage((char*)"listcontrol", (char*)"texturedown", (char*)"scroll-down.png");
      strTextureUpFocus = XBMCAddonUtils::getDefaultImage((char*)"listcontrol", (char*)"textureupfocus", (char*)"scroll-up-focus.png");
      strTextureDownFocus = XBMCAddonUtils::getDefaultImage((char*)"listcontrol", (char*)"texturedownfocus", (char*)"scroll-down-focus.png");
    }

    /*
     * set textures
     * (string textureUp, string textureDown, string textureUpFocus, string textureDownFocus)
     */
    /**
     * setTextures(up, down, upFocus, downFocus) -- Set's textures for this control.
     * 
     * texture are image files that are used for example in the skin
     */
    void ControlSpin::setTextures(const char* up, const char* down, 
                                  const char* upFocus, 
                                  const char* downFocus) throw(UnimplementedException)
    {
      strTextureUp = up;
      strTextureDown = down;
      strTextureUpFocus = upFocus;
      strTextureDownFocus = downFocus;
      /*
        PyXBMCGUILock();
        if (self->pGUIControl)
        {
        CGUISpinControl* pControl = (CGUISpinControl*)self->pGUIControl;
        pControl->se
        PyXBMCGUIUnlock();
      */
    }

    ControlSpin::~ControlSpin() {}
    // ============================================================

    // ============================================================
    //  ControlLabel
    // ============================================================
    ControlLabel::ControlLabel(long x, long y, long width, long height, 
                               const String& label,
                               const char* font, const char* _textColor, 
                               const char* _disabledColor,
                               long _alignment, 
                               bool hasPath, long angle) :
      Control("ControlLabel"), strFont("font13"), 
      textColor(0xffffffff), disabledColor(0x60ffffff),
      align(_alignment), bHasPath(hasPath), iAngle(angle)
    {
      dwPosX = x;
      dwPosY = y;
      dwWidth = width;
      dwHeight = height;

      strText = label;
      if (font)
        strFont = font;

      if (_textColor) 
        sscanf(_textColor, "%x", &textColor);

      if (disabledColor)
        sscanf( _disabledColor, "%x", &disabledColor );
    }

    ControlLabel::~ControlLabel() {}

    CGUIControl* ControlLabel::Create()  throw (WindowException)
    {
      CLabelInfo label;
      label.font = g_fontManager.GetFont(strFont);
      label.textColor = label.focusedColor = textColor;
      label.disabledColor = disabledColor;
      label.align = align;
      label.angle = (float)-iAngle;
      pGUIControl = new CGUILabelControl(
        iParentId,
        iControlId,
        (float)dwPosX,
        (float)dwPosY,
        (float)dwWidth,
        (float)dwHeight,
        label,
        false,
        bHasPath);
      ((CGUILabelControl *)pGUIControl)->SetLabel(strText);
      return pGUIControl;
    }    

    /**
     * setLabel(label) -- Set's text for this label.
     * 
     * label          : string or unicode - text string.
     * 
     * example:
     *   - self.label.setLabel('Status')
     */
    void ControlLabel::setLabel(const String& label, const char* font,
                                const char* textColor, const char* disabledColor,
                                const char* shadowColor, const char* focusedColor,
                                const String& label2) throw(UnimplementedException)
    {
      strText = label;
      CGUIMessage msg(GUI_MSG_LABEL_SET, iParentId, iControlId);
      msg.SetLabel(strText);
      g_windowManager.SendThreadMessage(msg, iParentId);
    }

    /**
     * getLabel() -- Returns the text value for this label.
     * 
     * example:
     *   - label = self.label.getLabel()\n
     */
    String ControlLabel::getLabel() throw(UnimplementedException)
    {
      if (!pGUIControl) 
        return NULL;
      return strText.c_str();
    }
    // ============================================================

    // ============================================================
    //  ControlEdit
    // ============================================================
    ControlEdit::ControlEdit(long x, long y, long width, long height, const String& label,
                             const char* font, const char* _textColor, 
                             const char* _disabledColor,
                             long _alignment, const char* focusTexture,
                             const char* noFocusTexture, bool isPassword) :
      Control("ControlEdit"), strFont("font13"), textColor(0xffffffff), disabledColor(0x60ffffff),
      align(_alignment), bIsPassword(isPassword)

    {
      strTextureFocus = focusTexture ? focusTexture :
        XBMCAddonUtils::getDefaultImage((char*)"edit", (char*)"texturefocus", (char*)"button-focus.png");

      strTextureNoFocus = noFocusTexture ? noFocusTexture :
        XBMCAddonUtils::getDefaultImage((char*)"edit", (char*)"texturenofocus", (char*)"button-focus.png");

      if (font) strFont = font;
      if (_textColor) sscanf( _textColor, "%x", &textColor );
      if (_disabledColor) sscanf( _disabledColor, "%x", &disabledColor );
    }

    CGUIControl* ControlEdit::Create()  throw (WindowException)
    {
      CLabelInfo label;
      label.font = g_fontManager.GetFont(strFont);
      label.textColor = label.focusedColor = textColor;
      label.disabledColor = disabledColor;
      label.align = align;
      pGUIControl = new CGUIEditControl(
        iParentId,
        iControlId,
        (float)dwPosX,
        (float)dwPosY,
        (float)dwWidth,
        (float)dwHeight,
        (CStdString)strTextureFocus,
        (CStdString)strTextureNoFocus,
        label,
        strText);

      if (bIsPassword)
        ((CGUIEditControl *) pGUIControl)->SetInputType(CGUIEditControl::INPUT_TYPE_PASSWORD, 0);
      return pGUIControl;
    }

    // setLabel() Method
    /**
     * setLabel(label) -- Set's text heading for this edit control.
     * 
     * label          : string or unicode - text string.
     * 
     * example:
     *   - self.edit.setLabel('Status')\n
     */
    void ControlEdit::setLabel(const String& label, const char* font,
                                const char* textColor, const char* disabledColor,
                                const char* shadowColor, const char* focusedColor,
                                const String& label2) throw(UnimplementedException)
    {
      strText = label;
      CGUIMessage msg(GUI_MSG_LABEL_SET, iParentId, iControlId);
      msg.SetLabel(strText);
      g_windowManager.SendThreadMessage(msg, iParentId);
    }

    /**
     * getLabel() -- Returns the text heading for this edit control.
     * 
     * example:
     *   - label = self.edit.getLabel()
     */
    String ControlEdit::getLabel() throw(UnimplementedException)
    {
      if (!pGUIControl) 
        return NULL;
      return strText.c_str();
    }

    // setText() Method
    /**
     * setText(value) -- Set's text value for this edit control.
     * 
     * value          : string or unicode - text string.
     * 
     * example:
     *   - self.edit.setText('online')\n
     */
    void ControlEdit::setText(const String& text) throw(UnimplementedException)
    {
      // create message
      CGUIMessage msg(GUI_MSG_LABEL2_SET, iParentId, iControlId);
      msg.SetLabel(text);

      // send message
      g_windowManager.SendThreadMessage(msg, iParentId);
    }

    // getText() Method
    /**
     * getText() -- Returns the text value for this edit control.
     * 
     * example:
     *   - value = self.edit.getText()
     */
    String ControlEdit::getText() throw(UnimplementedException)
    {
      CGUIMessage msg(GUI_MSG_ITEM_SELECTED, iParentId, iControlId);
      g_windowManager.SendMessage(msg, iParentId);

      return msg.GetLabel();
    }



    // ============================================================
    //  ControlList
    // ============================================================
    /**
     * ControlList class.
     * 
     * ControlList(x, y, width, height[, font, textColor, buttonTexture, buttonFocusTexture,
     *             selectedColor, imageWidth, imageHeight, itemTextXOffset, itemTextYOffset,
     *             itemHeight, space, alignmentY])\n"//, shadowColor])
     * 
     * x                  : integer - x coordinate of control.
     * y                  : integer - y coordinate of control.
     * width              : integer - width of control.
     * height             : integer - height of control.
     * font               : [opt] string - font used for items label. (e.g. 'font13')
     * textColor          : [opt] hexstring - color of items label. (e.g. '0xFFFFFFFF')
     * buttonTexture      : [opt] string - filename for focus texture.
     * buttonFocusTexture : [opt] string - filename for no focus texture.
     * selectedColor      : [opt] integer - x offset of label.
     * imageWidth         : [opt] integer - width of items icon or thumbnail.
     * imageHeight        : [opt] integer - height of items icon or thumbnail.
     * itemTextXOffset    : [opt] integer - x offset of items label.
     * itemTextYOffset    : [opt] integer - y offset of items label.
     * itemHeight         : [opt] integer - height of items.
     * space              : [opt] integer - space between items.
     * alignmentY         : [opt] integer - Y-axis alignment of items label - *Note, see xbfont.h
     * //"shadowColor        : [opt] hexstring - color of items label's shadow. (e.g. '0xFF000000')
     * 
     * *Note, You can use the above as keywords for arguments and skip certain optional arguments.
     *        Once you use a keyword, all following arguments require the keyword.
     *        After you create the control, you need to add it to the window with addControl().
     * 
     * example:
     *   - self.cList = xbmcgui.ControlList(100, 250, 200, 250, 'font14', space=5)
     */
    ControlList::ControlList(long x, long y, long width, long height, const char* font,
                             const char* ctextColor, const char* cbuttonTexture,
                             const char* cbuttonFocusTexture,
                             const char* cselectedColor,
                             long _imageWidth, long _imageHeight, long _itemTextXOffset,
                             long _itemTextYOffset, long _itemHeight, long _space, long _alignmentY) :
      Control("ControlList"),
      strFont("font13"), 
      textColor(0xe0f0f0f0), selectedColor(0xffffffff),
      imageHeight(_imageHeight), imageWidth(_imageWidth),
      itemHeight(_itemHeight), space(_space),
      itemTextOffsetX(_itemTextXOffset),itemTextOffsetY(_itemTextYOffset),
      alignmentY(_alignmentY)
    {
      dwPosX = x;
      dwPosY = y;
      dwWidth = width;
      dwHeight = height;

      // create a python spin control
      pControlSpin = new ControlSpin();

      // initialize default values
      if (font)
        strFont = font;

      if (ctextColor)
        sscanf( ctextColor, "%x", &textColor );

      if (cselectedColor)
        sscanf( cselectedColor, "%x", &selectedColor );

      strTextureButton = cbuttonTexture ? cbuttonTexture :
        XBMCAddonUtils::getDefaultImage((char*)"listcontrol", (char*)"texturenofocus", (char*)"list-nofocus.png");

      strTextureButtonFocus = cbuttonFocusTexture ? cbuttonFocusTexture :
        XBMCAddonUtils::getDefaultImage((char*)"listcontrol", (char*)"texturefocus", (char*)"list-focus.png");

      // default values for spin control
      pControlSpin->dwPosX = dwWidth - 35;
      pControlSpin->dwPosY = dwHeight - 15;
    }

    ControlList::~ControlList() { }

    CGUIControl* ControlList::Create() throw (WindowException)
    {
      CLabelInfo label;
      label.align = alignmentY;
      label.font = g_fontManager.GetFont(strFont);
      label.textColor = label.focusedColor = textColor;
      //label.shadowColor = shadowColor;
      label.selectedColor = selectedColor;
      label.offsetX = (float)itemTextOffsetX;
      label.offsetY = (float)itemTextOffsetY;
      // Second label should have the same font, alignment, and colours as the first, but
      // the offsets should be 0.
      CLabelInfo label2 = label;
      label2.offsetX = label2.offsetY = 0;
      label2.align |= XBFONT_RIGHT;

      pGUIControl = new CGUIListContainer(
        iParentId,
        iControlId,
        (float)dwPosX,
        (float)dwPosY,
        (float)dwWidth,
        (float)dwHeight - pControlSpin->dwHeight - 5,
        label, label2,
        (CStdString)strTextureButton,
        (CStdString)strTextureButtonFocus,
        (float)itemHeight,
        (float)imageWidth, (float)imageHeight,
        (float)space);

      return pGUIControl;
    }

    /**
     * addItem(item) -- Add a new item to this list control.
     * 
     * item               : string, unicode or ListItem - item to add.
     * 
     * example:
     *   - cList.addItem('Reboot XBMC')
     */
    void ControlList::addItemStream(const String& fileOrUrl) throw(UnimplementedException,WindowException)
    {
      addListItem(ListItem::fromString(fileOrUrl));
    }

    /**
     * addItem(item) -- Add a new item to this list control.
     * 
     * item               : string, unicode or ListItem - item to add.
     * 
     * example:
     *   - cList.addItem('Reboot XBMC')
     */
    void ControlList::addListItem(const XBMCAddon::xbmcgui::ListItem* pListItem) throw(UnimplementedException,WindowException)
    {
      if (pListItem == NULL)
        throw WindowException("NULL ListItem passed to ControlList::addListItem");

      // add item to objects vector
      vecItems.push_back(pListItem);

      // construct a CFileItemList to pass 'em on to the list
      CGUIListItemPtr items(new CFileItemList());
      for (unsigned int i = 0; i < vecItems.size(); i++)
        ((CFileItemList*)items.get())->Add(vecItems[i]->item);

      CGUIMessage msg(GUI_MSG_LABEL_BIND, iParentId, iControlId, 0, 0, items);
      msg.SetPointer(items.get());
      g_windowManager.SendThreadMessage(msg, iParentId);
    }

    /**
     * selectItem(item) -- Select an item by index number.
     * 
     * item               : integer - index number of the item to select.
     * 
     * example:
     *   - cList.selectItem(12)\n
     */
    void ControlList::selectItem(long item) throw(UnimplementedException)
    {
      // create message
      CGUIMessage msg(GUI_MSG_ITEM_SELECT, iParentId, iControlId, item);

      // send message
      g_windowManager.SendThreadMessage(msg, iParentId);
    }

    // reset() method
    /**
     * reset() -- Clear all ListItems in this control list.
     * 
     * example:
     *   - cList.reset()\n
     */
    void ControlList::reset() throw(UnimplementedException)
    {
      CGUIMessage msg(GUI_MSG_LABEL_RESET, iParentId, iControlId);

      g_windowManager.SendThreadMessage(msg, iParentId);

      // delete all items from vector
      // delete all ListItem from vector
      vecItems.clear(); // this should delete all of the objects
    }

    Control* ControlList::getSpinControl() throw (UnimplementedException)
    {
      return pControlSpin;
    }

    // getSelectedPosition() method
    /**
     * getSelectedPosition() -- Returns the position of the selected item as an integer.
     * 
     * *Note, Returns -1 for empty lists.
     * 
     * example:
     *   - pos = cList.getSelectedPosition()
     */
    long ControlList::getSelectedPosition() throw(UnimplementedException)
    {
      DelayedCallGuard dcguard(languageHook);
      LOCKGUI;
      
      // create message
      CGUIMessage msg(GUI_MSG_ITEM_SELECTED, iParentId, iControlId);
      long pos = -1;

      // send message
      if ((vecItems.size() > 0) && pGUIControl)
      {
        pGUIControl->OnMessage(msg);
        pos = msg.GetParam1();
      }

      return pos;
    }

    // getSelectedItem() method
    /**
     * getSelectedItem() -- Returns the selected item as a ListItem object.
     * 
     * *Note, Same as getSelectedPosition(), but instead of an integer a ListItem object
     *        is returned. Returns None for empty lists.
     *        See windowexample.py on how to use this.
     * 
     * example:
     *   - item = cList.getSelectedItem()
     */
    XBMCAddon::xbmcgui::ListItem* ControlList::getSelectedItem() throw (UnimplementedException)
    {
      DelayedCallGuard dcguard(languageHook);
      LOCKGUI;

      // create message
      CGUIMessage msg(GUI_MSG_ITEM_SELECTED, iParentId, iControlId);
      AddonClass::Ref<ListItem> pListItem = NULL;

      // send message
      if ((vecItems.size() > 0) && pGUIControl)
      {
        pGUIControl->OnMessage(msg);
        pListItem = vecItems[msg.GetParam1()];
      }

      return pListItem.get();
    }

    // setImageDimensions() method
    /**
     * setImageDimensions(imageWidth, imageHeight) -- Sets the width/height of items icon or thumbnail.
     * 
     * imageWidth         : [opt] integer - width of items icon or thumbnail.
     * imageHeight        : [opt] integer - height of items icon or thumbnail.
     * 
     * example:
     *   - cList.setImageDimensions(18, 18)\n
     */
    void ControlList::setImageDimensions(long imageWidth,long imageHeight) throw (UnimplementedException)
    {
      CLog::Log(LOGWARNING,"ControlList::setImageDimensions was called but ... it currently isn't defined to do anything.");
      /*
        PyXBMCGUILock();
        if (self->pGUIControl)
        {
        CGUIListControl* pListControl = (CGUIListControl*) self->pGUIControl;
        pListControl->SetImageDimensions((float)self->dwImageWidth, (float)self->dwImageHeight );
        }
        PyXBMCGUIUnlock();
      */
    }

    // setItemHeight() method
    /**
     * setItemHeight(itemHeight) -- Sets the height of items.
     * 
     * itemHeight         : integer - height of items.
     * 
     * example:
     *   - cList.setItemHeight(25)\n
     */
    void ControlList::setItemHeight(long height) throw (UnimplementedException)
    {
      CLog::Log(LOGWARNING,"ControlList::setItemHeight was called but ... it currently isn't defined to do anything.");
      /*
        PyXBMCGUILock();
        if (self->pGUIControl)
        {
        CGUIListControl* pListControl = (CGUIListControl*) self->pGUIControl;
        pListControl->SetItemHeight((float)self->dwItemHeight);
        }
        PyXBMCGUIUnlock();
      */
    }

    // setSpace() method
    /**
     * setSpace(space) -- Set's the space between items.
     * 
     * space              : [opt] integer - space between items.
     * 
     * example:
     *   - cList.setSpace(5)
     */
    void ControlList::setSpace(int space) throw (UnimplementedException)
    {
      CLog::Log(LOGWARNING,"ControlList::setSpace was called but ... it currently isn't defined to do anything.");
      /*
        PyXBMCGUILock();
        if (self->pGUIControl)
        {
        CGUIListControl* pListControl = (CGUIListControl*) self->pGUIControl;
        pListControl->SetSpaceBetweenItems((float)self->dwSpace);
        }
        PyXBMCGUIUnlock();
      */
    }

    // setPageControlVisible() method
    /**
     * setPageControlVisible(visible) -- Sets the spin control's visible/hidden state.
     * 
     * visible            : boolean - True=visible / False=hidden.
     * 
     * example:
     *   - cList.setPageControlVisible(True)
     */
    void ControlList::setPageControlVisible(bool visible) throw (UnimplementedException)
    {
      CLog::Log(LOGWARNING,"ControlList::setPageControlVisible was called but ... it currently isn't defined to do anything.");

      //      char isOn = true;

      /*
        PyXBMCGUILock();
        if (self->pGUIControl)
        {
        ((CGUIListControl*)self->pGUIControl)->SetPageControlVisible((bool)isOn );
        }
        PyXBMCGUIUnlock();
      */
    }

    // size() method
    /**
     * size() -- Returns the total number of items in this list control as an integer.
     * 
     * example:
     *   - cnt = cList.size()
     */
    long ControlList::size() throw (UnimplementedException)
    {
      return (long)vecItems.size();
    }

    // getItemHeight() Method
    /**
     * getItemHeight() -- Returns the control's current item height as an integer.
     * 
     * example:
     *   - item_height = self.cList.getItemHeight()\n
     */
    long ControlList::getItemHeight() throw(UnimplementedException)
    {
      return (long)itemHeight;
    }

    // getSpace() Method
    /**
     * getSpace() -- Returns the control's space between items as an integer.
     * 
     * example:
     *   - gap = self.cList.getSpace()\n
     */
    long ControlList::getSpace() throw (UnimplementedException)
    {
      return (long)space;
    }

    // getListItem() method
    /**
     * getListItem(index) -- Returns a given ListItem in this List.
     * 
     * index           : integer - index number of item to return.
     * 
     * *Note, throws a ValueError if index is out of range.
     * 
     * example:
     *   - listitem = cList.getListItem(6)\n
     */
    XBMCAddon::xbmcgui::ListItem* ControlList::getListItem(int index) throw (UnimplementedException,WindowException)
    {
      if (index < 0 || index >= (int)vecItems.size())
        throw WindowException("Index out of range");

      AddonClass::Ref<ListItem> pListItem = vecItems[index];
      return pListItem.get();
    }

    /**
     * setStaticContent(items) -- Fills a static list with a list of listitems.
     * 
     * items                : List - list of listitems to add.
     * 
     * *Note, You can use the above as keywords for arguments.
     * 
     * example:
     *   - cList.setStaticContent(items=listitems)\n
     */
    void ControlList::setStaticContent(const ListItemList* pitems) throw (UnimplementedException)
    {
      const ListItemList& vecItems = *pitems;

      std::vector<CGUIListItemPtr> items;

      for (unsigned int item = 0; item < vecItems.size(); item++)
      {
        ListItem* pItem = vecItems[item];

        // object is a listitem, and we set m_idpeth to 0 as this
        // is used as the visibility condition for the item in the list
        ListItem *listItem = (ListItem*)pItem;
        listItem->item->m_idepth = 0;

        items.push_back((CFileItemPtr &)listItem->item);
      }

      // set static list
      ((CGUIBaseContainer *)pGUIControl)->SetStaticContent(items);
    }

    // ============================================================

  }
}
